
#include "zigzaglinesegment.hpp"

ZigZagLineSegment::ZigZagLineSegment(const glm::vec3& position,
        float orientation, int input):
    StraightSegment(position, orientation, input, ROAD_ZIGZAG_TEXTURE)
{}

ZigZagLineSegment::~ZigZagLineSegment()
{}

