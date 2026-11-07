#include "vel/include/lookaheadRelaxV2.h"
#include <iostream>
#include <cassert>

void lookaheadV2_demo() {
    // Test basic functionality
    LookaheadParamV2 lookahead;
    
    // Set initial point
    lookahead.begPt.pos = 0.0;
    lookahead.begPt.vel = 0.0;
    lookahead.begPt.acc = 0.0;
    
    // Add constraint points
    Pvadj point1;
    point1.pos = 10.0;  // 10 units distance
    point1.vel = 5.0;   // target velocity
    point1.Acc = 2.0;   // acceleration
    point1.Dec = 2.0;   // deceleration
    point1.Jerk = 0.0;  // no jerk (trapezoidal)
    lookahead.push_back(point1);
    
    Pvadj point2;
    point2.pos = 20.0;  // 10 units distance
    point2.vel = 3.0;   // target velocity
    point2.Acc = 2.0;   // acceleration
    point2.Dec = 2.0;   // deceleration
    point2.Jerk = 0.0;  // no jerk (trapezoidal)
    lookahead.push_back(point2);
    
    // Perform calculation
    int result = lookahead.calc();
    
    if (result == 0) {
        std::cout << "Lookahead calculation successful!" << std::endl;
        
        const auto& velArr = lookahead.getVelArr();
        const auto& accArr = lookahead.getAccArr();
        
        std::cout << "Number of segments: " << lookahead.size() << std::endl;
        std::cout << "Number of points: " << velArr.size() << std::endl;
        
        for (size_t i = 0; i < velArr.size(); i++) {
            std::cout << "Point " << i << ": vel=" << velArr[i] 
                      << ", acc=" << accArr[i] << std::endl;
        }
    } else {
        std::cout << "Lookahead calculation failed with error code: " << result << std::endl;
    }
    
    // return 0;
}
