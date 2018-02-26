
#ifndef FRAMECAPTURE_HPP
#define FRAMECAPTURE_HPP

#include "opencv2/opencv.hpp"

#include "camera.hpp"
#include "camparams.hpp"
#include "virtualtrack.hpp"

using namespace cv;

// Type that relates the string identifier of a segment type with its
// enumeration
typedef struct {
    segment_type_t type;
    const char *str_id;
} segment_id_t;

class FrameCapture {

    public:

        FrameCapture();

        /* Constructor.
           Parameters:
             * cam_params: camera parameters.
             * camera_type: type of camera (as for now, "real" or "virtual").
             * track_file: for the virtual camera, the file that contains the
                 track description.
        */
        FrameCapture(const cam_params_t& cam_params, const string& camera_type,
            const string& track_file);

        ~FrameCapture();

        // Fetch the next frame
        void fetch();

        // Return the camera instance
        Camera *get_camera() const;

        // Start the working thread
        void start();

        // Return the next frame
        Mat& next();

    private:

        // Attributes necessary to build the camera
        cam_params_t cam_params;
        string camera_type;
        string track_file;

        // Component to obtain the camera frame
        Camera *camera;

        // Thread attributes
        pthread_t thread;
        pthread_cond_t cond_req;
        pthread_cond_t cond_avail;
        pthread_mutex_t mutex_req;
        pthread_mutex_t mutex_avail;

        // true if a new frame has been requested, false otherwise.
        bool frame_req;

        // true if a new frame is available, false otherwise.
        bool frame_avail;

        // Relates segment strings IDs with their enumerations
        static const segment_id_t segments_ids[];

        // Initialize the camera
        void init_camera();

        // Load a track file (for the virtual camera)
        void load_track_file(
            const string& track_file, vector<segment_t>& segments);

        // Run the tasks of this thread
        void run();

        /* Thread main routine.
           Parameters:
             * instance: instance of the FrameCapture object.
        */
        static void *thread_main(void *instance);

};

#endif

