
#include "realcamera.hpp"

// TODO: Use FollowException

RealCamera::RealCamera():
    c(0)
{
    size_t h, w;

    if(!c.isOpened())
        throw CAM_OPEN_ERROR;

    // Create the front and back frames
    h = get_height();
    w = get_width();
    front_buffer = Mat(h, w, CV_8UC3);
    back_buffer = Mat(h, w, CV_8UC3);
    set_buffers(front_buffer, back_buffer);
}

RealCamera::~RealCamera() {}

// Fetch the next frame
void
RealCamera::fetch()
{
    Mat aux;

    if (!c.grab())
        throw CAM_GRAB_ERROR;
    if (!c.retrieve(back_buffer))
        throw CAM_RETRIEVE_ERROR;

    // Swap the frames
    swap_buffers();
    aux = front_buffer;
    front_buffer = back_buffer;
    back_buffer = aux;
}

// Return frame's height
size_t
RealCamera::get_height()
{
    return c.get(CV_CAP_PROP_FRAME_HEIGHT);
}

// Return frame's width
size_t
RealCamera::get_width()
{
    return c.get(CV_CAP_PROP_FRAME_WIDTH);
}

