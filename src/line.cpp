
#include "line.hpp"

#include <math.h>
#include <stdio.h>

Line::Line()
{}

Line::~Line()
{}

/* Add a point to this line.
   Parameters:
     * x: X coordinates of the point.
     * y: Y coordinates of the point.
*/
void
Line::add(float x, float y)
{
    // If there was alread a point, compute the angle between them two
    size_t size = points.size();
    float angle;
    if (size) {
        float vx = x - points[size - 1].x;
        float vy = y - points[size - 1].y;
        vx /= sqrt(vx*vx + vy+vy);
        angle = acos(vx);
    } else {
        angle = M_PI/2.0;
    }
    points.push_back(line_point_t(x, y, angle));
}

// Remove all the points of this line.
void
Line::clear()
{
    points.clear();
}

// Return the angle between the last two points.
float
Line::get_last_angle() const
{
    return points[points.size() - 1].angle;
}

/* Return a point of the line.
   Parameters:
     * index: the index of the point in the line.
*/
const line_point_t&
Line::get_point(int index) const
{
    return points[index];
}

// Return the number of points of the line
size_t
Line::size() const
{
    return points.size();
}

