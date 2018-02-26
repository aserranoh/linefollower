
#include "narrowwidesegment.hpp"

NarrowWideSegment::NarrowWideSegment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_NARROWWIDE_TEXTURE)
{}

NarrowWideSegment::~NarrowWideSegment()
{}

