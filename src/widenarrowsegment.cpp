
#include "widenarrowsegment.hpp"

WideNarrowSegment::WideNarrowSegment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_WIDENARROW_TEXTURE)
{}

WideNarrowSegment::~WideNarrowSegment()
{}

