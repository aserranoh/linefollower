
#include "line.hpp"

#include <math.h>

Line::Line()
{}

Line::~Line()
{}

/* Add a point to this line.
   Parameters:
     * sx: X coordinates of the point in screen reference frame.
     * sy: Y coordinates of the point in screen reference frame.
     * wx: X coordinates of the point in world reference frame.
     * wy: Y coordinates of the point in world reference frame.
*/
void
Line::add(float sx, float sy, float wx, float wy)
{
    // If there was already a point, compute the angle between them two
    float sangle, wangle;

    if (points.size()) {
        line_point_t& p = points[points.size() - 1];
        float vsx = sx - p.sx;
        float vsy = sy - p.sy;
        float vwx = wx - p.wx;
        float vwy = wy - p.wy;
        vsx /= sqrt(vsx*vsx + vsy*vsy);
        sangle = acos(vsx);
        vwx /= sqrt(vwx*vwx + vwy*vwy);
        wangle = acos(vwx);
    } else {
        sangle = wangle = M_PI/2.0;
    }
    points.push_back(line_point_t(sx, sy, sangle, wx, wy, wangle));
}

// Remove all the points of this line.
void
Line::clear()
{
    points.clear();
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

