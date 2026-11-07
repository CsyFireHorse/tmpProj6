#ifndef _ECAT_MGR_H_
#define _ECAT_MGR_H_

#include "EcatBase.h"
#include "pair_manager.hpp"

namespace zmc{

class EcatManagerSg:public EcatPairSg<EcatBase>{
public:
    inline static int start(uint16_t idx){
        return Instance().get_ptr(idx)->start();
    }
    inline static int stop(uint16_t idx){
        return Instance().get_ptr(idx)->stop();
    }
    // static int use_txrxpdo(uint16_t idx);
    // inline static uint16_t get_rxpdo_size(uint16_t idx){
    //     return EcatManagerSg::get_ptr(idx)->get_rxpdo_size();
    // }
};

}

#endif