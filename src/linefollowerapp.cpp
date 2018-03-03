
#include <err.h>
#include <glm/glm.hpp>
#include <math.h>
#include "opencv2/opencv.hpp"

#include "config.h"
#include "differentialroadfinder.hpp"
#include "followexception.hpp"
#include "gpiomotors.hpp"
#include "linefollowerapp.hpp"
#include "realcamera.hpp"
#include "ssfapathfinder.hpp"
#include "utilities.hpp"

#define NS_PER_SECOND   1000000000

#include "virtualmotors.hpp"

#ifdef WITH_GTK

#define ROAD_SCALE  4
#define X_TO_SCR(x) (x * ROAD_SCALE + frame_width/2)
#define Y_TO_SCR(y) (frame_height - y * ROAD_SCALE)
#define ROAD_COLOR  Scalar(255, 0, 0)
#define LINE_COLOR  Scalar(0, 255, 255)
#define GOAL_COLOR  Scalar(0, 0, 255)
#define PATH_COLOR  Scalar(0, 255, 0)

#endif

using namespace utilities;

const char* LineFollowerApp::options_default[] = {
    "Camera", "real",
    "CameraWidth", "640",
    "CameraHeight", "480",
    "VideoCaptureIndex", "0",
    "Motors", "real",
    "RealMotorsType", "gpio",
    "RoadFinder", "differential",
    "ScanLines", "16",
    "MinDerivative", "30",
    "ColorDistanceThreshold", "1200",
    "ScanLinesFrame", "world",
    "PathFinder", "SSFA",
    "Port", "10101",
    "InactivityTimeout", "300",
    "VertexShader", DATADIR "/vertex.sl",
    "FragmentShader", DATADIR "/fragment.sl",
    "TexturesPath", DATADIR,
    "RoadDelta", "15.0",
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
    create_road_finder();
    create_path_finder();
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

// PRIVATE FUNCTIONS

// Create the motors
void
LineFollowerApp::create_motors()
{
    string motors_types = options.get_string("Motors");
    string real_motors_type;
    float max_speed, kp, ki, kd;

    // Read the parameters for the motors pilots
    max_speed = options.get_float("MaxSpeed");
    kp = options.get_float("Kp");
    ki = options.get_float("Ki");
    kd = options.get_float("Kd");

#ifdef WITH_GLES2
    if (motors_types == "virtual" || motors_types == "both") {
        // The type of camera must be virtual to use virtual motors
        if (options.get_string("Camera") != "virtual") {
            warnx("type of camera must be virtual to use virtual motors");
        } else {
            try {
                // Create virtual motors
                // TODO: pass options to the motors
                virtual_motors = new VirtualMotors(capture.get_camera(),
                    options.get_float("VirtualMotorsRpm"),
                    options.get_float("WheelDistance"),
                    options.get_float("WheelDiameter"));
                // Create the virtual motors pilot
                // TODO: pass options to Pilot
                virtual_motors_pilot = Pilot(
                    virtual_motors, max_speed, kp, ki, kd);
            } catch (exception &e) {
                warnx("cannot create virtual motors: %s", e.what());
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
                // TODO: pass options to motors
                real_motors = new GPIOMotors(
                    options.get_int("GPIOMotorsPWMLeft"),
                    options.get_int("GPIOMotorsPWMRight"),
                    options.get_int("GPIOMotorsDirection0Left"),
                    options.get_int("GPIOMotorsDirection1Left"),
                    options.get_int("GPIOMotorsDirection0Right"),
                    options.get_int("GPIOMotorsDirection1Right"),
                    NS_PER_SECOND/options.get_int("GPIOMotorsPWMFrequency"),
                    options.get_float("WheelDistance"));
            }
            // Create the real motors pilot
            // TODO: pass options to pilot
            real_motors_pilot = Pilot(real_motors, max_speed, kp, ki, kd);
        } catch (exception &e) {
            warnx("cannot create real motors: %s", e.what());
        }
    }
}

// Create the path finder instance
void
LineFollowerApp::create_path_finder()
{
    string path_finder_type = options.get_string("PathFinder");
    if (path_finder_type == "SSFA") {
        path_finder = new SSFAPathFinder();
    } else {
        throw FollowException("unknown path finder type");
    }
}

// Create the road finder instance
void
LineFollowerApp::create_road_finder()
{
    string road_finder_type = options.get_string("RoadFinder");
    if (road_finder_type == "differential") {
        road_finder = new DifferentialRoadFinder(options);
    } else {
        throw FollowException("unknown road finder type");
    }
}

// These functions are only defined if there's support for gtk
#ifdef WITH_GTK

// Draw the path
void
LineFollowerApp::draw_path(Mat& frame)
{
    glm::vec2 p0, p1;

    for (size_t i = 1; i < path.size(); i++) {
        p0 = path[i - 1];
        p1 = path[i];
        line(frame, Point(X_TO_SCR(p0[0]), Y_TO_SCR(p0[1])),
            Point(X_TO_SCR(p1[0]), Y_TO_SCR(p1[1])), PATH_COLOR);
    }
}

// Draw the road
void
LineFollowerApp::draw_road(Mat& frame)
{
    road_section_t s;

    for (size_t i = 0; i < road.get_size(); i++) {
        s = road.get_section(i);
        // Draw the left point of the current section
        circle(frame, Point(X_TO_SCR(s.left[0]), Y_TO_SCR(s.left[1])), 3,
            ROAD_COLOR, 2);
        // Draw the right point of the current section
        circle(frame, Point(X_TO_SCR(s.right[0]), Y_TO_SCR(s.right[1])), 3,
            ROAD_COLOR, 2);
        // Draw the line point of the current section
        if (s.line[0] != FLT_MAX) {
            circle(frame, Point(X_TO_SCR(s.line[0]), Y_TO_SCR(s.line[1])), 3,
                LINE_COLOR, 2);
        }
    }
    // Draw the goal
    glm::vec2 goal = road.get_goal();
    circle(frame, Point(X_TO_SCR(goal[0]), Y_TO_SCR(goal[1])), 3,
        GOAL_COLOR, 2);
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
        // The input to the pilot is the angle between the current direction
        // and the target direction. We use the approximation x ~= sin(x) near
        // 0 and calculate the sin by normalizing the direction vector
        // TODO: See why point 0 and point 1 are the same
        glm::vec2 v = glm::normalize(path[2] - path[0]);
        if (virtual_motors)
            virtual_motors_pilot.set_angle(-v[0]);
        if (real_motors)
            real_motors_pilot.set_angle(-v[0]);
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
        draw_road(frame);
        draw_path(frame);
        // This is necessary to show the window
        cv::waitKey(1);
        imshow("frame", frame);
    }
#endif

    // Print the fps
    printfps(1);

    move_motors();
    // Send the road and path to the possible subscriptors
    command.send_data(road, path);
}

/* Process the current frame.
   Parameters:
     * frame: current frame to process.
*/
void
LineFollowerApp::processing(Mat& frame)
{
    road_finder->find(frame, road);
    path_finder->find(road, path);
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

