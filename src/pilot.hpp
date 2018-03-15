
#ifndef PILOT_HPP
#define PILOT_HPP

#include "line.hpp"
#include "motors.hpp"

class Pilot {

    public:

        Pilot();

        /* Constructor.
           Parameters:
             * motors: the motors to control.
             * options: application's options.
        */
        Pilot(Motors *motors, const Options& options);

        /* Pilot the motors to follow the line.
           Parameters:
             * line: the line to follow.
        */
        void pilot(const Line& line);

    private:

        // The motors to control
        Motors *motors;

        // Speed regulation constants
        float kspeed;
        float max_speed;

        // PID constants
        float kp;
        float ki;
        float kd;

        // State of the PID algorithm
        float sum_angle;
        float prev_angle;

        /* Compute the speed to use depending on the line geometrics.
           For each point, a little of speed may be substracted of the final
           speed, according the following formula:
             * The more the angle towards the point the more the speed is
                 substracted.
             * The more distance to the point the less speed is substracted.
           Parameters:
             * line: the line to follow.
        */
        float compute_speed(const Line& line);

        /* Compute the turn to use depending on the line geometrics.
           To calculate the turn, a PID formula is used using as error the
           angle towards the first point of the line.
           Parameters:
             * line: the line to follow.
        */
        float compute_turn(const Line& line);

};

#endif

