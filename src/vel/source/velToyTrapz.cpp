#include "VelToyTrapz.h"
#include "vel_par_trapz.h"

namespace zmc{
 
int VelForwarderTrapz::get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec)const{
    if (total_time) *total_time = get_end_time();
    if (rVmax) *rVmax = vm;
    return 0;
};

int VelForwarderTrapz::get_slice(double slice, double *position, double *velocity, double *acc) const{
    int ret = E_VEL_UNCOVERED;
    if (slice < get_begin_time()) slice = 0;
    else if (slice >= get_end_time()) {
        slice = get_end_time() + 1e-6;
        ret = E_VEL_COVERED;
    }

    if (position) *position = get_distance(slice);
    if (velocity) *velocity = get_rate(slice);
    if (acc) *acc = get_acc(slice);
    return ret;
};

void VelForwarderTrapz::_buildTimeSegment(){
    Tarr[0] = 0; // pPar->T_0;
    Sarr[0] = 0; // pPar->S_0;

    switch (nseg)
    {
    case LADDER_30_SEGMENT:
        _buildTimeSegment_s30();
        break;
    case LADDER_20_SEGMENT:
        _buildTimeSegment_s20();
        break;
    default:
        _buildTimeSegment_s30();
        break;
    }
}
void VelForwarderTrapz::_buildTimeSegment_s20(){
    Tarr[1] = (vm - pPar->V_0) / am;
    Tarr[2] = 0;
    Tarr[3] = (vm - pPar->V_d) / an;
}
void VelForwarderTrapz::_buildTimeSegment_s30(){        
    Tarr[1] = (vm - pPar->V_0) / am;
    double s13 = ((TrapzParams*) pPar)->nseg20_vm2s(vm);
    Tarr[2] = (_S - s13) / vm;
    Tarr[3] = (vm - pPar->V_d) / an;
}

void VelForwarderTrapz::_buildTimeSegment_s31(){        
    Tarr[1] = (pPar->V_0 - vm) / an;
    double s12 = ((TrapzParams*) pPar)->nseg12_vm2s();
    Tarr[2] = (_S- s12) / vm;
    Tarr[3] = (vm - pPar->V_d) / an;
}

void VelForwarderTrapz::_buildTimeSegment_s23(){
    Tarr[1] = (pPar->V_0 - _vn) / an;
    Tarr[2] = 0;
    Tarr[3] = (_vs - _vn) / am;
}


void VelForwarderTrapz::_buildTimeSegment_s10(){
    Tarr[1] = 0;
    Tarr[2] = _S / pPar->V_0;
    Tarr[3] = 0;
}

void VelForwarderTrapz::_buildTimeSegment_s11(){
    Tarr[1] = (vm - pPar->V_0) / am;
    Tarr[2] = 0;
    Tarr[3] = 0;
}

void VelForwarderTrapz::_buildTimeSegment_s12(){
    Tarr[1] = 0;
    Tarr[2] = 0;
    Tarr[3] = (vm - _vs) / an;
}

void VelForwarderTrapz::_buildPosVecSegment(){
    switch (nseg)
    {
    case LADDER_30_SEGMENT:
        _buildPosVecSegment_s30();
        break;
    case LADDER_20_SEGMENT:
        _buildPosVecSegment_s20();
        break;
    case LADDER_10_SEGMENT:
        _buildPosVecSegment_s10();
        break;
    case LADDER_24_SEGMENT:
    case LADDER_31_SEGMENT:
        _buildPosVecSegment_s31();
        break;
    case LADDER_23_SEGMENT:
        _buildPosVecSegment_s23();
    default:
        _buildPosVecSegment_s30();
        break;
    }
}
void VelForwarderTrapz::_buildPosVecSegment_s30(){
    double& V_0 = pPar->V_0;
    Varr[0] = pPar->V_0;
    Varr[1] = pPar->V_0 + am * Tarr[1];
    Varr[2] = vm;
    Varr[3] = vm - an * Tarr[3];

    Sarr[1] = pPar->V_0 * Tarr[1] + am * SQUARE(Tarr[1]) / 2;
    Sarr[2] = vm * Tarr[2];
    Sarr[3] = vm * Tarr[3] - an * SQUARE(Tarr[3]) / 2;
}
void VelForwarderTrapz::_buildPosVecSegment_s20(){
    double& V_0 = pPar->V_0;
    Varr[0] = V_0;
    Varr[1] = V_0 + am * Tarr[1];
    Varr[2] = vm;
    Varr[3] = vm - an * Tarr[3];

    Sarr[1] = V_0 * Tarr[1] + am * SQUARE(Tarr[1]) / 2;
    Sarr[2] = 0;
    Sarr[3] = vm * Tarr[3] - an * SQUARE(Tarr[3]) / 2;
}
void VelForwarderTrapz:: _buildPosVecSegment_s10(){
    Varr[0] = pPar->V_0;
    Varr[1] = pPar->V_0;
    Varr[2] = vm;
    Varr[3] = vm;

    Sarr[1] = 0;
    Sarr[2] = Tarr[2] * vm;
    Sarr[3] = 0;
}
void VelForwarderTrapz:: _buildPosVecSegment_s31(){
    Varr[0] = pPar->V_0;
    Varr[1] = pPar->V_0 - an * Tarr[1];
    Varr[2] = vm;
    Varr[3] = vm - an * Tarr[3];

    Sarr[1] = pPar->V_0 * Tarr[1] - an * SQUARE(Tarr[1]) / 2;
    Sarr[2] = vm * Tarr[2];
    Sarr[3] = vm * Tarr[3] - an * SQUARE(Tarr[3]) / 2;
}


