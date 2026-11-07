#include "EcatBase.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/misc/compat_windows.h"
#include "common/zmc/zmc_predef.h"
#include "zecm/zecm_zh.h"
#include "zecm/zecm_ex.h"

#define ECAT_ETH0  ZMC_DEFAULT_NET_NAME
#define ECAT_ETH1  "zecm1"
#define RPC_SERVER_ETH0_PORT  5000            // 开放与上位机通讯的端口号
#define RPC_SERVER_ETH1_PORT  5001            // 开放与上位机通讯的端口号

#define MAIN_CYCLE_TIME  1000                 // 主站过程数据通讯周期
#define SUB_CYCLE_TIME  (MAIN_CYCLE_TIME * 5) // 邮箱数据通讯周期
#define COE_SDO_DEMO  0                       // 是否需要通过SDO配置从站
#define DETAIL_LOGS  0                        // 打印电机运行LOG


bool ecat_context_request_state(ECAT_SIMPLE_START_CONTEXT *ctx, ECAT_BYTE state, int retry)
{
    ECAT_RESULT ret;
    ECAT_BYTE query = EcatStateNotSet;

    LOG_RET(ret, EcatRequestMasterState(ctx->Master, state));
    if (ECAT_FAILED(ret)) return false;

    while (retry--) {
        LOG_RET(ret, EcatGetMasterState(ctx->Master, &query));
        if (ECAT_FAILED(ret)) break;
        _DBG_("request_state=%d, query_state=%d", state, query);
        if (query == state) return true;
        usleep(100000);
    }
    return false;
}

typedef struct {
    ECAT_WORD slave;
    ECAT_WORD index;
    ECAT_BYTE subindex;
    ECAT_DWORD len;
    ECAT_DWORD val;
} ecat_sdo_t;

int query_by_sdo(ECAT_SIMPLE_START_CONTEXT* ctx, uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t* val){
    ecat_sdo_t item = {slave, index, subindex, len, 0};      
    ECAT_DWORD len2 = item.len;
    int ret = EcatCoeSDOUpload(ctx->Master, item.slave, item.index, item.subindex,
                        ECAT_FALSE, &len2, (ECAT_BYTE*)&item.val, len2, 5000);  
    if (val)
        *val = item.val;
    printf("slave=%d, index=0x%04x, subindex=0x%02x, value=0x%08x(%d)\r\n", item.slave, item.index, item.subindex, item.val, item.val);
    return ret;
}

int write_by_sdo(ECAT_SIMPLE_START_CONTEXT* ctx, uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t val){
    ecat_sdo_t item = {slave, index, subindex, len, val};  
    ECAT_DWORD len2 = item.len;

    int ret = EcatCoeSDODownload(ctx->Master, item.slave, item.index, item.subindex,
                            ECAT_FALSE,&len2,(ECAT_BYTE*)&item.val,len2,5000);

    if(!ret) printf("coe_write ok\r\n");
    else     printf("fail : slave=%d, index=0x%04x, subindex=0x%02x, value=0x%08x(%d)\r\n", item.slave, item.index, item.subindex, item.val, item.val);
    return ret;
}

ECAT_RESULT servo_set_opmode_by_sdo(ECAT_SIMPLE_START_CONTEXT& ctx, int _num_servos, uint32_t op_mode)
{
    ECAT_RESULT ret = ECAT_S_OK;
    ECAT_DWORD len;
    uint32_t pp_speed = 500;

    ecat_sdo_t items[] = { 
        { 0, 0x6060, 0x00, 1, op_mode}, // 控制模式
        // { 0, 0x6081, 0x00, 4, pp_speed } //speed --> pp mode
    };

    for(uint32_t s=0; s<_num_servos; s++)
    {
        for (uint32_t i = 0; i < ARRAY_SIZE(items); i++)
        {
            auto &item = items[i];
            len = item.len;

            ret = EcatCoeSDODownload(ctx.Master, 
                s, item.index, item.subindex,
                ECAT_FALSE, &len, (ECAT_BYTE*)&item.val, len, 5000);
            if (ECAT_FAILED(ret)) break;
        }
    }

    return ret;
}


