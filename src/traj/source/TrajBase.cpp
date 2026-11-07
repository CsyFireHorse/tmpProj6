/*
 * @Author: ChenShengyong
 * @Date: 2026-08-19 17:46:00
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-08-27 16:40:00
 */

#include "TrajBase.h"
#include "string.h"

namespace zmc {

TrajBase3d::TrajBase3d(uint8_t axisNum) {
    _axisNum = 1;
    _mode = TRAJ_TYPE_NOP;
    _sub_mode = 0;
    _pad = 0;
    _distance = 0;
    for (int i = 0; i < 3; i++) {
        _beginPos[i] = 0;
        _auxPos[i] = 0;
        _cmdPos[i] = 0;
    }
    _pAlloc = nullptr;
    _cache = nullptr;
    _cacheBytes = 0;
    init_axis_num(axisNum);
}

void TrajBase3d::set_allocator(PmAllocator* pAlloc) {
    if (pAlloc == _pAlloc) {
        return;
    }
    // Return the live block to the allocator that issued it before rebinding.
    auto cache_bak = _cacheBytes;
    _free_cache();
    _pAlloc = pAlloc;
    // High-axis layouts need line cache immediately after the switch.
    if (cache_bak) {
        if (allocate(cache_bak) != 0) {
            _axisNum = 0;
            _mode = TRAJ_TYPE_NOP;
        }
    }
}

void TrajBase3d::set_default_allocator() { set_allocator(_get_default_allocator()); }

int TrajBase3d::init_axis_num(uint8_t axisNum) {
    if (axisNum == 0 || axisNum > MC_MOTION_MAX_AXES) {
        _axisNum = 0;
        _mode = TRAJ_TYPE_NOP;
        return ZMC_ERR_TRAJ_INVALID_AXIS_NUM;
    }
    if (axisNum > 3) {
        int ret = allocate(line_cache_bytes(axisNum));
        if (ret != 0) {
            _axisNum = 0;
            _mode = TRAJ_TYPE_NOP;
            return ret;
        }
    }
    _axisNum = axisNum;
    return 0;
}

void TrajBase3d::get_end_pos(double* pos) const {
    const double* beginPos = begin_pos_data();
    const double* cmdPos = cmd_pos_data();
    if (!pos || !beginPos || !cmdPos) return;
    if (is_relative()) {
        for (int i = 0; i < _axisNum; i++) {
            pos[i] = cmdPos[i] + beginPos[i];
        }
    } else {
        for (int i = 0; i < _axisNum; i++) {
            pos[i] = cmdPos[i];
        }
    }
}

int TrajBase3d::allocate(int cacheBytes) {
    if (cacheBytes <= _cacheBytes) return 0;
    auto* p = get_allocator();
    if (!p) return ZMC_ERR_TRAJ_ALLOC_FAILED;
    if (_cache) {
        p->deallocate(_cache, _cacheBytes);
        _cache = nullptr;
        _cacheBytes = 0;
    }
    auto ptr = p->allocate(cacheBytes);
    if (!ptr) return ZMC_ERR_TRAJ_ALLOC_FAILED;
    set_cache_raw(ptr, cacheBytes);
    return 0;
}

void TrajBase3d::_free_cache() {
    if (!_cache) return;
    auto* p = get_allocator();
    if (p) {
        p->deallocate(_cache, _cacheBytes);
    }
    _cache = nullptr;
    _cacheBytes = 0;
}

void TrajBase3d::_copy_from(const TrajBase3d& other) {
    _axisNum = other._axisNum;
    _mode = other._mode;
    _sub_mode = other._sub_mode;
    _pad = other._pad;
    _distance = other._distance;
    for (int i = 0; i < 3; i++) {
        _beginPos[i] = other._beginPos[i];
        _auxPos[i] = other._auxPos[i];
        _cmdPos[i] = other._cmdPos[i];
    }
    _pAlloc = other._pAlloc;
    _cache = nullptr;
    _cacheBytes = 0;
    if (!other._cache || other._cacheBytes <= 0) {
        return;
    }
    PmAllocator* p = get_allocator();
    if (!p) {
        return;
    }
    void* ptr = p->allocate(other._cacheBytes);
    if (!ptr) {
        return;
    }
    memcpy(ptr, other._cache, static_cast<size_t>(other._cacheBytes));
    set_cache_raw(ptr, other._cacheBytes);
}

TrajBase3d::TrajBase3d(const TrajBase3d& other) { _copy_from(other); }

TrajBase3d& TrajBase3d::operator=(const TrajBase3d& other) {
    if (this == &other) {
        return *this;
    }
    _free_cache();
    _copy_from(other);
    return *this;
}

}  // namespace zmc
