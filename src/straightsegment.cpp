
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <string.h>

#include "straightsegment.hpp"

#define STRAIGHT_NUM_VERTICES   12
#define STRAIGHT_NUM_INDICES    16

// Geometry
const gl_vertex_t StraightSegment::vertices[] = {
    // road
    {{0        ,  SEGMENT_W/2, SEGMENT_H}, {0,  0, 1}, {0, 0}},
    {{0        , -SEGMENT_W/2, SEGMENT_H}, {0,  0, 1}, {1, 0}},
    {{SEGMENT_L,  SEGMENT_W/2, SEGMENT_H}, {0,  0, 1}, {0, 1}},
    {{SEGMENT_L, -SEGMENT_W/2, SEGMENT_H}, {0,  0, 1}, {1, 1}},
    // left side
    {{SEGMENT_L,  SEGMENT_W/2, 0        }, {0,  1, 0}, {0, 0}},
    {{0        ,  SEGMENT_W/2, 0        }, {0,  1, 0}, {1, 0}},
    {{SEGMENT_L,  SEGMENT_W/2, SEGMENT_H}, {0,  1, 0}, {0, 1}},
    {{0        ,  SEGMENT_W/2, SEGMENT_H}, {0,  1, 0}, {1, 1}},
    // right side
    {{0        , -SEGMENT_W/2, 0        }, {0, -1, 0}, {0, 0}},
    {{SEGMENT_L, -SEGMENT_W/2, 0        }, {0, -1, 0}, {1, 0}},
    {{0        , -SEGMENT_W/2, SEGMENT_H}, {0, -1, 0}, {0, 1}},
    {{SEGMENT_L, -SEGMENT_W/2, SEGMENT_H}, {0, -1, 0}, {1, 1}},
};

const GLushort StraightSegment::indices[] = {
    0, 1, 2, 3,
    4, 5, 6, 6, 5, 7,
    8, 9, 10, 10, 9, 11,
};

StraightSegment::StraightSegment(const glm::vec3& position, float orientation,
        int input, GLint texture):
    SquareSegment(position, orientation, input, STRAIGHT_NUM_VERTICES,
        STRAIGHT_NUM_INDICES), texture(texture)
{}

StraightSegment::~StraightSegment()
{}

/* Get the next segment's position (the connection point)
   Parameters:
     * output: the index of the output to use.
     * pos: output position.
     * orient: output orientation.
*/
void
StraightSegment::get_output(int output, glm::vec3& pos, float& orient) const
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
StraightSegment::init_geometry(size_t first_vertex, size_t first_index,
    const gl_context_t& context)
{
    gl_vertex_t tr_vertices[STRAIGHT_NUM_VERTICES];
    GLushort tr_indices[STRAIGHT_NUM_INDICES];

    this->first_vertex = first_vertex;
    this->first_index = first_index;
    this->context = context;

    fill_buffers(vertices, tr_vertices, indices, tr_indices);

    // Precomputed variables used in the contains function
    set_corners(tr_vertices[8].position, tr_vertices[9].position,
        tr_vertices[5].position);
}

// Render this segment
void
StraightSegment::render()
{
    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the road
    glUniform1i(context.u_texture, texture);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT,
        (void *)(first_index * sizeof(GLushort)));
    // Draw the sides
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT,
        (void *)((first_index + 4) * sizeof(GLushort)));
}