EcatBase::EcatBase(const char* eth) {
    memset(&ctx, 0, sizeof(ctx));
    ctx.NIC0 = (char*)eth;
}
EcatBase::EcatBase(const std::string& ecat_name) {
    memset(&ctx, 0, sizeof(ctx));
    auto ptr = new std::string(ecat_name);
    ctx.NIC0 = (char*)ptr->c_str();
}
EcatBase::~EcatBase() { stop(); }
int EcatBase::set_eni(const char* eni) {
    ctx.ENI = (char*) eni;
    return 0;
}
void EcatBase::use_default_cfg() {
    ctx.MainCycleTimeUs = MAIN_CYCLE_TIME;
    ctx.SubCycleTimeUs = SUB_CYCLE_TIME;
    ctx.AutoRecoveryTimeoutMs = 1000;
    ctx.RpcServerPort = RPC_SERVER_ETH0_PORT;
}

int EcatBase::simple_start(){ 
    int ret = EcatSimpleStart((ECAT_SIMPLE_START_CONTEXT*) &ctx);
    if (ECAT_FAILED(ret)) {
        _ERR_("EcatSimpleStart failed: ret=%d", ret);
        return ret;
    }
    return ret;
}
int EcatBase::simple_start_wrap()
{
    ECAT_RESULT ret;
    char* eni = (char*) ctx.ENI;
    char* NIC0 = (char*) ctx.NIC0;
    memset(&ctx, 0, sizeof(ctx));
    ctx.NIC0 = NIC0;
    ctx.ENI = eni;
    use_default_cfg();
    return simple_start();
}

int EcatBase::start(){
    // if (!load(eni_dir.c_str())) return false;
    int ret = simple_start();
    if (ECAT_FAILED(ret)) {
        return ret & 0x8000;;
    }
    // 进入预操作状态
    if (!request_state(EcatStateO)){
		printf("request_state(EcatStateO) failed!\n");
        // return -1;
    }
    return 0;
}

bool EcatBase::request_state(int stat, int retry){
    if (!ecat_context_request_state((ECAT_SIMPLE_START_CONTEXT*) &ctx, stat, retry)) {
        _ERR_("ecat_master_request_state(%d) failed!", stat);
        return false;
    }
    // Record request state because EcatGetMasterReqState is not provided.
    master_req_state_ = static_cast<uint8_t>(stat);
    master_req_state_valid_ = true;
    return true;
    // BREAK_IF_FAILED(ret, EcatSetExtCtrlTaskHandler(ctx.Master, (EcatExtCtrlTaskHandler)&task, this));
};

int EcatBase::request_master_state(uint8_t state) {
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    const int ret = EcatRequestMasterState(ctx.Master, state);
    if (ret == 0) {
        // Record request state because EcatGetMasterReqState is not provided.
        master_req_state_ = state;
        master_req_state_valid_ = true;
    }
    return ret;
}

int EcatBase::bus_is_running(bool* running) {
    // Distinguish caller errors from an unavailable master. GPT-6, 2026-10-21.
    if (!running) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    uint8_t state = EcatStateNotSet;
    const int ret = EcatGetMasterState(ctx.Master, &state);
    if (ret != 0) return ret;
    // Record running state because EcatBusIsRunning is not provided.
    *running = ((state & EcatStateMask) >= EcatStateI);
    return 0;
}

int EcatBase::get_master_req_state(uint8_t* state) const {
    if (!state) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    if (!master_req_state_valid_) return ZMC_ERR_ECAT_MASTER_REQ_STATE_UNAVAILABLE;
    *state = master_req_state_;
    return 0;
}

int EcatBase::get_master_state(uint8_t* state){
    if (!state) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    return EcatGetMasterState(ctx.Master, state);
}

int EcatBase::get_slave_count(uint16_t* count) {
    if (!count) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    return EcatGetSlaveCount(ctx.Master, count);
}

int EcatBase::get_active_slave_count(uint16_t* count) {
    if (!count) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    return EcatGetActiveSlaveCount(ctx.Master, count);
}

int EcatBase::get_slave_state(uint16_t slave_id, uint8_t* state) {
    if (!state) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    return EcatGetSlaveState(ctx.Master, slave_id, state);
}

