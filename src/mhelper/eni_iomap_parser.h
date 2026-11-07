#ifndef _ENI_IOMAP_PARSER_H_
#define _ENI_IOMAP_PARSER_H_

#include <stdint.h>
#include <string>
#include <vector>
#include "mhelper/ecat_json.h"

namespace zmc{
struct pdo_item_t
{    
    uint8_t slv_idx; 
    uint8_t bit_size; 
    uint16_t index;     
    uint16_t subindex;  
    uint16_t bit_offset;
};

inline void ukey2index(uint32_t ukey, uint8_t& slv_idx, uint16_t& index, uint8_t& subindex){
    slv_idx = (ukey & 0xff000000)>>24;
    index = (ukey & 0xffff00)>>8;
    subindex = ukey & 0xff;
}
inline uint32_t index2ukey(uint8_t slv_idx, uint16_t index, uint8_t subindex){
    return (slv_idx << 24) | ( index << 8) | subindex;
}

// 单个从站的io端口
typedef struct proc_slv_dido_ofs{
    // uint8_t             io_servo_type;
    // int16_t             po_mode_offset;
    // int16_t             po_ctrl_word_offset;
    // int16_t             po_p_offset;
    // int16_t             po_v_offset;
    // int16_t             po_t_offset;
    // int16_t             pi_mode_offset;
    // int16_t             pi_stat_offset;
    // int16_t             pi_error_offset;    
    // int16_t             pi_p_offset;
    // int16_t             pi_v_offset;  
    // int16_t             pi_t_offset;      

    // int16_t pi_DI_ofs;
    // int16_t pi_DI_bitlen;
    // int16_t po_DO_ofs;
    // int16_t po_DO_bitlen;

    uint16_t pi_DI_num;
    pdo_item_t* pi_DI;

    uint16_t pi_AI_num;
    pdo_item_t* pi_AI;
    // uint16_t pi_AI_ofs[128];

    uint16_t po_DO_num;
    pdo_item_t* po_DO;

    uint16_t po_AO_num;
    pdo_item_t* po_AO;
};

// 从所有从站重排列的io端口
typedef struct proc_dido_ofs{
    uint16_t pi_DI_num;
    pdo_item_t* pi_DI;

    uint16_t pi_AI_num;
    pdo_item_t* pi_AI;

    uint16_t po_DO_num;
    pdo_item_t* po_DO;

    uint16_t po_AO_num;
    pdo_item_t* po_AO;
};


class EniIomapper{
public:
    // 主站信息，杂类信息
    MasterInfo                      master_pdo;
    // 包含每个从站的多种io端口
    std::vector<proc_slv_dido_ofs>      slave_pdo;
    // 重新排列之后的IO端口
    proc_dido_ofs      io_batch;
};

int load_pdo_iomap_from_file(const char* fname, EniIomapper& ioMap);

class PdoIoMapper{
    EniIomapper* _map;
    uint8_t*    _inPtr;
    uint8_t*    _outPtr;
public:
    PdoIoMapper(EniIomapper* map=nullptr){
        _map = map;
    }
    void bind_map(EniIomapper* map){
        _map = map;
    }
    void* get_map_ptr(){
        return _map;
    }

    void bind_in_out(uint8_t* inPtr, uint8_t* outPtr){
        _inPtr = inPtr;
        _outPtr = outPtr;
    }

    int get_di(uint8_t idx, uint8_t* val){
        if (idx >= _map->master_pdo.di_begin_index && idx < _map->master_pdo.di_end){
            // ukey2index(_map->master_pdo.di_keys[], slv_idx, index, subindex);
            pdo_item_t& dItem = _map->io_batch.pi_DI[idx - _map->master_pdo.di_begin_index];
            uint8_t res = dItem.bit_offset & 7;
            uint16_t bytLen = dItem.bit_offset/8;
            *val = (_inPtr[bytLen] & (1<<res)) > 0;
            return 0;
        }
        return -1;
    }
    int get_di_multi(uint32_t di_num, uint8_t *val_list){
        for (int idx = _map->master_pdo.di_begin_index; idx < _map->master_pdo.di_end; idx++){
            // ukey2index(_map->master_pdo.di_keys[], slv_idx, index, subindex);
            pdo_item_t& dItem = _map->io_batch.pi_DI[idx - _map->master_pdo.di_begin_index];
            uint8_t res = dItem.bit_offset & 7;
            uint16_t bytLen = dItem.bit_offset/8;
            if (bytLen >= di_num) return 0;
            val_list[0] = (_inPtr[bytLen] & (1<<res)) > 0;
            return 0;
        }
        return 0;
    }
    int get_ai(uint8_t idx, uint16_t* val){
        if (idx >= _map->master_pdo.ai_begin_index && idx < _map->master_pdo.ai_end){
            pdo_item_t& dItem = _map->io_batch.pi_AI[idx - _map->master_pdo.ai_begin_index];
            uint16_t bytLen = dItem.bit_offset/8;
            *val = *(uint16_t*) (_inPtr+bytLen);
            return 0;
        }
        return -1;
    }
    int get_do(uint8_t idx, uint8_t* val){
        if (idx >= _map->master_pdo.do_begin_index && idx < _map->master_pdo.do_end){
            pdo_item_t& dItem = _map->io_batch.po_DO[idx - _map->master_pdo.do_begin_index];
            uint8_t res = dItem.bit_offset & 7;
            uint16_t bytLen = dItem.bit_offset/8;
            *val = (_inPtr[bytLen] & (1<<res)) > 0;
            return 0;
        }
        return -1;
    }
    int set_do(uint8_t idx, uint8_t val){
        if (!_outPtr) return -2;
        if (idx >= _map->master_pdo.do_begin_index && idx < _map->master_pdo.do_end){
            pdo_item_t& dItem = _map->io_batch.po_DO[idx - _map->master_pdo.do_begin_index];
            uint8_t res = dItem.bit_offset & 7;
            uint16_t bytLen = dItem.bit_offset/8;

            if (val){
                _outPtr[bytLen] |= (1<<res);
            }else{
                _outPtr[bytLen] &= ~(1<<res);
            }
            return 0;
        }
        return -1;
    }
    int set_do_batch(uint8_t len, uint8_t* val){
        if (!_outPtr) return -2;
        for (int idx = _map->master_pdo.do_begin_index; idx < _map->master_pdo.do_end; idx++){
            pdo_item_t& dItem = _map->io_batch.po_DO[idx - _map->master_pdo.do_begin_index];
            uint8_t res = dItem.bit_offset & 7;
            uint16_t bytLen = dItem.bit_offset/8;
            if (bytLen >= len) return 0;
            if (val){
                _outPtr[bytLen] |= (1<<res);
            }else{
                _outPtr[bytLen] &= ~(1<<res);
            }
            return 0;
        }
        return -1;
    }
    int set_ao(uint8_t idx, int32_t val){
        if (!_outPtr) return -2;
        if (idx >= _map->master_pdo.ao_begin_index && idx < _map->master_pdo.ao_end){
            pdo_item_t& dItem = _map->io_batch.po_AO[idx - _map->master_pdo.ao_begin_index];
            uint16_t bytLen = dItem.bit_offset/8;
            // _outPtr[bytLen] = val; 
            *(uint16_t*) (_outPtr+bytLen) = val;
            return 0;
        }
        return -1;
    }
};

}
#endif