#ifndef __PDO_BFDOAOAI_H_
#define __PDO_BFDOAOAI_H_

#include "stdint.h"

// PDO process image.
#pragma pack(push, 1)

struct PdoRxAi_Eingang{
    union{
        struct{
            uint16_t underange:1;
            uint16_t overrange:1;
            uint16_t limit_1:2;
            uint16_t limit_2:2;
            uint16_t error:1;
            uint16_t _:7;
            uint16_t txPdoState:1;
            uint16_t txPdoToggle:1;
        };
        uint16_t status;
    };
    uint16_t pi_AI;
};

static_assert(sizeof(PdoRxAi_Eingang) == 4, "int size error");
struct PdoRxBfdoaoai{
    struct{
        PdoRxAi_Eingang aiWrap[8];
    }servos_3;
    union{
        struct{
            uint8_t pi_DI_0:1;
            uint8_t pi_DI_1:1;
            uint8_t pi_DI_2:1;
            uint8_t pi_DI_3:1;
            uint8_t pi_DI_4:1;
            uint8_t pi_DI_5:1;
            uint8_t pi_DI_6:1;
            uint8_t pi_DI_7:1;
        };
        uint8_t pi_DI;
    }servos_5;
    inline uint16_t* get_ai_ptr(unsigned int i){
        if (i < 8){
            return &servos_3.aiWrap[i].pi_AI;
        }else{
            return &servos_3.aiWrap[0].pi_AI;
        }
    }
    inline void set_ai(unsigned int i, uint16_t v){
        if (i < 8){
            servos_3.aiWrap[i].pi_AI = v;
        }else{
            servos_3.aiWrap[0].pi_AI = v;
        }
    }

    inline void set_di(unsigned int i, uint8_t v){
        if (i == 0){
            servos_5.pi_DI_0 = v;
        }else if (i == 1){
            servos_5.pi_DI_1 = v;
        }else if (i == 2){
            servos_5.pi_DI_2 = v;
        }else if (i == 3){
            servos_5.pi_DI_3 = v;
        }else if (i == 4){
            servos_5.pi_DI_4 = v;
        }else if (i == 5){
            servos_5.pi_DI_5 = v;
        }else if (i == 6){
            servos_5.pi_DI_6 = v;
        }else{
            servos_5.pi_DI_7 = v; 
        }
    }

    inline int get_di(unsigned int i)const{
        if (i == 0){
            return servos_5.pi_DI_0;
        }else if (i == 1){
            return servos_5.pi_DI_1;
        }else if (i == 2){
            return servos_5.pi_DI_2;
        }else if (i == 3){
            return servos_5.pi_DI_3;
        }else if (i == 4){
            return servos_5.pi_DI_4;
        }else if (i == 5){
            return servos_5.pi_DI_5;
        }else if (i == 6){
            return servos_5.pi_DI_6;
        }else{
            return servos_5.pi_DI_7;  
        }
    }
};

struct PdoTxBfdoaoai{
    // struct{}servos_0;
    union{
        struct{
            uint8_t po_DO_0:1;
            uint8_t po_DO_1:1;
            uint8_t po_DO_2:1;
            uint8_t po_DO_3:1;
            uint8_t _:4;
        };
        uint8_t po_DO;
    }servos_1;
    union{
        struct{
            uint16_t po_AO_0;
            uint16_t po_AO_1;
            uint16_t po_AO_2;
            uint16_t po_AO_3;
        };
        uint16_t po_AO[4];
    }servos_2;
    union{
        struct{
            uint16_t po_AO_0;
            uint16_t po_AO_1;
            uint16_t po_AO_2;
            uint16_t po_AO_3;
            uint16_t po_AO_4;
            uint16_t po_AO_5;
            uint16_t po_AO_6;
            uint16_t po_AO_7;
        };
        uint16_t    po_AO[8];
    }servos_4;
    inline int get_do(unsigned int i)const{
        if (i == 0){
            return servos_1.po_DO_0;
        }else if (i == 1){
            return servos_1.po_DO_1;
        }else if (i == 2){
            return servos_1.po_DO_2;
        }else{
            return servos_1.po_DO_3;
        }
    }
    inline uint16_t* get_ao_ptr(unsigned int i){
        if (i < 4){
            return &servos_2.po_AO[i];
        }else if (i < 12){
            return &servos_4.po_AO[i-4];
        }else{
            return &servos_2.po_AO_3;
        }
    }
};

#pragma pack(pop)

#endif
