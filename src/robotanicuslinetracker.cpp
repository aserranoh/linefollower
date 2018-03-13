
// This algorithm was inspired by this article:
// https://www.raspberrypi.org/blog/an-image-processing-robot-for-robocup-junior/
// Credits to Arne Baeyens, alias Robotanicus

#include "robotanicuslinetracker.hpp"

#include <math.h>

#include "followexception.hpp"
#include "utilities.hpp"

// TODO: Application's param
#define HORIZONTAL_SCANLINE_OFFSET  50
#define NUM_SCAN_CIRCLES    5
#define SCAN_CIRCLE_RADIUS  1.5f

#define NUM_ANGLES  128

using namespace utilities;

// Type that stores information of an angle
typedef struct {
    float angle;
    float sin;
    float cos;
} angle_t;

// Table of angles
static angle_t table_angles[NUM_ANGLES];

/* Constructor.
   Parameters:
     * options: application's options.
*/
RobotanicusLineTracker::RobotanicusLineTracker(const Options& options):
    cam_params(options.get_int("CameraWidth"), options.get_int("CameraHeight"),
        options.get_float("CameraFovh"), options.get_float("CameraFovv"),
        options.get_float("CameraZ"),
        to_rad(options.get_float("CameraAngle"))),
    horizontal_scanline_offset(HORIZONTAL_SCANLINE_OFFSET),
    num_scan_circles(NUM_SCAN_CIRCLES),
    scan_circle_radius(SCAN_CIRCLE_RADIUS)
{
    // Check the horizontal_scanline_offset param
    if (horizontal_scanline_offset > cam_params.height - 1) {
        throw FollowException(
            string("horizontal_scanline_offset must be at most")
            + std::to_string(cam_params.height - 1));
    }
    // Initialize the working row
    aux_row_size = cam_params.width;
    aux_row = new int[cam_params.width];

    // Compute some constants to accelerate the coordinates conversions
    float tan_cam_angle = tan(cam_params.cam_angle);
    float kv = tan(to_rad(cam_params.fovv/2.0)) / (cam_params.height/2.0);
    k2 = cam_params.cam_z * tan_cam_angle;
    k1 = cam_params.cam_z/kv + k2 * cam_params.height/2.0;
    k3 = tan_cam_angle/kv - cam_params.height/2.0;
    float kh = tan(to_rad(cam_params.fovh/2.0)) / (cam_params.width/2.0);
    k4 = kh * sin(cam_params.cam_angle) * cam_params.cam_z;
    k5 = kh * cos(cam_params.cam_angle);

    // Build the table of angles
    float angle;
    for (int i = 0; i < NUM_ANGLES; i++) {
        angle = 2.0 * M_PI * (float)i / (float)NUM_ANGLES;
        table_angles[i].angle = angle;
        table_angles[i].sin = sin(angle);
        table_angles[i].cos = cos(angle);
    }
}

RobotanicusLineTracker::~RobotanicusLineTracker()
{}

/* Track the line in the image.
   Parameters:
     * frame: image of the line.
     * line: the output line to follow.
*/
void
RobotanicusLineTracker::track(Mat& frame, Line& line)
{
    // Coordinates of the current point in screen reference
    int x, y;
    // Coordinates of the current point in world reference
    float wx, wy;
    float angle;
    int radius;

    // Make sure that the frame is of the correct size
    if (frame.cols != cam_params.width || frame.rows != cam_params.height) {
        throw FollowException(string("input frame must be of size ")
            + std::to_string(cam_params.width) + "x"
            + std::to_string(cam_params.height));
    }

    // Clear the line to start from zero with the new frame
    line.clear();

    // Convert the image to gray
    cvtColor(frame, gray_frame, COLOR_BGR2GRAY);

    // Find line in the horizontal scanline
    find_line_horizontal_scanline(gray_frame, x, y);
    cv::line(frame, Point(0, y), Point(frame.cols, y), Scalar(255, 0, 0));
    circle(frame, Point(x, y), 5, Scalar(0, 0, 255), -1);
    // Add the point to the line (in world reference)
    screen_to_world(x, y, wx, wy);
    line.add(wx, wy);

    // Track the line using the scan-circles
    // Initially, the center of the scan circle is the point found with the
    // horizontal scanline
    for (int i = 0; i < num_scan_circles; i++) {
        // The radius of the scan-circle depends on the distance to the center
        // of the circle
        radius = get_scan_circle_radius(wy);
        // The initial angle of scan depends on the angle between the last
        // two points
        angle = line.get_last_angle() - M_PI/2.0;
        ellipse(frame, Point(x, y), Size(radius, radius), 0, -to_deg(angle), -to_deg(angle) - 180, Scalar(255, 0, 0));
        find_line_scan_circle(gray_frame, x, y, radius, angle, x, y);
        circle(frame, Point(x, y), 5, Scalar(0, 0, 255), -1);
        screen_to_world(x, y, wx, wy);
        line.add(wx, wy);
    }
}

