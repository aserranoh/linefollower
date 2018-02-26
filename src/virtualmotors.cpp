
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <math.h>

#include "utilities.hpp"
#include "virtualmotors.hpp"

#define S_PER_MIN   60

#define MIN_TURN    0.001

// TODO: Don't use gettimeofday

using namespace utilities;

/* Constructor.
   Parameters:
     * camera: represents the camera (or the mobile) that the motors
               move.
     * max_speed: maximum speed of the motors (in rpm).
     * wheel_distance: distance between wheels (in cm).
     * wheel_diameter: diameter of the wheels (in cm).
*/
VirtualMotors::VirtualMotors(VirtualCamera *camera, float max_speed,
        float wheel_distance, float wheel_diameter):
    Motors(wheel_distance), camera(camera),
    max_speed(max_speed * M_PI * wheel_diameter / S_PER_MIN)
{}

VirtualMotors::~VirtualMotors()
{}

/* Move the motors by an arc.
   Parameters:
     * speed: mean speed (0.0 is no movement and 1.0 is max speed).
     * turn: difference of speed between the two wheels.
*/
void
VirtualMotors::move(float speed, float turn)
{
    struct timeval t_current;
    float dt;
    float s1, s2;
    float radius, angle;
    glm::vec3 position, orientation, normal, center;

    // Compute the elapsed time
    gettimeofday(&t_current, 0);
    dt = (float)(t_current.tv_sec - t_prev.tv_sec
        + (t_current.tv_usec - t_prev.tv_usec) * 1e-6);
    t_prev = t_current;

    // Compute the speeds for each wheel and the mean speed
    s1 = speed - turn;
    s2 = speed + turn;
    normalize_speeds(s1, s2);
    speed = (s1 + s2)/2.0;

    // Get the position and orientation of the camera
    camera->get_position(position, orientation, normal);

    // Modify the position and orientation by a given angle respect to the
    // center of the curve
    if (turn < MIN_TURN && turn > -MIN_TURN) {
        // Straight movement
        position += orientation * dt * speed * max_speed;
    } else {
        // Compute the radius and the angle to turn
        radius = wheel_distance * speed / (s2 - s1);
        if (radius != 0.0) {
            angle = speed * max_speed * dt / radius;
        } else {
            // spin
            angle = s2 * max_speed * dt * 2.0 / wheel_distance;
        }
        // Compute the center of the rotation
        center = position - glm::cross(orientation, normal)*radius;
        // Compute the new orientation
        orientation = glm::rotate(orientation, angle, normal);
        // Compute the new position
        position = center + glm::cross(orientation, normal)*radius;
    }

    // Set the new position and orientation
    camera->set_position(position, orientation, normal);
}

// Start the motors.
void
VirtualMotors::start()
{
    gettimeofday(&t_prev, 0);
}

// Stop the motors
void
VirtualMotors::stop()
{}

