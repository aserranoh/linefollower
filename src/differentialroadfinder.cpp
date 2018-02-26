
#include <glm/gtx/norm.hpp>
#include <math.h>

#include "differentialroadfinder.hpp"
#include "utilities.hpp"

// Originally this value was set to 16, but it can be too few depending on the
// surroundings of the road.
#define MAX_EDGES           32
#define MAX_WIDTH_PIXELS    3280

#define DEG_TO_RAD(d)       ((d) * M_PI / 180.0)

// TODO: Make it an algorithm parameter
#define ROAD_DELTA  15.0

using namespace utilities;

typedef enum {ST_START, ST_LANE1, ST_LINE, ST_LANE2, ST_END} state_t;

/* Constructor.
   Parameters:
     * cam_params: camera's parameters.
     * scanlines: number of lines of the original image to process.
     * min_derivative: value to use to filter the values in the derivative
           function.
     * color_distance_threshold: color distance threshold, used to identify
           white and black colors.
     * scanline_frame: SL_SCREEN, to use scanlines equidistant in screen
           coordinates or SL_WORLD, to use scanlines equidistant in world
           coordinates.
     * scanline_distance: if using the SL_WORLD frame, distance between
           the scanlines.
     * wheel_distance: distance between wheels (in cm).
*/
DifferentialRoadFinder::DifferentialRoadFinder(const cam_params_t& cam_params,
        size_t scanlines, int min_derivative, int color_distance_threshold,
        scanline_frame_t scanline_frame, float scanline_distance,
        float wheel_distance):
    cam_params(cam_params), scanlines(scanlines),
    min_derivative(min_derivative),
    color_distance_threshold(color_distance_threshold), scanline_params(0),
    wheel_distance(wheel_distance)
{
    init_scanline_params(scanline_frame, scanline_distance);
}

DifferentialRoadFinder::~DifferentialRoadFinder()
{
    if (scanline_params)
        delete[] scanline_params;
}

/* Find the road in the image.
   Parameters:
     * frame: image of the road.
     * road: output parameter that contains the road found.
*/
void
DifferentialRoadFinder::find(Mat& frame, Road& road)
{
    // NOTE: Add 2 to the edges array in order to account for the start and the
    //       end of the array.
    int edges[MAX_EDGES + 2];
    // NOTE: Add 1 to the color_segments array in order to add a sentinel.
    color_segment_t color_segments[MAX_EDGES + 2];
    size_t nedges;
    road_section_t section, prev_section = {{-1, -1}, {-1, -1}, {-1, -1}};
    uchar *gray_ptr, *color_ptr;
    bool goal_set = false;
    road_section_t last_section;

    // Empty the road
    road.clear();
    sections.clear();

    // 1) Convert the frame to gray color
    cvtColor(frame, gray_frame, COLOR_BGR2GRAY);

    // 2) Analize each row of the input frame.
    //    NOTE: The images are scanned top to bottom.
    for (size_t i = 0; i < scanlines; i++) {
        gray_ptr = gray_frame.data + scanline_params[i].offset;
        color_ptr = frame.data + scanline_params[i].offset * frame.elemSize();

        // 2.1) Obtain the edges between similar colors
        nedges = find_edges(gray_ptr, gray_frame.cols, edges);

        // 2.2) Obtain the color segments of the scanline
        find_color_segments(
            color_ptr, frame.elemSize(), edges, nedges, color_segments);

        // 2.3) Obtain the road limits of the scanline
        if (find_road_section(color_segments, nedges, i, prev_section,
            section))
        {
            sections.push_back(section);
            // 2.4) Update the goal
            if (section.line[0] != FLT_MAX) {
                road.set_goal(section.line);
                goal_set = true;
            }
            // Update the previous section
            prev_section = section;
        }
        // Draw the scanline
        /*line(frame, Point(0, scanline_params[i].offset/frame.cols),
            Point(frame.cols, scanline_params[i].offset/frame.cols),
            Scalar(0, 0, 255));*/
    }
    // 3) If the goal was not set, set it to the middle of the last section
    if (!goal_set && road.get_size()) {
        last_section = road.get_section(road.get_size());
        road.set_goal((last_section.left + last_section.right)*0.5f);
    }

    // 4) Reduce the road according to the dimensions of the robot
    add_secure_dist(road);
}

// PRIVATE FUNCTIONS

/* Add a security margin to both sides of the road to assure that the robot
   won't fall off the limits.
   Parameters:
     * road: the output road with the margins.
*/
void
DifferentialRoadFinder::add_secure_dist(Road& road)
{
    glm::vec2 ul, ur;
    float dist = wheel_distance/2.0;
    road_section_t s;
    size_t i;

    // Exit if there's no sections
    if (!sections.size())
        return;
    // Add the first section untouched
    road.add(sections[0]);
    for (i = 1; i < sections.size(); i++) {
        // Left
        ul = glm::normalize(sections[i].left - sections[i - 1].left);
        // As u is normalized, the x and y coordinates are the cos and sin
        // of the angle that the vector forms
        s.left[0] = sections[i].left[0] + dist/ul[1];
        s.left[1] = sections[i].left[1];

        // Right
        ur = glm::normalize(sections[i].right - sections[i - 1].right);
        s.right[0] = sections[i].right[0] - dist/ur[1];
        s.right[1] = sections[i].right[1];

        // Copy the line
        s.line = sections[i].line;

        // Add the new section
        if (s.left[0] < s.right[0])
            road.add(s);
    }
}

