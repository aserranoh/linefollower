
#include "pilot.hpp"

#include <math.h>

Pilot::Pilot()
{}

/* Constructor.
   Parameters:
     * motors: the motors to control.
     * options: application's options.
*/
Pilot::Pilot(Motors *motors, const Options& options):
    motors(motors), kp(options.get_float("Kp")), ki(options.get_float("Ki")),
    kd(options.get_float("Kd")), kspeed(options.get_float("KSpeed")),
    max_speed(options.get_float("MaxSpeed")), sum_error(0.0), prev_error(0.0)
{}

/* Pilot the motors to follow the line.
   Parameters:
     * line: the line to follow.
*/
void
Pilot::pilot(const Line& line)
{
    float speed, turn;

    speed = compute_speed(line);
    turn = compute_turn(line);

    // Give the order to the motors
    motors->move(speed, turn);
}

// PRIVATE FUNCTIONS

/* Compute the speed to use depending on the line geometrics.
   For each point, a little of speed may be substracted of the final speed,
   according the following formula:
     * The more the angle towards the point the more the speed is substracted.
     * The more distance to the point the less speed is substracted.
   Parameters:
     * line: the line to follow.
*/
float
Pilot::compute_speed(const Line& line)
{
    // Find the angle against the last point in the line
    const line_point_t& p = line.get_point(line.size() - 1);
    float angle = acos(p.wx/sqrt(p.wx*p.wx + p.wy*p.wy));
    float speed = (1.0 - kspeed * abs(angle - M_PI/2.0)) * max_speed;
    return speed;
}

/* Compute the turn to use depending on the line geometrics.
   To calculate the turn, a PID formula is used using as error the x value of
   the first point of the line.
   Parameters:
     * line: the line to follow.
*/
float
Pilot::compute_turn(const Line& line)
{
    float turn;
    float error = -line.get_point(0).wx;

    turn = kp*error + ki*sum_error + kd*(error - prev_error);
    // Update the PID state
    sum_error += error;
    prev_error = error;
    return turn;
}

