
#include <glm/glm.hpp>
#include <math.h>
#include "opencv2/opencv.hpp"

#include "config.h"
#include "followexception.hpp"
#include "gpiomotors.hpp"
#include "linefollowerapp.hpp"
#include "log.hpp"
#include "realcamera.hpp"
#include "robotanicuslinetracker.hpp"
#include "utilities.hpp"

#include "virtualmotors.hpp"

#ifdef WITH_GTK

#define LINE_SCALE  10
#define X_TO_SCR(x) (x * LINE_SCALE + frame_width/2)
#define Y_TO_SCR(y) (frame_height - y * LINE_SCALE)
#define LINE_COLOR  Scalar(0, 255, 255)

#endif

using namespace utilities;

const char* LineFollowerApp::options_default[] = {
    "Camera", "real",
    "CameraWidth", "640",
    "CameraHeight", "480",
    "VideoCaptureIndex", "0",
    "Motors", "real",
    "RealMotorsType", "gpio",
    "LineTracker", "robotanicus",
    "HorizontalScanlineOffset", "150",
    "NumScanCircles", "5",
    "ScanCircleRadius", "1.5",
    "Port", "10101",
    "InactivityTimeout", "300",
    "VertexShader", DATADIR "/" PACKAGE_NAME "/vertex.sl",
    "FragmentShader", DATADIR "/" PACKAGE_NAME "/fragment.sl",
    "TexturesPath", DATADIR "/" PACKAGE_NAME,
    NULL, NULL,
};

LineFollowerApp::LineFollowerApp(const char *options_file):
    options(options_file, options_default),
    frame_width(options.get_int("CameraWidth")),
    frame_height(options.get_int("CameraHeight")), capture(options),
    virtual_motors(0), real_motors(0), stop_req(false), command(options),
    following(false), speed(0.0), turn(0.0),
    has_display(getenv("DISPLAY") != NULL)
{
    create_motors();
    create_line_tracker();
    //start_motors();
}

LineFollowerApp::~LineFollowerApp()
{
    command.close();
    if (virtual_motors)
        delete virtual_motors;
    if (real_motors)
        delete real_motors;
}

// Run the main loop of the application
void
LineFollowerApp::run()
{
    cv::Mat frame;

    // Create a window, only if a display is available and supported
#ifdef WITH_GTK
    if (has_display)
        namedWindow("frame", 1);
#endif

    // Fetch the first frame
    capture.fetch();

    while (!stop_req) {
        inputs(frame);
        processing(frame);
        outputs(frame);
    }
}

// Stop the main loop
void
LineFollowerApp::stop()
{
    stop_req = true;
}

// PRIVATE FUNCTIONS

// Create the motors
void
LineFollowerApp::create_motors()
{
    string motors_types = options.get_string("Motors");
    string real_motors_type;

#ifdef WITH_GLES2
    if (motors_types == "virtual" || motors_types == "both") {
        // The type of camera must be virtual to use virtual motors
        if (options.get_string("Camera") != "virtual") {
            log_warn("type of camera must be virtual to use virtual motors");
        } else {
            try {
                // Create virtual motors
                virtual_motors = new VirtualMotors(capture.get_camera(),
                    options);
                // Create the virtual motors pilot
                virtual_motors_pilot = Pilot(virtual_motors, options);
            } catch (FollowException &e) {
                log_warn(e.what());
            }
        }
    }
#endif
    if (motors_types == "real" || motors_types == "both") {
        // Create real motors
        try {
            real_motors_type = options.get_string("RealMotorsType");
            if (real_motors_type == "gpio") {
                // Create GPIO motors
                real_motors = new GPIOMotors(options);
            }
            // Create the real motors pilot
            real_motors_pilot = Pilot(real_motors, options);
        } catch (FollowException &e) {
            log_warn(e.what());
        }
    }
}

// Create the line tracker instance
void
LineFollowerApp::create_line_tracker()
{
    string line_tracker_type = options.get_string("LineTracker");
    if (line_tracker_type == "robotanicus") {
        line_tracker = new RobotanicusLineTracker(options);
    } else {
        throw FollowException("unknown line tracker type");
    }
}

