
#ifndef TRACKSEGMENT_HPP
#define TRACKSEGMENT_HPP

#include <GLES2/gl2.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "gl.hpp"

// Default height of the segments
#define SEGMENT_H   2

class TrackSegment {

    public:

        TrackSegment(const glm::vec3& position, float orientation, int input,
            size_t num_vertices, size_t num_indices);
        virtual ~TrackSegment();

        // Return true if this segment contains the projection of point
        virtual bool contains(const glm::vec3& point) const = 0;

        /* Given a position and an orientation and normal vectors, correct them
           to make sure that they are over the segment.
           Parameters:
             * position: position to correct.
             * orientation: orientation to correct.
             * normal: normal to correct.
        */
        virtual void correct_position(glm::vec3& position,
            glm::vec3& orientation, glm::vec3& normal) const;

        // Get the bounding box for this segment
        void get_bounding_box(glm::vec2& min, glm::vec2& max) const;

        /* Get the next segment's position (the connection point)
           Parameters:
             * output: the index of the output to use.
             * pos: output position.
             * orient: output orientation.
        */
        virtual void get_output(
            int output, glm::vec3& pos, float& orient) const = 0;

        // Return the number of indices of the geometry
        size_t get_num_indices();

        // Return the number of vertices of the geometry
        size_t get_num_vertices();

        /* Initialize the geometry.
           Parameters:
             * first_vertex: index in the OpenGL Vertex Buffer for the first
                 vertex of this segment.
             * first_index: index in the OpenGL Index Buffer for the first
                 index of this segment.
             * context: OpenGL objects.
        */
        virtual void init_geometry(size_t first_vertex, size_t first_index,
            const gl_context_t& context) = 0;

        // Render this segment
        virtual void render() = 0;

    protected:

        // Segment position
        glm::vec3 position;

        // Segment orientation (angle in the plane, in degrees)
        float orientation;

        // OpenGL context
        gl_context_t context;

        // First vertex in the Vertex Buffer for this segment
        GLintptr first_vertex;

        // First index in the Vertex Buffer for this segment
        GLintptr first_index;

        // Input to use to connect this segment with the previous one
        int input;

        // Geometry size
        size_t num_vertices;
        size_t num_indices;

        // The bounding box
        glm::vec2 bb_min;
        glm::vec2 bb_max;

        // Compute the bounding box of this segment
        void compute_bounding_box(const gl_vertex_t *vertices, size_t nv);

        // Fill the vertex and index buffers
        void fill_buffers(const gl_vertex_t *vertices,
            gl_vertex_t *tr_vertices, const GLushort *indices,
            GLushort *tr_indices);

};

#endif

