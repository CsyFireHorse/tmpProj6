#ifndef _ZMC_AXIS_STORAGE_H_
#define _ZMC_AXIS_STORAGE_H_

#define AXIS_IMPL_SIZE (584)

typedef struct AxisStorage{
    char storage_[AXIS_IMPL_SIZE];
}AxisStorage;

#endif