// These functions are only defined if there's support for gtk
#ifdef WITH_GTK

/* Draw the line.
   Parameters:
     * frame: the destination frame of the drawing.
*/
void
LineFollowerApp::draw_line(Mat& frame)
{
    for (size_t i = 0; i < line.size(); i++) {
        const line_point_t& p = line.get_point(i);
        circle(frame, Point(X_TO_SCR(p.wx), Y_TO_SCR(p.wy)), 3, LINE_COLOR, 2);
    }
    // Draw a line in the middle of the screen
    cv::line(frame, Point(frame.cols/2, 0), Point(frame.cols/2, frame.rows),
        Scalar(0, 255, 0));
    // Draw a line fromt the camera to the target point of the line
    const line_point_t& p = line.get_point(0);
    cv::line(frame, Point(X_TO_SCR(p.wx), 0),
        Point(X_TO_SCR(p.wx), frame.rows), Scalar(0, 255, 255));
}

#endif

/* Get the inputs: camera frame.
   Parameters:
     * frame: variable where to write the next camera frame.
*/
void
LineFollowerApp::inputs(Mat& frame)
{
    msg_t cmd;

    // Get the next frame and launch the capture of a new one in parallel
    frame = capture.next();
    capture.fetch();

    // Process input commands
    while (command.get_command(cmd)) {
        switch (cmd.type) {
            case CMD_START: start_motors(); break;
            case CMD_STOP: stop_motors(); break;
            case CMD_MOVE:
                // Keep the speed and turn received values
                speed = cmd.speed;
                turn = cmd.turn;
                break;
            default: break;
        }
    }
}

// Move the motors, either in autonomous or in manual mode.
void
LineFollowerApp::move_motors()
{
    msg_t evt;
    bool moved = false;

    if (following) {
        // Autonomous mode, let pilot do its thing.
        if (virtual_motors)
            virtual_motors_pilot.pilot(line);
        if (real_motors)
            real_motors_pilot.pilot(line);
    } else {
        // Manual mode, move the motors the given amount in the command and
        // send back an event of confirmation
        if (virtual_motors) {
            virtual_motors->move(speed, turn);
            moved = true;
        }
        if (real_motors) {
            real_motors->move(speed, turn);
            moved = true;
        }
        if (moved) {
            evt.type = EVT_MOVED;
            evt.speed = speed;
            evt.turn = turn;
            command.send_event(evt);
        }
    }
}

/* Send outputs: motors movement
   Parameters:
     * frame: current frame
*/
void
LineFollowerApp::outputs(Mat& frame)
{
#ifdef WITH_GTK
    if (has_display) {
        draw_line(frame);
        // This is necessary to show the window
        cv::waitKey(1);
        imshow("frame", frame);
    }
#endif
    // Print the fps
    printfps(1);
    move_motors();
    // Send the tracked line to the possible subscriptors
    command.send_data(line);
}

/* Process the current frame.
   Parameters:
     * frame: current frame to process.
*/
void
LineFollowerApp::processing(Mat& frame)
{
    // Track the line
    line_tracker->track(frame, line);
    // Find the path to follow
    //path_finder->find(road, path);
}

// Start the motors (switch to autonomous mode)
void
LineFollowerApp::start_motors()
{
    msg_t evt;

    // Flag that indicates the autonomous mode
    following = true;
    // Start the motors
    if (virtual_motors)
        virtual_motors->start();
    if (real_motors)
        real_motors->start();
    // Send an event back
    evt.type = EVT_STARTED;
    command.send_event(evt);
}

// Stop the motors (switch to manual mode)
void
LineFollowerApp::stop_motors()
{
    msg_t evt;

    // Flag that indicates the manual mode
    following = false;
    // Stop the motors
    if (virtual_motors)
        virtual_motors->stop();
    if (real_motors)
        real_motors->stop();
    // Send an event back
    evt.type = EVT_STOPPED;
    command.send_event(evt);
}

