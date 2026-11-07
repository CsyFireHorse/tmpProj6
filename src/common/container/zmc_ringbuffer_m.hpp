/*
 * @Author: ChenShengyong 
 * @Date: 2024-04-12 19:42:37 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-26 17:24:05
 */
#ifndef _ZMC_RINGBUF_M_H_  
#define _ZMC_RINGBUF_M_H_  
   
#include "stdint.h"
#include <memory>

namespace zmc{  

/* 
  ringbuffer
   _   _   a   b    c   d    _   _   _
  [0] [1] [2] [3] [4] [5]  [6]  [7] [8]
          head        back rear
  
  head === front              
  when push_back, rear++
  when pop_front, head++

  {head, rear} sz
  {0, 0} sz = 0     empty
  {0, 1} sz = 1      not empty
  {255, 0} sz = 1    not empty
  {0, 255} sz = 255  full
  {1, 255} sz = 254  not full
  {1, 0} sz = 255    full
  {0, 256} sz = 256  overflow to empty {0, 0}
*/
template<typename _Tp, uint16_t sz=256, typename R=uint8_t>  
class ZmcRingbufM{  
  public:   
    enum{
      max_size = sz
    };
    inline bool push(const _Tp& value){  return try_push(value);}
    inline bool push(_Tp&& value){  return try_push(value);}
    inline bool pop(_Tp& value){ return try_pop(value);}
    // inline std::shared_ptr<_Tp> pop(){ return try_pop();};
    inline void pop_front()  {  
      _front = (_front + 1) % max_size;  
    }

    inline _Tp& front() { return _data[_front];}
    inline const _Tp& front() const { return _data[_front];}
    inline _Tp& back() { return _data[_rear>0? _rear-1: max_size-1];}
    inline const _Tp& back() const { return _data[_rear>0? _rear-1: max_size-1];}
    // ((_rear +1) %  max_size == _front ? true:false);

    inline bool full() const  { return size() == capacity(); }  
    inline bool empty() const { return _front == _rear; }  
    inline unsigned int capacity() const { return max_size - 1; } 
    inline unsigned int size() const { return (_rear >= _front)?(_rear - _front):(max_size + _rear - _front); }  
    inline void clear(){
        while (!empty()) pop_front();
    }

protected:
    /*
    inline void push_force(const _Tp& value){  
      if (full()) 
        pop_front();
      _push_back(value);
    }
    inline void push_force(_Tp&& value){  
      if (full()) 
        pop_front();
      _push_back(value);
    }
    inline void pop_force(_Tp& value){
      value = front();  
      pop_front();
    }
    */
    // true for success, false for failed
    inline bool try_push(const _Tp& value){  
      if (full()) 
        return false;  
      _push_back(value); 
      return true; 
    }  
    inline bool try_push(_Tp&& value){  
      if (full()) 
        return false;  
      _push_back(std::move(value)); 
      return true; 
    } 
    inline bool try_pop(_Tp& tv)  {  
      if (empty())  
        return false;  
      tv = front();  
      pop_front();
      return true; 
    } 

    inline void _push_index(){ 
      _rear = (_rear + 1) % max_size;  
    }

    inline void _push_back(const _Tp& value){  
      _data[_rear] = value;
      _push_index();
    }
    inline void _push_back(_Tp&& value){  
      _data[_rear] = std::move(value);  
      _push_index(); 
    }
 
  //   inline std::shared_ptr<_Tp> try_pop(){ 
  //     if (empty())  
  //       return {};  
  //     auto res = std::make_shared<_Tp>(std::move(front()));
  //     pop_front();
  //     return res;
	// } 
public:
    volatile R _front = 0;  // front index  
    volatile R _rear = 0;   // rear index  
    // R _size;   // queue data record number  
   
    _Tp  _data[sz];    // data record occupy symbol 
};  

template<typename _Tp, uint16_t SZ=256, typename R=uint8_t>  
class ZmcRingDeque:public ZmcRingbufM<_Tp, SZ, R>{  
  public:
    inline void pop_back()  {  
      this->_rear = (this->_rear - 1) % this->max_size;  
    }
    // void push_front()
};

template<typename _Tp, uint16_t SZ=256, typename R=uint8_t>  
class ZmcRingArray:public ZmcRingDeque<_Tp, SZ, R>{  
  public:  
    inline _Tp& data(R idx) { return this->_data[this->_front+idx % this->max_size];}
    inline const _Tp& data(R idx) const { return this->_data[this->_front+idx % this->max_size];}
    inline _Tp& operator[](R idx) { return data(idx);}
    inline const _Tp& operator[](R idx)const { return data(idx);}
};

} /* namespace zmc */

#endif /* _ZMC_RINGBUFFER_H_ */