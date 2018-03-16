
#include "motors.hpp"

Motors::~Motors()
{}

/* Make sure that the speeds are no greater than 1.0.
   At exit, the speeds s1 and s2 ar less or equal than 1.0 and their
   ratio is the same as at the input.
   Parameters:
     * s1: speed for the first wheel.
     * s2: speed for the second wheel.
*/
void
Motors::normalize_speeds(float& s1, float& s2)
{
    if (s1 > s2) {
        if (s1 > 1.0) {
            s2 /= s1;
            s1 = 1.0;
        }
    } else {
        if (s2 > 1.0) {
            s1 /= s2;
            s2 = 1.0;
        }
    }
}

