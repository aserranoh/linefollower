
#include "narrowsegment.hpp"

NarrowSegment::NarrowSegment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_NARROW_TEXTURE)
{}

NarrowSegment::~NarrowSegment()
{}

