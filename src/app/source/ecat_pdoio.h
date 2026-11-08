#ifndef _ECAT_PDOIO_H_
#define _ECAT_PDOIO_H_

#include "string.h"
#include <vector>
#include "common/container/buffer_3p.hpp"
#include "mhelper/eni_iomap_parser.h"

// PDO output control.
class Rxpdo{
    // uint8_t* arr;
    // uint32_t len;
    std::vector<uint8_t> dat;
public:
    inline uint8_t* data() { return dat.data();}
    Rxpdo& operator=(const Rxpdo& rx){
        dat = rx.dat;
        return *this;
    }
    void reserve(uint32_t sz){ dat.reserve(sz);}
    void resize(uint32_t sz){ dat.resize(sz);}
    inline unsigned int size() const{ return dat.size();}
    void assign(uint8_t* lhs, uint8_t* rhs) {
        return dat.assign(lhs, rhs);
    }
    void assign_by_sz(uint8_t* lhs) {
        return dat.assign(lhs, lhs + dat.size());
    }
    inline const uint8_t& operator[](uint16_t idx) const { return dat[idx];}
    inline uint8_t& operator[](uint16_t idx) { return dat[idx];}
    inline bool operator!=(const Rxpdo& rx) const{
        
        return !(this->dat == rx.dat);
    }
    template <typename T>
    inline T& get(uint32_t sz){
        return  (*(T*)(dat.data() + sz));
    }
};
struct Uint8Buf{
    uint8_t* arr;
    uint32_t len;
};

// PDO receive status.
class Txpdo{
    // uint8_t* arr;
    // uint32_t len;
    std::vector<uint8_t> dat;
public:
    unsigned int size() const{
        return dat.size();
    }
    void reserve(uint32_t sz){ dat.reserve(sz);}
    void resize(uint32_t sz){ dat.resize(sz);}
    inline uint8_t* data() { return dat.data();}
    void assign(uint8_t* lhs, uint8_t* rhs) {
        return dat.assign(lhs, rhs);
    }
    void assign_by_sz(uint8_t* lhs) {
        return dat.assign(lhs, lhs + dat.size());
    }
    inline const uint8_t& operator[](uint16_t idx) const { return dat[idx];}
    bool operator==(const Uint8Buf& rx) const{
        auto len = size();
        if (len != rx.len) return false;
        uint8_t* pArr = (uint8_t*) dat.data();
        uint8_t* pArr2 = rx.arr;
        for (int i=0; i<len; i++){
            if (*pArr++ != *pArr2++)
                return false;
        }
        return true;
    }
    inline bool operator!=(const Txpdo& rx) const{
        return !(this->dat == rx.dat);
    }
    template <typename T>
    inline T& get(uint32_t sz){
        return  (*(T*)(dat.data() + sz));
    }
};

// Buffers for both PDO types.
struct TxRxpdo{
    Txpdo tx;
    Rxpdo rx;
};

typedef int (*CtrlTaskHandler)(const void*, TxRxpdo*, Rxpdo*);

class RxtxpdoWrap {
public:
    bool firstFlag = true;
    volatile bool outChanged = false;
    buffer_3p<Rxpdo> output;
    buffer_3p<TxRxpdo> input;

    // used by ethercat thread as temp buffer
    TxRxpdo inBuf;
    Rxpdo outBuf;
    // used by client thread as temp buffer
    TxRxpdo _inBuf;
    Rxpdo _outBuf;

    CtrlTaskHandler fHandler = nullptr;
    void* pObj;

    void bind(CtrlTaskHandler cth, void* obj){
        fHandler = cth;
        pObj = obj;
    }
    // short setEcatRawData(uint16_t offset, uint16_t size, uint8_t *pValue){
    void init(int tx_sz, int rx_sz){
        input.forEach([=](TxRxpdo& pdo){
            pdo.rx.resize(rx_sz);
            pdo.tx.resize(tx_sz);
        });
        output.forEach([=](Rxpdo& pdo){ pdo.resize(rx_sz);});
        inBuf.rx.resize(rx_sz);
        inBuf.tx.resize(tx_sz);
        outBuf.resize(rx_sz);
        _inBuf.rx.resize(rx_sz);
        _inBuf.tx.resize(tx_sz);
        _outBuf.resize(rx_sz);
    }
    short setRxpdo(uint16_t offset, uint16_t size, const uint8_t *pValue){
        if (size > _outBuf.size()) size = _outBuf.size();
        memcpy(_outBuf.data() + offset, pValue, size);
        output.write(_outBuf);
        return 0;
    }
    
    // TOTEST
    short setRxpdoMask(uint16_t offset, uint16_t size, const uint8_t *pValue, const uint8_t *pMask){
        if (offset + size >= _outBuf.size()) return -1;
        for (int i=0; i< size; i++)
            _outBuf[offset+i] = (_inBuf.rx[offset+i] & ~pMask[i] ) | (pMask[i] & pValue[i]);
        output.write(_outBuf);
        return 0;
    }