void VelForwarderTrapz:: _buildPosVecSegment_s23(){
    Varr[0] = pPar->V_0;
    Varr[1] = pPar->V_0 - an * Tarr[1];
    Varr[2] = _vn;
    Varr[3] = _vn + am * Tarr[3];

    Sarr[1] = pPar->V_0 * Tarr[1] - an * SQUARE(Tarr[1]) / 2;
    Sarr[2] = 0;
    Sarr[3] = _vn * Tarr[3] + am * SQUARE(Tarr[3]) / 2;
}

void VelForwarderTrapz::_cumsum()
{
    cTarr[0] = Tarr[0];
    cTarr[1] = cTarr[0] + Tarr[1];
    cTarr[2] = cTarr[1] + Tarr[2];
    cTarr[3] = cTarr[2] + Tarr[3];

    cSarr[0] = Sarr[0];
    cSarr[1] = cSarr[0] + Sarr[1];
    cSarr[2] = cSarr[1] + Sarr[2];
    cSarr[3] = cSarr[2] + Sarr[3];
}
double VelForwarderTrapz::get_distance(double tm) const {
    if (tm < cTarr[0]){
        return cSarr[0];
    }
    switch(nseg){
        case LADDER_30_SEGMENT:
            return _get_distance_s30(tm);
        case LADDER_20_SEGMENT:
            return _get_distance_s20(tm);  
        case LADDER_23_SEGMENT:
            return _get_distance_s23(tm);
        case LADDER_24_SEGMENT:  
        case LADDER_31_SEGMENT:
            return _get_distance_s31(tm);
        default:
            return _get_distance_s30(tm);
    }
}
double VelForwarderTrapz::_get_distance_s30(double tm) const{
    double sc, dt;
    const double& V_0 = Varr[0];
    if (tm <= cTarr[0]){
        sc = cSarr[0];
    }else if (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        sc = cSarr[0] + V_0 * dt + am * dt * dt / 2;
    }else if (tm <= cTarr[2]){
        dt = tm - cTarr[1];
        sc = cSarr[1] + vm * dt;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        sc = cSarr[2] + vm * dt - an * dt * dt / 2;
    }else
        sc = cSarr[3];
    return sc;
}

double VelForwarderTrapz::_get_distance_s20(double tm) const{
    double dt, sc;
    const double& V_0 = Varr[0];
    if (tm <= cTarr[0]){
        sc = cSarr[0];
    }else if (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        sc = cSarr[0] + V_0 * dt + am * dt * dt / 2;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        sc = cSarr[2] + vm * dt - an * dt * dt / 2;
    }else
        sc = cSarr[3];
    return sc;
}


double VelForwarderTrapz:: _get_distance_s31(double tm) const{
    double sc, dt;
    if (tm <= cTarr[0]){
        sc = get_begin_distance();
    }else if (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        sc = cSarr[0] + pPar->V_0 * dt - an * dt * dt / 2;
    }else if (tm <= cTarr[2]){
        dt = tm - cTarr[1];
        sc = cSarr[1] + vm * dt;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        sc = cSarr[2] + vm * dt - an * dt * dt / 2;
    }else
        sc = cSarr[3];
    return sc;
}

double VelForwarderTrapz:: _get_distance_s23(double tm) const{
    double dt, sc;
    if (tm <= cTarr[0]){
        sc = get_begin_distance();
    }else if (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        sc = cSarr[0] + pPar->V_0 * dt - an * dt * dt / 2;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        sc = cSarr[2] + _vn * dt + am * dt * dt / 2;
    }else
        sc = cSarr[3];
    return sc;
}
double VelForwarderTrapz::get_rate(double tm) const{
    if (tm < cTarr[0]){
        return 0;
    }
    switch(nseg){
    case LADDER_30_SEGMENT:
        return _get_rate_s30(tm);
    case LADDER_20_SEGMENT:
        return _get_rate_s20(tm);   
    case LADDER_23_SEGMENT:
        return _get_rate_s23(tm);
    case LADDER_24_SEGMENT:    
    case LADDER_31_SEGMENT:
        return _get_rate_s31(tm);                                                        
    default:
        return _get_rate_s30(tm);
    }
}

