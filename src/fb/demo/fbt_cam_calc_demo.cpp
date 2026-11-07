#include "stdio.h"
#include "fb_cpp_def.h"
#include "cam_tappet_serialize.h"
#include "zmc_cam_table.h"

using namespace zmc;

void fbt_cam_calc_demo() {
    printf("=== FbtCamCalc Demo ===\n");
    
    // 创建功能块
    FbtReadCAM fbRdCam;
    FbtCamCalc fbCalc;
    
    // 读取凸轮文件
    fbRdCam.sFileName = ZMC_STRING_80{"abc3.cam"};
    fbRdCam.bExecute = true;
    
    printf("Reading CAM file: %s\n", fbRdCam.sFileName);
    
    // 执行读取
    for (int i = 0; i < 10; i++) {
        fbRdCam.runCycle();
        if (fbRdCam.bDone) {
            printf("CAM file read successfully\n");
            break;
        }
        if (fbRdCam.bError) {
            printf("Error reading CAM file: %d\n", fbRdCam.ErrorID);
            return;
        }
    }
    
    // 设置凸轮表
    fbCalc.CamTable = fbRdCam.CAM;
    
    // 设置默认参数
    fbCalc.MasterScaling = 1.0;
    fbCalc.SlaveScaling = 1.0;
    fbCalc.MasterOffset = 0.0;
    fbCalc.SlaveOffset = 0.0;
    fbCalc.MasterAbsolute = true;
    fbCalc.SlaveAbsolute = true;
    fbCalc.MasterEngagePos = 0.0;
    fbCalc.SlaveEngagePos = 0.0;
    
    printf("\nTesting CAM calculation:\n");
    printf("Cycle\tEnable\tMasterPos\tMasterVel\tMasterAcc\tSlavePos\tSlaveVel\tSlaveAcc\tBusy\tError\n");
    printf("--------------------------------------------------------------------------------------------------------\n");
    
    // 测试不同主轴位置
    double testPositions[] = {0.0, 45.0, 90.0, 135.0, 180.0, 225.0, 270.0, 315.0, 360.0};
    int numTests = sizeof(testPositions) / sizeof(testPositions[0]);
    
    for (int i = 0; i < numTests; i++) {
        // 设置主轴PVA
        fbCalc.MasterPosition = testPositions[i];
        fbCalc.MasterVelocity = 10.0;  // 固定速度
        fbCalc.MasterAcceleration = 0.0;  // 无加速度
        
        // 启用计算
        fbCalc.Enable = true;
        
        // 运行一个周期
        fbCalc.runCycle();
        
        // 输出结果
        printf("%d\t%d\t%.1f\t\t%.1f\t\t%.1f\t\t%.3f\t\t%.3f\t\t%.3f\t\t%d\t%d\n",
               i, fbCalc.Enable, fbCalc.MasterPosition, fbCalc.MasterVelocity, fbCalc.MasterAcceleration,
               fbCalc.SlavePosition, fbCalc.SlaveVelocity, fbCalc.SlaveAcceleration,
               fbCalc.Busy, fbCalc.Error);
        
        // 禁用计算
        fbCalc.Enable = false;
        fbCalc.runCycle();
    }
    
    printf("\nDemo completed successfully!\n");
}
