/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-06-13 17:50:40
 * @LastEditTime: 2022-06-16 14:26:49
 * @Description: Modify here please
 */

#ifndef ZMC_LOOKAHEAD_RELAXATION_H_
#define ZMC_LOOKAHEAD_RELAXATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @params
 *      varr double[]  Velocity array, length N.
 *      sarr double[]  Segment distance array, length N-1.
 *      N   int         Length of varr.
 *      acc double[]    Acceleration array, length N-1.
 *      dec double[]    Deceleration array, length N-1.
 *      jerk double[]   Jerk array, length N-1.
 *
 */
int lookahead_relaxation(double* varr, const double* sarr, int N, const double* acc,
                         const double* dec, const double* jerk);

#ifdef __cplusplus
}
#endif

#endif  // ZMC_LOOKAHEAD_RELAXATION_H_
        /* end of file */
