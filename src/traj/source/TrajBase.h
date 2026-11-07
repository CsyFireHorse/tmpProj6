/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 15:19:03
 */
#ifndef _TRAJ_BASE_H_
#define _TRAJ_BASE_H_
#include "math.h"
#include "stdint.h"

#include "TrajEnumDef.h"
#include "common/misc/ZmcAllocator.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
namespace zmc {


typedef enum _TrajType_t {
    TRAJ_TYPE_NOP,
    TRAJ_TYPE_SG_ABS,
    TRAJ_TYPE_SG_REL,
    TRAJ_TYPE_LINE_ABS,
    TRAJ_TYPE_LINE_REL,
    TRAJ_TYPE_CIRC_ABS,
    TRAJ_TYPE_CIRC_REL,
}TrajType;

class TrajBase {
   public:
    uint8_t get_axis_num() const;
    uint8_t get_traj_type() const;

    void set_begin_pos(const double *pos);
    int build();
    void get_begin_pos(double *pos) const;
    void get_end_pos(double *) const;
    double get_plan() const;
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;
    void get_slice3(double d, double v, double a, double *pos, double *difPos, double *dif2Pos) const;
};

class TrajBase3d : public TrajBase {
   public:
    TrajBase3d(uint8_t axisNum = 1);
    TrajBase3d(const TrajBase3d& other);
    TrajBase3d& operator=(const TrajBase3d& other);
    ~TrajBase3d() { _free_cache(); }
    int init_axis_num(uint8_t axisNum);
    inline uint8_t get_axis_num() const { return _axisNum; };
    inline uint8_t get_traj_type() const { return _mode; };

    // Rebind allocator; release any live cache with the previous owner first.
    void set_allocator(PmAllocator* pAlloc);
    void set_default_allocator();
    inline PmAllocator* get_allocator() const { return _pAlloc ? _pAlloc : _get_default_allocator(); }
    inline PmAllocator* get_allocator_raw() const { return _pAlloc; }

    inline void* cache_raw() const { return _cache; }
    inline int cache_bytes() const { return _cacheBytes; }
    inline void set_cache_raw(void* cache, int cacheBytes) {
        _cache = cache;
        _cacheBytes = cacheBytes;
    }

    void set_sg_param(double pos_or_dist, bool is_relative);
    void set_line_param(const double *pos_or_diff_arr, bool is_relative);
    void set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                        bool is_relative, SMC_CIRC_MODE mode);
    inline void set_begin_pos(const double *pos) {
        double *beginPos = begin_pos_data();
        if (!pos || !beginPos) return;
        for (int i = 0; i < _axisNum; i++) {
            beginPos[i] = pos[i];
        }
    };
    int build();
    inline void get_begin_pos(double *pos) const {
        const double *beginPos = begin_pos_data();
        if (!pos || !beginPos) return;
        for (int i = 0; i < _axisNum; i++) {
            pos[i] = beginPos[i];
        }
    };
    // True when the programmed command is a displacement.
    inline bool is_relative() const {
        return _mode == TRAJ_TYPE_SG_REL || _mode == TRAJ_TYPE_LINE_REL ||
               _mode == TRAJ_TYPE_CIRC_REL;
    }
    void get_end_pos(double *pos) const;
    inline double get_plan() const { return _distance; }
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;

   protected:
    // Return the storage size for three N-dimensional line arrays.
    static inline int line_cache_bytes(uint8_t axisNum) {
        return static_cast<int>(3 * axisNum * sizeof(double));
    }

    // Check whether the shared cache can hold all line arrays.
    inline bool has_line_cache() const {
        return _cache != nullptr && _cacheBytes >= line_cache_bytes(_axisNum);
    }

    // Return the active beginning-position storage.
    inline double *begin_pos_data() {
        if (_axisNum <= 3) return _beginPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<double *>(_cache);
    }
    inline const double *begin_pos_data() const {
        if (_axisNum <= 3) return _beginPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<const double *>(_cache);
    }

    // Return the active auxiliary or unit-direction storage.
    inline double *aux_pos_data() {
        if (_axisNum <= 3) return _auxPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<double *>(_cache) + _axisNum;
    }
    inline const double *aux_pos_data() const {
        if (_axisNum <= 3) return _auxPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<const double *>(_cache) + _axisNum;
    }

    // Return the active programmed-command storage.
    inline double *cmd_pos_data() {
        if (_axisNum <= 3) return _cmdPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<double *>(_cache) + 2 * _axisNum;
    }
    inline const double *cmd_pos_data() const {
        if (_axisNum <= 3) return _cmdPos;
        if (!has_line_cache()) return nullptr;
        return static_cast<const double *>(_cache) + 2 * _axisNum;
    }

    static inline PmAllocator* _get_default_allocator() {
        static AllocatorDefault alloc;
        return &alloc;
    }

    int allocate(int cacheBytes);

    inline void* get_cache() const { return _cache; }
    inline int get_cache_size() const { return _cacheBytes; }
    void _free_cache();

    // Copy fields and clone heap cache. Caller must not own a live cache.
    void _copy_from(const TrajBase3d& other);

    uint8_t _axisNum;
    uint8_t _mode;
    uint8_t _sub_mode = 0;
    uint8_t _pad;
    double _distance;
    double _beginPos[3];
    double _auxPos[3];
    double _cmdPos[3];

    PmAllocator* _pAlloc = nullptr;
    void* _cache = nullptr;
    int _cacheBytes = 0;
};

}  // namespace zmc
#endif
