
#ifndef DASHEDLINE2SEGMENT_HPP
#define DASHEDLINE2SEGMENT_HPP

#include "straightsegment.hpp"

class DashedLine2Segment: public StraightSegment {

    public:

        DashedLine2Segment(const glm::vec3& position, float orientation,
            int input);
        virtual ~DashedLine2Segment();

};

#endif

