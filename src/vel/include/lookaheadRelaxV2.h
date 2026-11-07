#include <vector>
#include <cmath>

struct Pva{
    double pos;
    double vel;
    double acc;
    // double jerk;
};

struct Pvadj{
    double pos; // point parameter
    double vel; // point parameter, can relaxed
    double Acc; // segment parameter
    double Dec; // segment parameter
    double Jerk;// segment parameter
};

// Helper functions for reachable velocity calculations
inline static double cal_reachable_trapz_vel(double S, double V_0, double A_m) {
    return sqrt(V_0 * V_0 + A_m * 2 * S);
}

inline static double cal_reachable_dist(double V_d, double V_0, double A_m, double J) {
    double acc = sqrt(J * (V_d - V_0));
    return 2 * A_m * V_0 / J + acc * acc * acc / J / J;
}

inline static double cal_reachable_vel_s21(double S, double V_0, double A_m, double J) {
    // Simplified cube equation solver - using approximation
    double acc = pow(J * J * S / (2 * V_0 * J), 1.0/3.0);
    double V_s = V_0 + acc * acc / J;
    return V_s;
}

inline static double cal_reachable_vel_s31(double S, double V_0, double A_m, double J) {
    // Simplified quadratic equation solver - using approximation
    double a = 1;
    double b = A_m * A_m / J;
    double c = V_0 * A_m * A_m / J - V_0 * V_0 - 2 * S * A_m;
    double discriminant = b * b - 4 * a * c;
    if (discriminant >= 0) {
        double V_s = (-b + sqrt(discriminant)) / (2 * a);
        return V_s;
    }
    return V_0; // fallback
}

inline static double cal_reachable_scurve_vel(double S_s, double V_0, double A_m, double J) {
    double VM_exact = V_0 + A_m * A_m / J;
    if (cal_reachable_dist(VM_exact, V_0, A_m, J) >= S_s)
        return cal_reachable_vel_s21(S_s, V_0, A_m, J);
    else
        return cal_reachable_vel_s31(S_s, V_0, A_m, J);
}

// number of segment is N
// number of points is N+1
class LookaheadParamV2{ 
public:
    Pva begPt;
    // Pva endPt;
    double endAcc = 0;
    // constrant.end().pos === endPt.pos
    // constrant.end().vel === endPt.vel

	int size()const { return constrant.size();} //  number of segment
    void push_back(const Pvadj& pvadj){ constrant.push_back(pvadj);}
    int calc(); // return errorCode;
    
    // Getter methods for output arrays
    const std::vector<double>& getVelArr() const { return velArr; }
    const std::vector<double>& getAccArr() const { return accArr; }
    
private:
    std::vector<Pvadj> constrant;
    // output
    std::vector<double> velArr; // size=N+1
    std::vector<double> accArr; // size=N+1
    
    // Helper methods for reachable velocity calculations
    double cal_reachable_pre_vel(int idx, double V_d) const;
    double cal_reachable_pro_vel(int idx, double V_0) const;
    
    // Calculate segment distance
    double getSegmentDistance(int idx) const;
};