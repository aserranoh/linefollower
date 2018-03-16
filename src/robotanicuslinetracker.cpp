
// This algorithm was inspired by this article:
// https://www.raspberrypi.org/blog/an-image-processing-robot-for-robocup-junior/
// Credits to Arne Baeyens, alias Robotanicus

#include "robotanicuslinetracker.hpp"

#include <math.h>

#include "followexception.hpp"
#include "utilities.hpp"

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
    horizontal_scanline_offset(options.get_int("HorizontalScanlineOffset")),
    num_scan_circles(options.get_int("NumScanCircles")),
    scan_circle_radius(options.get_float("ScanCircleRadius")),
    ref_min(INT_MIN), ref_max(INT_MAX)
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
    int xaxis, yaxis;

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
    line.add(x, y, wx, wy);

    // Track the line using the scan-circles
    // Initially, the center of the scan circle is the point found with the
    // horizontal scanline
    for (int i = 0; i < num_scan_circles; i++) {
        // The radius of the scan-circle depends on the distance to the center
        // of the circle
        get_scan_circle_axis(wy, y, xaxis, yaxis);
        // The initial angle of scan depends on the angle between the last
        // two points
        const line_point_t &p = line.get_point(line.size() - 1);
        angle = p.sangle - M_PI/2.0;
        //ellipse(frame, Point(x, y), Size(xaxis, yaxis), 0, -to_deg(angle), -to_deg(angle) - 180, Scalar(255, 0, 0));
        ellipse(frame, Point(x, y), Size(xaxis, yaxis), 0, 0, 360, Scalar(255, 0, 0));
        find_line_scan_circle(gray_frame, x, y, xaxis, yaxis, angle, x, y);
        if (x == INT_MAX) {
            break;
        }
        circle(frame, Point(x, y), 5, Scalar(0, 0, 255), -1);
        screen_to_world(x, y, wx, wy);
        line.add(x, y, wx, wy);
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
    const Mat &frame, int& x, int& y)
{
    // Calculate the derivative of the scanline
    size_t row_offset = cam_params.height - 1 - horizontal_scanline_offset;
    uchar *ptr = frame.data + frame.cols * row_offset;
    derivative(ptr, frame.cols, aux_row);

    // Search the absolutes minimum and maximum values in the derivative
    int min, max, minpos, maxpos;
    minmax(aux_row, frame.cols, min, minpos, max, maxpos);
    // The first time keep the values min and max as reference values
    if (ref_min == INT_MIN) {
        ref_min = min;
        ref_max = max;
    }

    // Check the cases when the line is at one side of the image
    if (min > ref_min*0.5) {
        // We "didn't" find min, so only Black->White transition was fount,
        // meaning that the line is in the right side
        x = (maxpos + cam_params.width) / 2;
    } else if (max < ref_max*0.5) {
        // We "didn't" find max, so only White->Black transition was found,
        // meaning that the line is in the left side
        x = minpos/2;
    } else {
        // Normal case, both min and max were found
        x = (minpos + maxpos) / 2;
    }
    y = row_offset;
}

/* Find the position of the next point of the line using a scan circle.
   Although it is called scan circle an ellipse is used because its a better
   approximation of a circle projected to a plane in perspective.
   Parameters:
     * frame: the image.
     * cx: X coordinate of the center of the scan circle.
     * cy: Y coordinate of the center of the scan circle.
     * xaxis: horizontal axis of the scan circle.
     * yaxis: vertical axis of the scan circle.
     * angle: start angle of the scan circle.
     * x: the output x coordinates of the line.
     * y: tye output y coordinates of the line.
*/
void
RobotanicusLineTracker::find_line_scan_circle(const Mat& frame, int cx, int cy,
    int xaxis, int yaxis, float angle, int& x, int& y)
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
    int j = 0, row, col, start = -1, end = -1;
    for (int k = i; j < NUM_ANGLES/2 && end < 0;
        j++, k = (k + 1) & (NUM_ANGLES - 1))
    {
        row = cy - (int)((float)yaxis * table_angles[k].sin);
        col = cx + (int)((float)xaxis * table_angles[k].cos);
        // Check that the pixel is inside the image
        if (row < 0 || row >= cam_params.height
            || col < 0 || col >= cam_params.width)
        {
            if (start >= 0) {
                end = j;
            }
        } else {
            if (start < 0) {
                start = j;
            }
            scan_circle[j] = *(frame.data + row * frame.cols + col);
        }
    }

    if (start < 0) {
        x = y = INT_MAX;
    } else {
        if (end < 0) {
            end = j;
        }
        // Find the derivative of the array
        derivative(scan_circle + start, end - start, aux_row);

        // Search the absolutes minimum and maximum values in the derivative
        int min, max, minpos, maxpos;
        minmax(aux_row, end - start, min, minpos, max, maxpos);
        // Check the cases where the line is not completely inside the scan
        // circle
        if (min > ref_min*0.5 || max < ref_max*0.5) {
            x = y = INT_MAX;
        } else {
            int angle_index = ((minpos + maxpos)/2 + start + i)
                & (NUM_ANGLES - 1);
            x = cx + (int)((float)xaxis * table_angles[angle_index].cos);
            y = cy - (int)((float)yaxis * table_angles[angle_index].sin);
        }
    }
}

/* Get the major and minor axis of the scan circle (ellipse) to use. The axis
   of the scan circle depends on the y coordinate of the center of the circle:
   the far from the viewer the circle is in the image, the smaller the two
   axis.
   Parameters:
     * y: Y coordinate of the circle center, in world coordinates.
     * sy: Y coordinate of the circle center, in screen coordinates.
     * xaxis: output parameter that contains the circle horizontal axis.
     * yaxis: output parameter that contains the circle vertical axis.
*/
void
RobotanicusLineTracker::get_scan_circle_axis(float y, int sy, int& xaxis,
    int& yaxis) const
{
    float kx = k4 + k5*y;
    float sy1 = (k1 - (y + scan_circle_radius)*k3)
        / ((y + scan_circle_radius) + k2);
    xaxis = (int)(scan_circle_radius/kx);
    yaxis = sy - sy1;
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

