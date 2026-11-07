/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-06-16 14:40:30
 * @LastEditTime: 2022-07-21 15:55:51
 */
#include <gtest/gtest.h>

#include "lookahead_relaxation.h"

#define LEN(x) sizeof(x) / sizeof(*(x))

class TestLookaheadRelax : public testing::Test {
   public:
};

template <int N>
struct LookaheadTestParams {
    double sArr[N];
    double accArr[N];
    double decArr[N];
    double jerkArr[N];
    double vArr[N + 1];
    double vRlArr[N + 1];
    inline int len() const { return N; }
};

template <typename T>
void _lookahead_test(T& par) {
    int ret = lookahead_relaxation(par.vArr, par.sArr, par.len(), par.accArr, par.decArr, par.jerkArr);
    EXPECT_EQ(ret, 0);
    for (int i = 0; i < par.len() + 1; i++) {
        EXPECT_NEAR(par.vArr[i], par.vRlArr[i], 1e-5);
    }
}

TEST_F(TestLookaheadRelax, ladder) {
    LookaheadTestParams<5> par = {{1, 2, 3, 2, 1},    {1, 1, 1, 1, 1},
                                  {1, 1, 1, 1, 1},    {0, 0, 0, 0, 0},
                                  {0, 1, 3, 1, 2, 0}, {0.0, 1.0, 2.236068, 1.0, 1.414214, 0.0}};
    _lookahead_test(par);
}

TEST_F(TestLookaheadRelax, ladder2) {
    LookaheadTestParams<5> par = {{1, 2, 3, 2, 1},    {3, 3, 3, 3, 3},
                                  {3, 3, 3, 3, 3},    {0, 0, 0, 0, 0},
                                  {0, 1, 3, 1, 2, 0}, {0.0, 1.0, 3.0, 1.0, 2.0, 0.0}};
    _lookahead_test(par);
}

TEST_F(TestLookaheadRelax, sladder) {
    LookaheadTestParams<5> par = {
        {1, 2, 3, 2, 1},    {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},    {10, 10, 10, 10, 10},
        {0, 1, 3, 1, 2, 0}, {0, 1, 2.1641589825484533, 1, 1.3650971698084906, 0}};
    _lookahead_test(par);
}

TEST_F(TestLookaheadRelax, sladder2) {
    LookaheadTestParams<6> par = {
        {34, 27, 35, 40, 22, 13},  {5, 5, 5, 5, 5, 5},
        {5, 5, 5, 5, 5, 5},        {8, 8, 8, 8, 8, 8},
        {0, 10, 4, 14, 18, 16, 0}, {0, 10, 4, 14, 15.47510656571184, 9.945819001922045, 0}};
    _lookahead_test(par);
}

TEST_F(TestLookaheadRelax, sladder4) {
    LookaheadTestParams<4> par = {
        {10, 10, 10, 10}, {3, 3, 3, 3},       {3, 3, 3, 3},
        {20, 20, 20, 20}, {0, 10, 10, 10, 0}, {0, 7.524233833096018, 10, 7.524233833096018, 0}};
    _lookahead_test(par);
}
TEST_F(TestLookaheadRelax, sladder5) {
    LookaheadTestParams<4> par = {
        {10, 10, 10, 10},   {3, 3, 3, 3},
        {3, 3, 3, 3},       {5, 5, 5, 5},
        {0, 10, 10, 10, 0}, {0, 6.898076685952761, 8.896781304620923, 6.898076685952761, 0}};
    _lookahead_test(par);
}
