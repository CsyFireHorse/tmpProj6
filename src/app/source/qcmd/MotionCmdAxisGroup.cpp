#include "qcmd/MotionCmdAxisGroup.hpp"

#include <cmath>

#include "fbmEnum.h"
#include "fbt_errorcode_def.h"
namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

static inline bool isFiniteCircularCoordinate(double value) { return std::isfinite(value); }

// Circular commands use the controller's fixed two-axis payload.
static inline bool isTwoAxisGroup(int axisCount, const int* axisIdxArr) {
    return axisCount == 2 && axisIdxArr != nullptr;
}

static inline MotionCmdCircularParam makeCircularParam(double end1, double end2, double aux1,
                                                       double aux2, uint16_t pathChoice) {
    MotionCmdCircularParam param = {};
    param.aux[0] = aux1;
    param.aux[1] = aux2;
    param.end[0] = end1;
    param.end[1] = end2;
    param.pathChoice = pathChoice;
    return param;
}

}  // namespace

int MotionCmdAxisGroup::multiMoveAbs(int axisCount, const int* axisIdxArr, const double* posArr) {
    if (axisCount <= 0 || static_cast<size_t>(axisCount) > MOTION_CMD_PARAM_MAX / sizeof(double) ||
        !axisIdxArr || !posArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    const size_t param_size = static_cast<size_t>(axisCount) * sizeof(double);
    return appendOrDeferItem(FBM_MULTI_MOVE, const_cast<double*>(posArr),
                             static_cast<uint16_t>(param_size));
}

int MotionCmdAxisGroup::multiMoveRel(int axisCount, const int* axisIdxArr, const double* posArr) {
    if (axisCount <= 0 || static_cast<size_t>(axisCount) > MOTION_CMD_PARAM_MAX / sizeof(double) ||
        !axisIdxArr || !posArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    const size_t param_size = static_cast<size_t>(axisCount) * sizeof(double);
    return appendOrDeferItem(FBM_MULTI_MOVE_REL, const_cast<double*>(posArr),
                             static_cast<uint16_t>(param_size));
}

int MotionCmdAxisGroup::moveCirc(int axisCount, const int* axisIdxArr, double end1, double end2,
                                 double centre1, double centre2, int direction) {
    if (!isTwoAxisGroup(axisCount, axisIdxArr)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (direction != 0 && direction != 1) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (!isFiniteCircularCoordinate(end1) || !isFiniteCircularCoordinate(end2) ||
        !isFiniteCircularCoordinate(centre1) || !isFiniteCircularCoordinate(centre2)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const MotionCmdCircularParam param =
        makeCircularParam(end1, end2, centre1, centre2, direction == 0 ? 1 : 0);
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    return appendOrDeferItem(FBM_MULTI_CIRCULAR, &param, sizeof(param));
}

int MotionCmdAxisGroup::moveCircAbs(int axisCount, const int* axisIdxArr, double end1, double end2,
                                    double centre1, double centre2, int direction) {
    if (!isTwoAxisGroup(axisCount, axisIdxArr)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (direction != 0 && direction != 1) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (!isFiniteCircularCoordinate(end1) || !isFiniteCircularCoordinate(end2) ||
        !isFiniteCircularCoordinate(centre1) || !isFiniteCircularCoordinate(centre2)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const MotionCmdCircularParam param =
        makeCircularParam(end1, end2, centre1, centre2, direction == 0 ? 1 : 0);
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    return appendOrDeferItem(FBM_MULTI_CIRCULAR_ABS, &param, sizeof(param));
}

int MotionCmdAxisGroup::moveCirc2(int axisCount, const int* axisIdxArr, double mid1, double mid2,
                                  double end1, double end2) {
    if (!isTwoAxisGroup(axisCount, axisIdxArr)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (!isFiniteCircularCoordinate(mid1) || !isFiniteCircularCoordinate(mid2) ||
        !isFiniteCircularCoordinate(end1) || !isFiniteCircularCoordinate(end2)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const MotionCmdCircularParam param = makeCircularParam(end1, end2, mid1, mid2, 0);
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    return appendOrDeferItem(FBM_MULTI_CIRCULAR2, &param, sizeof(param));
}

int MotionCmdAxisGroup::moveCirc2Abs(int axisCount, const int* axisIdxArr, double mid1,
                                     double mid2, double end1, double end2) {
    if (!isTwoAxisGroup(axisCount, axisIdxArr)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (!isFiniteCircularCoordinate(mid1) || !isFiniteCircularCoordinate(mid2) ||
        !isFiniteCircularCoordinate(end1) || !isFiniteCircularCoordinate(end2)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const MotionCmdCircularParam param = makeCircularParam(end1, end2, mid1, mid2, 0);
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    if (axisRet != 0) return axisRet;
    return appendOrDeferItem(FBM_MULTI_CIRCULAR2_ABS, &param, sizeof(param));
}

int MotionCmdAxisGroup::groupSelect(int idx) {
    return appendOrDeferItem(FBM_GROUP_SELECT, &idx, sizeof(int));
}

int MotionCmdAxisGroup::groupSetAxes(int n, const int* axisIdxArr) {
    if (n <= 0 || !axisIdxArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const size_t param_size = static_cast<size_t>(n) * sizeof(int);
    return appendOrDeferItem(FBM_AXIS_GROUP, const_cast<int*>(axisIdxArr),
                             static_cast<uint16_t>(param_size));
}

int MotionCmdAxisGroup::groupEnable(int axisCount, const int* axisIdxArr) {
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    return axisRet != 0 ? axisRet : appendOrDeferItem(FBM_GROUP_ENABLE, nullptr, 0);
}

int MotionCmdAxisGroup::groupDisable(int axisCount, const int* axisIdxArr) {
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    return axisRet != 0 ? axisRet : appendOrDeferItem(FBM_GROUP_DISABLE, nullptr, 0);
}

int MotionCmdAxisGroup::groupReset(int axisCount, const int* axisIdxArr) {
    const int axisRet = groupSetAxes(axisCount, axisIdxArr);
    return axisRet != 0 ? axisRet : appendOrDeferItem(FBM_GROUP_RESET, nullptr, 0);
}

}  // namespace zmc
