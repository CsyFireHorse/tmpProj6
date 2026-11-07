
#include "gtest/gtest.h"
#include "FbRepeaterDbg.h"
#include "stdio.h"
// #include "FbStatUn.h"
#include "fbt_impl_enum_def.h"
#include "fbt_fb_base.hpp"

using namespace zmc;

/*
- start to done (base)
- start to error
- start to abort
- start to done (quick clear)
- start & reload
- start & idle & restart
*/
TEST(FbRepeaterDbg, base) {
    FbRepeaterDbg fb;
    fb.Count = 3;
    
    for (int i = 0; i < 15; i++){
        
        if (i==2) fb.Execute = true;
        else if (i==8) fb.Execute = false;
        else if(i==10) fb.Execute = true;
        else if(i==12) fb.TrigAbort = true;
        fb.runCycle();
    
        auto mstat = get_fb_impl_stat(fb);
        if (i==1) EXPECT_EQ(mstat, FB_STAT_IDLE);
        else if (i==2) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE); EXPECT_EQ(fb.Value, 1);}
        else if(i==4) {EXPECT_EQ(mstat, FB_STAT_DONE);  EXPECT_EQ(fb.Value, 3);}
        else if(i==6) EXPECT_EQ(mstat, FB_STAT_DONE);
        else if(i==8) EXPECT_EQ(mstat, FB_STAT_IDLE);
        else if(i==9) { EXPECT_EQ(mstat, FB_STAT_IDLE); EXPECT_EQ(fb.Value, 0);}
        else if(i==10) EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);
        // printf("%d,%d,%d\n", i, mstat, fb.Value);
    }
}

TEST(FbRepeaterDbg, quick_clear) {
    FbRepeaterDbg fb;
    fb.Count = 3;
    
    for (int i = 0; i < 10; i++){
        if (i==2) fb.Execute = true;
        else if (i==3) fb.Execute = false;
        fb.runCycle();
        auto mstat = get_fb_impl_stat(fb);
        if (i==1) {EXPECT_EQ(mstat, FB_STAT_IDLE); EXPECT_EQ(fb.Value, 0);}
        else if (i==2) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE); EXPECT_EQ(fb.Value, 1);}
        else if(i==3) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);EXPECT_EQ(fb.Value, 2);}
        else if(i==4) {EXPECT_EQ(mstat, FB_STAT_DONE); EXPECT_EQ(fb.Value, 3);}
        else if(i==5) { EXPECT_EQ(mstat, FB_STAT_IDLE); EXPECT_EQ(fb.Value, 0);}
        // printf("%d,%d,%d\n", i, mstat, fb.Value);
    }
}

TEST(FbRepeaterDbg, reload) {
    FbRepeaterDbg fb;
    fb.Count = 5;
    
    for (int i = 0; i < 10; i++){
        if (i==2) fb.Execute = true;
        else if (i==3) fb.Execute = false;
        else if(i==4) fb.Count = 7;
        else if(i==5) {
            fb.Execute = true;
        }
        fb.runCycle();
        auto mstat = get_fb_impl_stat(fb);
        if (i==1) {EXPECT_EQ(mstat, FB_STAT_IDLE); EXPECT_EQ(fb.Value, 0);}
        else if (i==2) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE); EXPECT_EQ(fb.Value, 1);}
        else if(i==3) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);EXPECT_EQ(fb.Value, 2);}
        // else if(i==4) {EXPECT_EQ(mstat, FB_MID_STAT_EXE_DONE); EXPECT_EQ(fb.getValue(), 3);}
        else if(i==5) { EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE); EXPECT_EQ(fb.Value, 4);}
        else if(i==8) { EXPECT_EQ(mstat, FB_STAT_DONE); EXPECT_EQ(fb.Value, 7);}
        // printf("%d,%d,%d\n", i, mstat, fb.Value);
    }
}


TEST(FbRepeaterDbg, error) {
    FbRepeaterDbg fb;
    fb.Count = 4;
    
    for (int i = 0; i < 8; i++){
        if (i==2) fb.Execute = true;
        else if (i==3) fb.TrigAbort = true;
        else if(i==5) fb.Execute = false;

        fb.runCycle();
        auto mstat = get_fb_impl_stat(fb);
        if (i==1) {EXPECT_EQ(mstat, FB_STAT_IDLE); EXPECT_EQ(fb.Value, 0);}
        else if (i==2) {EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE); EXPECT_EQ(fb.Value, 1);}
        else if(i==3) {
            EXPECT_EQ(mstat, FB_STAT_ABORTED);
            EXPECT_EQ(fb.Value, 2);
            // EXPECT_EQ(fb.getErrorID(), ZMC_ERR_REPDBG_TRIG_ERROR);
        }
        else if(i==5) { 
            EXPECT_EQ(mstat, FB_STAT_IDLE); 
            EXPECT_EQ(fb.Value, 0);
            // EXPECT_EQ(fb.getErrorID(), 0);
        }
        // printf("%d,%d,%d\n", i, mstat, fb.Value);
    }
}