/* Convert the line of pixels in a sequence of color segments.
   Parameters:
     * row: pointer to the row in the image that contains the colored pixels.
     * channels: number of color channels in the source image.
     * edges: the positions of the segments edges.
     * nedges: the number of edges in the edges array.
     * color_segments: the colors of each segment.
*/
void
DifferentialRoadFinder::find_color_segments(uchar *row, size_t channels,
    int edges[], size_t nedges, color_segment_t color_segments[])
{
    size_t seglen;
    Scalar mean_color;
    uchar *ptr = row;
    size_t i;

    for (i = 0; i < nedges - 1; i++) {
        seglen = edges[i + 1] - edges[i];
        color_segments[i].start = edges[i];
        color_segments[i].length = seglen;
        // Compute the mean color of the current segment
        meancolor(ptr, seglen, channels, mean_color);
        // Classify the mean color in white, black or other
        if (colordistance(mean_color, Scalar(255, 255, 255))
            < color_distance_threshold)
        {
            color_segments[i].color = COLOR_WHITE;
        } else if (colordistance(mean_color, Scalar(0, 0, 0))
            < color_distance_threshold)
        {
            color_segments[i].color = COLOR_BLACK;
        } else {
            color_segments[i].color = COLOR_OTHER;
        }
        ptr += seglen * channels;
    }
    // Add a sentinel
    color_segments[i].start = color_segments[i - 1].start
        + color_segments[i - 1].length;
    color_segments[i].length = 0;
    color_segments[i].color = COLOR_OTHER;
    
}

/* Find the road limits.
   Parameters:
     * row: pointer to the row in the image to analize.
     * len: number of pixels in row.
     * edges: contains the found edges positions.
   Return the number of edges found.
*/
size_t
DifferentialRoadFinder::find_edges(uchar *row, size_t len, int edges[])
{
    size_t nedges;
    int aux_row[MAX_WIDTH_PIXELS];

    // 1) Compute the derivative of the row
    derivative(row, len, aux_row);
    // 2) Compute the absolute value of the derivative
    abs(aux_row, len, aux_row);
    // 3) Filter out (set to zero) the low values of the derivative
    filterlt(aux_row, len, min_derivative, aux_row);
    // 4) Search the local maximums (to stablish a point of max derivative)
    //    NOTE: leave position 0 for the start of the array.
    nedges = localmax(aux_row, len, MAX_EDGES, edges + 1);
    // 5) Add the start and end positions of the row (for convenience to the
    //    next algorithms to apply).
    edges[0] = 0;
    edges[nedges + 1] = len;
    nedges += 2;
    return nedges;
}

/* Find a section of the road.
   This function is implemented as a state machine using switch statements.
   Parameters:
     * color_segments: the colors of the section of the image.
     * nsegments: number of color segments.
     * index: index of the current scanline.
     * prev_section: the previous section.
     * section: the output road section found.
   Returns true if a road section has been found, false otherwise.
*/
bool
DifferentialRoadFinder::find_road_section(color_segment_t color_segments[],
    size_t nsegments, size_t index, road_section_t &prev_section,
    road_section_t& section)
{
    state_t state = ST_START;
    float start = 0.0, end = 0.0, line_start = 0.0, line_end = 0.0;
    float line;

    for (size_t i = 0; i < nsegments; i++) {
        switch (state) {
            case ST_START:
                line = FLT_MAX;
                switch (color_segments[i].color) {
                    case COLOR_BLACK:
                        // Enter to the first lane
                        start = get_world_x(color_segments[i].start, index);
                        state = ST_LANE1;
                        break;
                    case COLOR_WHITE:
                    case COLOR_OTHER:
                        break;
                }
                break;
            case ST_LANE1:
                end = get_world_x(color_segments[i].start, index);
                switch (color_segments[i].color) {
                    case COLOR_BLACK: break;
                    case COLOR_WHITE:
                        // White line after the first lane
                        line_start = end;
                        state = ST_LINE;
                        break;
                    case COLOR_OTHER:
                        if (generate_new_section(
                            start, end, line, index, prev_section, section))
                        {
                            // Only one lane visible
                            return true;
                        } else {
                            state = ST_START;
                        }
                        break;
                }
                break;
            case ST_LINE:
                switch (color_segments[i].color) {
                    case COLOR_BLACK:
                        // Enter to the second lane
                        line_end = get_world_x(color_segments[i].start, index);
                        state = ST_LANE2;
                        break;
                    case COLOR_WHITE: break;
                    case COLOR_OTHER:
                        if (generate_new_section(
                            start, end, line, index, prev_section, section))
                        {
                            // Only one lane visible
                            return true;
                        } else {
                            state = ST_START;
                        }
                        break;
                }
                break;
            case ST_LANE2:
                switch (color_segments[i].color) {
                    case COLOR_BLACK: break;
                    case COLOR_WHITE:
                    case COLOR_OTHER:
                        // End of second lane
                        end = get_world_x(color_segments[i].start, index);
                        line = (line_start + line_end) / 2.0;
                        if (generate_new_section(
                            start, end, line, index, prev_section, section))
                        {
                            return true;
                        } else {
                            if (color_segments[i].color == COLOR_OTHER) {
                                state = ST_START;
                            } else {
                                // If color is white, go to ST_LINE
                                start = line_end;
                                line_start = end;
                                state = ST_LINE;
                            }
                        }
                        break;
                }
                break;
            case ST_END:
                break;
        }
    }
    return false;
}

