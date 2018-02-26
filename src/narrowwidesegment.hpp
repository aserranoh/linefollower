
#ifndef NARROWWIDESEGMENT_HPP
#define NARROWWIDESEGMENT_HPP

#include "straightsegment.hpp"

class NarrowWideSegment: public StraightSegment {

    public:

        NarrowWideSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~NarrowWideSegment();

};

#endif

