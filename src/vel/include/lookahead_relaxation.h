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
 *      varr double[]  速度数组，长度为N
 *      sarr double[]  片段距离数组，长度为N-1
 *      N   int         varr的长度
 *      acc double[]    加速度 长度为N-1
 *      dec double[]    减速度 长度为N-1
 *      jerk double[]   加加速度 长度为N-1
 *
 */
int lookahead_relaxation(double* varr, const double* sarr, int N, const double* acc,
                         const double* dec, const double* jerk);

#ifdef __cplusplus
}
#endif

#endif  // ZMC_LOOKAHEAD_RELAXATION_H_
        /* end of file */