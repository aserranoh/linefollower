
#include "dashedline2segment.hpp"

DashedLine2Segment::DashedLine2Segment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_DASHED2_TEXTURE)
{}

DashedLine2Segment::~DashedLine2Segment()
{}

