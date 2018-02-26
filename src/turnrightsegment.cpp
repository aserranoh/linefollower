
#include <glm/glm.hpp>
#include <math.h>

#include "turnrightsegment.hpp"

// Geometry
gl_vertex_t TurnRightSegment::vertices[TURN_NUM_VERTICES];
GLushort TurnRightSegment::indices[TURN_NUM_INDICES];

// Flag that indicates if the geometry has been defined
bool TurnRightSegment::geometry_defined = false;

TurnRightSegment::TurnRightSegment(const glm::vec3& position,
        float orientation, int input):
    TurnSegment(position, orientation, input, vertices, indices)
{
    if (!geometry_defined) {
        init_geometry_static();
    }
}

TurnRightSegment::~TurnRightSegment()
{}

/* Get the next segment's position (the connection point)
   Parameters:
     * output: the index of the output to use.
     * pos: output position.
     * orient: output orientation.
*/
void
TurnRightSegment::get_output(int output, glm::vec3& pos, float& orient) const
{
    pos[0] = position[0]
        + SEGMENT_R/2 * (cos(orientation) + cos(M_PI/2 - orientation));
    pos[1] = position[1]
        + SEGMENT_R/2 * (sin(orientation) - sin(M_PI/2 - orientation));
    pos[2] = position[2];
    orient = orientation - M_PI/2;
}

// PRIVATE FUNCTIONS

// Initialize the geometry
void
TurnRightSegment::init_geometry_static()
{
    float a, cosa, sina;

    // Corner vertex of the triangle fan (road)
    vertices[0].position = glm::vec3(0, -SEGMENT_R/2, SEGMENT_H);
    vertices[0].normal = glm::vec3(0, 0, 1);
    vertices[0].texcoord = glm::vec2(1, 0);

    for (int i = 0; i <= NTRIANGLES; i++) {
        // Compute the current angle
        a = (M_PI/2) * (1 - float(i) / float(NTRIANGLES));
        cosa = cos(a);
        sina = sin(a);

        // Perimeter vertices of the triangle fan (road)
        vertices[i + 1].position = glm::vec3(
            sina * SEGMENT_R, (cosa - 0.5) * SEGMENT_R, SEGMENT_H);
        vertices[i + 1].normal = glm::vec3(0, 0, 1);
        vertices[i + 1].texcoord = glm::vec2(0, 0);

        // Triangle strip (wood side)
        vertices[NTRIANGLES + 2 + i*2].position = glm::vec3(
            sina * SEGMENT_R, (cosa - 0.5) * SEGMENT_R, SEGMENT_H);
        vertices[NTRIANGLES + 2 + i*2].normal = glm::vec3(sina, cosa, 0);
        vertices[NTRIANGLES + 2 + i*2].texcoord = glm::vec2(i/NTRIANGLES, 0);

        vertices[NTRIANGLES + 3 + i*2].position = glm::vec3(
            sina * SEGMENT_R, (cosa - 0.5) * SEGMENT_R, 0);
        vertices[NTRIANGLES + 3 + i*2].normal = glm::vec3(sina, cosa, 0);
        vertices[NTRIANGLES + 3 + i*2].texcoord = glm::vec2(i/NTRIANGLES, 1);
    }
    for (int i = 0; i < TURN_NUM_INDICES; i++) {
        indices[i] = i;
    }
    geometry_defined = true;
}

