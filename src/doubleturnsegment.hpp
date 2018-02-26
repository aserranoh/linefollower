
#ifndef DOUBLETURNSEGMENT_HPP
#define DOUBLETURNSEGMENT_HPP

#include "tracksegment.hpp"

#define NSEGMENTS   16

#define DOUBLETURN_NUM_VERTICES (NSEGMENTS * 6 + 6)
#define DOUBLETURN_NUM_INDICES  DOUBLETURN_NUM_VERTICES

#define SEGMENT_R1  30.0f
#define SEGMENT_R2  60.0f

class DoubleTurnSegment: public TrackSegment {

    public:

        DoubleTurnSegment(const glm::vec3& position, float orientation,
            int input, const gl_vertex_t *vertices, const GLushort *indices);
        virtual ~DoubleTurnSegment();

        // Return true if this segment contains the projection of point
        virtual bool contains(const glm::vec3& point) const;

        /* Initialize the geometry.
           Parameters:
             * first_vertex: index in the OpenGL Vertex Buffer for the first
                 vertex of this segment.
             * first_index: index in the OpenGL Index Buffer for the first
                 index of this segment.
             * context: OpenGL objects.
        */
        virtual void init_geometry(size_t first_vertex, size_t first_index,
            const gl_context_t& context);

        // Render this segment
        virtual void render();

    private:

        // Pointers to geometry
        const gl_vertex_t *vertices;
        const GLushort *indices;

        // Precomputed variables to compute if a point is inside this segment
        glm::vec3 a;
        glm::vec3 ab;
        glm::vec3 ad;

};

#endif

