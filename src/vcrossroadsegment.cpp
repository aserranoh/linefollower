
#include <math.h>

#include "vcrossroadsegment.hpp"

#define VCROSS_NUM_VERTICES   8
#define VCROSS_NUM_INDICES    8

// Geometry
const gl_vertex_t VCrossroadSegment::vertices[] = {
    // road
    {{0        ,  SEGMENT_W/2, SEGMENT_H}, {0, 0, 1}, {0, 0}},
    {{0        , -SEGMENT_W/2, SEGMENT_H}, {0, 0, 1}, {1, 0}},
    {{SEGMENT_L,  SEGMENT_W/2, SEGMENT_H}, {0, 0, 1}, {0, 1}},
    {{SEGMENT_L, -SEGMENT_W/2, SEGMENT_H}, {0, 0, 1}, {1, 1}},
    // side
    {{SEGMENT_L, -SEGMENT_W/2, 0        }, {1, 0, 0}, {0, 0}},
    {{SEGMENT_L,  SEGMENT_W/2, 0        }, {1, 0, 0}, {1, 0}},
    {{SEGMENT_L, -SEGMENT_W/2, SEGMENT_H}, {1, 0, 0}, {0, 1}},
    {{SEGMENT_L,  SEGMENT_W/2, SEGMENT_H}, {1, 0, 0}, {1, 1}},
};

const GLushort VCrossroadSegment::indices[] = {
    0, 1, 2, 3,
    4, 5, 6, 7
};

VCrossroadSegment::VCrossroadSegment(const glm::vec3& position,
        float orientation, int input):
    SquareSegment(position, orientation, input, VCROSS_NUM_VERTICES,
        VCROSS_NUM_INDICES)
{}

VCrossroadSegment::~VCrossroadSegment()
{}

/* Get the next segment's position (the connection point)
   Parameters:
     * output: the index of the output to use.
     * pos: output position.
     * orient: output orientation.
*/
void
VCrossroadSegment::get_output(int output, glm::vec3& pos, float& orient) const
{
    if (output == 0) {
        pos[0] = position[0] + cos(orientation) * SEGMENT_L/2
            + cos(orientation + M_PI/2) * SEGMENT_W/2;
        pos[1] = position[1] + sin(orientation) * SEGMENT_L/2
            + sin(orientation + M_PI/2) * SEGMENT_W/2;
        pos[2] = position[2];
        orient = orientation + M_PI/2;
    } else {
        pos[0] = position[0] + cos(orientation) * SEGMENT_L/2
            + cos(M_PI/2 - orientation) * SEGMENT_W/2;
        pos[1] = position[1] + sin(orientation) * SEGMENT_L/2
            - sin(M_PI/2 - orientation) * SEGMENT_W/2;
        pos[2] = position[2];
        orient = orientation - M_PI/2;
    }
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
VCrossroadSegment::init_geometry(size_t first_vertex, size_t first_index,
    const gl_context_t& context)
{
    gl_vertex_t tr_vertices[VCROSS_NUM_VERTICES];
    GLushort tr_indices[VCROSS_NUM_INDICES];

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
VCrossroadSegment::render()
{
    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the road
    glUniform1i(context.u_texture, ROAD_VCROSSROAD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT,
        (void *)(first_index * sizeof(GLushort)));
    // Draw the side
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT,
        (void *)((first_index + 4) * sizeof(GLushort)));
}

