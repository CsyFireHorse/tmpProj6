#include <gtest/gtest.h>
#include "fb_cpp_def.h"
#include "mc_ref_def.h"
#include "camGenHelper.hpp"
#include "cam_tappet_serialize.h"
#include "mc_cam_table.h"

using namespace zmc;

TEST(cam, WriteReadCycle) {
    constexpr int CAM_XYVA_LEN = 20;
    const ZMC_STRING_80 filename{"abc.cam"};

    CamXyva dat[CAM_XYVA_LEN]{};
    fillXyvaToy(reinterpret_cast<Xyva*>(dat), CAM_XYVA_LEN, 90);

    FbtWriteCAM wrCam;
    wrCam.bExecute = true;
    wrCam.sFileName = filename;
    wrCam.CAM = makeCamRef(dat, CAM_XYVA_LEN);


    wrCam.runCycle();
    EXPECT_TRUE(wrCam.bDone);
    EXPECT_FALSE(wrCam.bError);


    FbtReadCAM rdCam;
    rdCam.bExecute = true;
    rdCam.sFileName = filename;
    

    rdCam.runCycle();
    EXPECT_TRUE(rdCam.bDone);
    EXPECT_FALSE(rdCam.bError);

    // ASSERT_NE(rdCam.CAM.pData, nullptr);
    auto pce = (CamXyva*)rdCam.CAM->pce;
    for (int i = 0; i < CAM_XYVA_LEN; i++) {
        EXPECT_NEAR(pce[i].dX, dat[i].dX, 1e-7);
        EXPECT_NEAR(pce[i].dY, dat[i].dY, 1e-7);
        EXPECT_NEAR(pce[i].dV, dat[i].dV, 1e-7);
        EXPECT_NEAR(pce[i].dA, dat[i].dA, 1e-7); 
    }
    // EXPECT_EQ(memcmp(rdCam.CAM->pce, dat, sizeof(CamXyva)*CAM_XYVA_LEN), 0);
}