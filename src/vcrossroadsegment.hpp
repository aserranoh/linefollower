
#ifndef VCROSSROADSEGMENT_HPP
#define VCROSSROADSEGMENT_HPP

#include "squaresegment.hpp"

class VCrossroadSegment: public SquareSegment {

    public:

        VCrossroadSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~VCrossroadSegment();

        /* Get the next segment's position (the connection point)
           Parameters:
             * output: the index of the output to use.
             * pos: output position.
             * orient: output orientation.
        */
        virtual void get_output(
            int output, glm::vec3& pos, float& orient) const;

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

        // Geometry definition
        static const gl_vertex_t vertices[];
        static const GLushort indices[];

};

#endif

