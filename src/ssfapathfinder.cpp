
#include "ssfapathfinder.hpp"

/* Find an optimal path along the road.

   This is an implementation of the Simple Stupid Funnel Algorithm, that can
   be found here:
   http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html

   Parameters:
     * road: the road where to find the path.
     * path: the output path.
*/
// TODO: Exit after the first corner is found
void
SSFAPathFinder::find(const Road& road, vector<glm::vec2>& path)
{
    vector<road_section_t> portals;
    glm::vec2 start = glm::vec2(0, 0);
    glm::vec2 goal = road.get_goal();
    road_section_t start_section = {start, start};
    road_section_t goal_section = {goal, goal};
    glm::vec2 portal_apex, portal_left, portal_right, left, right;
    int apex_index = 0, left_index = 0, right_index = 0;

    // Empth the path
    path.clear();

    // Prepare a road with two extra portals, one at the beginning made with
    // the starting position as both left and right points, and one made by
    // the goal position also as both left and right points.
    portals.push_back(start_section);
    for (size_t i = 0; i < road.get_size(); i++) {
        // Use a secton only if its y coordinate is lower than the goal
        road_section_t s = road.get_section(i);
        if (s.left[1] >= goal_section.left[1])
            break;
        portals.push_back(s);
    }
    portals.push_back(goal_section);

    portal_apex = portals[0].left;
    portal_left = portal_apex;
    portal_right = portals[1].right;

    // Add start point
    path.push_back(portal_apex);

    for (size_t i = 0; i < portals.size(); i++) {
        left = portals[i].left;
        right = portals[i].right;

        // Update right vertex
        if (triarea2(portal_apex, portal_right, right) <= 0.0f) {
            if (vequal(portal_apex, portal_right)
                || triarea2(portal_apex, portal_left, right) > 0.0f)
            {
                // Tighten the funnel.
                portal_right = right;
                right_index = i;
            } else {
                // Right over left, insert left to path and restart scan from
                // portal left point.
                path.push_back(portal_left);
                // Make current left the new apex.
                portal_apex = portal_left;
                apex_index = left_index;
                // Reset portal
                portal_left = portal_right = portal_apex;
                left_index = right_index = apex_index;
                // Restart scan
                i = apex_index;
                continue;
            }
        }

        // Update left vertex.
        if (triarea2(portal_apex, portal_left, left) >= 0.0f) {
            if (vequal(portal_apex, portal_left)
                || triarea2(portal_apex, portal_right, left) < 0.0f)
            {
                // Tighten the funnel.
                portal_left = left;
                left_index = i;
            } else {
                // Left over right, insert right to path and restart scan from
                // portal right point.
                path.push_back(portal_right);
                // Make current right the new apex.
                portal_apex = portal_right;
                apex_index = right_index;
                // Reset portal
                portal_left = portal_right = portal_apex;
                left_index = right_index = apex_index;
                // Restart scan
                i = apex_index;
                continue;
            }
        }
    }

    // Append last point to path.
    path.push_back(goal);
}

