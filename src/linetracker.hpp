
#ifndef LINETRACKER_HPP
#define LINETRACKER_HPP

#include "opencv2/opencv.hpp"

using namespace cv;

class LineTracker {

    public:

        /* Track the line in the image.
           Parameters:
             * frame: image of the line.
             * line: the output line to follow.
        */
        virtual void track(Mat& frame, Line& line) = 0;

};

#endif

