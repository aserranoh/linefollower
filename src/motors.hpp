
#ifndef MOTORS_HPP
#define MOTORS_HPP

class Motors {

    public:

        /* Constructor.
           Params:
             * wheel_distance: distance between wheels (in cm).
        */
        Motors(float wheel_distance);

        virtual ~Motors();

        /* Move the motors by an arc.
           Parameters:
             * speed: mean speed (0.0 is no movement and 1.0 is max speed).
             * turn: difference of speed between the two wheels.
        */
        virtual void move(float speed, float turn) = 0;

        // Start the motors.
        virtual void start() = 0;

        // Stop the motors
        virtual void stop() = 0;

    protected:

        // Distance between wheels
        float wheel_distance;

        /* Make sure that the speeds are no greater than 1.0.
           At exit, the speeds s1 and s2 ar less or equal than 1.0 and their
           ratio is the same as at the input.
           Parameters:
             * s1: speed for the first wheel.
             * s2: speed for the second wheel.
        */
        void normalize_speeds(float& s1, float& s2);

};

#endif

