#ifndef _ZMC_ENUM_CDEF_H_
#define _ZMC_ENUM_CDEF_H_

// ENUM
typedef short ZMC_DIRECTION;
typedef unsigned short ZMC_BUFFER_MODE;
typedef unsigned short ZMC_STARTMODE;
typedef unsigned short ZMC_ERROR;
typedef unsigned short ZMC_HOMING_MODE;
typedef unsigned short ZMC_TAPPETMODE;
typedef unsigned short ZMC_RAMPTYPE;
typedef unsigned short ZMC_CONTROLLER_MODE;
typedef unsigned short ZMC_RAMPTYPE;

typedef unsigned short ZMC_VALUESELECT;

enum ZMC_BUFFER_MODE_U32{
    /**
     * @brief Start FB immediately (default mode).
     * The next FB aborts an ongoing motion and the command
     * affects the axis immediately. The buffer is cleared.
     */
    mcAborting = 0,
    /**
     * @brief Start FB after current motion has finished.
     * The next FB affects the axis as soon as the previous movement is 'Done'.
     * There is no blending.
     */
    mcBuffered = 1,
    /**
     * @brief The velocity is blended with the lowest velocity of both FBs.
     *
     */
    mcBlendingLow = 2,       // The velocity is blended with the lowest velocity of both FBs
    mcBlendingPrevious = 3,  // The velocity is blended with the velocity of the first FB
    mcBlendingNext = 4,      // The velocity is blended with velocity of the second FB
    mcBlendingHigh = 5       // The velocity is blended with highest velocity of both FBs
};


enum ZMC_DIRECTION_I32 {
    mcNegativeDirection = -1,
    mcShortestWay = 0,  // only modulo axes
    mcPositiveDirection,
    mcCurrentDirection,  // only modulo axes
    mcFastestWay,        // only modulo axes (codesys only)
    mcFiniteDirection = 4,  // MoveAbsolute uses the target as a finite-axis position
};

enum ZMC_STARTMODE_U32 {
    mcAbsolute,
    mcRelative,
    mcRampIn,
    mcRampInPos,  // codesys only
    mcRampInNeg,  // codesys only
};

enum ZMC_HOMING_MODE_U32 {
    FAST_BSLOW_S_STOP =
        0,  // Move with fast velocity onto the reference switch; invert and move with slow
            // velocity out of the reference switch; execute "set position"; stop
    FAST_BSLOW_STOP_S =
        1,  // Move with fast velocity onto the reference switch; invert and move with slow
            // velocity out of the reference switch; stop; execute "set position"
    FAST_BSLOW_I_S_STOP = 2,  // Move with fast velocity onto the reference switch; invert and move
                              // with slow velocity out of the reference switch; wait for the index
                              // pulse; execute "set position"; stop
    FAST_SLOW_S_STOP = 4,     // Move with fast velocity onto the reference switch; move with slow
                           // velocity out of the reference switch; execute "set position"; stop
    FAST_SLOW_STOP_S = 5,  // Move with fast velocity onto the reference switch; move with slow
                           // velocity out of the reference switch; stop; execute "set position"
    FAST_SLOW_I_S_STOP =
        6,  // Move with fast velocity onto the reference switch; move with slow velocity out of
            // the reference switch; wait for the index pulse; execute "set position"; stop

    // SLOW_STOP_S,
    // SLOW_S_STOP,
    // DIRECT_S,
};

enum ZMC_TAPPETMODE_U32 {
    tp_mode_auto,            //  0 Auto mode
    tp_mode_demandposition,  // Use set values
    tp_mode_actualposition,  // Use actual values
};

#endif
