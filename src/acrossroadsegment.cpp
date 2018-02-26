
#include <math.h>

#include "acrossroadsegment.hpp"

#define ACROSS_NUM_VERTICES   8
#define ACROSS_NUM_INDICES    8

// Geometry
const gl_vertex_t ACrossroadSegment::vertices[] = {
    // road
    {{0        ,  SEGMENT_W/2, SEGMENT_H}, { 0, 0, 1}, {0, 0}},
    {{0        , -SEGMENT_W/2, SEGMENT_H}, { 0, 0, 1}, {1, 0}},
    {{SEGMENT_L,  SEGMENT_W/2, SEGMENT_H}, { 0, 0, 1}, {0, 1}},
    {{SEGMENT_L, -SEGMENT_W/2, SEGMENT_H}, { 0, 0, 1}, {1, 1}},
    // side
    {{0        ,  SEGMENT_W/2, 0        }, {-1, 0, 0}, {0, 0}},
    {{0        , -SEGMENT_W/2, 0        }, {-1, 0, 0}, {1, 0}},
    {{0        ,  SEGMENT_W/2, SEGMENT_H}, {-1, 0, 0}, {0, 1}},
    {{0        , -SEGMENT_W/2, SEGMENT_H}, {-1, 0, 0}, {1, 1}},
};

const GLushort ACrossroadSegment::indices[] = {
    0, 1, 2, 3,
    4, 5, 6, 7
};

ACrossroadSegment::ACrossroadSegment(const glm::vec3& position,
        float orientation, int input):
    SquareSegment(position, orientation, input, ACROSS_NUM_VERTICES,
        ACROSS_NUM_INDICES)
{
    if (input == 0) {
        this->position[0] += (SEGMENT_W/2 * cos(orientation)
            + SEGMENT_L/2 * cos(M_PI/2 - orientation));
        this->position[1] += (SEGMENT_W/2 * sin(orientation)
            - SEGMENT_L/2 * sin(M_PI/2 - orientation));
        this->orientation += M_PI/2;
    } else {
        this->position[0] += (SEGMENT_W/2 * cos(orientation)
            + SEGMENT_L/2 * cos(orientation + M_PI/2));
        this->position[1] += (SEGMENT_W/2 * sin(orientation)
            + SEGMENT_L/2 * sin(orientation + M_PI/2));
        this->orientation -= M_PI/2;
    }
}

ACrossroadSegment::~ACrossroadSegment()
{}

/* Get the next segment's position (the connection point)
   Parameters:
     * output: the index of the output to use.
     * pos: output position.
     * orient: output orientation.
*/
void
ACrossroadSegment::get_output(int output, glm::vec3& pos, float& orient) const
{
    pos = position
        + glm::vec3(cos(orientation), sin(orientation), 0) * SEGMENT_L;
    orient = orientation;
}

/* Initialize the geometry.
   Parameters:
     * first_vertex: index in the OpenGL Vertex Buffer for the first
         vertex of this segment.
     * first_index: index in the OpenGL Index Buffer for the first
         index of this segment.
     * context: OpenGL objects.
*/
void
ACrossroadSegment::init_geometry(size_t first_vertex, size_t first_index,
    const gl_context_t& context)
{
    gl_vertex_t tr_vertices[ACROSS_NUM_VERTICES];
    GLushort tr_indices[ACROSS_NUM_INDICES];

    this->first_vertex = first_vertex;
    this->first_index = first_index;
    this->context = context;

    fill_buffers(vertices, tr_vertices, indices, tr_indices);

    // Precomputed variables used in the contains function
    set_corners(tr_vertices[1].position, tr_vertices[3].position,
        tr_vertices[0].position);
}

// Render this segment
void
ACrossroadSegment::render()
{
    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the road
    glUniform1i(context.u_texture, ROAD_ACROSSROAD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT,
        (void *)(first_index * sizeof(GLushort)));
    // Draw the side
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT,
        (void *)((first_index + 4) * sizeof(GLushort)));
}


