#ifndef _AXIS_INI_LOADER_HPP_
#define _AXIS_INI_LOADER_HPP_

#include <stdint.h>

#include <memory>

#include "AxisIniLoader.h"
#include "common/zmc/zmc_predef.h"
#include "io/IoManager.hpp"
#include "zmc_cam_table.h"

enum { AXIS_BINDING_MAX_COUNT = MC_MOTION_MAX_AXES };
enum { IO_BINDING_ALIAS_CAPACITY = MC_IO_MAX_COUNT };
enum { INI_CAM_XYVA_MAX_COUNT = 512 };
enum { INI_CAM_TAPPET_MAX_COUNT = MAX_NUM_TAPPETS };

// Own the storage referenced by the inherited MC_CAM_TABLE fields.
class IniCamTable : public zmc::CamTable {
   public:
    IniCamTable() {}
    IniCamTable(const IniCamTable&) = delete;
    IniCamTable& operator=(const IniCamTable&) = delete;

    int table_index = 0;
    int ver_major = 1;
    int ver_minor = 0;
    int ver_patch = 0;

    CamXyva xyva[INI_CAM_XYVA_MAX_COUNT] = {};
    Tappet tappets[INI_CAM_TAPPET_MAX_COUNT] = {};
};

struct AxisBindingEntry {
    char name[MAX_AXIS_NAME_LEN];
    int axis_index;
    uint32_t address;
};

struct AxisBindingConfig {
    int section_present;
    int binding_count;
    AxisBindingEntry entries[AXIS_BINDING_MAX_COUNT];
};

struct IoBindingEntry {
    uint32_t io_number;
    // Selects PDO mapping or the independent Virtual IO Bank.
    zmc::IoSourceKind source_kind;
    zmc::IoObjectKey key;
    uint32_t mask;  // Zero means no digital bit selector is configured.
    uint32_t initial_value;  // Raw DO/AO value configured by INI.
    bool has_initial_value;  // Distinguishes omitted values from zero.
};

struct IoBindingConfig {
    int section_present;
    int di_count;
    int do_count;
    int ai_count;
    int ao_count;
    IoBindingEntry di_entries[IO_BINDING_ALIAS_CAPACITY];
    IoBindingEntry do_entries[IO_BINDING_ALIAS_CAPACITY];
    IoBindingEntry ai_entries[IO_BINDING_ALIAS_CAPACITY];
    IoBindingEntry ao_entries[IO_BINDING_ALIAS_CAPACITY];
};

// Retain one parsed INI source for all configuration reads.
class AxisIniLoader {
   public:
    AxisIniLoader(const char* source, bool is_content);
    ~AxisIniLoader();

    AxisIniLoader(const AxisIniLoader&) = delete;
    AxisIniLoader& operator=(const AxisIniLoader&) = delete;

    // Load [base] from the retained reader.
    int load_base(AxisCommonConfig* cfg) const;

    // Load one axis section from the retained reader.
    int load_axis(AxisCiaConfig* cfg, const char* section) const;

    // Load one indexed axis, including its optional JSON base configuration.
    int load_axis_by_index(int index, const AxisCommonConfig* common_cfg,
                           AxisCiaConfig* cfg) const;

    // Load [axis_binding] from the retained reader.
    int load_binding(AxisBindingConfig* cfg) const;

    // Load [io_binding] from the retained reader.
    int load_io_binding(IoBindingConfig* cfg) const;

    // Load the complete INI cam selected by cam->table_index.
    int load_cam_table(IniCamTable* cam) const;

   private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif
