#ifndef _ZMC_AX_LOG_H_
#define _ZMC_AX_LOG_H_

#include <map>
#include <memory>
#include <string>

#include "axLogBase.hpp"
#include "stdio.h"
#include "AxVerboseFlag.h"

namespace zmc {

class BufferAxLogger : public AxLogger {
   public:
    BufferAxLogger() {}
    virtual void write(const AxisFrame& posFrm) override {
        // TODO
        //_pLog->push(posFrm); }
    } 
   private:
    void* _pLog = new AxisFrame[1024];
};


class ConsoleAxLogger : public AxLogger {
   public:
    void write(const AxisFrame& posFrm) override {
        if (!hasWriteHeader){
            printf("%s\n", LOGGER_HEADER_LAG_FULL);
            hasWriteHeader = true;
        }
        printf("%d,%d,%d,%f,%f,%f,%f,%f,%f,%f\n", posFrm.cnt, posFrm.axis_id, posFrm.time, posFrm.axis_state_,
               posFrm.posActual, posFrm.velActual, posFrm.accActual, posFrm.pos, posFrm.vel, posFrm.acc);
    }
    void log(int level, const char* fmt) override{
        if (level) {
            printf(fmt);
        }
    }
    bool hasWriteHeader = false;
};


class FileAxLogger : public AxLogger {
   public:
    FileAxLogger(const std::string& filename) : _filename(filename) {
        _file = fopen(filename.c_str(), "a+");
        if (_file) {
            //  csv format
            fprintf(_file, "%s\n", LOGGER_HEADER_LAG_FULL);
        }
    }
    
    ~FileAxLogger() {
        if (_file) {
            fclose(_file);
            _file = nullptr;
        }
    }

    virtual void write(const AxisFrame& posFrm) override {
        if (_file) {
            fprintf(_file, "%d,%d,%f,%d,%f,%f,%f,%f,%f,%f\n", 
                   posFrm.cnt, posFrm.axis_id, posFrm.time, posFrm.axis_state_,
                   posFrm.posActual, posFrm.velActual, posFrm.accActual, posFrm.pos, posFrm.vel, posFrm.acc);
            fflush(_file);
        }
    }
    void log(int level, const char* fmt) override{
        if (_file && level) {
            fprintf(_file, fmt);
            fflush(_file);
        }
    }
   private:
    std::string _filename;
    FILE* _file = nullptr;
};
}  // namespace zmc
#endif