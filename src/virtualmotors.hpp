
#ifndef VIRTUALMOTORS_HPP
#define VIRTUALMOTORS_HPP

#include <sys/time.h>

#include "motors.hpp"
#include "virtualcamera.hpp"

/* Represents a set of two motors that move two wheels in an axis in a virtual
   environment.
*/
class VirtualMotors: public Motors {

    public:

        /* Constructor.
           Parameters:
             * camera: represents the camera (or the mobile) that the motors
                 move.
             * options: application's options.
        */
        VirtualMotors(Camera *camera, const Options& options);

        virtual ~VirtualMotors();

        /* Move the motors by an arc.
           Parameters:
             * speed: mean speed (0.0 is no movement and 1.0 is max speed).
             * turn: difference of speed between the two wheels.
        */
        virtual void move(float speed, float turn);

        // Start the motors.
        virtual void start();

        // Stop the motors
        virtual void stop();

    private:

        // Virtual camera attached to these virtual motors
        VirtualCamera *camera;

        // Maximum speed (in cm/s)
        float max_speed;

        // Time in previous invocation (used to calculate the distance to move)
        struct timespec t_prev;

};

#endif

