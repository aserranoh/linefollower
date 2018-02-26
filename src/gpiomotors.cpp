
#include "gpiomotors.hpp"

#include <err.h>

#include "followexception.hpp"

/* Constructor.
   Params:
     * pwm_left: PWM channel for the left motor.
     * pwm_right: PWM channel for the right motor.
     * dir0_left: GPIO direction pin 0 for the left motor.
     * dir1_left: GPIO direction pin 1 for the left motor.
     * dir0_right: GPIO direction pin 0 for the right motor.
     * dir1_right: GPIO direction pin 1 for the right motor.
     * pwm_period: period of the PWM signal (in ns).
     * wheel_distance: distance between wheels (in cm).
*/
GPIOMotors::GPIOMotors(pwm_channel_t pwm_left, pwm_channel_t pwm_right,
        gpio_pin_t dir0_left, gpio_pin_t dir1_left, gpio_pin_t dir0_right,
        gpio_pin_t dir1_right, pwm_time_t pwm_period, float wheel_distance):
    Motors(wheel_distance), pwm_period(pwm_period)
{
    direction0_left.pin = dir0_left;
    direction1_left.pin = dir1_left;
    direction0_left.flags = direction1_left.flags = 0;
    direction0_right.pin = dir0_right;
    direction1_right.pin = dir1_right;
    direction0_right.flags = direction1_right.flags = 0;
    this->pwm_left.chip = this->pwm_right.chip = 0;
    this->pwm_left.channel = pwm_left;
    this->pwm_right.channel = pwm_right;
    this->pwm_left.flags = this->pwm_right.flags = 0;
    this->pwm_left.period = this->pwm_right.period = pwm_period;
    try {
        if (rfs_gpio_open(&direction0_left, RFS_GPIO_OUT_LOW)) {
            warn("initializing direction0_left");
            throw FollowException("cannot initialize left motor");
        }
        if (rfs_gpio_open(&direction1_left, RFS_GPIO_OUT_LOW)) {
            warn("initializing direction1_left");
            throw FollowException("cannot initialize left motor");
        }
        if (rfs_gpio_open(&direction0_right, RFS_GPIO_OUT_LOW)) {
            warn("initializing direction0_right");
            throw FollowException("cannot initialize right motor");
        }
        if (rfs_gpio_open(&direction1_right, RFS_GPIO_OUT_LOW)) {
            warn("initializing direction1_right");
            throw FollowException("cannot initialize right motor");
        }
        if (rfs_pwm_open(&(this->pwm_left))) {
            warn("initializing pwm_left");
            throw FollowException("cannot initialize left motor");
        }
        if (rfs_pwm_open(&(this->pwm_right))) {
            warn("initializing pwm_right");
            throw FollowException("cannot initialize right motor");
        }
    } catch (FollowException &e) {
        this->close();
        throw;
    }
}

GPIOMotors::~GPIOMotors()
{
    this->close();
}

/* Move the motors by an arc.
   Parameters:
     * speed: mean speed (0.0 is no movement and 1.0 is max speed). Negative
         values of speed inverse the movement of the wheels.
     * turn: difference of speed between the two wheels. A positive value means
         a turn towards the left.
*/
void
GPIOMotors::move(float speed, float turn)
{
    float s1, s2;

    // Compute the speeds for each wheel
    s1 = speed - turn;
    s2 = speed + turn;
    normalize_speeds(s1, s2);

    // Send the orders to the motors driver
    set_speed_motor(pwm_left, direction0_left, direction1_left, s1);
    set_speed_motor(pwm_right, direction0_right, direction1_right, s2);
}

// PRIVATE FUNCTIONS

// Close the GPIO pins and PWM channels.
void
GPIOMotors::close()
{
    rfs_gpio_close(&direction0_left);
    rfs_gpio_close(&direction1_left);
    rfs_gpio_close(&direction0_right);
    rfs_gpio_close(&direction1_right);
    rfs_pwm_close(&pwm_left);
    rfs_pwm_close(&pwm_right);
}

/* Set the speed of a motor.
   Parameters:
     * pwm: PWM object of one of the motors.
     * dir0: GPIO object for direction 0 of the motor.
     * dir1: GPIO object for direction 1 of the motor.
     * speed: motor's speed (float between 0 and 1).
*/
void
GPIOMotors::set_speed_motor(struct pwm_t& pwm, struct gpio_t& dir0,
    struct gpio_t& dir1, float speed)
{
    if (speed == 0.0) {
        rfs_gpio_set_value(&dir0, RFS_GPIO_LOW);
        rfs_gpio_set_value(&dir1, RFS_GPIO_LOW);
        rfs_pwm_set_duty_cycle(&pwm, 0);
    } else if (speed > 0.0) {
        rfs_gpio_set_value(&dir0, RFS_GPIO_HIGH);
        rfs_gpio_set_value(&dir1, RFS_GPIO_LOW);
        rfs_pwm_set_duty_cycle(&pwm, speed * pwm_period);
    } else {
        rfs_gpio_set_value(&dir0, RFS_GPIO_LOW);
        rfs_gpio_set_value(&dir1, RFS_GPIO_HIGH);
        rfs_pwm_set_duty_cycle(&pwm, speed * pwm_period);
    }
}

// Start the motors.
void
GPIOMotors::start()
{}

// Stop the motors
void
GPIOMotors::stop()
{}

