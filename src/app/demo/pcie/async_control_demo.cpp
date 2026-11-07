#include <stdio.h>
#include <stdlib.h>

#include "pci_errno.h"
#include "pci_zecm.h"
#include "pci_dbg.h"
#include "string.h"

#define RETURN_IF_FAILED(expr)      { int ret = (expr); if (ret != 0) { _ERR_(#expr " failed: err=%d", ret); return -1; } else { _INF_(#expr " succeeded"); } }

#include "servo_eni_cfg.h"

static uint32_t g_op_mode = INIT_OP_MODE;
static U32 g_run_task = 1;

static servos_t __servos[NUM_SERVOS] = {0};


/************************ motor config end ***************************/
static bool state_control(int axis, servo_stat_word *s0, const servo_stat_word *s1, servo_ctrl_word *c, bool show)
{
    servos_t *servo = &__servos[axis];
    do {
        c->v = 0;
        c->b.enable_voltage = 1;
        c->b.quick_stop = 1;
        c->b.enable_operation = s1->b.switched_on || s1->b.operation_enabled;
        c->b.switch_on = s1->b.ready_to_switch_on;

        if (s1->b.fault && !servo->fault_reset_delay) 
        {
            // 延时1秒再尝试错误恢复，防止伺服在错误和恢复中反复快速切换损坏电路
            _DBG_("axis %d: delay fault reset, ctrl:%d, stat:%x", axis, c->v, (int)s1->v);
            servo->fault_reset_delay = 2000; // 2s(即2000个周期)
            servo->run_delay = 2000; // 当错误恢复成功伺服出力后，延时2s再开始加速(根据抱闸动作时间适当调整)
            break;
        }
        // 伺服发生错误，且在延时等待处理中
        if (servo->fault_reset_delay) 
        {
            servo->fault_reset_delay--;
            if (!servo->fault_reset_delay) 
            {
                // 延时减到0，尝试进行错误恢复
                _DBG_("axis %d: start fault reset", axis);
                c->b.fault_reset = 1;
            }
            break;
        }

    } while (0);
    
    s0->v = s1->v;

    return s1->b.operation_enabled;
}

static  proc_img_o	__pi_o;
static  proc_img_i	__pi_i;
static  proc_img_i *pi = NULL;
static  proc_img_o *po = NULL;
static uint8_t *input_buff = NULL;
static uint8_t *output_buff = NULL;

static proc_img_o *pi_get_output(const pi_offset_config *config, void *output)
{
	for (uint32_t i = 0; i < NUM_SERVOS; i++)
	{
		servo_ctrl *ctrl = &__pi_o.servos[i];
		const pi_offset_config *curr = &config[i];

		ctrl->mode = (uint8_t *)((curr->po_mode_offset != -1) ? (char *)output + curr->po_mode_offset : 0);
		ctrl->ctrl = (servo_ctrl_word *)((curr->po_ctrl_word_offset != -1) ? (char *)output + curr->po_ctrl_word_offset : 0);
		ctrl->p = (I32 *)((curr->po_p_offset != -1) ? (char *)output + curr->po_p_offset : 0);
	}
    return &__pi_o;
}

static proc_img_i *pi_get_input(const pi_offset_config *config, const void *output)
{
	for (uint32_t i = 0; i < NUM_SERVOS; i++)
	{
		servo_stat *stat = &__pi_i.servos[i];
		const pi_offset_config *curr = &config[i];
		stat->mode = (uint8_t *)((curr->pi_mode_offset != -1) ? (char *)output + curr->pi_mode_offset : 0);
		stat->stat = (servo_stat_word *)((curr->pi_stat_offset != -1) ? (char *)output + curr->pi_stat_offset : 0);
		stat->p = (I32 *)((curr->pi_p_offset != -1) ? (char *)output + curr->pi_p_offset : 0);
	}
    return &__pi_i;
}



static int motor_mode_init(ECAT_HANDLE ecat_dev,uint32_t op_mode)
{
    int ret;
    uint32_t mode = 0;
    uint32_t len;
    int fail_try = 10;
    int i;

    for(i = 0; i < NUM_SERVOS && fail_try > 0; )
    {
        int idx = servo_to_slave_index[i];
        len = 1;
        ret=EcatCoeSDODownloadEx(ecat_dev,idx,0x6060,0x0,0,1,&op_mode,100);
        usleep(1000*100);
        
        ret=EcatCoeSDOUploadEx(ecat_dev,idx,0x6061,0x0,0,&len,&mode,100);
        if(mode != op_mode)
        {
            _INF_("request is: %d, curr is: %d", op_mode,mode);    
            usleep(1000*100);
            fail_try --;
            ret = -1;
            continue;
        }    
        
        fail_try = 10;
        i++;
    }

    return ret;
}

static int motor_state_init(ECAT_HANDLE ecat_dev,ECAT_ENI_INFO ptInfo)
{    
    int  i = 0;
    int fail_try = 1000;
    bool run = false;
    uint32_t max_speed = 5000;


    for (i = 0; i < NUM_SERVOS && fail_try > 0;)
    {
        servos_t *servo = &__servos[i];

        if(EcatPIRead(ecat_dev,PI_AREA_SHADOW_INPUT,0,ptInfo.u32PIInputByteSize,input_buff))
        {
            printf("EcatPIRead Fail\r\n");
        }

        run = state_control(i, &servo->ss, pi->servos[i].stat, &servo->sc, false);
        usleep(1000);

        // 更新输出变量
        *po->servos[i].ctrl = servo->sc;
        *po->servos[i].p = 0;

        if (EcatPIWrite(ecat_dev,PI_AREA_FAST_OUTPUT,0,ptInfo.u32PIOutputByteSize,output_buff) != 0)
        {
            printf("EcatPIWrite Fail\r\n");
        }
  
        if(!run)
        {
            fail_try--;
            continue;
        }

        //初始位置初始化
        servo->out_p = *pi->servos[i].p;
        servo->zero_p = (float)servo->out_p;
        servo->offset_p = 0;
        servo->out_v = 0;
        servo->out_t = 0;
        servo->reverse = false;
        
        //初始话最高速度
        EcatCoeSDODownloadEx(ecat_dev,i,0x6081,0x0,0,4,&max_speed,100);

        fail_try = 1000;
        i++;
    }
    
    return run? 0 : -1;
}