int EcatBase::clear_slave_state_error(uint16_t slave_id) {
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    return EcatClearSlaveStateError(ctx.Master, slave_id);
}

int EcatBase::pi_enable_set(bool enable) {
    pi_enabled_ = enable;
    return 0;
}

int EcatBase::pi_enable_get(bool* enable) const {
    if (!enable) return ZMC_ERR_ECAT_NULL_ARG;
    *enable = pi_enabled_;
    return 0;
}

int EcatBase::coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                                uint8_t complete_access, const void* data, uint32_t data_len,
                                uint32_t timeout_ms) {
    if (!data) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    if (data_len == 0) return ZMC_ERR_SDO_INVALID_LENGTH;
    ECAT_DWORD len = data_len;
    return EcatCoeSDODownload(ctx.Master, slave_id, index, subindex,
                              static_cast<ECAT_BOOL>(complete_access), &len,
                              const_cast<ECAT_BYTE*>(static_cast<const ECAT_BYTE*>(data)),
                              data_len, timeout_ms);
}

int EcatBase::coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                              uint8_t complete_access, uint32_t* data_len, void* data,
                              uint32_t timeout_ms) {
    if (!data_len || !data) return ZMC_ERR_ECAT_NULL_ARG;
    if (!ctx.Master) return ZMC_ERR_ECAT_INVALID_HANDLE;
    if (*data_len == 0) return ZMC_ERR_SDO_INVALID_LENGTH;
    return EcatCoeSDOUpload(ctx.Master, slave_id, index, subindex,
                            static_cast<ECAT_BOOL>(complete_access), data_len,
                            static_cast<ECAT_BYTE*>(data), *data_len, timeout_ms);
}

int EcatBase::stop(){ return EcatSimpleStop((ECAT_SIMPLE_START_CONTEXT*) &ctx);}
int EcatBase::bind(EcatExtCtrlTaskHandler cb, void* obj){
    return EcatSetExtCtrlTaskHandler(ctx.Master, (EcatExtCtrlTaskHandler)cb, obj);
}
void EcatBase::unbind(){
    EcatResetExtCtrlTaskHandler(ctx.Master);
}
int EcatBase::query_by_sdo(uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t* val){
    ecat_sdo_t item = {slave, index, subindex, len, 0};      
    ECAT_DWORD len2 = item.len;
    int ret = EcatCoeSDOUpload(ctx.Master, item.slave, item.index, item.subindex,
                        ECAT_FALSE, &len2, (ECAT_BYTE*)&item.val, len2, 5000);  
    if (!ret){
    if (val)
        *val = item.val;
    // printf("slave=%d, index=0x%04x, subindex=0x%02x, value=0x%08x(%d)\r\n", item.slave, item.index, item.subindex, item.val, item.val);
    }
    return ret;
}
int EcatBase::write_by_sdo(uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t val){
    ecat_sdo_t item = {slave, index, subindex, len, val};  
    ECAT_DWORD len2 = item.len;

    int ret = EcatCoeSDODownload(ctx.Master, item.slave, item.index, item.subindex,
                            ECAT_FALSE,&len2,(ECAT_BYTE*)&item.val,len2,5000);

    if(!ret) {
        // printf("coe_write ok\r\n");
    }
    else     printf("fail : slave=%d, index=0x%04x, subindex=0x%02x, value=0x%08x(%d)\r\n", item.slave, item.index, item.subindex, item.val, item.val);
    return ret;
}
int EcatBase::set_opmode_by_sdo(int num_servos, uint32_t op_mode){
    return servo_set_opmode_by_sdo((ECAT_SIMPLE_START_CONTEXT&) ctx, num_servos, op_mode);
}

ethercat_env::ethercat_env()
{
    ECAT_RESULT ret;
    LOG_RET(ret, EcatMasterLibInit(ECAT_NULL));
    if (ECAT_FAILED(ret))
    {
        _ERR_("EcatMasterLibInit error, ret=%d", ret);
    }
}
ethercat_env::~ethercat_env()
{
    ECAT_RESULT ret;
    LOG_RET(ret, EcatMasterLibExit());
}
