
#ifndef ROAD_HPP
#define ROAD_HPP

#include <glm/vec2.hpp>
#include <sys/types.h>
#include <vector>

using namespace std;

typedef struct {
    // Left point
    glm::vec2 left;

    // Right point
    glm::vec2 right;

    // Central line
    glm::vec2 line;
} road_section_t;

class Road {

    public:

        Road();
        ~Road();

        /* Add a section at the end of the road.
           Parameters:
             * section: the section to add.
        */
        void add(const road_section_t& section);

        // Empty the road (remove all sections)
        void clear();

        // Return the goal position in the road (point where to go).
        const glm::vec2& get_goal() const;

        /* Return a section of the road given its index.
           Parameters:
             * index: index of the section to retrieve.
        */
        const road_section_t& get_section(unsigned int index) const;

        // Return the number of sections.
        size_t get_size() const;

        /* Set the goal position.
           Parameters:
             * goal: the new goal position.
        */
        void set_goal(const glm::vec2& goal);

    private:

        // The sequence of sections
        vector<road_section_t> sections;

        // The goal
        glm::vec2 goal;

};

#endif