int async_control_demo(uint32_t index, uint32_t channel){
    int ret = 0;
    char index_buff[10];

    uint64_t loops = 0;
    ECAT_HANDLE ecat_dev;
    ECAT_ENI_INFO ptInfo;
    //PCIe-2E 只有一个通道,PCIe-4E有两个通道
    RETURN_IF_FAILED(EcatOpen(&ecat_dev, BOARD_ALIAS(index_buff, index), channel));
    
    RETURN_IF_FAILED(EcatBusRun(ecat_dev, ENI_FILE));

    // 请求切换状态
    uint8_t state;
    RETURN_IF_FAILED(EcatRequestMasterState(ecat_dev, EcatStateO));
    for (uint32_t i = 0; ; i++)
    {
        RETURN_IF_FAILED(EcatGetMasterState(ecat_dev, &state));
        _INF_("request is: 8, curr is: %d", state);
        usleep(500000);
        if (state == EcatStateO) break;
    }

    RETURN_IF_FAILED(motor_mode_init(ecat_dev, g_op_mode));

    EcatGetEniInfo(ecat_dev,&ptInfo);
    printf("InputByteSize:%d\r\n",ptInfo.u32PIInputByteSize);
    printf("OutputByteSize:%d\r\n",ptInfo.u32PIOutputByteSize);

    input_buff  = (uint8_t *)malloc(1024);
    output_buff = (uint8_t *)malloc(1024);

    memset(input_buff, 0, 1024);
    memset(output_buff, 0, 1024);

    if(input_buff == NULL || output_buff == NULL)
    {
        printf("malloc fail\r\n");
        return 0;
    }

    pi = pi_get_input(__pi_config,input_buff);
    po = pi_get_output(__pi_config,output_buff);

    RETURN_IF_FAILED(EcatPIEnable(ecat_dev));

    while(motor_state_init(ecat_dev,ptInfo))
    {
        printf("wait motor state init...\r\n");      

        usleep(1000*500);
    }
    
    printf("mootor init ok, test start!\r\n");

    while (1)
    {
        loops++;
        if(!EcatPIRead(ecat_dev,PI_AREA_SHADOW_INPUT,0,ptInfo.u32PIInputByteSize,input_buff))
        {
            for (int i = 0; i < NUM_SERVOS; i++)
            {
                servos_t *servo = &__servos[i];

                if (loops % 10 == 0) 
                {
                    _INF_("IN  servo[%d]: p=%10d, ctrl:%08x stat=%08x", i, *pi->servos[i].p, po->servos[i].ctrl->v, pi->servos[i].stat->v);
                    _INF_("OUT servo[%d]: p=%10d", i,  *po->servos[i].p);
                }

                // 伺服已出力，延时结束，进行正常控制
                // 不同轴单位可能不一样，下表定义各轴位置范围及增量
                I32 range_min[] = { -500, -500, -1500 };
                I32 range_max[] = {  500, 500, 1500 };
                float step_size[] = { 1, 1, 10 };
                if (servo->offset_p < range_min[i]) servo->reverse = false;
                if (servo->offset_p > range_max[i]) servo->reverse = true;
                servo->offset_p += (servo->reverse ? -1 : 1) * step_size[i];
                servo->out_p = (I32)(servo->zero_p + servo->offset_p);

                // FIXME: cant reach min or max 
                servo->out_p = MIN_LIMIT(servo->out_p ,range_min[i]);
                servo->out_p = MAX_LIMIT(servo->out_p ,range_max[i]);

                // 更新输出变量
                po->servos[i].ctrl->v = 0x2F;
                *po->servos[i].p = servo->out_p;
            }
            
            if (EcatPIWrite(ecat_dev,PI_AREA_FAST_OUTPUT,0,ptInfo.u32PIOutputByteSize,output_buff) != 0)
            {
                _ERR_("write PI_AREA_FAST_OUTPUT error");
                break;
            }
            usleep(1000*2);

            //pp mode need 0x6040: 0x2f -> 0x3f
            for (int i = 0; i < NUM_SERVOS; i++)
            {
                po->servos[i].ctrl->v = 0x3F;
                servos_t *servo = &__servos[i];
            }
            if (EcatPIWrite(ecat_dev,PI_AREA_FAST_OUTPUT,0,ptInfo.u32PIOutputByteSize,output_buff) != 0)
            {
                _ERR_("write PI_AREA_FAST_OUTPUT error");
                break;
            }
        }

        usleep(1000);
    }

    free(input_buff);
    free(output_buff);
	return 0;
}

int sync_control_demo(uint32_t index, uint32_t channel){
    return 0;
}
// argv = [index, channel, ...]
int pci_main(int argc, char** argv)
{
    uint32_t index = 0;

    uint32_t channel = 0;

    if (argc != 2)
    {
        printf("usage: ./program encoder_id chnnal\r\n");
        return 0;
    }

    index = atoi(argv[0]);
    channel = atoi(argv[1]);
    if(channel > 1) channel = 1;

    if (1){
        return async_control_demo(index, channel);
    }else
    {
        return sync_control_demo(index, channel);
    }
}
