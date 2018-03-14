
#include "pilot.hpp"

Pilot::Pilot()
{}

/* Constructor.
   Parameters:
     * motors: the motors to control.
     * options: application's options.
*/
Pilot::Pilot(Motors *motors, const Options& options):
    motors(motors), max_speed(options.get_float("MaxSpeed")),
    kp(options.get_float("Kp")), ki(options.get_float("Ki")),
    kd(options.get_float("Kd")), sum_angle(0.0), prev_angle(0.0)
{}

/* Set the new angle that the vehicle must turn.
   Parameters:
     * angle: the angle to turn.
*/
void
Pilot::set_angle(float angle)
{
    float turn;

    // Calculate the turn value to use for the motors (PID formula)
    turn = kp*angle + ki*sum_angle + kd*(angle - prev_angle);

    // Give the order to the motors
    motors->move(max_speed, turn);

    // Update the PID state
    sum_angle += angle;
    prev_angle = angle;
}

