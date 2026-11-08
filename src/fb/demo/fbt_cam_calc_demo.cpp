#include "stdio.h"
#include "fb_cpp_def.h"
#include "cam_tappet_serialize.h"
#include "zmc_cam_table.h"

using namespace zmc;

void fbt_cam_calc_demo() {
    printf("=== FbtCamCalc Demo ===\n");
    
    // Create the function block.
    FbtReadCAM fbRdCam;
    FbtCamCalc fbCalc;
    
    // Read the cam file.
    fbRdCam.sFileName = ZMC_STRING_80{"abc3.cam"};
    fbRdCam.bExecute = true;
    
    printf("Reading CAM file: %s\n", fbRdCam.sFileName);
    
    // Execute the read operation.
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
    
    // Set the cam table.
    fbCalc.CamTable = fbRdCam.CAM;
    
    // Set the default parameters.
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
    
    // Test different master positions.
    double testPositions[] = {0.0, 45.0, 90.0, 135.0, 180.0, 225.0, 270.0, 315.0, 360.0};
    int numTests = sizeof(testPositions) / sizeof(testPositions[0]);
    
    for (int i = 0; i < numTests; i++) {
        // Set the master position, velocity, and acceleration.
        fbCalc.MasterPosition = testPositions[i];
        fbCalc.MasterVelocity = 10.0;  // Constant velocity.
        fbCalc.MasterAcceleration = 0.0;  // No acceleration.
        
        // Enable calculation.
        fbCalc.Enable = true;
        
        // Run one cycle.
        fbCalc.runCycle();
        
        // Output the result.
        printf("%d\t%d\t%.1f\t\t%.1f\t\t%.1f\t\t%.3f\t\t%.3f\t\t%.3f\t\t%d\t%d\n",
               i, fbCalc.Enable, fbCalc.MasterPosition, fbCalc.MasterVelocity, fbCalc.MasterAcceleration,
               fbCalc.SlavePosition, fbCalc.SlaveVelocity, fbCalc.SlaveAcceleration,
               fbCalc.Busy, fbCalc.Error);
        
        // Disable calculation.
        fbCalc.Enable = false;
        fbCalc.runCycle();
    }
    
    printf("\nDemo completed successfully!\n");
}
