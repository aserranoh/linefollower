
#ifndef DIFFERENTIALROADFINDER_HPP
#define DIFFERENTIALROADFINDER_HPP

#include "camparams.hpp"
#include "options.hpp"
#include "roadfinder.hpp"

// Enumeration for the segment's colors
typedef enum {COLOR_WHITE, COLOR_BLACK, COLOR_OTHER} color_t;

// Parameters for each scanline
typedef struct scanline_params {
    // Number of rows offset of the scanline in the image
    size_t offset;

    // Y component in world coordinates for a given scanline
    float y;

    // Constant to multiply the X component in camera coordinates to obtain
    // the X in world coordinates
    float kx;
} scanline_params_t;

// Data type that represents a color segment
typedef struct {
    // Start offset of the segment, in pixels
    size_t start;

    // Length of the segment, in pixels
    size_t length;

    // Color of the segment (white, black or other)
    color_t color;
} color_segment_t;

// Frame of reference for the scanlines mutual distance
typedef enum {SL_SCREEN, SL_WORLD} scanline_frame_t;

class DifferentialRoadFinder: public RoadFinder {

    public:

        /* Constructor.
           Parameters:
             * options: application's options.
        */
        DifferentialRoadFinder(const Options& options);

        ~DifferentialRoadFinder();

        /* Find the road in the image.
           Parameters:
             * frame: image of the road.
             * road: output parameter that contains the road found.
        */
        virtual void find(Mat& frame, Road& road);

    private:

        // The camera parameters
        cam_params_t cam_params;

        // Number of lines to explore in the input image
        size_t scanlines;

        // Value of the minimum derivative accepted
        int min_derivative;

        // Threshold to identify white and black colors
        unsigned int color_distance_threshold;

        // Destination images form some transformations
        Mat gray_frame;

        // Parameters related with each scanline
        scanline_params_t *scanline_params;

        // Temporary list to keep the found road sections
        vector<road_section_t> sections;

        // Distance between wheels
        float wheel_distance;

        /* Add a security margin to both sides of the road to assure that the
           robot won't fall off the limits.
           Parameters:
             * road: the output road with the margins.
        */
        void add_secure_dist(Road& road);

        /* Convert the line of pixels in a sequence of color segments.
           Parameters:
             * row: pointer to the row in the image that contains the colored
                    pixels.
             * channels: number of color channels in the source image.
             * edges: the positions of the segments edges.
             * nedges: the number of edges in the edges array.
             * color_segments: the colors of each segment.
        */
        void find_color_segments(uchar *row, size_t channels, int edges[],
            size_t nedges, color_segment_t color_segments[]);

        /* Find the road limits.
           Parameters:
             * row: pointer to the row in the image to analize.
             * len: number of pixels in row.
             * edges: contains the found edges positions.
           Return the number of edges found.
        */
        size_t find_edges(uchar *row, size_t len, int edges[]);

        /* Find a section of the road.
           Parameters:
             * color_segments: the colors of the section of the image.
             * nsegments: number of color segments.
             * index: index of the current scanline.
             * prev_section: the previous section.
             * section: the output road section found.
           Returns true if a road section has been found, false otherwise.
        */
        bool find_road_section(color_segment_t color_segments[],
            size_t nsegments, size_t index, road_section_t &prev_section,
            road_section_t& section);

        /* If start and end corresponds to a road section, generate it.
           Parameters:
             * start: start position of the new road section.
             * end: end position of the new road section.
             * line: position of the central line.
             * index: index of the scanline.
             * prev_section: previous section of the road.
             * section: output new section.
        */
        bool generate_new_section(float start, float end, float line,
            int index, road_section_t& prev_section, road_section_t& section);

        /* Convert a screen x coordinate to world.
           Parameters:
             * x: screen x coordinate.
             * index: index of the current scanline.
        */
        inline float get_world_x(float x, size_t index) const {
            return (x - (int)cam_params.width/2) * scanline_params[index].kx;
        }

        /* Initialize the parameters for each scanline
           Parameters:
             * scanline_frame: SL_SCREEN, to use scanlines equidistant in
                   screen coordinates or SL_WORLD, to use scanlines
                   equidistant in world coordinates.
             * scanline_distance: if using the SL_WORLD frame, distance between
                   the scanlines.
        */
        void init_scanline_params(
            scanline_frame_t scanline_frame, float scanline_distance);

};

#endif

