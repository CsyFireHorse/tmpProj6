#ifndef _BUFFER_3P_HPP_
#define _BUFFER_3P_HPP_
#include "stdint.h"
#include <functional>

template <class _Ty>
struct buffer_3p
{
    buffer_3p()
    {
        _r_req = 0;
        _w_done = 0;
    }

    template <class Y>
    void write(Y&& val)
    {
        static const uint8_t select[3][3] = { 
            {1, 2, 1}, {2, 0, 0}, {1, 0, 0}
        };
        uint8_t w = select[_w_done][_r_req];
        _buff[w] = std::forward<Y>(val);
        _w_done = w;
    }
    template <class Y>
    void swap_write(Y& val) noexcept
    {
        static const uint8_t select[3][3] = { 
            {1, 2, 1}, {2, 0, 0}, {1, 0, 0}
        };
        uint8_t w = select[_w_done][_r_req];
        std::swap(_buff[w], val);
        _w_done = w;
    }
    void _read(_Ty &val)
    {
        val = tell_r();
    }
    bool read(_Ty &val){
        if (has_new()){
            val = tell_r();
            return true;
        }else
            return false;        
    }

    inline bool has_new() const {
        return _r_req != _w_done;
    }
    _Ty& read()
    {
        return tell_r();
    }
    _Ty& tell_r() // copy by user
    {
        _r_req = _w_done;
        return _buff[_r_req];
    }
    inline _Ty& get_buf(uint8_t idx) { return _buff[idx]; }
    template< typename F>
    void forEach(F&& func){
        for (int i=0; i<3; i++)
            std::forward<F>(func)(_buff[i]);
    }
private:
    _Ty                     _buff[3];
    volatile uint32_t       _r_req;
    volatile uint32_t       _w_done;
};

#endif