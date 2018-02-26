
#ifndef NARROWSEGMENT_HPP
#define NARROWSEGMENT_HPP

#include "straightsegment.hpp"

class NarrowSegment: public StraightSegment {

    public:

        NarrowSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~NarrowSegment();

};

#endif

