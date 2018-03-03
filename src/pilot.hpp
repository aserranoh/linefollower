
#ifndef PILOT_HPP
#define PILOT_HPP

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

        /* Set the new angle that the vehicle must turn.
           Parameters:
             * angle: the angle to turn.
        */
        void set_angle(float angle);

    private:

        // The motors to control
        Motors *motors;

        // Maximum speed to use
        float max_speed;

        // PID constants
        float kp;
        float ki;
        float kd;

        // State of the PID algorithm
        float sum_angle;
        float prev_angle;

};

#endif

