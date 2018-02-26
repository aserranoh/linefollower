
#include <glm/gtc/matrix_transform.hpp>

#include "squaresegment.hpp"

SquareSegment::SquareSegment(const glm::vec3& position, float orientation,
        int input, size_t num_vertices, size_t num_indices):
    TrackSegment(position, orientation, input, num_vertices, num_indices)
{}

SquareSegment::~SquareSegment()
{}

/* Return true if this segment contains the projection of point.
   https://math.stackexchange.com/questions/190111/how-to-check-if-a-point-is-inside-a-rectangle
*/
bool
SquareSegment::contains(const glm::vec3& point) const
{
    glm::vec2 am(glm::vec2(point) - a);
    float am_ab = dot(am, ab);
    float am_ad = dot(am, ad);
    return am_ab > 0 && am_ab < ab_ab && am_ad > 0 && am_ad < ad_ad;
}

/* Set the corners that delimite this square.
   Parameters:
     * a: first corner.
     * b: second corner.
     * d: third corner.
*/
void
SquareSegment::set_corners(
    const glm::vec3& a, const glm::vec3& b, const glm::vec3& d)
{
    this->a = glm::vec2(a);
    ab = glm::vec2(b) - this->a;
    ad = glm::vec2(d) - this->a;
    ab_ab = dot(ab, ab);
    ad_ad = dot(ad, ad);
}

