
#ifndef CAMPARAMS_HPP
#define CAMPARAMS_HPP

#include <sys/types.h>

typedef struct {
    // The camera's viewport's size
    size_t width;
    size_t height;

    // The camera's field of view (in degrees)
    float fovh;
    float fovv;

    // Distance of the camera to the floor
    float cam_z;

    // Angle of the camera with the horizontal plane (in radians)
    float cam_angle;
} cam_params_t;

#endif

