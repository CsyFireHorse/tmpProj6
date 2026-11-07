#ifndef _DELAY_FILTER_HPP_
#define _DELAY_FILTER_HPP_

#include "stdint.h"
#include "stdio.h"

namespace zmc{
template<typename T>
class DelayFilter{
public:
    DelayFilter(unsigned int sz=1): _buf(new T[sz+1]), _sz(sz){
        // printf("sz=%d\n", sz);
        _cur = 0;
    }
    DelayFilter(unsigned int sz, const T& val): _buf(new T[sz+1]), _sz(sz){
        _cur = 0;
        init(val);
    }
    ~DelayFilter(){
        delete _buf;
    }
    void set_order(unsigned int sz){
        // printf("set sz=%d\n", sz);
        _buf = new T[sz+1], 
        _sz = sz;
        _cur = 0;
    }
    void init(const T& val){
        for (int i=0; i<_sz; i++){
            _buf[i] = val;
        }
    }
    T step(const T& inp){
        if (_sz == 0) return inp;
        T oup = _buf[_cur];
        _buf[_cur] = inp;
        _cur++;
        if (_cur >= _sz){
            _cur -= _sz;
        }
        return oup;
    }
private:
    uint16_t _cur;
    uint16_t _sz;
    T* _buf;
};


}
#endif