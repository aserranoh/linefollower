
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <math.h>

#include "utilities.hpp"
#include "virtualmotors.hpp"

#define S_PER_MIN   60

#define MIN_TURN    0.001

using namespace utilities;

/* Constructor.
   Parameters:
     * camera: represents the camera (or the mobile) that the motors
               move.
     * options: application's options.
*/
VirtualMotors::VirtualMotors(Camera *camera, const Options& options):
    camera((VirtualCamera*)camera),
    wheel_distance(options.get_float("WheelDistance")),
    wheel_axis_offset(options.get_float("WheelAxisOffset")),
    max_speed(options.get_float("VirtualMotorsRpm")
        * M_PI * options.get_float("WheelDiameter") / S_PER_MIN)
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
    struct timespec t_current;
    float dt, t0, t1;
    float s1, s2;
    float radius, angle;
    glm::vec3 position, orientation, normal, center;

    // Compute the elapsed time
    clock_gettime(CLOCK_MONOTONIC, &t_current);
    t0 = timespec2double(t_prev);
    t1 = timespec2double(t_current);
    dt = t1 - t0;
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
        center = position - glm::cross(orientation, normal)*radius
            + orientation*wheel_axis_offset;
        // Compute the new orientation
        orientation = glm::rotate(orientation, angle, normal);
        // Compute the new position
        position = center + glm::cross(orientation, normal)*radius
            - orientation*wheel_axis_offset;
    }

    // Set the new position and orientation
    camera->set_position(position, orientation, normal);
}

// Start the motors.
void
VirtualMotors::start()
{
    clock_gettime(CLOCK_MONOTONIC, &t_prev);
}

// Stop the motors
void
VirtualMotors::stop()
{}

