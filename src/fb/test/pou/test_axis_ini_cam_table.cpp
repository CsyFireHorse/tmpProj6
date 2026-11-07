#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "common/zmc/fbt_errorcode_def.h"
#include "pou/AxisIniLoader.hpp"

namespace {

const char kCompleteCamIni[] =
    "[cam_table_0]\n"
    "version = 2.3.0\n"
    "xStart = -10\n"
    "xEnd = 10\n"
    "xyva_count = 2\n"
    "xyva_0 = -1e1, -2e1, -3e0, -4e0\n"
    "xyva_1 = 10, 20, 3, 4\n"
    "tappet_count = 2\n"
    "tappet_0 = 1, 0, 0, -5, 0, 0\n"
    "tappet_1 = 2, 1, 3, 5, 1000, 2000\n"
    "[cam_table_1]\n"
    "version = 1.0.0\n"
    "xStart = 0\n"
    "xEnd = 360\n"
    "xyva_count = 2\n"
    "xyva_0 = 0, 0, 0, 0\n"
    "xyva_1 = 360, 1, 0, 0\n"
    "tappet_count = 0\n";

// Keep file-input coverage local to the test working directory.
class TempIniFile {
   public:
    TempIniFile(const char* path, const char* content) : path_(path) {
        std::remove(path_.c_str());
        std::ofstream stream(path_.c_str(), std::ios::out | std::ios::trunc);
        stream << content;
        valid_ = stream.good();
    }

    ~TempIniFile() { std::remove(path_.c_str()); }

    bool valid() const { return valid_; }
    const std::string& path() const { return path_; }

   private:
    std::string path_;
    bool valid_ = false;
};

}  // namespace

TEST(AxisIniCamTable, LoadsCompleteInheritedCamTable) {
    AxisIniLoader loader(kCompleteCamIni, true);
    IniCamTable cam;

    ASSERT_EQ(0, loader.load_cam_table(&cam));

    EXPECT_EQ(2, cam.ver_major);
    EXPECT_EQ(3, cam.ver_minor);
    EXPECT_EQ(0, cam.ver_patch);
    EXPECT_DOUBLE_EQ(-10.0, cam.xStart);
    EXPECT_DOUBLE_EQ(10.0, cam.xEnd);
    EXPECT_EQ(2, cam.nElements);
    EXPECT_EQ(reinterpret_cast<ZMC_BYTE*>(cam.xyva), cam.pce);
    EXPECT_DOUBLE_EQ(-20.0, cam.xyva[0].dY);
    EXPECT_DOUBLE_EQ(4.0, cam.xyva[1].dA);

    ASSERT_EQ(2, cam.nTappets);
    EXPECT_EQ(reinterpret_cast<SMC_CAMTappet*>(cam.tappets), cam.pt);
    EXPECT_EQ(1u, cam.tappets[0].iGroupID);
    EXPECT_EQ(TAPPET_pos, cam.tappets[0].ctt);
    EXPECT_EQ(TAPPETACTION_on, cam.tappets[0].cta);
    EXPECT_DOUBLE_EQ(-5.0, cam.tappets[0].x);
    EXPECT_EQ(0xffffffffu, cam.tappets[0].dwActive);
    EXPECT_EQ(2u, cam.tappets[1].iGroupID);
    EXPECT_EQ(TAPPET_all, cam.tappets[1].ctt);
    EXPECT_EQ(TAPPETACTION_time, cam.tappets[1].cta);
    EXPECT_EQ(1000u, cam.tappets[1].dwDelay);
    EXPECT_EQ(2000u, cam.tappets[1].dwDuration);

    zmc::CamTable* cpp_ref = &cam;
    MC_CAM_REF cam_ref = &cam;
    EXPECT_EQ(static_cast<zmc::CamTable*>(&cam), cpp_ref);
    EXPECT_EQ(static_cast<MC_CAM_TABLE*>(&cam), cam_ref);
}

