#ifndef _ECAT_WRAP_H_
#define _ECAT_WRAP_H_

#include "EcatBase.h"
#include "ecat_mgr.h"
#include "mhelper/pdo_servo_addr.h"

namespace zmc {

typedef int (*PdoHandler)(void*, const void*, void*);

// Master application.
class EcatServo : public EcatBase {
   public:
    EcatServo(const char* eth);
    EcatServo(const std::string& eth);
    ~EcatServo();
    int load_eni_json(const char* eni);
    int _bindDefaultHandler();
    int bindPdoHandler(PdoHandler cb, void* obj) {
        _fPdoHandler = cb;
        _pObj = obj;
        return _bindDefaultHandler();
    }
    int config_servo_op_mode(uint32_t op_mode);
    int servo_size() const;
    PdoHandler _fPdoHandler;
    void* _pObj;
    pdo_addr pdoAddr;
};

class EcatWrapMgrSg : public EcatPairSg<EcatServo> {
   public:
    inline static int start(uint16_t idx) { return Instance().get_ptr(idx)->start(); }
    inline static int stop(uint16_t idx) { return Instance().get_ptr(idx)->stop(); }

   private:
    // ethercat_env* _m_ecat_env;
};

}  // namespace zmc
#endif
