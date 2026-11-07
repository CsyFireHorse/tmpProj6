#ifndef _ZMC_SERVO_ADAPTER_BASE_HPP_
#define _ZMC_SERVO_ADAPTER_BASE_HPP_

class AxisBase;
class AxisCiaConfig;

namespace zmc{


class ServoAdapterBase{
public:
    void onInit(AxisBase& axis){}
    void onDeinit(AxisBase& axis){}
    void onSetConfig(AxisBase& axis, const AxisCiaConfig* cfg){}
    void onSetFakConfig(AxisBase& axis, const AxisCiaConfig* cfg){}
    void onBeforeRunCycle(AxisBase& axis){}
    void onAfterRunCycle(AxisBase& axis){}
    void onFakRunCycle(AxisBase& axis){}
};

struct ZmClassImpl {
    void* pImpl;
};

inline void* get_impl_from_class_impl(void* pAxis){
    auto pAxisInner = static_cast<ZmClassImpl*>(((ZmClassImpl*)pAxis)->pImpl);
    return pAxisInner->pImpl;
}


template<typename R, class B>
class MixinAxisServoAdapter:public B{
public:
    // MixinAxisServoAdapter(){
    //     static_cast<T*>(this)->get_servo_adapter().onInit();
    // }
    void onInit(){
        get_servo_adapter().onInit(*this);
    }
    void set_config(const AxisCiaConfig* cfg){
        get_servo_adapter().onSetConfig(*this, cfg);
        B::set_config(cfg);
        get_servo_adapter().onSetFakConfig(*this, cfg);
    }
    void reload_config(){
        this->set_config(&this->get_config_ref());
    }
    void runCycle(){
        get_servo_adapter().onBeforeRunCycle(*this);
        B::runCycle();
        get_servo_adapter().onAfterRunCycle(*this);
        get_servo_adapter().onFakRunCycle(*this);
    }

    inline R& get_servo_adapter(){
        return *((R*)get_impl_from_class_impl(this));
    }
    inline const R& get_servo_adapter() const{
        return *((const R*)get_impl_from_class_impl(this));
    }
};
}

#endif