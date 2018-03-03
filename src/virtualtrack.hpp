
#ifndef VIRTUALTRACK_HPP
#define VIRTUALTRACK_HPP

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "gl.hpp"
#include "options.hpp"
#include "tracksegment.hpp"

using namespace std;

// Enumeration of the segments type, used to build the track
typedef enum {
    SEGMENT_NULL,
    SEGMENT_STRAIGHT,
    SEGMENT_TURNLEFT,
    SEGMENT_TURNRIGHT,
    SEGMENT_DASHED1,
    SEGMENT_DASHED2,
    SEGMENT_ZIGZAG,
    SEGMENT_WIDENARROW,
    SEGMENT_NARROW,
    SEGMENT_NARROWWIDE,
    SEGMENT_VCROSSROAD,
    SEGMENT_ACROSSROAD,
    SEGMENT_DOUBLETURNLEFT,
    SEGMENT_DOUBLETURNRIGHT
} segment_type_t;

// Type that relates the string identifier of a virtual track segment type with
// its enumeration
typedef struct {
    segment_type_t type;
    const char *str_id;
} segment_id_t;

// Information about the segments
typedef struct {
    // Type of segment
    segment_type_t type;
    // Input of the segment that connects with the previous segment
    int input;
    // Index of the previous segment
    int prev;
    // Output of the previous segment that connects with this segment
    int output;
} segment_t;

// Struct to keep texture info
typedef struct {
    const char* filename;
    size_t w;
    size_t h;
    GLint index;
} texture_info_t;

// Represents a virtual track where a virtual robot car runs
class VirtualTrack {

    public:

        /* Constructor.
           Parameters:
             * segments: sequence of segments to build the track. The last one
                         must be SEGMENT_NULL.
        */
        VirtualTrack(const Options& options);

        // Destructor
        ~VirtualTrack();

        /* Given a position and an orientation and normal vectors, correct
           them to make sure that they are over the current segment.
           Parameters:
             * position: position to correct.
             * orientation: orientation to correct.
             * normal: normal to correct.
        */
        void correct_position(glm::vec3& position, glm::vec3& orientation,
            glm::vec3& normal) const;

        // Return the scene's bounding sphere
        void get_bounding_sphere(glm::vec3& center, float& radius) const;

        /* Get the starting position for the mobile.
           Parameters:
             * position: returned starting position.
             * orientation: returned starting orientation.
             * normal: returned starting normal.
        */
        void get_start_position(
            glm::vec3& position, glm::vec3& orientation, glm::vec3& normal)
                const;

        // Render the scene
        void render();

        /* Set the projection transformation.
           Parameters:
             * fovh: horizontal Field Of View.
             * fovv: vertical Field Of View.
             * znear: near clip distance.
             * zfar: far clip distance.
        */
        void set_projection(float fovh, float fovv, float znear, float zfar);

        /* Set the point of view.
           Parameters:
             * eye: the position of the eye.
             * center: the position where the eye looks at.
             * up: vector that points to the up direction.
             
        */
        void set_view(const glm::vec3& eye, const glm::vec3& center,
            const glm::vec3& up);

    private:

        // Compile a vertex/fragment shader
        GLuint compile_shader(GLenum type, const char *shader_src);

        // Compute the bounding box and sphere of the scene
        void compute_bounding_box();

        // Creates a texture from an array
        void create_texture_from_array(GLint texid, GLsizei width,
            GLsizei height, const GLvoid *data);

        // Creates a texture from a file
        void create_texture_from_file(GLint texid, GLsizei width,
            GLsizei height, const char *file);

        // Destroy the opengl resources
        void destroy_gl();

        // Destroy the track segments
        void destroy_segments();

        // Initialize the geometry.
        void init_geometry();

        // Initialize OpenGL Vertex and Index buffers.
        void init_gl_buffers();

        // Initialize OpenGL lights.
        void init_gl_lights();

        // Initialize OpenGL shader program.
        void init_gl_program(const Options& options);

        // Initialize textures
        void init_gl_textures(const Options& options);

        // Build the track segments.
        void init_segments(const Options& options);

        // Define the segments geometry.
        void init_segments_geometry();

        // Load a track file (for the virtual camera)
        void load_track_file(
            const string& track_file, vector<segment_t>& segments);

        // The track segments
        vector<TrackSegment *> segments;

        // GL render context
        gl_context_t context;

        // The projection matrix
        glm::mat4 projection_matrix;

        // The bounding box
        glm::vec3 bb_min;
        glm::vec3 bb_max;

        // The bounding sphere
        glm::vec3 bs_center;
        float bs_radius;

        // Relates segment strings IDs with their enumerations
        static const segment_id_t segments_ids[];

        // Texture info
        static const texture_info_t texture_info[];
};

#endif

