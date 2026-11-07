#ifndef ZECM_ENUM_DEF_H_
#define ZECM_ENUM_DEF_H_



enum _EcatState {
    EcatStateNotSet = 0,
    EcatStateI = 1,
    EcatStateP = 2,
    EcatStateB = 3,
    EcatStateS = 4,
    EcatStateO = 8,
    EcatStateMask = 15,
    EcatStateErrorFlag = 16,
    EcatStateII = 129,
    EcatStatePP = 130,
    EcatStateSS = 132,
    InternalEcatStateMask = 128
};
typedef enum _EcatState EcatState;

#endif