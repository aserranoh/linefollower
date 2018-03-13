
#ifndef ROBOTANICUSLINETRACKER_HPP
#define ROBOTANICOSLINETRACKER_HPP

// This algorithm was inspired by this article:
// https://www.raspberrypi.org/blog/an-image-processing-robot-for-robocup-junior/
// Credits to Arne Baeyens, alias Robotanicus

#include "opencv2/core.hpp"

#include "camparams.hpp"
#include "line.hpp"
#include "linetracker.hpp"
#include "options.hpp"

using namespace cv;

class RobotanicusLineTracker: public LineTracker {

    public:

        /* Constructor.
           Parameters:
             * options: application's options.
        */
        RobotanicusLineTracker(const Options& options);

        ~RobotanicusLineTracker();

        /* Track the line in the image.
           Parameters:
             * frame: image of the line.
             * line: the output line to follow.
        */
        virtual void track(Mat& frame, Line& line);

    private:

        // The camera parameters
        cam_params_t cam_params;

        // Height in the image of the horizontal scanline
        size_t horizontal_scanline_offset;

        // Number of scan circles to use to track the line
        size_t num_scan_circles;

        // Radius of the scan circles, in world coordinates
        float scan_circle_radius;

        // Destination images form some transformations
        Mat gray_frame;

        // Some constants to accelerate the calculations
        float k1, k2, k3, k4, k5;

        // Working variables
        int *aux_row;
        size_t aux_row_size;

        /* Find the position of the line in the zone of the image traversed
           by an horizontal scanline.
           Parameters:
             * frame: the image.
             * x: the output x coordinates of the line.
             * y: the output y coordinates of the line.
        */
        void find_line_horizontal_scanline(const Mat &frame, int& x, int& y)
            const;

        /* Find the position of the next point of the line using a scan circle.
           Parameters:
             * frame: the image.
             * cx: X coordinate of the center of the scan circle.
             * cy: Y coordinate of the center of the scan circle.
             * radius: radius of the scan circle.
             * angle: start angle of the scan circle.
             * x: the output x coordinates of the line.
             * y: tye output y coordinates of the line.
        */
        void find_line_scan_circle(const Mat& frame, int cx, int cy,
            int radius, float angle, int& x, int& y);

        /* Get the radius of the scan circle to use. The radius of the scan
           circle depends on the y coordinate of the center of the circle:
           the far from the viewer the circle is in the image, the smaller
           the radius.
           Parameters:
             * y: Y coordinate of the circle center, in world coordinates.
        */
        int get_scan_circle_radius(float y) const;

        /* Transform a point from screen to world coordinates.
           Parameters:
             * sx: x coordinates in screen frame.
             * sy: y coordinates in screen frame.
             * x: output x in world coordinates.
             * y: output y in world coordinates.
        */
        void screen_to_world(int sx, int sy, float& x, float& y);

};

#endif

