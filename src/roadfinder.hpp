
#ifndef ROADFINDER_HPP
#define ROADFINDER_HPP

#include "opencv2/opencv.hpp"

#include "road.hpp"

using namespace cv;

class RoadFinder {

    public:

        virtual void find(Mat& frame, Road& road) = 0;

};

#endif

