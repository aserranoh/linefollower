
#ifndef SSFAPATHFINDER_HPP
#define SSFAPATHFINDER_HPP

#include "pathfinder.hpp"

#define EPSILON (0.001f*0.001f)

class SSFAPathFinder: public PathFinder {

    public:

        /* Find an optimal path along the road.
           Parameters:
             * road: the road where to find the path.
             * path: the output path.
        */
        virtual void find(const Road& road, vector<glm::vec2>& path);

    private:

        /* Compute the cross product (c-a)x(b-a).
           Parameters:
             * a: first point.
             * b: second point.
             * c: third point.
        */
        inline float triarea2(
            const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
        {
            return (c[0] - a[0])*(b[1] - a[1]) - (b[0] - a[0])*(c[1] - a[1]);
        }

        /* Return true if two points are very close from each other.
           Parameters:
             * a: first point.
             * b: second point.
        */
        inline bool vequal(const glm::vec2& a, const glm::vec2& b)
        {
            return (b[0] - a[0])*(b[0] - a[0]) + (b[1] - a[1])*(b[1] - a[1])
                < EPSILON;
        }

};

#endif

