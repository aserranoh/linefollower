
#include <glm/glm.hpp>
#include <math.h>

#include "doubleturnleftsegment.hpp"

// Geometry
gl_vertex_t DoubleTurnLeftSegment::vertices[DOUBLETURN_NUM_VERTICES];
GLushort DoubleTurnLeftSegment::indices[DOUBLETURN_NUM_INDICES];

// Flag that indicates if the geometry has been defined
bool DoubleTurnLeftSegment::geometry_defined = false;

DoubleTurnLeftSegment::DoubleTurnLeftSegment(const glm::vec3& position,
        float orientation, int input):
    DoubleTurnSegment(position, orientation, input, vertices, indices)
{
    if (!geometry_defined) {
        init_geometry_static();
    }
}

DoubleTurnLeftSegment::~DoubleTurnLeftSegment()
{}

/* Get the next segment's position (the connection point)
   Parameters:
     * output: the index of the output to use.
     * pos: output position.
     * orient: output orientation.
*/
void
DoubleTurnLeftSegment::get_output(
    int output, glm::vec3& pos, float& orient) const
{
    pos[0] = position[0] + (SEGMENT_R2 + SEGMENT_R1)/2
        * (cos(orientation) + cos(orientation + M_PI/2));
    pos[1] = position[1] + (SEGMENT_R2 + SEGMENT_R1)/2
        * (sin(orientation) + sin(orientation + M_PI/2));
    pos[2] = position[2];
    orient = orientation + M_PI/2;
}

// PRIVATE FUNCTIONS

// Initialize the geometry
void
DoubleTurnLeftSegment::init_geometry_static()
{
    float a, cosa, sina;

    for (int i = 0; i <= NSEGMENTS; i++) {
        // Compute the current angle
        a = (M_PI/2) * float(i) / float(NSEGMENTS);
        cosa = cos(a);
        sina = sin(a);

        // Road vertices
        vertices[i*2].position = glm::vec3(sina * SEGMENT_R1,
            -cosa * SEGMENT_R1 + (SEGMENT_R1 + SEGMENT_R2)/2, SEGMENT_H);
        vertices[i*2].normal = glm::vec3(0, 0, 1);
        vertices[i*2].texcoord = glm::vec2(0, 0);
        vertices[i*2 + 1].position = glm::vec3(sina * SEGMENT_R2,
            -cosa * SEGMENT_R2 + (SEGMENT_R1 + SEGMENT_R2)/2, SEGMENT_H);
        vertices[i*2 + 1].normal = glm::vec3(0, 0, 1);
        vertices[i*2 + 1].texcoord = glm::vec2(1, 0);

        // wooden left side
        vertices[NSEGMENTS*2 + 2 + i*2].position = glm::vec3(sina * SEGMENT_R1,
            -cosa * SEGMENT_R1 + (SEGMENT_R1 + SEGMENT_R2)/2, 0);
        vertices[NSEGMENTS*2 + 2 + i*2].normal = glm::vec3(-sina, cosa, 0);
        vertices[NSEGMENTS*2 + 2 + i*2].texcoord = glm::vec2(i/NSEGMENTS, 0);
        vertices[NSEGMENTS*2 + 2 + i*2 + 1].position = glm::vec3(
            sina * SEGMENT_R1,
            -cosa * SEGMENT_R1 + (SEGMENT_R1 + SEGMENT_R2)/2, SEGMENT_H);
        vertices[NSEGMENTS*2 + 2 + i*2 + 1].normal = glm::vec3(-sina, cosa, 0);
        vertices[NSEGMENTS*2 + 2 + i*2 + 1].texcoord = glm::vec2(
            i/NSEGMENTS, 1);

        // wooden right side
        vertices[NSEGMENTS*4 + 4 + i*2].position = glm::vec3(sina * SEGMENT_R2,
            -cosa * SEGMENT_R2 + (SEGMENT_R1 + SEGMENT_R2)/2, SEGMENT_H);
        vertices[NSEGMENTS*4 + 4 + i*2].normal = glm::vec3(sina, -cosa, 0);
        vertices[NSEGMENTS*4 + 4 + i*2].texcoord = glm::vec2(i/NSEGMENTS, 1);
        vertices[NSEGMENTS*4 + 4 + i*2 + 1].position = glm::vec3(
            sina * SEGMENT_R2,
            -cosa * SEGMENT_R2 + (SEGMENT_R1 + SEGMENT_R2)/2, SEGMENT_H);
        vertices[NSEGMENTS*4 + 4 + i*2 + 1].normal = glm::vec3(sina, -cosa, 0);
        vertices[NSEGMENTS*4 + 4 + i*2 + 1].texcoord = glm::vec2(
            i/NSEGMENTS, 0);
    }
    for (int i = 0; i < DOUBLETURN_NUM_INDICES; i++) {
        indices[i] = i;
    }
    geometry_defined = true;
}

