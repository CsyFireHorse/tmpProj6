/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2023-04-17 16:29:03
 * @LastEditTime: 2023-04-17 16:29:03
 * @Description: Modify here please
 */
#ifndef _ZAL_UTILS_H_
#define _ZAL_UTILS_H_
// #include <vector>

// nearly lower_bound 
inline int lower_bound_left(const double *arr, int len, double val) {
    if (val <= arr[0]) {
        return 0;
    } else if (val >= arr[len - 1]) {
        return len - 2;
    }
    int i = 1;
    for (i; i < len; i++) {
        if (val < arr[i]) return i - 1;
        i++;
    }
    return i;
}


template<typename T>
inline int findGroups(const T &seg, double val) {
    if (val <= seg.front()) {
        return 0;
    } else if (val >= seg.back()) {
        return seg.size() - 2;
    }
    int i = 0;
    for (const auto &s : seg) {
        if (val < s) return i - 1;
        i++;
    }
    return i;
}

template<typename T>
inline int rfindGroups(const T &seg, double val) {
    int N = seg.size();
    if (val >= seg[N - 1]) {
        return N - 1;
    } else if (val < seg.front()) {
        return 0;
    }
    int i = N - 1;
    for (i; i > -1; i--) {
        if (val > seg[i]) return i + 1;
    }
    return i;
}
#endif