    template<typename T>
    short _setRxpdoMask(uint16_t offset, const T& Value, const T& mask){
        if (sizeof(T) + offset >= _outBuf.size()) return -1;
        _outBuf.get<T>(offset) = ( _inBuf.rx.get<T>(offset) & ~mask ) | (mask & Value);
        // memcpy(_outBuf.data() + offset, &mask, 1);
        output.write(_outBuf);
        return 0;
    }
    inline short setRxpdoByteMask(uint16_t offset, uint8_t Value, uint8_t mask){
        return _setRxpdoMask<uint8_t>(offset, Value, mask);
    }
    inline short setRxpdoWordMask(uint16_t offset, uint16_t Value, uint16_t mask){
        return _setRxpdoMask<uint16_t>(offset, Value, mask);
    }
    inline short setRxpdoDwordMask(uint16_t offset, uint32_t Value, uint32_t mask){
        return _setRxpdoMask<uint32_t>(offset, Value, mask);
    }

    template<typename T>
    short _setRxpdoInverse(uint16_t offset, const T& mask){
        if (sizeof(T) + offset >= _outBuf.size()) return -1;
        _outBuf.get<T>(offset) = _inBuf.rx.get<T>(offset) ^ mask;
        output.write(_outBuf);
        return 0;
    }
    short setRxpdoDataInverse(uint16_t offset, uint16_t size, const uint8_t *pMask){
        if (size + offset >= _outBuf.size()) return -1;
        for (int i=0; i< size; i++)
            _outBuf[offset+i] = _inBuf.rx[offset+i] ^ pMask[i];
        output.write(_outBuf);
        return 0;
    }

    inline short setRxpdoByteInverse(uint16_t offset, uint8_t mask){
        return _setRxpdoInverse<uint8_t>(offset, mask);
    }
    inline short setRxpdoWordInverse(uint16_t offset, uint16_t mask){
        return _setRxpdoInverse<uint16_t>(offset, mask);
    }
    inline short setRxpdoDwordInverse(uint16_t offset, uint32_t mask){
        return _setRxpdoInverse<uint32_t>(offset, mask);
    }

    short getRxpdo(uint16_t offset, uint16_t size, uint8_t *pValue){
        input.read(_inBuf);
        if (size > _inBuf.rx.size()) size = _inBuf.rx.size();
        memcpy(pValue, _inBuf.rx.data() + offset, size);
        return 0;
    }
    short getTxpdo(uint16_t offset, uint16_t size, uint8_t *pValue){
        input.read(_inBuf);
        if (size > _inBuf.tx.size()) size = _inBuf.tx.size();
        memcpy(pValue, _inBuf.tx.data() + offset, size);
        return 0;
    }
    short getRxpdoMask(uint16_t offset, uint16_t size, uint8_t *pValue, const uint8_t *pMask){
        input.read(_inBuf);
        if (offset + size >= _inBuf.rx.size()) return -1;
        for (int i=0; i<size; i++)
            pValue[i] = (pValue[i] & ~pMask[i] ) | (pMask[i] & _inBuf.rx[offset+i]);
        return 0;
    }

    short getTxpdoMask(uint16_t offset, uint16_t size, uint8_t *pValue, const uint8_t *pMask){
        input.read(_inBuf);
        if (offset + size >= _inBuf.tx.size()) return -1;
        // memcpy(pValue, _inBuf.tx.data() + offset, size);
        for (int i=0; i<size; i++)
            pValue[i] = (pValue[i] & ~pMask[i] ) | (pMask[i] & _inBuf.tx[offset+i]);
        return 0;
    }

    template<typename T>
    short _getRxpdoByteMask(uint16_t offset, const T& mask, T& buf){
        if ( sizeof(T) + offset >= _inBuf.rx.size()) return -1;
        input.read(_inBuf);
        buf = (buf & ~mask ) | (mask & _inBuf.rx.get<T>(offset));
        return 0;
    }

    template<typename T>
    short _getTxpdoByteMask(uint16_t offset, const T& mask, T& buf){
        if ( sizeof(T) + offset >= _inBuf.tx.size()) return -1;
        input.read(_inBuf);
        buf = (buf & ~mask ) | (mask &  _inBuf.tx.get<T>(offset));
        return 0;
    }

    short getRxpdoByteMask(uint16_t offset, uint8_t mask, uint8_t* pBuf){
        return _getRxpdoByteMask<uint8_t>(offset, mask, *pBuf);
    }
    short getTxpdoByteMask(uint16_t offset, uint8_t mask, uint8_t* pBuf){
        return _getTxpdoByteMask<uint8_t>(offset, mask, *pBuf);
    }
    inline short getRxpdoWordMask(uint16_t offset, uint16_t mask, uint16_t* pBuf){
        return _getRxpdoByteMask<uint16_t>(offset, mask, *pBuf);
    }
    inline short getTxpdoWordMask(uint16_t offset, uint16_t mask, uint16_t* pBuf){
        return _getTxpdoByteMask<uint16_t>(offset, mask, *pBuf);
    }
    inline short getRxpdoDwordMask(uint16_t offset, uint32_t mask, uint32_t* pBuf){
        return _getRxpdoByteMask<uint32_t>(offset, mask, *pBuf);
    }
    inline short getTxpdoDwordMask(uint16_t offset, uint32_t mask, uint32_t* pBuf){
        return _getTxpdoByteMask<uint32_t>(offset, mask, *pBuf);
    }
};

#endif
