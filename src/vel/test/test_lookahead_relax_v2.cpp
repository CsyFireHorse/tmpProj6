#include <gtest/gtest.h>
#include "lookaheadRelaxV2.h"

// Keep this parameter type an aggregate for MSVC2015 C++11 initialization.
template<int N>
struct LookaheadV2TestParams{
    Pva begPt;
    double endAcc;
    Pvadj pvadjArr[N];
    double expectVelArr[N + 1];
    double expectAccArr[N + 1];
    inline int len() const { return N; }
};

template <typename T>
void _lookahead_v2_test(T& par) {
    LookaheadParamV2 lookahead;
    lookahead.begPt = par.begPt;
    lookahead.endAcc = par.endAcc;
    
    for (int i = 0; i < par.len(); i++) {
        lookahead.push_back(par.pvadjArr[i]);
    }
    
    int ret = lookahead.calc();
    EXPECT_EQ(ret, 0);
    
    const auto& velArr = lookahead.getVelArr();
    const auto& accArr = lookahead.getAccArr();
    
    for (int i = 0; i < par.len() + 1; i++) {
        EXPECT_NEAR(velArr[i], par.expectVelArr[i], 1e-5);
        EXPECT_NEAR(accArr[i], par.expectAccArr[i], 1e-5);
    }
}

class TestLookaheadRelaxV2 : public testing::Test {
public:
};

TEST_F(TestLookaheadRelaxV2, ladder) {
    LookaheadV2TestParams<5> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{1.0, 1.0, 1.0, 1.0, 0.0}, {3.0, 3, 1.0, 1.0, 0.0}, {6.0, 1, 1.0, 1.0, 0.0}, {8.0, 2.0, 1.0, 1.0, 0.0}, {9.0, 0, 1.0, 1.0, 0.0}},
        {0.0, 1.0, 2.236068, 1.0, 1.414214, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

TEST_F(TestLookaheadRelaxV2, ladder2) {
    LookaheadV2TestParams<5> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{1.0, 1.0, 3.0, 3.0, 0.0}, {3.0, 3.0, 3.0, 3.0, 0.0}, {6.0, 1.0, 3.0, 3.0, 0.0}, {8.0, 2.0, 3.0, 3.0, 0.0}, {9.0, 0.0, 3.0, 3.0, 0.0}},
        {0.0, 1.0, 3.0, 1.0, 2.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

TEST_F(TestLookaheadRelaxV2, sladder) {
    LookaheadV2TestParams<5> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{1.0, 1.0, 1.0, 1.0, 10.0}, {3.0, 2.1641589825484533, 1.0, 1.0, 10.0}, {6.0, 1.0, 1.0, 1.0, 10.0}, {8.0, 1.3650971698084906, 1.0, 1.0, 10.0}, {9.0, 0.0, 1.0, 1.0, 10.0}},
        {0.0, 1.0, 2.1641589825484533, 1.0, 1.3650971698084906, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

TEST_F(TestLookaheadRelaxV2, sladder2) {
    LookaheadV2TestParams<6> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{34.0, 10.0, 5.0, 5.0, 8.0}, {61.0, 4.0, 5.0, 5.0, 8.0}, {96.0, 14.0, 5.0, 5.0, 8.0}, {136.0, 18.0, 5.0, 5.0, 8.0}, {158.0, 16.0, 5.0, 5.0, 8.0}, {171.0, 0.0, 5.0, 5.0, 8.0}},
        {0.0, 10.0, 4.0, 14.0, 15.47510656571184, 9.945819001922045, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

TEST_F(TestLookaheadRelaxV2, sladder4) {
    LookaheadV2TestParams<4> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{10.0, 10.0, 3.0, 3.0, 20.0}, {20.0, 10.0, 3.0, 3.0, 20.0}, {30.0, 10.0, 3.0, 3.0, 20.0}, {40.0, 0.0, 3.0, 3.0, 20.0}},
        {0.0, 7.524233833096018, 10.0, 7.524233833096018, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

TEST_F(TestLookaheadRelaxV2, sladder5) {
    LookaheadV2TestParams<4> par = {
        {0.0, 0.0, 0.0}, 0.0,
        {{10.0, 10.0, 3.0, 3.0, 5.0}, {20.0, 10.0, 3.0, 3.0, 5.0}, {30.0, 10.0, 3.0, 3.0, 5.0}, {40.0, 0.0, 3.0, 3.0, 5.0}},
        {0.0, 6.898076685952761, 8.896781304620923, 6.898076685952761, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0}
    };
    _lookahead_v2_test(par);
}

// AI modification signature: GPT-5 Codex
