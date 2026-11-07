
#include "pdoPreparer.h"
#include "ServoFakAlloc.hpp"
#include "ServoFakFileState.h"
#include "mhelper/eni_servo_parser.h"
#include "mhelper/pdo_servo_addr.h"
#include "stdio.h"

using namespace zmc;

struct EniPdoPrepareCtx {
    EniServoMapper mapper;
    proc_img_i_addr pi_i;
    proc_img_o_addr pi_o;
    ServoSgFaker* svArr;
};

static PdoPreparerConfig empty_preparer_config() {
    PdoPreparerConfig config = {nullptr, nullptr, 0, 0, 0, 0, 0};
    return config;
}

static uint32_t build_servo_mask(const EniServoMapper& mapper) {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < mapper.slvInfo.size(); ++i) {
        if (mapper.slvInfo.slaves[i].is_axis()) {
            mask |= (1u << i);
        }
    }
    return mask;
}

static int ecatFakePreparePdoFromEni(void* obj, uint8_t* pi, uint8_t* po) {
    EniPdoPrepareCtx* ctx = (EniPdoPrepareCtx*)obj;
    if (!ctx || !ctx->svArr) {
        return -1;
    }

    pdo_get_input(ctx->pi_i, ctx->mapper, pi);
    pdo_get_output(ctx->pi_o, ctx->mapper, po);

    int n = (int)ctx->mapper.servo_size();
    for (int i = 0; i < n; ++i) {
        ServoSgFaker& sv = ctx->svArr[i];
        servo_ctrl_addr& ctrl = ctx->pi_o.servos[i];
        servo_stat_addr& stat = ctx->pi_i.servos[i];

        if (ctrl.ctrl) {
            sv.wCtrlword = ctrl.ctrl->v;
        }
        if (ctrl.mode) {
            sv.byOpMode = *ctrl.mode;
        }
        if (ctrl.p) {
            sv.diSetPosition = *ctrl.p;
        }
        if (ctrl.v) {
            sv.diSetVelocity = *ctrl.v;
        }
        sv.runCycle();
        if (stat.mode) {
            *stat.mode = sv.byOpModeDisplay;
        }
        if (stat.stat) {
            stat.stat->v = sv.wStatusword;
        }
        if (stat.error_code) {
            *stat.error_code = sv.errCode;
        }
        if (stat.p) {
            *stat.p = sv.diActPosition;
        }
        if (stat.v) {
            *stat.v = sv.diActVelocity;
        }
        if (stat.t) {
            *stat.t = sv.diActTorque;
        }
    }
    servo_fak_state_on_cycle();
    return 0;
}

void freeEniPdoPrepareCtx(void* p) { delete (EniPdoPrepareCtx*)p; }

uint32_t eni_pdo_ctx_get_cycle_time(EniPdoPrepareCtx* ctx) {
    if (!ctx) {
        return 0;
    }
    return ctx->mapper.master_pdo.cycle_time;
}

void eni_pdo_ctx_set_sv_arr(EniPdoPrepareCtx* ctx, void* svArr) {
    if (ctx) {
        ctx->svArr = (ServoSgFaker*)svArr;
    }
}

PdoPreparerConfig guess_preparer(const char* ENI) {
    if (!ENI) {
        printf("guess_preparer: ENI path is null\n");
        return empty_preparer_config();
    }

    EniPdoPrepareCtx* ctx = new EniPdoPrepareCtx();
    ctx->svArr = nullptr;

    if (load_pdo_offset_from_file(ENI, ctx->mapper) != 0) {
        printf("guess_preparer: failed to parse ENI file %s\n", ENI);
        delete ctx;
        return empty_preparer_config();
    }

    PdoPreparerConfig config = empty_preparer_config();
    config.ctx = ctx;
    config.rxpdoLen = (ctx->mapper.slvInfo.get_rx_end_addr() + 7) / 8;
    config.txpdoLen = (ctx->mapper.slvInfo.get_tx_end_addr() + 7) / 8;
    config.slaveCount = (int)ctx->mapper.size();
    config.servoSize = ctx->mapper.servo_size();
    config.servoMask = build_servo_mask(ctx->mapper);
    config.handler = config.servoSize > 0 ? ecatFakePreparePdoFromEni : nullptr;

    return config;
}
