#include "ecat_pdoio.h"
#include "ecat_pdoioWrap.h"

int __proc(void* p, uint8_t* inp, uint8_t* oup){
    RxtxpdoWrap& iomapWrap = *(RxtxpdoWrap*) p;
    TxRxpdo& inBuf = iomapWrap.inBuf;
    static unsigned int cnt = 0;
    if (iomapWrap.firstFlag){
        iomapWrap.firstFlag = false;
        inBuf.tx.assign_by_sz(inp);
        inBuf.rx.assign_by_sz(oup);
        printf("first init pdo\n");
        return 0;
    }
    Txpdo* inptr = (Txpdo*) inp;
	if (!(inBuf.tx == Uint8Buf{inp, inBuf.tx.size()})){
        inBuf.tx.assign_by_sz(inp);
        if (iomapWrap.outChanged){
            iomapWrap.outChanged = false;
        }
        iomapWrap.input.write(inBuf);
	}else if (iomapWrap.outChanged){
        iomapWrap.outChanged = false;
        iomapWrap.input.write(inBuf);
    }

    // do sth
    if (iomapWrap.fHandler){
        iomapWrap.fHandler(iomapWrap.pObj, &iomapWrap.input.read(), &iomapWrap.outBuf);
    }
    // after do sth
    Rxpdo& outBuf = iomapWrap.outBuf; 
	if (iomapWrap.output.read(outBuf)){
        if (inBuf.rx != outBuf){
            inBuf.rx = outBuf;
            iomapWrap.outChanged = true;
        }
	}
    memcpy(oup, outBuf.data(), outBuf.size());
    return 0;
};

static RxtxpdoWrap iomapWrap[2];

int init_txrxpdo_data(uint16_t core, uint16_t sz, uint16_t sz2){
    iomapWrap[core].init(sz, sz2);
    return 0;
}
void* get_txrxpdo(uint16_t core){
    return &iomapWrap[core];
}

int get_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val){
    return iomapWrap[core].getRxpdo(offset, size, val);
}
int get_txpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val){
    return iomapWrap[core].getTxpdo(offset, size, val);
}
int set_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val){
    return iomapWrap[core].setRxpdo(offset, size, val);
}

int get_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask){
    return iomapWrap[core].getRxpdoMask(offset, size, val, mask);
}
int get_txpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask){
    return iomapWrap[core].getTxpdoMask(offset, size, val, mask);
}

int set_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val, const uint8_t* mask){
    return iomapWrap[core].setRxpdoMask(offset, size, val, mask);
}
int set_rxpdo_data_inverse(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* mask){
    return iomapWrap[core].setRxpdoDataInverse(offset, size, mask);
}

int get_txpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t mask, uint8_t* val){
    return iomapWrap[core].getRxpdoByteMask(offset, mask, val);
}
int get_txpdo_word_mask(uint16_t core, uint16_t offset, uint16_t mask, uint16_t* val){
    return iomapWrap[core].getRxpdoWordMask(offset, mask, val);
}
int get_txpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t mask, uint32_t* val){
    return iomapWrap[core].getRxpdoDwordMask(offset, mask, val);
}
int get_rxpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t mask, uint8_t* val){
    return iomapWrap[core].getRxpdoByteMask(offset, mask, val);
}
int get_rxpdo_word_mask(uint16_t core, uint16_t offset, uint16_t mask, uint16_t* val){
    return iomapWrap[core].getRxpdoWordMask(offset, mask, val);
}
int get_rxpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t mask, uint32_t* val){
    return iomapWrap[core].getRxpdoDwordMask(offset, mask, val);
}

int set_rxpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t val, uint8_t mask){
    return iomapWrap[core].setRxpdoByteMask(offset, val, mask);
}
int set_rxpdo_word_mask(uint16_t core, uint16_t offset, uint16_t val, uint16_t mask){
    return iomapWrap[core].setRxpdoWordMask(offset, val, mask);
}
int set_rxpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t val, uint32_t mask){
    return iomapWrap[core].setRxpdoDwordMask(offset, val, mask);
}

int set_rxpdo_byte_inverse(uint16_t core, uint16_t offset, uint8_t mask){
    return iomapWrap[core].setRxpdoByteInverse(offset, mask);
}
int set_rxpdo_word_inverse(uint16_t core, uint16_t offset, uint16_t mask){
    return iomapWrap[core].setRxpdoWordInverse(offset, mask);
}
int set_rxpdo_dword_inverse(uint16_t core, uint16_t offset, uint32_t mask){
    return iomapWrap[core].setRxpdoDwordInverse(offset, mask);
}