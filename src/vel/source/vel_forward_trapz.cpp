#include "vel_forward_trapz.h"
#include "vel_forward_trapz_rev.h"
#include "vel_par_trapz.h"
#include "base/include/solver.h"

namespace zmc{
 
void www_demo(){
    VelForwarderTrapzMixer<Accer<false>, Accer<true>, true> vft;
    vft.get_distance(3);
    vft.get_acc(3);
    vft.get_rate(3);
}

} /* namespace zmc */
