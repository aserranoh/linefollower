
#include "road.hpp"

Road::Road()
{}

Road::~Road()
{}

/* Add a section at the end of the road.
   Parameters:
     * section: the section to add.
*/
void
Road::add(const road_section_t& section)
{
    sections.push_back(section);
}

// Empty the road (remove all sections)
void
Road::clear()
{
    sections.clear();
}

// Return the goal position in the road (point where to go).
const glm::vec2&
Road::get_goal() const
{
    return goal;
}

/* Return a section of the road given its index.
   Parameters:
     * index: index of the section to retrieve.
*/
const road_section_t&
Road::get_section(unsigned int index) const
{
    return sections[index];
}

// Return the number of sections.
size_t
Road::get_size() const
{
    return sections.size();
}

/* Set the goal position.
   Parameters:
     * goal: the new goal position.
*/
void
Road::set_goal(const glm::vec2& goal)
{
    this->goal = goal;
}

