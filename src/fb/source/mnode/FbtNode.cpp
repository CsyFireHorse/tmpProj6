#include "FbtNode.h"
#include "fbt_fb_base.hpp"

namespace zmc {

// FbtNode::~FbtNode() {
//     if (fbtImpl) {
//         delete fbtImpl;
//         fbtImpl = nullptr;
//     }
// }

bool FbtNode::isDone() const {
    return fbtImpl ? static_cast<FbtExeBase*>(fbtImpl)->isDone() : false;
}

bool FbtNode::isError() const {
    return fbtImpl ? static_cast<FbtExeBase*>(fbtImpl)->isError() : false;
}

bool FbtNode::isAborted() const {
    return fbtImpl ? static_cast<FbtExeBase*>(fbtImpl)->isAborted() : false;
}


void FbtNode::syncFbStat() {
    if (fbtImpl) {
    }
}

} // namespace zmc 