double VelForwarderTrapz::_get_rate_s30(double tm) const{
    double dt, vc;
    const double& V_0 = Varr[0];
    if (tm < cTarr[0])
        vc = Varr[0];
    else if  (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        vc = V_0 + am * dt;
    }else if (tm <= cTarr[2])
        vc = vm;
    else if (tm <= cTarr[3])
        vc = vm - an * (tm - cTarr[2]);
    else
        vc = 0;
    return vc;
}
double VelForwarderTrapz::_get_rate_s20(double tm) const{
    double dt, vc;
    const double& V_0 = Varr[0];
    if (tm < cTarr[0]){
        vc = Varr[0];
    }else if  (tm <= cTarr[1] ){
        dt = tm - cTarr[0];
        vc = V_0 + am * dt;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        vc = vm - an * dt;
    }else
        vc = 0;
    return vc;
}

double VelForwarderTrapz:: _get_rate_s31(double tm) const{
    double dt, vc;
    if (tm < cTarr[0])
        vc = Varr[0];
    else if  (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        vc = pPar->V_0 - an * dt;
    }else if (tm <= cTarr[2])
        vc = vm;
    else if (tm <= cTarr[3])
        vc = vm - an * (tm - cTarr[2]);
    else
        vc = 0;
    return vc;
}
double VelForwarderTrapz:: _get_rate_s23(double tm) const{
    double dt, vc;
    if (tm < cTarr[0]){
        vc = Varr[0];
    }else if  (tm <= cTarr[1] ){
        dt = tm - cTarr[0];
        vc = pPar->V_0 - an * dt;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        vc = _vn + am * dt;
    }else
        vc = 0;
    return vc;
}

double VelForwarderTrapz::_get_rate_s24(double tm) const{
    double dt, vc;
    if (tm < cTarr[0]){
        vc = Varr[0];
    }else if  (tm <= cTarr[1] ){
        dt = tm - cTarr[0];
        vc = pPar->V_0 - an * dt;
    }else if (tm <= cTarr[2])
        vc = vm;
    else
        vc = 0;
    return vc;
}

double VelForwarderTrapz::get_acc(double tm) const{
    switch(nseg){
        case LADDER_30_SEGMENT:
            return _get_acc_s30(tm);
        case LADDER_20_SEGMENT:
            return _get_acc_s20(tm);   
        case LADDER_21_SEGMENT:
            return _get_acc_s21(tm);
        case LADDER_22_SEGMENT:
            return _get_acc_s22(tm);
        case LADDER_23_SEGMENT:
            return _get_acc_s23(tm);
        case LADDER_24_SEGMENT:
            return _get_acc_s24(tm);
        case LADDER_31_SEGMENT:
            return _get_acc_s31(tm);
        case LADDER_10_SEGMENT:
            return _get_acc_s10(tm);
        case LADDER_12_SEGMENT:
            return _get_acc_s12(tm);
        case LADDER_11_SEGMENT:
            return _get_acc_s11(tm);
        default:
            return _get_acc_s30(tm);
    }
}

double VelForwarderTrapz::_get_acc_s30(double tm) const{
    double ac;
    if (tm < cTarr[0]){
        ac = 0;
    }else if (tm <= cTarr[1])
        ac = am;
    else if (tm <= cTarr[2])
        ac = 0;
    else if (tm <= cTarr[3])
        ac = - an;
    else
        ac = 0;
    return ac;
}

double VelForwarderTrapz::_get_acc_s20(double tm) const{
    double ac;
    if (tm < cTarr[0]){
        ac = 0;
    }else if (tm <= cTarr[1])
        ac = am;
    else if (tm <= cTarr[3])
        ac = - an;
    else
        ac = 0;
    return ac;
}


    double VelForwarderTrapz:: _get_acc_s31(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = - an;
        else if (tm <= cTarr[2])
            ac = 0;
        else if (tm <= cTarr[3])
            ac = - an;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s21(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = am;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s22(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[2])
            ac = 0;
        else if (tm <= cTarr[3])
            ac = - an;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s23(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = - an;
        else if (tm <= cTarr[2])
            ac = 0;
        else if (tm <= cTarr[3])
            ac = am;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s24(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = - an;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s10(double tm) const{
        double ac;
        if (tm < cTarr[1]){
            ac = 0;
        }else if (tm <= cTarr[2])
            ac = 0;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s12(double tm) const{
        double ac;
        if (tm < cTarr[2]){
            ac = 0;
        }else if (tm <= cTarr[3])
            ac = - an;
        else
            ac = 0;
        return ac;
    }

    double VelForwarderTrapz:: _get_acc_s11(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = am;
        else
            ac = 0;
        return ac;
    }

    int FuncVelToyTrapz::operator()(const VelParam& param, VelForwarderTrapz& fw){
        TrapzParams& par = * (TrapzParams*) &param;
        return velBuild<TrapzRelaxParam>(par, fw);
    }
}