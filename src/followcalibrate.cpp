
/*
CAMERA CALIBRATION
==================

To calibrate the camera, we capture an image of a straight segment of the road,
centered in the middle of the image. In this segment, we know that there's
strong color transitions at x = -14, x = -1, x = 1 and x = 14. We trace some
scanlines and we get the X values of these transitions in screen coordinates.
Then, we explore the domain of values for the two variables (camera Z, camera
angle), and for each set of values we calculate the expected X in the screen
for the known points in the scanlines.

Then for each scanline and value of X we compute the square of the difference
between the found X and the expected X, and sum that values for all the
scanlines and values of X. We keep the set of values (camera Z, camera angle)
that minimizes this sum.

*/

#include <err.h>    // errx
#include <getopt.h> // getopt_long
#include <limits.h> // INT_MAX
#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include "opencv2/opencv.hpp"

#include "config.h"
#include "cameraparameters.hpp"
#include "utilities.hpp"

// Short options:
//   * h: help
//   * v: version
#define OPTSTRING   "hvH:V:"

// Name of the program, to use it in the version and help string
#define PROGNAME    "follow-calibrate"

// TODO: command line parameters
#define DEFAULT_NUM_SCANLINES   5
#define DEFAULT_FIRST_SCANLINE  50
#define DEFAULT_SEP_SCANLINES   80
#define DEFAULT_CAM_ANGLE_INC   0.005
#define DEFAULT_MAX_CAM_Z       5.0
#define DEFAULT_CAM_Z_INC       0.02

#define NUM_EDGES_SCANLINE  2
#define LINE_LEFT_LIMIT     -1.0
#define LINE_RIGHT_LIMIT    1.0

using namespace cv;
using namespace utilities;

// Number of scanlines to use for calibration
size_t num_scanlines = DEFAULT_NUM_SCANLINES;

// First scanline offset
int first_scanline = DEFAULT_FIRST_SCANLINE;

// Separation (in pixels) between scanlines
int sep_scanlines = DEFAULT_SEP_SCANLINES;

// Increment of camera angle per iteration
float cam_angle_inc = DEFAULT_CAM_ANGLE_INC;

// Maximum camera Z
float max_cam_z = DEFAULT_MAX_CAM_Z;

// Camera Z increment
float cam_z_inc = DEFAULT_CAM_Z_INC;

// Camera's FOV
float fovh = 0.0, fovv = 0.0;

// Camera's Viewport
size_t width, height;

// Work vector
int *aux_row;

// Print help message and exits
void
print_help()
{
    printf("Usage: " PROGNAME " [options]\n"
"Options:\n"
"  -h, --help                  Show this message and exit.\n"
"  -v, --version               Show version information.\n\n"

"Report bugs to:\n"
"Antonio Serrano Hernandez (" PACKAGE_BUGREPORT ")\n"
    );
    exit(0);
}

// Print version message and exits
void
print_version()
{
    printf(PROGNAME " (" PACKAGE_NAME ") " PACKAGE_VERSION "\n"
"Copyright (C) 2018 Antonio Serrano\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
    );
    exit(0);
}

// Parse the command line arguments
void
parse_args(int argc, char **argv)
{
    struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"fovh", required_argument, 0, 'H'},
        {"fovv", required_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    int o;

    do {
        o = getopt_long(argc, argv, OPTSTRING, long_opts, 0);
        switch (o) {
            case 'h':
                print_help();
            case 'v':
                print_version();
            case 'H':
                fovh = strtof(optarg, NULL);
                break;
            case 'V':
                fovv = strtof(optarg, NULL);
                break;
            case '?':
                exit(1);
            default:
                break;
        }
    } while (o != -1);

    // Check the FOV values
    if (fovh <= 0.0) {
        errx(1, "wrong fovh value");
    }
    if (fovv <= 0.0) {
        errx(1, "wrong fovv value");
    }
}

