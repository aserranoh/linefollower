
#include <glm/glm.hpp>
#include <math.h>

#include "doubleturnsegment.hpp"

DoubleTurnSegment::DoubleTurnSegment(const glm::vec3& position,
        float orientation, int input, const gl_vertex_t *vertices,
        const GLushort *indices):
    TrackSegment(position, orientation, input, DOUBLETURN_NUM_VERTICES,
        DOUBLETURN_NUM_INDICES), vertices(vertices), indices(indices)
{}

DoubleTurnSegment::~DoubleTurnSegment()
{}

/* Return true if this segment contains the projection of point.
*/
bool
DoubleTurnSegment::contains(const glm::vec3& point) const
{
    glm::vec3 am(point - a);

    am[2] = 0;
    return cross(ab, am)[2] >= 0.0 && cross(am, ad)[2] >= 0.0
        && length(am) >= SEGMENT_R1 && length(am) <= SEGMENT_R2;
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
DoubleTurnSegment::init_geometry(size_t first_vertex, size_t first_index,
    const gl_context_t& context)
{
    gl_vertex_t tr_vertices[DOUBLETURN_NUM_VERTICES];
    GLushort tr_indices[DOUBLETURN_NUM_INDICES];

    this->first_vertex = first_vertex;
    this->first_index = first_index;
    this->context = context;

    fill_buffers(vertices, tr_vertices, indices, tr_indices);

    // Precomputed variables used in the contains function
    
    a = tr_vertices[0].position*2.0f - tr_vertices[1].position;
    a[2] = 0;
    ab = tr_vertices[1].position - a;
    ab[2] = 0;
    ad = tr_vertices[NSEGMENTS*2 + 1].position - a;
    ad[2] = 0;
}

// Render this segment
void
DoubleTurnSegment::render()
{
    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the road
    glUniform1i(context.u_texture, ROAD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, NSEGMENTS * 2 + 2, GL_UNSIGNED_SHORT,
        (void *)(first_index * sizeof(GLushort)));
    // Draw the left side
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, NSEGMENTS * 2 + 2, GL_UNSIGNED_SHORT,
        (void *)((first_index + NSEGMENTS * 2 + 2) * sizeof(GLushort)));
    // Draw the right side
    glUniform1i(context.u_texture, WOOD_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, NSEGMENTS * 2 + 2, GL_UNSIGNED_SHORT,
        (void *)((first_index + NSEGMENTS * 4 + 4) * sizeof(GLushort)));
}

