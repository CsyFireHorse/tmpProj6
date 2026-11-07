#ifndef MATH_GCD_HPP
#define MATH_GCD_HPP

namespace zmc{
inline int _gcd(int x, int y){
   while (y != 0){
      int temp = y;
      y = x % y;
      x = temp;
   }
   return x;
}
inline int gcd(int a, int b){
    if (a >= b){
        return _gcd(a, b);
    }else{
        return _gcd(b, a);
    }
}
inline int lcm(int a, int b){ return a*b / gcd(a, b); }

}
#endif