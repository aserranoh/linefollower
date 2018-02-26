
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "gl.hpp"
#include "tracksegment.hpp"

TrackSegment::TrackSegment(const glm::vec3& position, float orientation,
        int input, size_t num_vertices, size_t num_indices):
    position(position), orientation(orientation), input(input),
    num_vertices(num_vertices), num_indices(num_indices),
    bb_min(INT_MAX, INT_MAX), bb_max(INT_MIN, INT_MIN)
{}

TrackSegment::~TrackSegment() {}

/* Given a position and an orientation and normal vectors, correct them to make
   sure that they are over the segment.
   Parameters:
     * position: position to correct.
     * orientation: orientation to correct.
     * normal: normal to correct.
*/
void
TrackSegment::correct_position(
    glm::vec3& position, glm::vec3& orientation, glm::vec3& normal) const
{
    glm::vec3 normal_in = normal;
    float a;

    // The output position is the same as the input but with the Z corrected
    position[2] = SEGMENT_H;

    // The output normal is the Z axis
    normal = glm::vec3(0, 0, 1);

    // The output orientation is the result of rotating the input orientation
    // through the vector (input_normal) x (output_normal) the angle between
    // the input_normal and the output_normal.
    a = glm::angle(normal_in, normal);
    if (a > 0) {
        orientation = glm::rotate(
            orientation, a, glm::normalize(glm::cross(normal_in, normal)));
    }
    glm::normalize(orientation);
}

// Get the bounding box for this segment
void
TrackSegment::get_bounding_box(glm::vec2& min, glm::vec2& max) const
{
    min = bb_min;
    max = bb_max;
}

// Return the number of indices of the geometry
size_t
TrackSegment::get_num_indices()
{
    return num_indices;
}

// Return the number of vertices of the geometry
size_t
TrackSegment::get_num_vertices()
{
    return num_vertices;
}

// PRIVATE FUNCTIONS

void
TrackSegment::compute_bounding_box(const gl_vertex_t *vertices, size_t nv)
{
    glm::vec2 v;

    for (size_t i = 0; i < nv; i++) {
        v = glm::vec2(vertices[i].position);
        if (v[0] < bb_min[0]) bb_min[0] = v[0];
        if (v[1] < bb_min[1]) bb_min[1] = v[1];
        if (v[0] > bb_max[0]) bb_max[0] = v[0];
        if (v[1] > bb_max[1]) bb_max[1] = v[1];
    }
}

// Fill the vertex and index buffers
void
TrackSegment::fill_buffers(const gl_vertex_t *vertices,
    gl_vertex_t *tr_vertices, const GLushort *indices, GLushort *tr_indices)
{
    glm::mat4 m(1);

    // Compute the vertices for this segment (rotate orientation and translate
    // to the origin)
    m = glm::rotate(
        glm::translate(m, position), orientation, glm::vec3(0, 0, 1));
    for (size_t i = 0; i < num_vertices; i++) {
        tr_vertices[i].position = glm::vec3(
            m * glm::vec4(vertices[i].position, 1.0));
        tr_vertices[i].normal = glm::vec3(
            m * glm::vec4(vertices[i].normal, 0.0));
        tr_vertices[i].texcoord = vertices[i].texcoord;
    }

    // Add to the indices the first_vertex offset
    for (size_t i = 0; i < num_indices; i++) {
        tr_indices[i] = indices[i] + first_vertex;
    }

    // Insert the vertex and index data in the buffers
    glBindBuffer(GL_ARRAY_BUFFER, context.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, first_vertex * sizeof(gl_vertex_t),
        num_vertices * sizeof(gl_vertex_t), tr_vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, first_index * sizeof(GLushort),
        num_indices * sizeof(GLushort), tr_indices);

    // Compute the bounding box
    compute_bounding_box(tr_vertices, num_vertices);
}

