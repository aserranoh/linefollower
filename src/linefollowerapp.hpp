
#ifndef LINEFOLLOWERAPP_HPP
#define LINEFOLLOWERAPP_HPP

#include <glm/vec2.hpp>
#include <map>
#include <vector>

#include "camparams.hpp"
#include "command.hpp"
#include "framecapture.hpp"
#include "motors.hpp"
#include "options.hpp"
#include "pathfinder.hpp"
#include "pilot.hpp"
#include "road.hpp"
#include "roadfinder.hpp"
#include "virtualtrack.hpp"

using namespace cv;
using namespace std;

class LineFollowerApp {

    public:

        /* Constructor.
           Parameters:
             * config: name of the configuration file.
        */
        LineFollowerApp(const char *options_file);

        ~LineFollowerApp();

        // Run the main loop of the application
        void run();

    private:

        // Application options
        Options options;

        // The camera parameters
        cam_params_t cam_params;

        // The frame capture object
        FrameCapture capture;

        // Motors (one virtual and one real)
        Motors *virtual_motors;
        Motors *real_motors;

        // The road finder instance and the road object
        RoadFinder *road_finder;
        Road road;

        // The path finder instance and the path object
        PathFinder *path_finder;
        vector<glm::vec2> path;

        // The PID modules that pilot the motors (one for the real and one
        // for the virtual)
        Pilot virtual_motors_pilot;
        Pilot real_motors_pilot;

        // True when the application stop has been requested
        bool stop_req;

        // Command receiver
        Command command;

        // Flags to start the line following
        bool following;

        // Commanded speed
        float speed;
        float turn;

        // Flag that tells if there's display or not
        bool has_display;

        // PRIVATE METHODS

        // Create the frame capture instance
        void create_frame_capture();

        // Create the motors
        void create_motors();

        // Create the path finder instance
        void create_path_finder();

        // Create the road finder instance
        void create_road_finder();

        // Draw the path
        void draw_path(Mat& frame);

        // Draw the road
        void draw_road(Mat& frame);

        // Get the inputs: camera frame
        void inputs(Mat& frame);

        // Move the motors, either in autonomous or in manual mode.
        void move_motors();

        /* Send outputs: motors movement
           Parameters:
             * frame: current frame
        */
        void outputs(Mat& frame);

        // Process the current frame
        void processing(Mat& frame);

        /* Set the default options.
           Parameters:
             * defaults: map where to put the default options.
        */
        void set_default_options(map<string, string>& defaults);

        // Start the motors (switch to autonomous mode)
        void start_motors();

        // Stop the motors (switch to manual mode)
        void stop_motors();

};

#endif

