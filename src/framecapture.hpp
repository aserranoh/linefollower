
#ifndef FRAMECAPTURE_HPP
#define FRAMECAPTURE_HPP

#include "opencv2/opencv.hpp"

#include "config.h"
#include "camera.hpp"
#include "cameraparameters.hpp"
#include "options.hpp"

using namespace cv;

class FrameCapture {

    public:

        /* Constructor.
           Parameters:
             * options: application options.
        */
        FrameCapture(const Options& options);

        ~FrameCapture();

        // Fetch the next frame
        void fetch();

        // Return the camera instance
        Camera *get_camera();

        // Return the next frame
        Mat& next();

    private:

        // Store a reference to the application options to use it in the other
        // thread
        const Options& options;

        // Attributes necessary to build the camera
        CameraParameters cam_params;
        string camera_type;

        // Component to obtain the camera frame
        Camera *camera;

        // Thread attributes
        pthread_t thread;
        pthread_cond_t cond_frame_req;
        pthread_cond_t cond_frame_avail;
        pthread_cond_t cond_cam_avail;
        pthread_mutex_t mutex_frame_req;
        pthread_mutex_t mutex_frame_avail;
        pthread_mutex_t mutex_cam_avail;

        // true if a new frame has been requested, false otherwise.
        bool frame_req;

        // true if a new frame is available, false otherwise.
        bool frame_avail;

        // true if the camera initialization has concluded
        bool cam_init_finished;

        // Stop the capturing thread
        bool stop_req;

        // Initialize the camera
        void init_camera();

        // Run the tasks of this thread
        void run();

        /* Thread main routine.
           Parameters:
             * instance: instance of the FrameCapture object.
        */
        static void *thread_main(void *instance);

};

#endif

