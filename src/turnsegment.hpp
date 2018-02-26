
#ifndef TURNSEGMENT_HPP
#define TURNSEGMENT_HPP

#include "tracksegment.hpp"

#define NTRIANGLES  16

#define TURN_NUM_VERTICES   (NTRIANGLES * 3 + 4)
#define TURN_NUM_INDICES    TURN_NUM_VERTICES

#define SEGMENT_R   30.0f

class TurnSegment: public TrackSegment {

    public:

        TurnSegment(const glm::vec3& position, float orientation, int input,
            const gl_vertex_t *vertices, const GLushort *indices);
        virtual ~TurnSegment();

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

