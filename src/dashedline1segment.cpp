
#include "dashedline1segment.hpp"

DashedLine1Segment::DashedLine1Segment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_DASHED1_TEXTURE)
{}

DashedLine1Segment::~DashedLine1Segment()
{}

