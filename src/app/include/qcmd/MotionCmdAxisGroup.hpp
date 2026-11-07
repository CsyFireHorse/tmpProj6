#ifndef _ZMC_MOTION_CMD_AXIS_GROUP_HPP_
#define _ZMC_MOTION_CMD_AXIS_GROUP_HPP_

#include "qcmd/MotionCmdSingleAxis.hpp"

namespace zmc {

// Typed commands that target an explicitly supplied axis group.
class MotionCmdAxisGroup : public MotionCmdSingleAxis {
   public:
    MotionCmdAxisGroup() : MotionCmdSingleAxis() {}
    int multiMoveAbs(int axisCount, const int* axisIdxArr, const double* posArr);
    int multiMoveRel(int axisCount, const int* axisIdxArr, const double* posArr);
    // Enqueues a relative center-defined circular move for an explicit two-axis group.
    int moveCirc(int axisCount, const int* axisIdxArr, double end1, double end2, double centre1,
                 double centre2, int direction);
    // Enqueues an absolute center-defined circular move for an explicit two-axis group.
    int moveCircAbs(int axisCount, const int* axisIdxArr, double end1, double end2, double centre1,
                    double centre2, int direction);
    // Enqueues a relative three-point circular move for an explicit two-axis group.
    int moveCirc2(int axisCount, const int* axisIdxArr, double mid1, double mid2, double end1,
                  double end2);
    // Enqueues an absolute three-point circular move for an explicit two-axis group.
    int moveCirc2Abs(int axisCount, const int* axisIdxArr, double mid1, double mid2, double end1,
                     double end2);
    int groupSelect(int idx);
    int groupSetAxes(int n, const int* axisIdxArr);
    int groupEnable(int axisCount, const int* axisIdxArr);
    int groupDisable(int axisCount, const int* axisIdxArr);
    int groupReset(int axisCount, const int* axisIdxArr);
};

}  // namespace zmc

#endif  // _ZMC_MOTION_CMD_AXIS_GROUP_HPP_
