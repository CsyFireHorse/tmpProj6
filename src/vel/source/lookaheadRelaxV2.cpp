#include "vel/include/lookaheadRelaxV2.h"

// Calculate segment distance between consecutive points
double LookaheadParamV2::getSegmentDistance(int idx) const {
    if (idx < 0 || idx >= constrant.size()) {
        return 0.0;
    }
    
    if (idx == 0) {
        // Distance from begPt to first constraint point
        return constrant[idx].pos - begPt.pos;
    } else {
        // Distance between consecutive constraint points
        return constrant[idx].pos - constrant[idx - 1].pos;
    }
}

// Calculate reachable velocity from previous point (backward calculation)
double LookaheadParamV2::cal_reachable_pre_vel(int idx, double V_d) const {
    if (idx < 0 || idx >= constrant.size()) {
        return V_d;
    }
    
    double S = getSegmentDistance(idx);
    double Dec = constrant[idx].Dec;
    double Jerk = constrant[idx].Jerk;
    
    if (Jerk < 1e-6) {
        return cal_reachable_trapz_vel(S, V_d, Dec);
    } else {
        return cal_reachable_scurve_vel(S, V_d, Dec, Jerk);
    }
}

// Calculate reachable velocity to next point (forward calculation)
double LookaheadParamV2::cal_reachable_pro_vel(int idx, double V_0) const {
    if (idx < 0 || idx >= constrant.size()) {
        return V_0;
    }
    
    double S = getSegmentDistance(idx);
    double Acc = constrant[idx].Acc;
    double Jerk = constrant[idx].Jerk;
    
    if (Jerk < 1e-6) {
        return cal_reachable_trapz_vel(S, V_0, Acc);
    } else {
        return cal_reachable_scurve_vel(S, V_0, Acc, Jerk);
    }
}

// Main calculation method implementing the relaxation algorithm
int LookaheadParamV2::calc() {
    int N = constrant.size(); // number of segments
    if (N == 0) {
        return -1; // error: no segments
    }
    
    // Initialize velocity array with N+1 points
    velArr.resize(N + 1);
    accArr.resize(N + 1);
    
    // Initialize velocities from constraint points
    velArr[0] = begPt.vel; // starting velocity
    for (int i = 0; i < N; i++) {
        velArr[i + 1] = constrant[i].vel; // constraint velocities
    }
    
    // Forward relaxation pass (from start to end)
    for (int i = 0; i < N; i++) {
        double v = cal_reachable_pro_vel(i, velArr[i]);
        if (v < velArr[i + 1] - 1e-6) {
            if (i == N - 1) {
                return -1; // error: cannot reach end point
            }
            velArr[i + 1] = v;
        } else if (v < velArr[i + 1] + 1e-6) {
            // velocities are approximately equal
        }
    }
    
    // Backward relaxation pass (from end to start)
    for (int i = N - 1; i >= 0; i--) {
        double v = cal_reachable_pre_vel(i, velArr[i + 1]);
        if (v < velArr[i] - 1e-6) {
            if (i == 0) {
                return -1; // error: cannot reach start point
            }
            velArr[i] = v;
        } else if (v < velArr[i] + 1e-6) {
            // velocities are approximately equal
        }
    }
    
    // not supported accArr[i] != 0 in this version
    for (int i = 0; i < N; i++) {
        accArr[i] = 0.0;
        // double S = getSegmentDistance(i);
        // if (S > 1e-6) {
        //     // Calculate average acceleration for the segment
        //     double velDiff = velArr[i + 1] - velArr[i];
        //     accArr[i] = velDiff * velDiff / (2 * S);
        // }
    }
    
    // Set acceleration for the last point
    accArr[N] = endAcc;
    
    return 0; // success
}