/* Ask the user to center the robot and wait until she/he presses a key.
   Parameters:
     * cap: the VideoCapture object.
     * frame: the output frame, to use for calibration.
*/
void
wait_center(VideoCapture &cap, Mat &frame)
{
    printf("Center the green line with the road's white central line.\n"
        "Then press any key to continue.\n");
    // Show a frame and wait until the user presses a key
    while (1) {
        cap >> frame;
        // Draw a green line in the center
        cv::line(frame, Point(frame.cols/2, 0),
            Point(frame.cols/2, frame.rows - 1), Scalar(0, 255, 0));
        imshow("camera", frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }
    // Recapture the frame to return it without the vertical line
    cap >> frame;
}

/* Find the Y values of the scanlines to work with.
   Parameters:
     * frame: the image of the road.
     * num_scanlines: number of scanlines to generate.
     * sep_scanlines: separation (in pixels) between two consecutive scanlines.
   Return a new allocated array of num_scanlines positions with the Y
   coordinate for each scanline.
*/
int*
generate_scanlines(Mat& frame, size_t num_scanlines, int sep_scanlines)
{
    int *scanlines = new int[num_scanlines];
    int row = frame.rows - 1 - first_scanline;

    // Check if all the scanlines fit in the image
    if (row - sep_scanlines * ((int)num_scanlines - 1) < 0) {
        errx(1, "cannot generate scanlines: last scanline = %d",
            row - sep_scanlines * ((int)num_scanlines - 1));
    } else {
        // Calculate the Y for the scanlines
        for (int i = 0; i < num_scanlines; i++) {
            scanlines[i] = row;
            row -= sep_scanlines;
        }
    }
    return scanlines;
}

/* Find real line position in the given scanlines.
   Parameters:
     * frame: the image of the road.
     * scanlines: the list of Y values of the scanlines.
     * num_scanlines: number of scanlines.
*/
int*
find_line_limits(Mat& frame, int* scanlines, size_t num_scanlines)
{
    int *line_limits = new int[num_scanlines * NUM_EDGES_SCANLINE];
    uchar *ptr;
    int min, minpos, max, maxpos;
    Mat gray_frame;

    aux_row = new int[frame.cols + 1];
    // Convert the image to gray
    cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
    for (int i = 0; i < num_scanlines; i++) {
        ptr = gray_frame.data + scanlines[i] * gray_frame.cols;
        // Find the line position
        derivative(ptr, gray_frame.cols, aux_row);
        minmax(aux_row, gray_frame.cols, min, minpos, max, maxpos);
        // Draw the scanline and the found line limits
        cv::line(frame, Point(0, scanlines[i]),
            Point(frame.cols, scanlines[i]), Scalar(0, 0, 255));
        circle(frame, Point(minpos, scanlines[i]), 3, Scalar(255, 0, 0), 2);
        circle(frame, Point(maxpos, scanlines[i]), 3, Scalar(255, 0, 0), 2);
        line_limits[2 * i] = maxpos;
        line_limits[2 * i + 1] = minpos;
    }
    return line_limits;
}

/* Do the calibration process.
   Takes samples of angle and camera Z and compares the expected distances
   with the real ones.
   Return the values of angle and camera Z that minimizes the sum of the
   square of the differences between the expected values and the real ones.
   Parameters:
     * scanlines: Y position of the scanlines (in screen coordinates).
     * num_scanlines: number of scanlines used.
     * road_limits: for each scanline, two values that contain the X
         coordinates of the left and right line limits.
     * cam_angle: output value of the camera angle.
     * cam_z: output value of the camera z.
*/
void
calibrate(int *scanlines, size_t num_scanlines, int *line_limits,
    float& cam_angle, float& cam_z)
{
    int min_e = INT_MAX;

    // Generate all the combinations of cam_angle and cam_z
    int it = 0;
    for (float a = CameraParameters::min_angle;
        a < CameraParameters::max_angle; a += cam_angle_inc)
    {
        for (float z = CameraParameters::min_z; z < max_cam_z; z += cam_z_inc)
        {
            int e = 0;

            // Create a set of camera paramters
            CameraParameters cp(width, height, fovv, fovh, z, a);
            // Calculate the errors in the line limits
            for (int i = 0; i < num_scanlines; i++) {
                //printf("angle: %.2f, z: %.2f\n", cam_angle, z);
                float wy = cp.get_world_y(scanlines[i]);
                //printf("first scanline: %d, wy: %.2f\n", scanlines[i], wy);
                int wx0 = cp.get_screen_x(LINE_LEFT_LIMIT, wy);
                int wx1 = cp.get_screen_x(LINE_RIGHT_LIMIT, wy);
                e += (wx0 - line_limits[2*i]) * (wx0 - line_limits[2*i]);
                e += (wx1 - line_limits[2*i+1]) * (wx1 - line_limits[2*i+1]);
                //printf("wx0: %d, wx1: %d\n", wx0, wx1);
            }
            // Check if we found a better solution
            if (e < min_e) {
                min_e = e;
                cam_angle = a;
                cam_z = z;
            }
            it++;
            printf("%d: a: %.2f, z: %.2f, e: %d, min_e: %d\n", it, a, z, e, min_e);
        }
    }
}

int
main(int argc, char **argv)
{
    VideoCapture cap(0);
    Mat frame;
    int *scanlines;
    int *line_limits;
    float cam_angle, cam_z;

    parse_args(argc, argv);
    // Check if the camera is well opened
    if(!cap.isOpened()) {
        errx(1, "cannot open camera");
        exit(1);
    } else {
        width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    }
    // Create a window
    namedWindow("camera", 1);
    // Start the calibration process
    wait_center(cap, frame);
    //scanlines = generate_scanlines(frame, num_scanlines, sep_scanlines);
    //line_limits = find_line_limits(frame, scanlines, num_scanlines);
    int scanlines_static[] = {429, 379, 329, 279, 229};
    scanlines = scanlines_static;
    int line_limits_static[] = {181, 463, 209, 440, 238, 411, 263, 383, 289, 358};
    line_limits = line_limits_static;
    calibrate(scanlines, num_scanlines, line_limits, cam_angle, cam_z);

    // Print the results
    printf("camera angle: %.2f deg\n", to_deg(cam_angle));
    printf("camera Z: %.2f cm\n", cam_z);
}

