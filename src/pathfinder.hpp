
#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <glm/vec2.hpp>
#include <vector>

#include "road.hpp"

using namespace std;

class PathFinder {

    public:

        /* Find an optimal path along the road.
           Parameters:
             * road: the road where to find the path.
             * path: the output path.
        */
        virtual void find(const Road& road, vector<glm::vec2>& path) = 0;

};

#endif

