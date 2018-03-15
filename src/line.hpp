
#ifndef LINE_HPP
#define LINE_HPP

#include <vector>

using namespace std;

// Represents a point in the line
typedef struct line_point_s {
    // Coordinates and angle in screen reference frame
    int sx;
    int sy;
    float sangle;
    // Coordinates and angle in world reference frame
    float wx;
    float wy;
    float wangle;
    // Cumulated distance along the line
    float dist;

    // Constructor
    line_point_s(int sx, int sy, float sangle, float wx, float wy,
            float wangle, float dist):
        sx(sx), sy(sy), sangle(sangle), wx(wx), wy(wy), wangle(wangle),
        dist(dist)
    {}
} line_point_t;

class Line {

    public:

        Line();
        ~Line();

        /* Add a point to this line.
           Parameters:
             * sx: X coordinates of the point in screen reference frame.
             * sy: Y coordinates of the point in screen reference frame.
             * wx: X coordinates of the point in world reference frame.
             * wy: Y coordinates of the point in world reference frame.
        */
        void add(float sx, float sy, float wx, float wy);

        // Remove all the points of this line.
        void clear();

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

