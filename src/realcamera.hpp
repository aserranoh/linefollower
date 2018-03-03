
/* realcamera.hpp
   Represents a real camera, managed through OpenCV.
*/

#ifndef REALCAMERA_HPP
#define REALCAMERA_HPP

#include "opencv2/opencv.hpp"

#include "camera.hpp"
#include "options.hpp"

using namespace cv;

class RealCamera: public Camera {

    public:

        RealCamera(const Options& options);
        virtual ~RealCamera();

        // Fetch the next frame
        virtual void fetch();

        // Return frame's height
        virtual size_t get_height();

        // Return frame's width
        virtual size_t get_width();

    private:

        // OpenCV capture object
        VideoCapture c;

        // The front and back frames
        Mat front_buffer;
        Mat back_buffer;

};

#endif

