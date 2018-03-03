
#ifndef GPIOMOTORS_HPP
#define GPIOMOTORS_HPP

#ifdef __cplusplus
extern "C"{
#endif
#include "rfsgpio.h"
#ifdef __cplusplus
}
#endif

#include "motors.hpp"

class GPIOMotors: public Motors {

    public:

        /* Constructor.
           Params:
             * options: application's options.
        */
        GPIOMotors(const Options& options);

        virtual ~GPIOMotors();

        /* Move the motors by an arc.
           Parameters:
             * speed: mean speed (0.0 is no movement and 1.0 is max speed).
                 Negative values of speed inverse the movement of the wheels.
             * turn: difference of speed between the two wheels. A positive
                 value means a turn towards the left.
        */
        virtual void move(float speed, float turn);

        // Start the motors.
        virtual void start();

        // Stop the motors
        virtual void stop();

    private:

        // Close the GPIO pins and PWM channels.
        void close();

        /* Set the speed of a motor.
           Parameters:
             * pwm: PWM object of one of the motors.
             * dir0: GPIO object for direction 0 of the motor.
             * dir1: GPIO object for direction 1 of the motor.
             * speed: motor's speed (float between 0 and 1).
        */
        void set_speed_motor(struct pwm_t& pwm, struct gpio_t& dir0,
            struct gpio_t& dir1, float speed);

        // PWM drivers for each motor
        struct pwm_t pwm_left;
        struct pwm_t pwm_right;

        // GPIO pins that control the directon of each motor
        struct gpio_t direction0_left;
        struct gpio_t direction1_left;
        struct gpio_t direction0_right;
        struct gpio_t direction1_right;

        // Period of the PWM signal
        pwm_time_t pwm_period;

};

#endif

