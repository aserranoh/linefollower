
#include <glm/glm.hpp>
#include <math.h>

#include "turnleftsegment.hpp"

TurnSegment::TurnSegment(const glm::vec3& position, float orientation,
        int input, const gl_vertex_t *vertices, const GLushort *indices):
    TrackSegment(position, orientation, input, TURN_NUM_VERTICES,
        TURN_NUM_INDICES), vertices(vertices), indices(indices)
{}

TurnSegment::~TurnSegment()
{}

/* Return true if this segment contains the projection of point.
*/
bool
TurnSegment::contains(const glm::vec3& point) const
{
    glm::vec3 am(point - a);

    am[2] = 0;
    return cross(ab, am)[2] >= 0 && cross(am, ad)[2] >= 0.0
        && length(am) <= SEGMENT_R;
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
TurnSegment::init_geometry(size_t first_vertex, size_t first_index,
    const gl_context_t& context)
{
    gl_vertex_t tr_vertices[TURN_NUM_VERTICES];
    GLushort tr_indices[TURN_NUM_INDICES];

    this->first_vertex = first_vertex;
    this->first_index = first_index;
    this->context = context;

    fill_buffers(vertices, tr_vertices, indices, tr_indices);

    // Precomputed variables used in the contains function
    a = tr_vertices[0].position;
    a[2] = 0;
    ab = tr_vertices[1].position - a;
    ab[2] = 0;
    ad = tr_vertices[NTRIANGLES + 1].position - a;
    ad[2] = 0;
}

// Render this segment
void
TurnSegment::render()
{
    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the road
    glUniform1i(context.u_texture, ROAD_TEXTURE);
    glDrawElements(GL_TRIANGLE_FAN, NTRIANGLES + 2, GL_UNSIGNED_SHORT,
        (void *)(first_index * sizeof(GLushort)));
    // Draw the side
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, NTRIANGLES * 2 + 2, GL_UNSIGNED_SHORT,
        (void *)((first_index + NTRIANGLES + 2) * sizeof(GLushort)));
}

