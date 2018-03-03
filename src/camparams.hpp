
#ifndef CAMPARAMS_HPP
#define CAMPARAMS_HPP

#include <math.h>
#include <sys/types.h>

#define MIN_CAMANGLE    0.0
#define MAX_CAMANGLE    M_PI / 2.0

typedef struct cam_params_s {
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

    cam_params_s(size_t width, size_t height, float fovh, float fovv,
            float cam_z, float cam_angle):
        width(width), height(height), fovh(fovh), fovv(fovv), cam_z(cam_z),
        cam_angle(cam_angle)
    {}

} cam_params_t;

#endif

