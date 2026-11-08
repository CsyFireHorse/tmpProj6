#include "eni_servo_parser.h"
#include "common/ecat/ecat_pdo_key.h"
#include "mhelper/ecat_json.h"
#include "string.h"
#include "common/zmc/zmc_predef.h"

// Address offsets are measured in bytes; a char is at least 8 bits.
#define OFFSET_UNIT  8                        

namespace zmc{

// TODO: sub_index support
    pi_offset_config pi_offset_config_init(const SLAVES_INFO_t &slaves, int channIdx=0)
{
    pi_offset_config pi_config;
    memset(&pi_config,-1,sizeof(pi_offset_config));

    pi_config.slave_idx = slaves.phys_addr - 1001;
    pi_config.slave_channel_idx = channIdx;
    for(uint32_t i=0; i<slaves.outputs.size(); i++)
    {
        if(slaves.outputs.at(i).subindex ) continue;
        uint16_t index = slaves.outputs.at(i).index - channIdx * PDO_MULT_SERVO_OFFSET;
        switch(index)
        {
            case PDO_PO_MODE: pi_config.po_mode_offset = slaves.outputs.at(i).bit_offset / OFFSET_UNIT; break;
            case PDO_PO_CTRL: pi_config.po_ctrl_word_offset = slaves.outputs.at(i).bit_offset / OFFSET_UNIT; break;
           
            case PDO_PO_POS: pi_config.po_p_offset = slaves.outputs.at(i).bit_offset / OFFSET_UNIT; break;
            case PDO_PO_VEL: pi_config.po_v_offset = slaves.outputs.at(i).bit_offset / OFFSET_UNIT; break;            
            case PDO_PO_TORCH: pi_config.po_t_offset = slaves.outputs.at(i).bit_offset / OFFSET_UNIT; break;
           
            /*add obj ...*/
            default: break;              
        }
    }

    for( uint32_t i=0; i<slaves.inputs.size(); i++)
    {
        if(slaves.inputs.at(i).subindex ) continue;

        uint16_t index = slaves.inputs.at(i).index - channIdx * PDO_MULT_SERVO_OFFSET;
        switch(index)
        {
            case PDO_PI_MODE: pi_config.pi_mode_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT;  break;
            case PDO_PI_STAT: pi_config.pi_stat_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT; break;
           
            case PDO_PI_POS: pi_config.pi_p_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT;   break;                      
            case PDO_PI_VEL: pi_config.pi_v_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT; break; 
            case PDO_PI_TORCH: pi_config.pi_t_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT; break;
            
            case PDO_PI_ERROR: pi_config.pi_error_offset = slaves.inputs.at(i).bit_offset / OFFSET_UNIT; break;

            /*add obj ...*/
            default: break;              
        }
    }

    return pi_config;            
}

void pi_offset_batch_init(std::vector<pi_offset_config> &v_pi_config, const SlaveInfo &slvInfo){
    const std::vector<SLAVES_INFO_t>& slaves = slvInfo.slaves;
    auto num_slave = slaves.size();
    // XXX add support for 1 slave with 2 servo 
    v_pi_config.reserve(num_slave);
    v_pi_config.resize(0);

    int slave_max_sevo_num = 1;
    if (1){
        slave_max_sevo_num = SLAVE_HAS_SERVO_MAX_NUM;
    }

    for(uint32_t i=0; i<num_slave; i++){
        for (int j=0; j<slave_max_sevo_num; j++){
            if (!slaves[i].is_axis(j)) continue;
            auto pcfg =  pi_offset_config_init(slaves[i], j);
            v_pi_config.push_back(pcfg);
        }        
    }
}

int load_pdo_offset_from_file(const char* fname, EniServoMapper& svMap){
    return svMap.load_from_eni_file(fname);
}

void print_eni_servo_mapper(const EniServoMapper& mapper) {
    const MasterInfo& master = mapper.master_pdo;
    printf("master: cycle_time=%u mailbox_cycle_time=%u auto_recovery_timeout=%u\n",
           master.cycle_time, master.mailbox_cycle_time, master.auto_recovery_timeout);
    printf("slaves=%u servos=%u\n", mapper.size(), mapper.servo_size());
    for (const auto& servo : mapper.servo_pdo) {
        printf("slave_index:%u,%u, po_mode_offset:%d po_ctrl_word_offset:%d po_p_offset:%d,"
               "pi_mode_offset:%d pi_stat_offset:%d pi_p_offset:%d\n",
               static_cast<unsigned>(servo.slave_idx),
               static_cast<unsigned>(servo.slave_channel_idx), servo.po_mode_offset,
               servo.po_ctrl_word_offset, servo.po_p_offset, servo.pi_mode_offset,
               servo.pi_stat_offset, servo.pi_p_offset);
    }
}

int EniServoMapper::load_from_eni_file(const char* fname)
{
    if (fname == nullptr) {
        return -1;
    }
    // ecat eni parse
    const int ret = load_slaves_from_eni_xml_json(fname, slvInfo, &master_pdo);
    if (ret != 0) {
        printf("eni load error %d\n", ret);
        return ret;
    }
    pi_offset_batch_init(servo_pdo, slvInfo);
    if (servo_pdo.size() > MAX_NUM_SERVOS) {
        printf("Eni or slaves num error\n");
        return ZMC_ERR_ENI_AXIS_OVERFLOW;
    }
    return 0;
}

namespace {

// AxisFbBridge needs a slave image capacity even when the ENI path only gives per-axis PDO offsets.
void fill_minimal_slave_info(SlaveInfo& slv, uint32_t tx_cap_bytes, uint32_t rx_cap_bytes)
{
    SLAVES_INFO_t slave{};
    pdo_info_t tx_end{};
    pdo_info_t rx_end{};
    tx_end.bit_offset = 0;
    tx_end.bit_size = static_cast<uint16_t>(tx_cap_bytes * 8u);
    rx_end.bit_offset = 0;
    rx_end.bit_size = static_cast<uint16_t>(rx_cap_bytes * 8u);
    slave.inputs.push_back(tx_end);
    slave.outputs.push_back(rx_end);
    slv.slaves.clear();
    slv.slaves.push_back(slave);
}

// Flatten one direction's raw PDO entries into the compact POD cell array so that
// IoManager can resolve objects/aliases by concrete PDO identity, not just by capacity.
// Returns false once the fixed-size cell array is full.
bool append_pdo_cells(EniDescriptorPod& pod, const std::vector<pdo_info_t>& entries,
                       uint8_t slave_idx, uint8_t is_input)
{
    for (uint32_t i = 0; i < entries.size(); ++i) {
        const pdo_info_t& entry = entries[i];
        // Compact POD stores absolute bit addressing; skip empty/out-of-range entries.
        if (entry.bit_size == 0 || entry.bit_size > 0xffu) {
            continue;
        }
        if (pod.pdo_cell_count >= MC_ENI_POD_MAX_PDO_CELLS) {
            return false;
        }
        RtBusPdoCell& cell = pod.pdo_cells[pod.pdo_cell_count];
        cell.index = entry.index;
        cell.bit_offset = entry.bit_offset;
        cell.sub_index = static_cast<uint8_t>(entry.subindex);
        cell.bit_size = static_cast<uint8_t>(entry.bit_size);
        cell.is_input = is_input;
        cell.slave_idx = slave_idx;
        ++pod.pdo_cell_count;
    }
    return true;
}

} // namespace

int eni_servo_mapper_from_pio_layout(EniServoMapper* out, const ZmcPioOffsetLayout* layout)
{
    if (out == nullptr || layout == nullptr) {
        return -1;
    }
    out->servo_pdo.clear();
    const size_t slot_count = zmc_pio_layout_slot_count(layout);
    if (slot_count > MAX_NUM_SERVOS) {
        return ZMC_ERR_ENI_AXIS_OVERFLOW;
    }
    out->servo_pdo.reserve(slot_count);
    for (size_t i = 0; i < slot_count; ++i) {
        out->servo_pdo.push_back(*zmc_pio_layout_slot_at(layout, i));
    }
    const EniDescriptorPod* pod = zmc_pio_layout_descriptor_pod(layout);
    if (pod == nullptr) {
        return -1;
    }
    fill_minimal_slave_info(out->slvInfo, pod->tx_cap, pod->rx_cap);
    return 0;
}

int pio_layout_from_eni_servo_mapper(const EniServoMapper* mapper, ZmcPioOffsetLayout* out)
{
    if (mapper == nullptr || out == nullptr) {
        return -1;
    }
    const uint32_t servo_num = mapper->servo_size();
    if (servo_num > MAX_NUM_SERVOS) {
        return ZMC_ERR_ENI_AXIS_OVERFLOW;
    }
    memset(out, 0, sizeof(*out));
    out->slot_count = static_cast<uint16_t>(servo_num);
    for (uint32_t i = 0; i < servo_num; ++i) {
        out->slots[i] = mapper->servo_pdo[i];
    }
    const std::vector<SLAVES_INFO_t>& slaves = mapper->slvInfo.slaves;
    out->descriptor_pod.tx_cap =
        (mapper->slvInfo.get_tx_end_addr() + OFFSET_UNIT - 1u) / OFFSET_UNIT;
    out->descriptor_pod.rx_cap =
        (mapper->slvInfo.get_rx_end_addr() + OFFSET_UNIT - 1u) / OFFSET_UNIT;
    out->descriptor_pod.slave_count = static_cast<uint16_t>(mapper->size());
    // Populate pdo_cells so IoManager object/alias access works via this layout path.
    for (uint32_t s = 0; s < slaves.size(); ++s) {
        const uint8_t slave_idx = static_cast<uint8_t>(s);
        // outputs = master Tx (is_input=0), inputs = master Rx (is_input=1).
        if (!append_pdo_cells(out->descriptor_pod, slaves[s].outputs, slave_idx, 0u) ||
            !append_pdo_cells(out->descriptor_pod, slaves[s].inputs, slave_idx, 1u)) {
            break;
        }
    }
    return 0;
}

bool EniServoMapper::contain_key(int servo_idx, int idx)const{

    if (servo_idx < 0 || servo_idx >= servo_size()) return false;

    auto& servo = servo_pdo[servo_idx];
    switch (idx)
    {
    case PDO_PO_MODE:
        /* code */
        if (servo.po_mode_offset != -1) return true;
        break;
    case PDO_PO_CTRL:
        if (servo.po_ctrl_word_offset != -1) return true;
        break;
    case PDO_PO_POS:
        if (servo.po_p_offset != -1) return true;
        break;
    case PDO_PO_VEL:
        if (servo.po_v_offset != -1) return true;
        break;
    case PDO_PO_TORCH:
        if (servo.po_t_offset != -1) return true;
        break;
    case PDO_PI_MODE:
        if (servo.pi_mode_offset != -1) return true;
        break;
    case PDO_PI_STAT:
        if (servo.pi_stat_offset != -1) return true;
        break;
    case PDO_PI_ERROR:
        if (servo.pi_error_offset != -1) return true;
        break;
    case PDO_PI_POS:
        if (servo.pi_p_offset != -1) return true;
        break;
    case PDO_PI_VEL:
        if (servo.pi_v_offset != -1) return true;
        break;
    case PDO_PI_TORCH:
        if (servo.pi_t_offset != -1) return true;
        break;
    default:
        // add SlaveInfo judge?
        break;
    }
    return false;
}
int EniServoMapper::get_servo_slave_idx(int servo_idx)const{
    if (servo_idx < 0 || servo_idx >= servo_size()) return -1;
    return servo_pdo[servo_idx].slave_idx;
}
int EniServoMapper::get_servo_offset(int servo_idx) const{
    if (servo_idx < 0 || servo_idx >= servo_size()) return 0;
    return servo_pdo[servo_idx].slave_channel_idx * PDO_MULT_SERVO_OFFSET;
}

} /* namespace zmc */