TEST(AxisIniCamTable, LoadsZeroTappetTable) {
    AxisIniLoader loader(kCompleteCamIni, true);
    IniCamTable cam;
    cam.table_index = 1;

    ASSERT_EQ(0, loader.load_cam_table(&cam));
    EXPECT_EQ(2, cam.nElements);
    EXPECT_EQ(0, cam.nTappets);
    EXPECT_EQ(nullptr, cam.pt);
    EXPECT_DOUBLE_EQ(0.0, cam.xStart);
    EXPECT_DOUBLE_EQ(360.0, cam.xEnd);
}

TEST(AxisIniCamTable, LoadsCompleteTableFromFile) {
    TempIniFile ini_file("test_axis_ini_cam_table_temp.ini", kCompleteCamIni);
    ASSERT_TRUE(ini_file.valid());
    AxisIniLoader loader(ini_file.path().c_str(), false);
    IniCamTable cam;

    ASSERT_EQ(0, loader.load_cam_table(&cam));
    EXPECT_EQ(2, cam.nElements);
    EXPECT_EQ(2, cam.nTappets);
    EXPECT_DOUBLE_EQ(10.0, cam.xEnd);
}

TEST(AxisIniCamTable, RejectsMissingSectionAndDuplicateKeys) {
    AxisIniLoader missing_section("[base]\ncycle_us=1000\n", true);
    IniCamTable cam;
    EXPECT_EQ(ZMC_ERR_INI_MISSING_SECTION, missing_section.load_cam_table(&cam));

    const char* duplicate_ini[] = {
        "[cam_table_0]\nversion=1.0.0\nversion=2.0.0\n"
        "xStart=0\nxEnd=1\nxyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\nxyva_0=1,1,1,1\ntappet_count=0\n",
    };
    for (size_t i = 0; i < sizeof(duplicate_ini) / sizeof(duplicate_ini[0]); ++i) {
        AxisIniLoader loader(duplicate_ini[i], true);
        EXPECT_EQ(ZMC_ERR_INI_DUPLICATE, loader.load_cam_table(&cam));
    }
}

TEST(AxisIniCamTable, RejectsMissingMetadataAndRecords) {
    const char* invalid_ini[] = {
        "[cam_table_0]\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=2\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=1\n",
    };
    for (size_t i = 0; i < sizeof(invalid_ini) / sizeof(invalid_ini[0]); ++i) {
        AxisIniLoader loader(invalid_ini[i], true);
        IniCamTable cam;
        EXPECT_EQ(ZMC_ERR_INI_MISSING_KEY, loader.load_cam_table(&cam));
    }
}

TEST(AxisIniCamTable, RejectsInvalidMetadataCountsAndKeys) {
    const char* format_ini[] = {
        "[cam_table_0]\nversion=-1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=value\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
    };
    for (size_t i = 0; i < sizeof(format_ini) / sizeof(format_ini[0]); ++i) {
        AxisIniLoader loader(format_ini[i], true);
        IniCamTable cam;
        EXPECT_EQ(ZMC_ERR_INI_VALUE_FORMAT, loader.load_cam_table(&cam));
    }

    const char* range_ini[] = {
        "[cam_table_0]\nversion=2147483648.0.0\n"
        "xStart=0\nxEnd=1\nxyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=nan\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=0\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=513\ntappet_count=0\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=513\n",
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\ntappet_0=1,0,0,0,0,0\n",
    };
    for (size_t i = 0; i < sizeof(range_ini) / sizeof(range_ini[0]); ++i) {
        AxisIniLoader loader(range_ini[i], true);
        IniCamTable cam;
        EXPECT_EQ(ZMC_ERR_INI_VALUE_RANGE, loader.load_cam_table(&cam));
    }

    AxisIniLoader unknown_key(
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=0\nname=cam\n",
        true);
    IniCamTable cam;
    EXPECT_EQ(ZMC_ERR_INI_KEY, unknown_key.load_cam_table(&cam));

    AxisIniLoader invalid_index(
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=1\nxyva_01=0,0,0,0\ntappet_count=0\n",
        true);
    EXPECT_EQ(ZMC_ERR_INI_KEY, invalid_index.load_cam_table(&cam));
}

