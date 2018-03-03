
#include <pthread.h>
#include <sstream>

#include "followexception.hpp"
#include "framecapture.hpp"
#include "log.hpp"
#include "realcamera.hpp"

#ifdef WITH_GLES2

#include "virtualcamera.hpp"

#endif

/* Constructor.
   Parameters:
     * options: application options.
*/
FrameCapture::FrameCapture(const Options& options):
    options(options),
    cam_params(options.get_int("CameraWidth"), options.get_int("CameraHeight"),
        options.get_float("CameraFovh"), options.get_float("CameraFovv"),
        options.get_float("CameraZ"),
        options.get_float("CameraAngle") * M_PI / 180.0),
    camera(0), camera_type(options.get_string("Camera")),
    cond_frame_req(PTHREAD_COND_INITIALIZER),
    cond_frame_avail(PTHREAD_COND_INITIALIZER),
    cond_cam_avail(PTHREAD_COND_INITIALIZER),
    mutex_frame_req(PTHREAD_MUTEX_INITIALIZER),
    mutex_frame_avail(PTHREAD_MUTEX_INITIALIZER),
    mutex_cam_avail(PTHREAD_MUTEX_INITIALIZER), frame_req(false),
    frame_avail(false), cam_init_finished(false), stop_req(false)
{
    // Check the camera angle
    if (cam_params.cam_angle > MAX_CAMANGLE
        || cam_params.cam_angle < MIN_CAMANGLE)
    {
        throw FollowException(
            "camera angle must be between %.1f and %.1f degrees");
    }
    // Create the thread
    pthread_create(&thread, 0, thread_main, this);
    // Wait until the camera is initialized
    pthread_mutex_lock(&mutex_cam_avail);
    if (!cam_init_finished) {
        pthread_cond_wait(&cond_cam_avail, &mutex_cam_avail);
    }
    pthread_mutex_unlock(&mutex_cam_avail);
    if (!camera) {
        throw FollowException("camera not available");
    }
}

FrameCapture::~FrameCapture()
{
    if (camera) {
        stop_req = true;
        // Do a last request to force the main loop to check the loop condition
        fetch();
        pthread_join(thread, 0);
        delete camera;
    }
}

// Fetch the next frame
void
FrameCapture::fetch()
{
    pthread_mutex_lock(&mutex_frame_req);
    frame_req = true;
    pthread_mutex_unlock(&mutex_frame_req);
    pthread_cond_signal(&cond_frame_req);
}

// Return the camera instance
Camera*
FrameCapture::get_camera()
{
    return camera;
}

// Return the next frame.
Mat&
FrameCapture::next()
{
    pthread_mutex_lock(&mutex_frame_avail);
    if (!frame_avail) {
        pthread_cond_wait(&cond_frame_avail, &mutex_frame_avail);
    }
    if (frame_avail) {
        frame_avail = false;
        pthread_mutex_unlock(&mutex_frame_avail);
    } else {
        pthread_mutex_unlock(&mutex_frame_avail);
        throw FollowException("cannot get next frame");
    }
    return camera->next();
}

// PRIVATE FUNCTIONS

// Initialize the camera
void
FrameCapture::init_camera()
{
    // Create the camera
    if (camera_type == "real") {
        camera = new RealCamera(options);
    }
#ifdef WITH_GLES2
    else if (camera_type == "virtual") {
        camera = new VirtualCamera(options);
    }
#endif
    else {
        throw FollowException("unknown camera type '" + camera_type + "'");
    }
}

// Run the tasks of this thread
void
FrameCapture::run()
{
    try {
        // Initialize the camera
        init_camera();
        pthread_mutex_lock(&mutex_cam_avail);
        cam_init_finished = true;
        pthread_mutex_unlock(&mutex_cam_avail);
        pthread_cond_signal(&cond_cam_avail);

        // Do the main loop
        while (!stop_req) {
            // Wait until there's a request to capture a frame
            pthread_mutex_lock(&mutex_frame_req);
            if (!frame_req) {
                pthread_cond_wait(&cond_frame_req, &mutex_frame_req);
            }
            frame_req = false;
            pthread_mutex_unlock(&mutex_frame_req);

            // Capture the frame
            camera->fetch();

            // Notify that a new frame is available
            pthread_mutex_lock(&mutex_frame_avail);
            frame_avail = true;
            pthread_mutex_unlock(&mutex_frame_avail);
            pthread_cond_signal(&cond_frame_avail);
        }
    // In case of error report it and stop the thread
    } catch (FollowException& e) {
        log_warn(e.what());
        // Notify the main thread to unlock all the conditions
        pthread_mutex_lock(&mutex_cam_avail);
        cam_init_finished = true;
        pthread_mutex_unlock(&mutex_cam_avail);
        pthread_cond_signal(&cond_cam_avail);
        pthread_cond_signal(&cond_frame_avail);
    }
}

/* Thread main routine.
   Parameters:
     * instance: instance of the FrameCapture object.
*/
void*
FrameCapture::thread_main(void *instance)
{
    ((FrameCapture *)instance)->run();
    return 0;
}

