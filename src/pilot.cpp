
#include "pilot.hpp"

#include <stdio.h>

Pilot::Pilot()
{}

/* Constructor.
   Parameters:
     * motors: the motors to control.
     * max_speed: maximum speed to give to the motors.
     * kp: PID proportional constant.
     * ki: PID integrative constant.
     * kd: PID derivative constant.
*/
Pilot::Pilot(Motors *motors, float max_speed, float kp, float ki, float kd):
    motors(motors), max_speed(max_speed), kp(kp), ki(ki), kd(kd),
    sum_angle(0.0), prev_angle(0.0)
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

