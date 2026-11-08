#include "eni_iomap_parser.h"

#include "common/ecat/ecat_pdo_key.h"
#include "common/zmc/zmc_predef.h"
#include "fbt_errorcode_def.h"
#include "string.h"


// Address offsets are measured in bytes; a char is at least 8 bits.
#define OFFSET_UNIT 8

namespace zmc {
// mast.di_keys
/** Extract the corresponding slave dictionaries from IO data and build a list of IO port dictionaries.
 *
 * v_out    \param[out]     Port dictionary for each slave.
 * pDiItem  \param[out]     Dictionary for the reordered ports.
 * slaves   \param[in]      Slave information.
 */
void fill_slave_pdo_item(std::vector<proc_slv_dido_ofs>& v_out, pdo_item_t*& pDiItem,
                         uint16_t& di_num, const vector<uint32_t>& keys, const SlaveInfo& slvInfo,
                         uint8_t di_do_ai_ao_flag) {
    const vector<SLAVES_INFO_t>& slaves = slvInfo.slaves;
    int sz = slaves.size();
    // Corresponds to pDiItem.
    std::vector<pdo_item_t> v_di_item;
    // Corresponds to v_out.
    std::vector<pdo_item_t>* v_slave_item = new std::vector<pdo_item_t>[sz];

    uint8_t slv_idx, subindex;
    uint16_t index;
    bool use_input = !(di_do_ai_ao_flag & 1);
    for (const auto& ukey : keys) {
        if (!ukey) continue;
        ukey2index(ukey, slv_idx, index, subindex);
        const auto& slv = use_input ? slaves[slv_idx].inputs : slaves[slv_idx].outputs;
        for (const auto& itm : slv) {
            if (itm.index == index && itm.subindex == subindex) {
                pdo_item_t p = {slv_idx, itm.bit_size, index, subindex, itm.bit_offset};
                v_di_item.push_back(p);
                v_slave_item[slv_idx].push_back(p);
                break;
            }
        }
    }
    // out.pi_DI_num

    pdo_item_t** pItem;
    uint16_t* num;
    for (int i = 0; i < sz; i++) {
        int s_sz = v_slave_item[i].size();
        if (!s_sz) continue;
        proc_slv_dido_ofs& out = v_out[i];
        switch (di_do_ai_ao_flag) {
            case 0:
                pItem = &out.pi_DI;
                num = &out.pi_DI_num;
                break;
            case 1:
                pItem = &out.po_DO;
                num = &out.po_DO_num;
                break;
            case 2:
                pItem = &out.pi_AI;
                num = &out.pi_AI_num;
                break;
            default:
                pItem = &out.po_AO;
                num = &out.po_AO_num;
        }
        *num = s_sz;
        *pItem = new pdo_item_t[s_sz];
        memcpy(*pItem, v_slave_item[i].data(), s_sz * sizeof(pdo_item_t));
    }
    di_num = v_di_item.size();
    pDiItem = new pdo_item_t[di_num];
    memcpy(pDiItem, v_di_item.data(), di_num * sizeof(pdo_item_t));
    delete[] v_slave_item;
}

void guess_pi_cfg(EniIomapper& iomap, const SlaveInfo& slaves) {
    // memset(&pi_config, 0, sizeof(pi_offset_config));
    int sz = slaves.size();
    iomap.slave_pdo.resize(sz);
    MasterInfo& mast = iomap.master_pdo;
    fill_slave_pdo_item(iomap.slave_pdo, iomap.io_batch.pi_DI, iomap.io_batch.pi_DI_num,
                        mast.di_keys, slaves, 0);
    fill_slave_pdo_item(iomap.slave_pdo, iomap.io_batch.po_DO, iomap.io_batch.po_DO_num,
                        mast.do_keys, slaves, 1);
    fill_slave_pdo_item(iomap.slave_pdo, iomap.io_batch.pi_AI, iomap.io_batch.pi_AI_num,
                        mast.ai_keys, slaves, 2);
    fill_slave_pdo_item(iomap.slave_pdo, iomap.io_batch.po_AO, iomap.io_batch.po_AO_num,
                        mast.ao_keys, slaves, 3);
}

void pi_offset_batch_init(EniIomapper& v_pi_config, const SlaveInfo& slaves) {
    auto num_slave = slaves.size();
    v_pi_config.slave_pdo.resize(num_slave);
    guess_pi_cfg(v_pi_config, slaves);
}

int load_pdo_iomap_from_file(const char* fname, EniIomapper& ioMap) {
    // ecat eni parse
    SlaveInfo slaves;
    MasterInfo& mInfo = ioMap.master_pdo;
    int ret = load_slaves_from_eni_xml_json(fname, slaves, &mInfo);
    if (ret) return ret;
    if (slaves.size() > MAX_NUM_SLAVES) {
        printf("Eni or slaves num error\n");
        return ZMC_ERR_ENI_SLAVE_OVERFLOW;
    }
    pi_offset_batch_init(ioMap, slaves);
    return 0;
}

}  // namespace zmc
