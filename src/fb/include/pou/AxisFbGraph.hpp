/*
 * @Author: ChenShengyong
 * @Date: 2024-09-03 09:29:04
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-03-03 13:53:55
 */
#ifndef _ZMC_AXIS_FB_GRAPH_HPP_
#define _ZMC_AXIS_FB_GRAPH_HPP_

#include "AxPou.hpp"
#include "common/zmc/zmc_predef.h"
#include "motbuf/FbMoveWrapV3.hpp"

// #include "axfb/AxisManager.h"
// #include "fbt_axis_impl.h"
#include "pdo_struct.h"
// #include "AxisIniLoader.h"
// #include "FbMoveWrap.hpp"
// #include "common/misc/DelayFilter.hpp"
// #include "fbt_axis_cia402.h"
#include "stdio.h"
// #include "common/get_file_path.hpp"

namespace zmc {

typedef servo_stat AxisCiaInput;
typedef servo_ctrl AxisCiaOutput;

class AxPouInout : public AxPouTaskBinderAuto {
   public:
    AxPouInout() : AxPouTaskBinderAuto() { reserve(MC_MOTION_MAX_AXES); }

    int configure(const std::string& iniPath = "", const std::string& iniContent = "",
                  unsigned int axNum = 1);
    int getAddress(int axisIdx) const;
    int bindAddress(int axisIdx, uint32_t address);
    // map axisIdx  to address, then map  axisIdx+num-1 to address+num-1
    /*
     * @brief Bind a range of axes to a range of addresses.
     * @param axisIdx The starting index of the axis to bind.
     * @param address The starting address to bind the axis to.
     * @param num The number of consecutive axes and addresses to bind.
     * @return 0 on success, or an error code on failure.
     */
    int bindAddressAutoBatch(int axisIdx, uint32_t address, uint32_t num);
    /*
     * @brief Bind a list of axes to a list of addresses.
     * @param[in] axisCount  length of bindAxisArr
     * @param[in] bindAxisArr An array of axis indices to bind.
     * @param[in] servo_num The number of available servo.
     * @return 0 on success, or an error code on failure.
     */
    int binAddressByAxisIdxArr(int axisCount, const int* bindAxisArr, int servo_num);
    int checkAddressUniqueness() const;
    void printAddress() const;

    void resize(int servo_num);
    void reserve(int servo_num);
    void reset_runtime();
    void postRunCycle() override;

    std::vector<AxisCiaInput> inpArr;
    std::vector<AxisCiaOutput> oupArr;
};

template <class T, bool isVirtual = false>
class AxisFbGraphTmpl : public AxPouInout {
   public:
    explicit AxisFbGraphTmpl() : AxPouInout(), fbGrp(0) {
        fbGrp.setCtx(&this->ctx());
        fbGrp.axisMotbuf.reserve(MC_MOTION_MAX_AXES);
        this->bind(0, &fbGrp);
    }
    int configure(const std::string& iniPath = "", const std::string& iniContent = "",
                  unsigned int axNum = 1) {
        int ret = AxPouInout::configure(iniPath, iniContent, axNum);
        if (ret != 0) {
            return ret;
        }
        fbGrp.setCtx(&this->ctx());
        if (fbGrp.resize(static_cast<int>(axis_num())) != 0) {
            return -1;
        }
        if (isVirtual) {
            this->_setAllAxisVirtual();
        }
        return 0;
    }
    void _init(int) {
        if (isVirtual) this->_setAllAxisVirtual();
    }
    void reset_runtime() {
        fbGrp.reset_runtime();
        AxPouInout::reset_runtime();
    }
    void preRunCycle() override { _preRunCycle(); }
    void _preRunCycle() {
        if (getIndex() > 2) {
            // std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
        fbGrp.preRunCycle();
        this->skipFlag = fbGrp.skipFlag;
        _exitFlag = fbGrp.exitFlag;
    }

    T fbGrp;
    // std::chrono::steady_clock::time_point last_time;
};

using AxisFbGraph = AxisFbGraphTmpl<FbMoveWrap>;
using AxisFbGraphSimu = AxisFbGraphTmpl<FbMoveWrap, true>;
}  // namespace zmc
#endif
