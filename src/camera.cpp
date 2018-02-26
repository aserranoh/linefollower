
#include "camera.hpp"

Camera::Camera()
{}

Camera::~Camera()
{}

Mat&
Camera::next()
{
    // Return the front frame
    return front_frame;
}

// PROTECTED FUNCTIONS

/* Set the buffers that the camera has to use.
   Parameters:
     * front: the front buffer.
     * back: the back buffer.
*/
void
Camera::set_buffers(Mat& front, Mat& back)
{
    back_frame = back;
    front_frame = front;
}

// Swap the front and back buffers
void
Camera::swap_buffers()
{
    // Swap back frame and front frame
    Mat aux = front_frame;
    front_frame = back_frame;
    back_frame = aux;
}

