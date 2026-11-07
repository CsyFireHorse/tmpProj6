#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "AxisCiaConfig.hpp"
#include "AxisRef.h"
#include "axis/fbt_axis_morph.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "misc/cpp14_backport.hpp"
#include "pou/AxisIniLoader.hpp"

namespace zmc {

class AxPouImpl {
   public:
    // Logical axis capacity includes real and virtual axes.
    static const unsigned kAxisCapNum = MC_MOTION_MAX_AXES;

    AxPouImpl() {
        axisComcfg_default_init(&cfg_);
        binding_cfg_ = AxisBindingConfig();
        io_binding_cfg_ = IoBindingConfig();
        axes_.reserve(kAxisCapNum);
        for (unsigned i = 0; i < kAxisCapNum; ++i) {
            axes_.push_back(make_unique<AxisMorph>());
        }
        axis_num_ = 0;
        enabled_axis_num_ = 0;
    }

    AxPouImpl(const std::string& iniFile, const std::string& iniContent, unsigned int axNumMin = 1)
        : AxPouImpl() {
        configure(iniFile, iniContent, axNumMin);
    }

    AxPouImpl(const std::string& iniFile) : AxPouImpl() { configure(iniFile, "", 0); }

    AxPouImpl(int axNum) : AxPouImpl() {
        configureCount(static_cast<unsigned>(axNum > 0 ? axNum : 0));
    }

    uint32_t axis_cap_num() const { return axes_.capacity(); }
    uint32_t axis_num() const { return axis_num_; }
    uint32_t enabled_axis_num() const { return enabled_axis_num_; }

    void reset_runtime() {
        for (auto& axis : axes_) {
            axis->reset_runtime();
        }
        axis_num_ = 0;
        enabled_axis_num_ = 0;
        axisComcfg_default_init(&cfg_);
        binding_cfg_ = AxisBindingConfig();
        io_binding_cfg_ = IoBindingConfig();
    }

    int configure(const std::string& iniFile, const std::string& iniContent,
                  unsigned int axNumMin = 1) {
        axis_num_ = 0;
        enabled_axis_num_ = 0;
        axisComcfg_default_init(&cfg_);
        binding_cfg_ = AxisBindingConfig();
        io_binding_cfg_ = IoBindingConfig();

        unsigned int fileAxisNum = 0;
        unsigned int contentAxisNum = 0;
        std::unique_ptr<AxisIniLoader> file_loader;
        std::unique_ptr<AxisIniLoader> content_loader;
        // Preserve loader error categories for the configure caller. AI modification signature: GPT-5 Codex

        if (iniFile.size()) {
            file_loader.reset(new AxisIniLoader(iniFile.c_str(), false));
            int ret = file_loader->load_base(&cfg_);
            if (ret) {
                printf("load %s failed,err = %d\n", iniFile.c_str(), ret);
                return ret;
            }
            ret = file_loader->load_binding(&binding_cfg_);
            if (ret) {
                printf("load [axis_binding] failed,err = %d\n", ret);
                return ret;
            }
            ret = file_loader->load_io_binding(&io_binding_cfg_);
            if (ret) {
                printf("load [io_binding] failed,err = %d\n", ret);
                return ret;
            }
            fileAxisNum = cfg_.axis_num;
        }
        if (iniContent.size()) {
            content_loader.reset(new AxisIniLoader(iniContent.c_str(), true));
            AxisCommonConfig contentCfg;
            axisComcfg_default_init(&contentCfg);
            int ret = content_loader->load_base(&contentCfg);
            if (ret) {
                printf("load ini content failed,err = %d\n", ret);
                return ret;
            }
            AxisBindingConfig content_binding;
            content_binding = AxisBindingConfig();
            ret = content_loader->load_binding(&content_binding);
            if (ret) {
                printf("load content [axis_binding] failed,err = %d\n", ret);
                return ret;
            }
            if (content_binding.section_present) binding_cfg_ = content_binding;
            IoBindingConfig content_io_binding;
            content_io_binding = IoBindingConfig();
            ret = content_loader->load_io_binding(&content_io_binding);
            if (ret) {
                printf("load content [io_binding] failed,err = %d\n", ret);
                return ret;
            }
            if (content_io_binding.section_present) io_binding_cfg_ = content_io_binding;
            contentAxisNum = contentCfg.axis_num;
            axisComcfg_merge_by_mask(&cfg_, &contentCfg);
        }

        unsigned int computed = cfg_.axis_num;
        if (computed < axNumMin) {
            computed = axNumMin;
        }
        if (computed > kAxisCapNum) {
            printf("axis_num %u exceeds cap %u\n", computed, kAxisCapNum);
            return ZMC_ERR_HOST_OVER_RANGE;
        }

        axis_num_ = computed;
        cfg_.axis_num = axis_num_;
        // printf("axis_num = %u\n", axis_num_);

        if (iniFile.size()) {
            for (unsigned i = 0; i < fileAxisNum && i < axis_num_; ++i) {
                AxisCiaConfig& axisCfg = axes_[i]->get_config_ref();
                file_loader->load_axis_by_index(static_cast<int>(i), &cfg_, &axisCfg);
                axes_[i]->reload_config();
            }
        }
        if (iniContent.size()) {
            for (unsigned i = 0; i < contentAxisNum && i < axis_num_; ++i) {
                AxisCiaConfig& axisCfg = axes_[i]->get_config_ref();
                content_loader->load_axis(&axisCfg, ("axis_" + std::to_string(i)).c_str());
                axes_[i]->reload_config();
            }
        }

        enabled_axis_num_ = axis_num_;
        return 0;
    }

    int configureCount(unsigned int axNum) {
        axis_num_ = 0;
        enabled_axis_num_ = 0;
        if (axNum > kAxisCapNum) {
            printf("axis_num %u exceeds cap %u\n", axNum, kAxisCapNum);
            return ZMC_ERR_HOST_OVER_RANGE;
        }
        axisComcfg_default_init(&cfg_);
        io_binding_cfg_ = IoBindingConfig();
        axis_num_ = axNum;
        cfg_.axis_num = axis_num_;
        enabled_axis_num_ = axis_num_;
        return 0;
    }

    int set_enabled_axis_num(uint32_t n) {
        if (n > axis_num_) {
            return ZMC_ERR_HOST_INI_AXIS_NUM_OVER;
        }
        enabled_axis_num_ = n;
        return 0;
    }

    std::vector<AXIS_REF> getAxisRefs() const {
        std::vector<AXIS_REF> refs;
        refs.reserve(axis_num_);
        for (unsigned i = 0; i < axis_num_; ++i) {
            refs.push_back(axes_[i].get());
        }
        return refs;
    }

    void runCycle() {
        for (unsigned i = 0; i < enabled_axis_num_; ++i) {
            axes_[i]->runCycle();
        }
    }

    void print(int verbose) const {
        for (unsigned i = 0; i < axis_num_; ++i) {
            axes_[i]->print(verbose);
        }
    }

   private:
    std::vector<std::unique_ptr<AxisMorph>> axes_;
    uint32_t axis_num_ = 0;
    uint32_t enabled_axis_num_ = 0;

   public:
    AxisCommonConfig cfg_;
    AxisBindingConfig binding_cfg_;
    IoBindingConfig io_binding_cfg_;
};

}  // namespace zmc