/* If start and end corresponds to a road section, generate it.
   Parameters:
     * start: start position of the new road section.
     * end: end position of the new road section.
     * line: position of the central line.
     * index: index of the scanline.
     * prev_section: previous section of the road.
     * section: output new section.
*/
bool
DifferentialRoadFinder::generate_new_section(float start, float end,
    float line, int index, road_section_t& prev_section,
    road_section_t& section)
{
    bool is_first_section = (prev_section.left[1] < 0.0);
    bool has_line, prev_has_line, left_aligned, right_aligned;
    bool add_section = false;

    if (is_first_section) {
        // Add the section if it's the first one
        add_section = true;
    } else {
        has_line = (line != FLT_MAX);
        prev_has_line = (prev_section.line[0] != FLT_MAX);
        if (has_line && prev_has_line) {
            // Add the section if the lines are aligned
            add_section = (line > prev_section.line[0] - ROAD_DELTA
                && line < prev_section.line[0] + ROAD_DELTA);
        } else {
            // Add the section if the left or right edges are aligned
            left_aligned = (start > prev_section.left[0] - ROAD_DELTA
                && start < prev_section.left[0] + ROAD_DELTA);
            right_aligned = (end > prev_section.right[0] - ROAD_DELTA
                && end < prev_section.right[0] + ROAD_DELTA);
            add_section = left_aligned || right_aligned;
        }
    }
    if (add_section) {
        section.left[0] = start;
        section.right[0] = end;
        section.line[0] = line;
        section.left[1] = section.right[1] = section.line[1]
            = scanline_params[index].y;
    }
    return add_section;
}

/* Initialize the parameters for each scanline
   Parameters:
     * scanline_frame: SL_SCREEN, to use scanlines equidistant in screen
           coordinates or SL_WORLD, to use scanlines equidistant in world
           coordinates.
     * scanline_distance: if using the SL_WORLD frame, distance between
           the scanlines.
*/
void
DifferentialRoadFinder::init_scanline_params(scanline_frame_t scanline_frame,
    float scanline_distance)
{
    int row_offset = cam_params.height - 1;
    float tan_cam_angle = tan(-cam_params.cam_angle);
    float kv = tan(DEG_TO_RAD(cam_params.fovv/2.0)) / (cam_params.height/2.0);
    float ymin = cam_params.cam_z
        / tan(-cam_params.cam_angle + DEG_TO_RAD(cam_params.fovv/2.0));
    float tan_phi, dc;

    // Allocate the scanline params array
    scanline_params = new scanline_params_t[scanlines];

    for (size_t i = 0; i < scanlines; i++) {
        // Mode equidistant scanlines in screen coordinates
        if (scanline_frame == SL_SCREEN) {
            // Compute the byte offset of the scanline
            scanline_params[i].offset = row_offset * cam_params.width;

            // Compute the y world coordinate corresponding to each scanline
            tan_phi = kv * ((int)(cam_params.height/2) - row_offset);
            scanline_params[i].y = cam_params.cam_z
                * (1 + tan_cam_angle*tan_phi) / (tan_cam_angle - tan_phi);
            row_offset -= cam_params.height/scanlines;
        // Mode equidistant scanlines in world coordinates
        } else {
            // Compute the y world coordinate corresponding to each scanline
            scanline_params[i].y = ymin + scanline_distance * i;

            // Compute the byte offset of the scanline
            tan_phi = (tan_cam_angle*scanline_params[i].y - cam_params.cam_z)
                / (cam_params.cam_z*tan_cam_angle + scanline_params[i].y);
            row_offset = (int)(cam_params.height/2) - tan_phi / kv - 1;
            scanline_params[i].offset = row_offset * cam_params.width;

            // Stop if two scanlies are superposed
            if (i > 0
                && scanline_params[i].offset == scanline_params[i - 1].offset)
            {
                scanlines = i;
                break;
            }
        }

        // Compute the constant to obtain the x world coordinates. The
        // expression to compute the distance camera-projection plane (dc) is
        // derived from the formula to compute the distance between a plane and
        // a point.
        // https://mathinsight.org/distance_point_plane
        dc = fabsf(sin(-cam_params.cam_angle)*cam_params.cam_z
            + cos(-cam_params.cam_angle)*scanline_params[i].y);
        scanline_params[i].kx = tan(DEG_TO_RAD(cam_params.fovh/2.0)) * dc
            / (cam_params.width/2.0);
    }
}

