
#ifndef LINE_HPP
#define LINE_HPP

#include <vector>

using namespace std;

// Represents a point in the line
typedef struct line_point_s {
    float x;
    float y;
    float angle;

    line_point_s(float x, float y, float angle): x(x), y(y), angle(angle) {}
} line_point_t;

class Line {

    public:

        Line();
        ~Line();

        /* Add a point to this line.
           Parameters:
             * x: X coordinates of the point.
             * y: Y coordinates of the point.
        */
        void add(float x, float y);

        // Remove all the points of this line.
        void clear();

        // Return the angle between the last two points.
        float get_last_angle() const;

        /* Return a point of the line.
           Parameters:
             * index: the index of the point in the line.
        */
        const line_point_t& get_point(int index) const;

        // Return the number of points of the line
        size_t size() const;

    private:

        vector<line_point_t> points;

};

#endif

