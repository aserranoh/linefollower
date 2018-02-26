
#ifndef ZIGZAGLINESEGMENT_HPP
#define ZIGZAGLINESEGMENT_HPP

#include "straightsegment.hpp"

class ZigZagLineSegment: public StraightSegment {

    public:

        ZigZagLineSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~ZigZagLineSegment();

};

#endif

