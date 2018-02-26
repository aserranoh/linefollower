
#ifndef DASHEDLINE1SEGMENT_HPP
#define DASHEDLINE1SEGMENT_HPP

#include "straightsegment.hpp"

class DashedLine1Segment: public StraightSegment {

    public:

        DashedLine1Segment(const glm::vec3& position, float orientation,
            int input);
        virtual ~DashedLine1Segment();

};

#endif

