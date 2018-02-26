
#include <err.h>
#include <fstream>
#include <pthread.h>
#include <sstream>

#include "followexception.hpp"
#include "framecapture.hpp"
#include "realcamera.hpp"
#include "virtualcamera.hpp"

const segment_id_t FrameCapture::segments_ids[] = {
    {SEGMENT_STRAIGHT, "Straight"},
    {SEGMENT_TURNLEFT, "TurnLeft"},
    {SEGMENT_TURNRIGHT, "TurnRight"},
    {SEGMENT_DASHED1, "Dashed1"},
    {SEGMENT_DASHED2, "Dashed2"},
    {SEGMENT_ZIGZAG, "ZigZag"},
    {SEGMENT_WIDENARROW, "WideNarrow"},
    {SEGMENT_NARROW, "Narrow"},
    {SEGMENT_NARROWWIDE, "NarrowWide"},
    {SEGMENT_VCROSSROAD, "VCrossroad"},
    {SEGMENT_ACROSSROAD, "ACrossroad"},
    {SEGMENT_DOUBLETURNLEFT, "DoubleTurnLeft"},
    {SEGMENT_DOUBLETURNRIGHT, "DoubleTurnRight"}
};

FrameCapture::FrameCapture():
    camera(0)
{}

/* Constructor.
   Parameters:
     * cam_params: camera parameters.
     * camera_type: type of camera (as for now, "real" or "virtual").
     * track_file: for the virtual camera, the file that contains the track
         description.
*/
FrameCapture::FrameCapture(const cam_params_t& cam_params,
        const string& camera_type, const string& track_file):
    cam_params(cam_params), camera_type(camera_type), track_file(track_file),
    camera(0), cond_req(PTHREAD_COND_INITIALIZER),
    cond_avail(PTHREAD_COND_INITIALIZER), mutex_req(PTHREAD_MUTEX_INITIALIZER),
    mutex_avail(PTHREAD_MUTEX_INITIALIZER), frame_req(false),
    frame_avail(false)
{}

FrameCapture::~FrameCapture()
{
    if (camera) {
        pthread_cancel(thread);
        pthread_join(thread, 0);
        delete camera;
    }
}

// Fetch the next frame
void
FrameCapture::fetch()
{
    pthread_mutex_lock(&mutex_req);
    frame_req = true;
    pthread_mutex_unlock(&mutex_req);
    pthread_cond_signal(&cond_req);
}

// Return the camera instance
Camera *
FrameCapture::get_camera() const
{
    return camera;
}

// Start the working thread
void
FrameCapture::start()
{
    // Create the thread
    pthread_create(&thread, 0, thread_main, this);
    pthread_detach(thread);
}

// Return the next frame.
Mat&
FrameCapture::next()
{
    pthread_mutex_lock(&mutex_avail);
    if (!frame_avail) {
        pthread_cond_wait(&cond_avail, &mutex_avail);
    }
    frame_avail = false;
    pthread_mutex_unlock(&mutex_avail);
    return camera->next();
}

// PRIVATE FUNCTIONS

// Initialize the camera
void
FrameCapture::init_camera()
{
    vector<segment_t> segments;

    try {
        // Create the virtual or real camera
        if (camera_type == "virtual") {
            // Create the virtual camera
            // Load the track
            load_track_file(track_file, segments);

            // Instantiate the camera
            camera = new VirtualCamera(segments, cam_params);
        } else {
            // Create the real camera
            camera = new RealCamera();
        }
    } catch (FollowException& e) {
        errx(1, "cannot create FrameCapture object: %s", e.what());
    }
}

// Load a track file (for the virtual camera)
void
FrameCapture::load_track_file(
    const string& track_file, vector<segment_t>& segments)
{
    string line;
    size_t linenum = 1, sep;
    ifstream f(track_file);
    segment_t s;

    if (f.fail()) {
        err(1, "cannot open file %s", track_file.c_str());
    } else {
        while (getline(f, line)) {
            // Ignore comments
            if (line[0] == '#') continue;
            // Get the elements of the line
            stringstream ss(line);
            string stype, sinput, soutput;
            ss >> stype >> sinput >> soutput;
            // Get the type of segment
            s.type = SEGMENT_NULL;
            for (size_t i = 0; i < sizeof(segments_ids)/sizeof(segment_id_t);
                i++)
            {
                if (stype == segments_ids[i].str_id) {
                    s.type = segments_ids[i].type;
                }
            }
            if (s.type == SEGMENT_NULL) {
                errx(1, "wrong track segment '%s' (line %zd)",
                    line.c_str(), linenum);
            }
            // Get the input
            s.input = (sinput != "") ? atoi(sinput.c_str()) : 0;
            // Get the output
            if (soutput != "") {
                sep = soutput.find(':');
                if (sep != soutput.npos) {
                    s.prev = atoi(soutput.substr(0, sep).c_str());
                    s.output = atoi(soutput.substr(sep + 1).c_str());
                } else {
                    s.prev = atoi(soutput.c_str());
                    s.output = 0;
                }
            } else {
                s.prev = -1;
                s.output = 0;
            }
            segments.push_back(s);
            linenum++;
        }
    }
}

// Run the tasks of this thread
void
FrameCapture::run()
{
    // Initialize the camera
    init_camera();

    while (1) {
        // Wait until there's a request to capture a frame
        pthread_mutex_lock(&mutex_req);
        if (!frame_req) {
            pthread_cond_wait(&cond_req, &mutex_req);
        }
        frame_req = false;
        pthread_mutex_unlock(&mutex_req);

        // Capture the frame
        camera->fetch();

        // Notify that a new frame is available
        pthread_mutex_lock(&mutex_avail);
        frame_avail = true;
        pthread_mutex_unlock(&mutex_avail);
        pthread_cond_signal(&cond_avail);
    }
}

/* Thread main routine.
   Parameters:
     * instance: instance of the FrameCapture object.
*/
void *
FrameCapture::thread_main(void *instance)
{
    ((FrameCapture *)instance)->run();
    return 0;
}