TEST(AxisIniCamTable, RejectsInvalidTappetFields) {
    const char* invalid_tappet[] = {
        "1, 3, 0, 0, 0, 0",   "1, 0, 4, 0, 0, 0",          "65536, 0, 0, 0, 0, 0",
        "1, 0, 0, inf, 0, 0", "1, 0, 0, 0, 4294967296, 0", "1, 0, 0, 0, 0",
    };
    const int expected[] = {
        ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_FORMAT,
    };
    for (size_t i = 0; i < sizeof(invalid_tappet) / sizeof(invalid_tappet[0]); ++i) {
        const std::string ini =
            "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
            "xyva_count=1\nxyva_0=0,0,0,0\ntappet_count=1\ntappet_0=" +
            std::string(invalid_tappet[i]) + "\n";
        AxisIniLoader loader(ini.c_str(), true);
        IniCamTable cam;
        EXPECT_EQ(expected[i], loader.load_cam_table(&cam));
    }
}

TEST(AxisIniCamTable, RejectsInvalidXyvaFields) {
    const char* invalid_xyva[] = {
        "0, 0, 0",
        "0, 0, 0, 0, 0",
        "0, 0, nan, 0",
        "0, 0, 0, 0tail",
    };
    const int expected[] = {
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_FORMAT,
    };
    for (size_t i = 0; i < sizeof(invalid_xyva) / sizeof(invalid_xyva[0]); ++i) {
        const std::string ini =
            "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
            "xyva_count=1\nxyva_0=" +
            std::string(invalid_xyva[i]) + "\ntappet_count=0\n";
        AxisIniLoader loader(ini.c_str(), true);
        IniCamTable cam;
        EXPECT_EQ(expected[i], loader.load_cam_table(&cam));
    }
}

TEST(AxisIniCamTable, FailureLeavesInheritedAndOwnedStateUnchanged) {
    const char ini[] =
        "[cam_table_0]\nversion=1.0.0\nxStart=0\nxEnd=1\n"
        "xyva_count=2\nxyva_0=0,0,0,0\nxyva_1=invalid\ntappet_count=0\n";
    AxisIniLoader loader(ini, true);
    IniCamTable cam;
    cam.ver_major = 7;
    cam.xStart = 12.0;
    cam.nElements = 9;
    cam.xyva[0] = CamXyva{42.0, 43.0, 44.0, 45.0};
    cam.tappets[0].dwActive = 123u;

    EXPECT_EQ(ZMC_ERR_INI_VALUE_FORMAT, loader.load_cam_table(&cam));
    EXPECT_EQ(7, cam.ver_major);
    EXPECT_DOUBLE_EQ(12.0, cam.xStart);
    EXPECT_EQ(9, cam.nElements);
    EXPECT_DOUBLE_EQ(42.0, cam.xyva[0].dX);
    EXPECT_EQ(123u, cam.tappets[0].dwActive);
}

TEST(AxisIniCamTable, LeavesCamRangeValidationToCamTable) {
    const char ini[] =
        "[cam_table_0]\nversion=1.0.0\n"
        "xStart=-1\nxEnd=10\nxyva_count=2\n"
        "xyva_0=0,0,0,0\nxyva_1=10,1,0,0\ntappet_count=0\n";
    AxisIniLoader loader(ini, true);
    IniCamTable cam;

    ASSERT_EQ(0, loader.load_cam_table(&cam));
    EXPECT_EQ(SMC_CAM_TABLE_DOES_NOT_COVER_MASTER_SCALE, cam.validate());
}

TEST(AxisIniCamTable, LeavesTappetPositionValidationToCamTable) {
    const char ini[] =
        "[cam_table_0]\nversion=1.0.0\n"
        "xStart=0\nxEnd=10\nxyva_count=2\n"
        "xyva_0=0,0,0,0\nxyva_1=10,1,0,0\n"
        "tappet_count=1\ntappet_0=1,0,0,11,0,0\n";
    AxisIniLoader loader(ini, true);
    IniCamTable cam;

    ASSERT_EQ(0, loader.load_cam_table(&cam));
    EXPECT_EQ(SMC_CI_MASTER_OUT_OF_SCALE, cam.validate());
}

// AI modification signature: GPT-5 Codex
