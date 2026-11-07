#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_



#include <mutex>

template<class T>
class Singleton
{
public:
    static T& Instance()
    {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _ptr = new T();
        });
        return *_ptr;
    }
private:
    static T* _ptr;
};

template<class T>
T* Singleton<T>::_ptr = 0;

class SingletonFoo
{
public:
    static SingletonFoo& Instance()
    {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _ptr = new SingletonFoo();
        });

        return *_ptr;
    }
private:
    static SingletonFoo* _ptr;
};

template<class T>
class SingletonEager
{
public:
    SingletonEager(const SingletonEager&) = delete;
    SingletonEager& operator=(const SingletonEager&) = delete;
    static T& Instance()
    {
        static T pc;
        return pc;
    }
};
// class PosCheckerSg : public SingletonEager < PosChecker >{};
#endif /* _SINGLETON_HPP_ */