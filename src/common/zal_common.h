/*
 * @Author: Chen Sy
 * @LastEditors: Chen Sy
 * @Date: 2022-03-24 16:01:11
 * @LastEditTime: 2022-08-04 15:14:53
 * @Description: Modify here please
 */

#ifndef _ZAL_COMMON_H_
#define _ZAL_COMMON_H_

#define EPS (1e-6)

#define LEN(x) sizeof(x)/sizeof(*(x))
#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<=(b)?(a):(b))

#define ABS(a) (((a)>=0)?(a):(-(a)))
#define SQUARE(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))
#define POWER4(x) ((x)*(x)*(x)*(x))


#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif


#endif  // _ZAL_COMMON_H_