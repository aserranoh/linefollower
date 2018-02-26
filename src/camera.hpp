
/* camera.hpp
   This is the base class for the objects that are used to capture the image
   of the road to be used for the line follower robot.
*/

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "opencv2/opencv.hpp"

using namespace cv;

// Represents a camera to capture images of some kind
class Camera {

    public:

        Camera();
        virtual ~Camera();

        // Fetch the next frame
        virtual void fetch() = 0;

        // Return frame's height
        virtual size_t get_height() = 0;

        // Return frame's width
        virtual size_t get_width() = 0;

        // Retrieves the next frame.
        Mat& next();

    protected:

        /* Set the buffers that the camera has to use.
           Parameters:
             * front: the front buffer.
             * back: the back buffer.
        */
        void set_buffers(Mat& front, Mat& back);

        // Swap the front and back buffers
        void swap_buffers();

    private:

        // The back and front frames
        Mat back_frame, front_frame;

};

#endif

