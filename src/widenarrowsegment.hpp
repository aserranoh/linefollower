
#ifndef WIDENARROWSEGMENT_HPP
#define WIDENARROWSEGMENT_HPP

#include "straightsegment.hpp"

class WideNarrowSegment: public StraightSegment {

    public:

        WideNarrowSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~WideNarrowSegment();

};

#endif

