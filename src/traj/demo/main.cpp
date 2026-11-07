#include "common/misc/CmdParseItem.hpp"
#include "stdio.h"
#include "string.h"

void TrajSg_demo();
void TrajLinear_demo();
void TrajCircBorder2D_demo();
void MotionBuffer_line_demo();
void MotionBuffer_corner_demo();
void MotionBuffer_arc_demo();
void LinearSplineProfile_demo();
void CubicPoly_demo();
void CubicPoly2_demo();
void FifthPoly_demo();
void FifthPoly_demo2();
void CubicRecursionProfile_demo();
void CubicSplineProfile_demo();
void FifthSplineProfile_demo();
void PoliFitterWrap_demo();
void PoliFitterWrap_pos_demo();
void PoliFitterWrap_xyva_demo();

using namespace zmc;
static std::string help_str = "";

int printHelp() {
    printf("%s\n", help_str.c_str());
    return 0;
}

static const char* EXE_NAME = "plcfb_demo";

int printVersion() {
    printf(
        "%s: traj demo tool\n"
        "version 1.0.0\n"
        "create by csy on 2024/12/25,"
        "update on 2024/12/25\n",
        EXE_NAME);
    return 0;
}

static int cmdParseItem_demo(int argc, char** argv) {
    const static std::vector<CmdParseItem> cmds = {
        {{}, CMD_WRAP(printHelp)},
        {{"--help"}, CMD_WRAP(printHelp)},
        {{"--version"}, CMD_WRAP(printVersion)},
        {{"traj", "cubic_poly"}, CMD_WRAP(CubicPoly_demo)},
        {{"traj", "cubic_poly2"}, CMD_WRAP(CubicPoly2_demo)},
        {{"traj", "linear_poly_spline"}, CMD_WRAP(LinearSplineProfile_demo)},
        {{"traj", "cubic_poly_spline"}, CMD_WRAP(CubicSplineProfile_demo)},
        {{"traj", "cubic_recursion_spline"}, CMD_WRAP(CubicRecursionProfile_demo)},
        {{"traj", "fifth_poly_spline"}, CMD_WRAP(FifthSplineProfile_demo)},
        {{"traj", "poly_spline_wrap"}, CMD_WRAP(PoliFitterWrap_demo)},
        {{"traj", "cubic"}, CMD_WRAP(PoliFitterWrap_pos_demo)},
        {{"traj", "wrap"}, CMD_WRAP(PoliFitterWrap_demo)},
        {{"traj", "wrap_xyva"}, CMD_WRAP(PoliFitterWrap_xyva_demo)},
        {{"traj", "fifthpoly"}, CMD_WRAP(FifthPoly_demo)},
        {{"traj", "fifthpoly2"}, CMD_WRAP(FifthPoly_demo2)},
        {{"sg"}, CMD_WRAP(TrajSg_demo)},
        {{"linear"}, CMD_WRAP(TrajLinear_demo)},
        {{"circ3"}, CMD_WRAP(TrajCircBorder2D_demo)},
        {{"motion_buffer"}, CMD_WRAP(MotionBuffer_line_demo)},
        {{"motion_buffer", "line"}, CMD_WRAP(MotionBuffer_line_demo)},
        {{"motion_buffer", "corner"}, CMD_WRAP(MotionBuffer_corner_demo)},
        {{"motion_buffer", "arc"}, CMD_WRAP(MotionBuffer_arc_demo)},
    };
    genHelpStr(cmds, help_str);
    return cmdParseItem_match(cmds, argc, argv);
}


int main(int argc, char** argv) {
    return cmdParseItem_demo(argc - 1, argv + 1);
}