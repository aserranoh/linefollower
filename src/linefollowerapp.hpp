
#ifndef LINEFOLLOWERAPP_HPP
#define LINEFOLLOWERAPP_HPP

#include <glm/vec2.hpp>
#include <map>
#include <vector>

#include "camparams.hpp"
#include "command.hpp"
#include "framecapture.hpp"
#include "line.hpp"
#include "linetracker.hpp"
#include "motors.hpp"
#include "options.hpp"
#include "pilot.hpp"
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

        // Stop the main loop
        void stop();

    private:

        // Application options
        Options options;

        // The frame dimensions
        size_t frame_width;
        size_t frame_height;

        // The frame capture object
        FrameCapture capture;

        // Motors (one virtual and one real)
        Motors *virtual_motors;
        Motors *real_motors;

        // The line tracker instance and the line object
        LineTracker *line_tracker;
        Line line;

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

        // Default option values
        static const char* options_default[];

        // PRIVATE METHODS

        // Create the motors
        void create_motors();

        // Create the line tracker instance
        void create_line_tracker();

        /* Draw the line.
           Parameters:
             * frame: the destination frame of the drawing.
        */
        void draw_line(Mat& frame);

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

        // Start the motors (switch to autonomous mode)
        void start_motors();

        // Stop the motors (switch to manual mode)
        void stop_motors();

};

#endif

