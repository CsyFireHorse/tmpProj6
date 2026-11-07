#ifndef _TRAJ_ENUM_DEF_H_
#define _TRAJ_ENUM_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SMC_CIRC_MODE {
    CIRC_MODE_BORDER,  //  Restriction to angles < 2π, auto decide clockwise and ingore PathChoice,
                       //  Note
    // The three points (starting point, end point, and aux point) must not be collinear.
    CIRC_MODE_CENTER,  // Restriction to angles < 2π and ≠ π
    // The user defines the end point and the center point (input AuxPoint) of the circle.
    // When using this mode, the input PathChoice defines, which of the two possible arcs is used.
    // The plane normal is the unit vector in the direction of (start - center) x (dest - center).
    // The direction of the arc is chosen according to the rule of right thumb: counter-clockwise
    // direction is along the fingers bending around the axis of the thumb which represents the
    // normal vector.
    CIRC_MODE_RADIUS  // AuxPoint is not a position but a direction vector. The radius of the
                      // circle is the length of the vector.
    // The input PathChoice defines the direction of the arc
    // Note
    // If start- and end-point are not in the plane defined by the normal vector, a helical
    // movement from start- to end-point is created. That means a circular movement in the plane
    // plus a linear movement orthogonal to the plane.

} SMC_CIRC_MODE;

#ifdef __cplusplus
}
#endif
#endif