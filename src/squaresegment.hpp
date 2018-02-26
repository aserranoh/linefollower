
#ifndef SQUARESEGMENT_HPP
#define SQUARESEGMENT_HPP

#include "tracksegment.hpp"

#define SEGMENT_W   30.0f
#define SEGMENT_L   30.0f

class SquareSegment: public TrackSegment {

    public:

        SquareSegment(const glm::vec3& position, float orientation,
            int input, size_t num_vertices, size_t num_indices);
        virtual ~SquareSegment();

        // Return true if this segment contains the projection of point
        virtual bool contains(const glm::vec3& point) const;

    protected:

        /* Set the corners that delimite this square.
           Parameters:
             * a: first corner.
             * b: second corner.
             * d: third corner.
        */
        void set_corners(
            const glm::vec3& a, const glm::vec3& b, const glm::vec3& d);

    private:

        // Precomputed variables to compute if a point is inside this segment
        glm::vec2 a;
        glm::vec2 ab;
        glm::vec2 ad;
        float ab_ab;
        float ad_ad;

};

#endif