// PRIVATE FUNCTIONS

/* Find the position of the line in the zone of the image traversed
   by an horizontal scanline.
   Parameters:
     * frame: the image.
     * x: the output x coordinates of the line.
     * y: the output y coordinates of the line.
*/
void
RobotanicusLineTracker::find_line_horizontal_scanline(
    const Mat &frame, int& x, int& y) const
{
    // Calculate the derivative of the scanline
    size_t row_offset = cam_params.height - 1 - horizontal_scanline_offset;
    uchar *ptr = frame.data + frame.cols * row_offset;
    derivative(ptr, frame.cols, aux_row);

    // Search the absolutes minimum and maximum values in the derivative
    int min, max;
    minmax(aux_row, frame.cols, min, max);
    x = (min + max) / 2;
    y = row_offset;
}

/* Find the position of the next point of the line using a scan circle.
   Parameters:
     * frame: the image.
     * cx: X coordinate of the center of the scan circle.
     * cy: Y coordinate of the center of the scan circle.
     * radius: radius of the scan circle.
     * angle: start angle of the scan circle.
     * x: the output x coordinates of the line.
     * y: tye output y coordinates of the line.
*/
void
RobotanicusLineTracker::find_line_scan_circle(const Mat& frame, int cx, int cy,
    int radius, float angle, int& x, int& y)
{
    int i;
    uchar scan_circle[NUM_ANGLES/2];

    // Search in the table the first angle to use
    for (i = 0; i < NUM_ANGLES; i++) {
        if (table_angles[i].angle >= angle) {
            break;
        }
    }

    // i contains the position of the first angle to use in the table of
    // angles
    // Build the array of grey pixel values
    int j = 0, row, col;
    for (int k = i; j < NUM_ANGLES/2; j++, k = (k + 1) & (NUM_ANGLES - 1)) {
        row = cy - (int)((float)radius * table_angles[k].sin);
        col = cx + (int)((float)radius * table_angles[k].cos);
        scan_circle[j] = *(frame.data + row * frame.cols + col);
    }

    // Find the derivative of the array
    derivative(scan_circle, NUM_ANGLES/2, aux_row);

    // Search the absolutes minimum and maximum values in the derivative
    int min, max;
    minmax(aux_row, NUM_ANGLES/2, min, max);
    int angle_index = ((min + max)/2 + i) & (NUM_ANGLES - 1);
    x = cx + (int)((float)radius * table_angles[angle_index].cos);
    y = cy - (int)((float)radius * table_angles[angle_index].sin);
}

/* Get the radius of the scan circle to use. The radius of the scan
   circle depends on the y coordinate of the center of the circle:
   the far from the viewer the circle is in the image, the smaller
   the radius.
   Parameters:
     * y: Y coordinate of the circle center, in world coordinates.
*/
int
RobotanicusLineTracker::get_scan_circle_radius(float y) const
{
    float kx = k4 + k5*y;
    return (int)(scan_circle_radius/kx);
}

/* Transform a point from screen to world coordinates.
   Parameters:
     * sx: x coordinates in screen frame.
     * sy: y coordinates in screen frame.
     * x: output x in world coordinates.
     * y: output y in world coordinates.
*/
void
RobotanicusLineTracker::screen_to_world(int sx, int sy, float& x, float& y)
{
    float kx;

    y = (k1 - k2*sy)/(k3 + sy);
    kx = k4 + k5*y;
    x = (sx - (float)cam_params.width/2.0)*kx;
}

