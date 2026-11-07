#include "AxPou.hpp"

#include "AxPouImpl.hpp"
#include "AxisFbGraph.hpp"
#include "fbt_axis_base.h"
#include "fbt_axis_opaque.h"
#include "pou/AxisManager.h"

namespace zmc {

void inline set_pou_cfg(const AxisCommonConfig& cfg, AxPou& pou) {
    if (cfg.config_mask & AX_COM_MASK_COUNT_TO_EXIT) pou.count_to_exit = cfg.count_to_exit;
    if (cfg.config_mask & AX_COM_MASK_VERBOSE_FLAG) pou.verbose_flag = cfg.verbose_flag;
    if (cfg.config_mask & AX_COM_MASK_VERBOSE_MODULO) pou.verbose_modulo = cfg.verbose_modulo;
}

AxPou::AxPou() {
    pImpl = new zmc::AxPouImpl();
    axisRefArr = static_cast<zmc::AxPouImpl*>(pImpl)->getAxisRefs();
}

AxPou::AxPou(const std::string& iniPath) {
    pImpl = new zmc::AxPouImpl(iniPath);
    axisRefArr = static_cast<zmc::AxPouImpl*>(pImpl)->getAxisRefs();
    const auto& cfg = static_cast<zmc::AxPouImpl*>(pImpl)->cfg_;
    set_pou_cfg(cfg, *this);
}
AxPou::AxPou(const std::string& iniPath, const std::string& iniContent, unsigned int axNum) {
    pImpl = new zmc::AxPouImpl(iniPath, iniContent, axNum);
    const auto& cfg = static_cast<zmc::AxPouImpl*>(pImpl)->cfg_;
    axisRefArr = static_cast<zmc::AxPouImpl*>(pImpl)->getAxisRefs();
    set_pou_cfg(cfg, *this);
}
AxPou::AxPou(int axNum) {
    pImpl = new zmc::AxPouImpl(axNum);
    axisRefArr = static_cast<zmc::AxPouImpl*>(pImpl)->getAxisRefs();
}
AxPou::~AxPou() {
    delete static_cast<zmc::AxPouImpl*>(pImpl);
    pImpl = nullptr;
}

int AxPou::configure(const std::string& iniPath, const std::string& iniContent,
                     unsigned int axNum) {
    int ret = static_cast<zmc::AxPouImpl*>(pImpl)->configure(iniPath, iniContent, axNum);
    if (ret != 0) {
        return ret;
    }
    axisRefArr = static_cast<zmc::AxPouImpl*>(pImpl)->getAxisRefs();
    const auto& cfg = static_cast<zmc::AxPouImpl*>(pImpl)->cfg_;
    set_pou_cfg(cfg, *this);
    return 0;
}

uint32_t AxPou::axis_num() const { return static_cast<zmc::AxPouImpl*>(pImpl)->axis_num(); }

uint32_t AxPou::axis_cap_num() const {
    return static_cast<zmc::AxPouImpl*>(pImpl)->axis_cap_num();
}

uint32_t AxPou::enabled_axis_num() const {
    return static_cast<zmc::AxPouImpl*>(pImpl)->enabled_axis_num();
}

int AxPou::set_enabled_axis_num(uint32_t n) {
    return static_cast<zmc::AxPouImpl*>(pImpl)->set_enabled_axis_num(n);
}

void AxPou::reset_runtime() {
    static_cast<zmc::AxPouImpl*>(pImpl)->reset_runtime();
    idx = 0;
    _exitFlag = false;
    verbose_flag = 0;
    verbose_modulo = 1;
    count_to_exit = 0;
    skipFlag = false;
}

void AxPou::print(int verbose) const { static_cast<zmc::AxPouImpl*>(pImpl)->print(verbose); }
void AxPou::preRun() {
    printAxisHeader(verbose_flag);
}
void AxPou::postRunCycle() {
    axRunCycle();
    printAxis();
}

void AxPou::printAxis() const {
    if (this->verbose_flag != 0 && (this->idx % this->verbose_modulo == 0)) {
        static_cast<zmc::AxPouImpl*>(pImpl)->print(this->verbose_flag);
    }
}

void AxPou::axRunCycle() { static_cast<zmc::AxPouImpl*>(pImpl)->runCycle(); }

AxisCiaConfig& AxPou::get_config_ref(int i) {
    return static_cast<zmc::AxisOpaque*>(axisRefArr[i])->get_config_ref();
}
// void AxPouEnhanced::_init(){
//     for (int i = 0; i < axisRefArr.size(); i++){
//         // setAxisSimulated(i, true);
//     }
// }
void AxPou::_setAllAxisVirtual() {
    for (uint32_t i = 0; i < axis_num(); ++i) {
        setAxisSimulated(static_cast<int>(i), true);
    }
}
void AxPou::setAxisSimulated(int idx, bool bSimulated) {
    static_cast<zmc::AxisOpaque*>(axisRefArr[idx])->get_config_ref().is_axis_virtual = bSimulated;
    static_cast<zmc::AxisOpaque*>(axisRefArr[idx])->reload_config();
}

void AxPou::reload_config(int i) { static_cast<zmc::AxisOpaque*>(axisRefArr[i])->reload_config(); }
void AxPou::set_cycle_time(int us) {
    for (uint32_t i = 0; i < axis_num(); ++i) {
        auto axis = (AxisImplBase*)axisRefArr[i];
        axis->cycleTime = us * 1e-6;
        static_cast<zmc::AxisOpaque*>(axisRefArr[i])->get_config_ref().cycle_time = us;
    }
}
int AxPou::get_cycle_time() const {
    if (axisRefArr.size()) {
        auto axis = (AxisImplBase*)axisRefArr[0];
        return axis->cycleTime * 1e6;
    }
    return 0;
}
int AxPou::get_verbose_flag() const { return verbose_flag; }
void AxPou::set_cycles(int cycles) { count_to_exit = cycles; }
void AxPou::set_verbose_flag(int flag) { verbose_flag = flag; }
uint32_t AxPouTaskBinderAuto::registerAllAxis(const std::string& namePrefix) {
    uint32_t registeredCount = 0;

    for (uint32_t i = 0; i < axis_num(); ++i) {
        std::string axisName = namePrefix + std::to_string(i);
        AXIS_REF axis = axisRefArr[i];

        if (SgAxisArr::addAxis(axis, axisName.c_str())) {
            registeredCount++;
        }
    }

    return registeredCount;
}

// AxPouTaskBinderAuto implementation
// AxPouTaskBinderAuto::AxPouTaskBinderAuto(const std::string& iniPath, const std::string&
// namePrefix)
//     : AxPouTaskBinder(iniPath), axisNamePrefix(namePrefix) {
//     registerAllAxis(axisNamePrefix);
// }

AxPouTaskBinderAuto::AxPouTaskBinderAuto() : axisNamePrefix("axis_") {}

AxPouTaskBinderAuto::AxPouTaskBinderAuto(const std::string& iniPath, const std::string& iniContent,
                                         unsigned int axNum, const std::string& namePrefix)
    : AxPouTaskBinder(iniPath, iniContent, axNum), axisNamePrefix(namePrefix) {
    registerAllAxis(axisNamePrefix);
}

int AxPouTaskBinderAuto::configure(const std::string& iniPath, const std::string& iniContent,
                                   unsigned int axNum, const std::string& namePrefix) {
    axisNamePrefix = namePrefix;
    int ret = AxPou::configure(iniPath, iniContent, axNum);
    if (ret != 0) {
        return ret;
    }
    SgAxisArr::clear();
    registerAllAxis(axisNamePrefix);
    return 0;
}

void AxPouTaskBinderAuto::reset_runtime() {
    AxPou::reset_runtime();
    // SgAxisArr::reset_runtime();
}

AxPouTaskBinderAuto::AxPouTaskBinderAuto(int axNum, const std::string& namePrefix)
    : AxPouTaskBinder(axNum), axisNamePrefix(namePrefix) {
    registerAllAxis(axisNamePrefix);
}

AxPouTaskBinderAuto::~AxPouTaskBinderAuto() {
    SgAxisArr::clear();
    // ~AxPouTaskBinder(this);
}
AXIS_REF AxPouTaskBinderAuto::getAxisByIndex(uint32_t index) {
    return (AXIS_REF)SgAxisArr::getAxisByIndex(index);
}
int AxPouTaskBinderAuto::getAxisCount() { return SgAxisArr::getSize(); }

int AxPouTaskBinderAuto::applyIoBinding() {
    // Rebuild every configured PDO and virtual alias as one registry.
    IoManager& io = ctx_.io();
    const IoBindingConfig& binding = static_cast<AxPouImpl*>(pImpl)->io_binding_cfg_;
    io.clear_aliases();
    int ret = 0;
    for (int i = 0; i < binding.di_count; ++i) {
        const IoBindingEntry& entry = binding.di_entries[i];
        if (entry.source_kind == IO_SOURCE_KIND_NOP) continue;
        if (entry.source_kind == IO_SOURCE_KIND_VIRTUAL) {
            ret = io.register_virtual_di(entry.io_number);
        } else if (entry.source_kind == IO_SOURCE_KIND_PDO) {
            ret = entry.mask == 0u ? io.register_di_alias(entry.io_number, entry.key)
                                  : io.register_di_alias(entry.io_number, entry.key, entry.mask);
        } else {
            ret = ZMC_ERR_INI_IO_BINDING_CONFIG;
        }
        if (ret != 0) {
            io.clear_aliases();
            return ret;
        }
    }
    for (int i = 0; i < binding.do_count; ++i) {
        const IoBindingEntry& entry = binding.do_entries[i];
        if (entry.source_kind == IO_SOURCE_KIND_NOP) continue;
        if (entry.source_kind == IO_SOURCE_KIND_VIRTUAL) {
            ret = io.register_virtual_do(entry.io_number);
        } else if (entry.source_kind == IO_SOURCE_KIND_PDO) {
            ret = entry.mask == 0u ? io.register_do_alias(entry.io_number, entry.key)
                                  : io.register_do_alias(entry.io_number, entry.key, entry.mask);
        } else {
            ret = ZMC_ERR_INI_IO_BINDING_CONFIG;
        }
        // Seed configured DO values after alias registration for PDO and virtual outputs.
        if (ret == 0 && entry.has_initial_value) {
            ret = io.write_do(entry.io_number, entry.initial_value != 0u);
        }
        if (ret != 0) {
            io.clear_aliases();
            return ret;
        }
    }
    for (int i = 0; i < binding.ai_count; ++i) {
        const IoBindingEntry& entry = binding.ai_entries[i];
        if (entry.source_kind == IO_SOURCE_KIND_NOP) continue;
        if (entry.source_kind == IO_SOURCE_KIND_VIRTUAL) {
            ret = io.register_virtual_ai(entry.io_number);
        } else if (entry.source_kind == IO_SOURCE_KIND_PDO) {
            ret = io.register_ai_alias(entry.io_number, entry.key);
        } else {
            ret = ZMC_ERR_INI_IO_BINDING_CONFIG;
        }
        if (ret != 0) {
            io.clear_aliases();
            return ret;
        }
    }
    for (int i = 0; i < binding.ao_count; ++i) {
        const IoBindingEntry& entry = binding.ao_entries[i];
        if (entry.source_kind == IO_SOURCE_KIND_NOP) continue;
        if (entry.source_kind == IO_SOURCE_KIND_VIRTUAL) {
            ret = io.register_virtual_ao(entry.io_number);
        } else if (entry.source_kind == IO_SOURCE_KIND_PDO) {
            ret = io.register_ao_alias(entry.io_number, entry.key);
        } else {
            ret = ZMC_ERR_INI_IO_BINDING_CONFIG;
        }
        // Seed configured AO values after alias registration for PDO and virtual outputs.
        if (ret == 0 && entry.has_initial_value) {
            ret = io.write_ao(entry.io_number, entry.initial_value);
        }
        if (ret != 0) {
            io.clear_aliases();
            return ret;
        }
    }
    return 0;
}

void AxPouInout::reserve(int servo_num) {
    inpArr.reserve(servo_num);
    oupArr.reserve(servo_num);
}

void AxPouInout::resize(int servo_num) {
    inpArr.resize(servo_num);
    oupArr.resize(servo_num);
    for (int i = 0; i < servo_num; i++) {
        inpArr[i] = {0};
        oupArr[i] = {0};
    }
}

void AxPouInout::reset_runtime() { AxPouTaskBinderAuto::reset_runtime(); }

int AxPouInout::configure(const std::string& iniPath, const std::string& iniContent,
                          unsigned int axNum) {
    int ret = AxPouTaskBinderAuto::configure(iniPath, iniContent, axNum);
    if (ret != 0) return ret;

    resize(static_cast<int>(axis_num()));
    for (uint32_t i = 0; i < axis_num(); ++i) {
        axisRef(static_cast<int>(i))->get_config_ref().debug.axisAddr = 0;
    }

    const AxisBindingConfig& binding = static_cast<AxPouImpl*>(pImpl)->binding_cfg_;
    if (!binding.section_present) {
        return bindAddressAutoBatch(0, 1, axis_num());
    }
    for (int i = 0; i < binding.binding_count; ++i) {
        const AxisBindingEntry& entry = binding.entries[i];
        ret = bindAddress(entry.axis_index, entry.address);
        if (ret != 0) return ret;
    }
    return checkAddressUniqueness();
}

int AxPouInout::getAddress(int axisIdx) const {
    if (axisIdx < 0 || axisIdx >= axis_num()) return -1;
    return axisRef(axisIdx)->get_config_ref().debug.axisAddr;
}

int AxPouInout::bindAddress(int axisIdx, uint32_t address) {
    if (address == 0) return ZMC_ERR_AX_BIND_OVER_RANGE;
    // int servoIdx = address -1 ;
    AXIS_REF axis = axisRef(axisIdx);
    if (!axis) return ZMC_ERR_AX_BIND_ADDR_FAILED;
    axis->get_config_ref().debug.axisAddr = address;
    // TODO add address unique check, if address is already used, return -1
    return 0;
}

int AxPouInout::bindAddressAutoBatch(int axisIdx, uint32_t address, uint32_t num) {
    int ret = 0;
    for (int i = 0; i < num; i++) {
        ret = bindAddress(axisIdx + i, address + i);
        if (ret != 0) return ret;
        if (getAddress(axisIdx + i) > num) {  // over servo_num
            return ZMC_ERR_AX_BIND_ADDR_FAILED;
        }
    }
    return checkAddressUniqueness();
}

int AxPouInout::binAddressByAxisIdxArr(int servoCount, const int* bindAxisArr, int servo_num) {
    if (servoCount > servo_num) {
        printf("cant scan %d servos to bind\n", servoCount - servo_num);
        return ZMC_ERR_AX_BIND_OVER_RANGE;
    }
    int ret = 0;
    for (int i = 0; i < servoCount; i++) {
        int axisIdx = bindAxisArr[i];
        if (axisIdx < 0 || axisIdx >= axis_num()) {
            return ZMC_ERR_AX_BIND_OVER_RANGE;
        };
        ret = bindAddress(axisIdx, i + 1);
        if (ret != 0) return ret;
        if (getAddress(axisIdx) > servo_num) {  // over servo_num
            return ZMC_ERR_AX_BIND_OVER_RANGE;
        }
    }
    // Prepare a bitmask to check uniqueness by address
    return checkAddressUniqueness();
}

int AxPouInout::checkAddressUniqueness() const {
    uint64_t addrMask = 0;
    for (int i = 0; i < axis_num(); i++) {
        int addr = getAddress(i);
        if (addr > 0 && addr <= MAX_NUM_SERVOS) {
            if (addrMask & (1ULL << addr)) {
                return ZMC_ERR_AX_BIND_OVERLAP;
            }
            addrMask |= (1ULL << addr);
        }
    }
    return 0;
}

void AxPouInout::printAddress() const {
    int axNum = axis_num();
    for (int i = 0; i < axNum; i++) {
        int addr = getAddress(i);
        printf("axis[%d].address=[%d]\n", i, addr);
    }
}

void AxPouInout::postRunCycle() {
    for (int i = 0; i < inpArr.size(); i++) {
        AXIS_REF axis = axisRef(i);
        if (axis) {
            ((AxisCia402*)axis)->set_input(&inpArr[i]);
        }
    }
    this->axRunCycle();
    for (int i = 0; i < oupArr.size(); i++) {
        AXIS_REF axis = axisRef(i);
        if (axis) {
            ((AxisCia402*)axis)->get_output(&oupArr[i]);
        }
    }
    this->printAxis();
}

}  // namespace zmc
