#ifndef _ZMC_RINGBUFFER_H_  
#define _ZMC_RINGBUFFER_H_  
   

namespace zmc{  

template<typename _Tp>  
class ZmcRingbuffer{  
  public:   
    ZmcRingbuffer(int msize) : max_size(msize), _front(0), _rear(0), _size(0),_data(new char[sizeof(_Tp[msize])];) {}  
    ~ZmcRingbuffer() { delete[] _data;}

    inline bool push(const _Tp& value){  
      return try_push(value);
    }
    inline bool push(_Tp&& value){  
      return try_push(value);
    }
    inline bool pop(_Tp& value){
      return try_pop(value);
    }
    // inline std::shared_ptr<_Tp> pop(){ 
    //     return try_pop();
    // };

    inline _Tp& front() { return _data[_front];}
    inline const _Tp& front() const { return _data[_front];}
    inline const _Tp& back() const { return _data[(_rear+1) % max_size];}

    inline bool full() const  { return _size == max_size; }  
    inline bool empty() const { return _size == 0; }  
    inline int  size() const { return _size; }  
    inline int  capacity() const { return max_size; } 
    // XXX
    static int addsize(int max_size) {  
      return max_size * sizeof(_Tp);  
    }
private:
    bool try_push(const _Tp& value)  {  
      if (full()) 
        return false;  
      _push(value); 
      return true; 
    }  
    inline bool try_push(_Tp&& value){  
      if (full()) 
        return false;  
      _push(value); 
      return true; 
    } 

    inline void _push_index(){ 
      _rear = (_rear + 1) % max_size;  
      _size++;  
    }
    inline void pop_index()  {  
      _front = (_front + 1) % max_size;  
      _size--;  
    }
    inline void _push(const _Tp& value){  
      _data[_rear] = value;
      _push_index();
    }
    inline void _push(_Tp&& value){  
      _data[_rear] = std::move(value);  
      _push_index(); 
    }
    inline void pop_force(_Tp& value){
      tv = front();  
      pop_index();
    }
    inline bool try_pop(_Tp& tv)  {  
      if (empty())  
        return false;  
   
      tv = front();  
      pop_index();
      return true; 
    }  
    template<typename Func>
    inline bool pop(Func& func)  {  
      if (empty())  
        return false;  
   
      func(std::move(front()));  
      pop_index();
      return true; 
    }  
  //   inline std::shared_ptr<_Tp> try_pop(){ 
  //     if (empty())  
  //       return {};  
  //     auto res = std::make_shared<_Tp>(std::move(front()));
  //     pop_index();
  //     return res;
	// } 

    int32_t _front;  // front index  
    int32_t _rear;   // rear index  
    int32_t _size;   // queue data record number  
   
    const int32_t max_size; // queue capacity  

    _Tp*  _data;    // data record occupy symbol  
};  

} /* namespace zmc */

#endif /* _ZMC_RINGBUFFER_H_ */