#ifndef _ZMC_PAIR_MANAGER_HPP_
#define _ZMC_PAIR_MANAGER_HPP_

#include "common/Singleton.hpp"
#include <vector>
#include <algorithm>

namespace zmc{

template<class T>
class KeyValueVector{
    ~KeyValueVector(){
        for (auto& ptr:vct_ptrs)
            delete ptr;
        vct_ptrs.clear();
        vkeys.clear();
    }

    std::vector<T*> vct_ptrs;
    std::vector<std::string> vkeys;
    inline T* _find_ptr(const std::string& net_name){
        auto fd = std::find(vkeys.cbegin(), vkeys.cend(), net_name);
        if (fd != vkeys.end())
            return vct_ptrs[fd - vkeys.begin()];
        else
            return nullptr;
    }
public:
    int find_key(const std::string& net_name)const{
        auto search = std::find(vkeys.cbegin(), vkeys.cend(), net_name);
        if (search != vkeys.cend())
            return search - vkeys.cbegin();
        else
            return -1;  
    }
    bool contains(const std::string& net_name)const{
        auto search = std::find(vkeys.cbegin(), vkeys.cend(), net_name);
        if (search != vkeys.cend())
            return true;
        else
            return false;        
    }

    int create(const std::string& net_name){
        if (!contains(net_name)){
            auto ptr = new T(net_name);
            vkeys.push_back(net_name);
            vct_ptrs.push_back(ptr);
        }
        return 0;
    }
    inline T* get_ptr(unsigned int idx){
        return vct_ptrs.at(idx);
    }
    T* get_ptr(const std::string& net_name){
        return _find_ptr(net_name);
    }
    void destroy(const std::string& net_name){
        delete _find_ptr(net_name);
    }
};


template <typename T, class KV = KeyValueVector<T> >
class EcatPairSg:public Singleton<KV>{
public:
    EcatPairSg();
    ~EcatPairSg();

    static KV&  Instance(){
        return Singleton<KV>::Instance();
    }
    static int init_ecat(const std::string& net_name){
        return Instance().create(net_name);
    }
    static int find_netname(const std::string& net_name){
        return Instance().find_key(net_name);
    }
    inline static T* get_ecat(const std::string& net_name){
        return Instance().get_ptr(net_name);
    }
    inline static T* get_ecat(unsigned int idx){
        return Instance().get_ptr(idx);
    }

    static void destroy(const std::string& net_name){
        return Instance().destroy(net_name);
    }
};


}

#endif