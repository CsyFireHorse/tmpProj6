#include "vel/VelWrapCommon.h"

namespace zmc {

template <typename T>
struct XValueAccessor;


template <>
struct XValueAccessor<double> {
    static double get(const double* arr, int index) {
        return arr[index];
    }
};


template <>
struct XValueAccessor<Xyva> {
    static double get(const Xyva* arr, int index) {
        return arr[index].x;
    }
};

template <typename T>
int findDoubledIndex(double x, const T* arr, int nPoints) {
// int findDoubledIndex(double x, double * arr, int nPoints)
    constexpr double POINT_TOLERANCE = 1e-6; 

    if (x <= XValueAccessor<T>::get(arr, 0)) {
        return 0;
    }
    if (x >= XValueAccessor<T>::get(arr, nPoints-1)) {
        return 2*nPoints-2;
    }
    for (int i = 0; i < nPoints-1; i++) {
        if (x < XValueAccessor<T>::get(arr, i+1)){
            if (x - XValueAccessor<T>::get(arr, i) > POINT_TOLERANCE) {
                return 2*i + 1; 
            }else 
                return 2*i;
        }
    }
    return -1;
}

}