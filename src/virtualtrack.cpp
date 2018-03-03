
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <limits.h>
#include <math.h>

#include "acrossroadsegment.hpp"
#include "dashedline1segment.hpp"
#include "dashedline2segment.hpp"
#include "doubleturnleftsegment.hpp"
#include "doubleturnrightsegment.hpp"
#include "followexception.hpp"
#include "narrowsegment.hpp"
#include "narrowwidesegment.hpp"
#include "straightsegment.hpp"
#include "turnleftsegment.hpp"
#include "turnrightsegment.hpp"
#include "utilities.hpp"
#include "vcrossroadsegment.hpp"
#include "virtualtrack.hpp"
#include "widenarrowsegment.hpp"
#include "zigzaglinesegment.hpp"

using namespace utilities;

// Shaders files
#define VERTEX_SHADER_FILE      "vertex.sl"
#define FRAGMENT_SHADER_FILE    "fragment.sl"

// Vertex shader attributes
#define ATTR_POSITION   0
#define ATTR_NORMAL     1
#define ATTR_TEXCOORD   2

// The walls height
#define WALLS_H 250

// Margin to the bounding box, for the size of the virtual room
#define MARGIN  200

// Size of the carpet texture against the size of the floor
#define TEXTURE_COORDS_MULT 20

// Ilumination constants
#define ATTENUATION 0.00002

// Geometry constants
#define ROOM_NUM_VERTICES   24
#define ROOM_NUM_INDICES    34

const segment_id_t VirtualTrack::segments_ids[] = {
    {SEGMENT_STRAIGHT, "Straight"},
    {SEGMENT_TURNLEFT, "TurnLeft"},
    {SEGMENT_TURNRIGHT, "TurnRight"},
    {SEGMENT_DASHED1, "Dashed1"},
    {SEGMENT_DASHED2, "Dashed2"},
    {SEGMENT_ZIGZAG, "ZigZag"},
    {SEGMENT_WIDENARROW, "WideNarrow"},
    {SEGMENT_NARROW, "Narrow"},
    {SEGMENT_NARROWWIDE, "NarrowWide"},
    {SEGMENT_VCROSSROAD, "VCrossroad"},
    {SEGMENT_ACROSSROAD, "ACrossroad"},
    {SEGMENT_DOUBLETURNLEFT, "DoubleTurnLeft"},
    {SEGMENT_DOUBLETURNRIGHT, "DoubleTurnRight"},
    {SEGMENT_NULL, ""}
};

const texture_info_t VirtualTrack::texture_info[NUM_TEXTURES] = {
    {"carpet.data", 256, 256, CARPET_TEXTURE},
    {"wall.data", 1, 1, WALL_TEXTURE},
    {"road.data", 30, 1, ROAD_TEXTURE},
    {"wood.data", 256, 64, WOOD_TEXTURE},
    {"road_dashed1.data", 30, 6, ROAD_DASHED1_TEXTURE},
    {"road_dashed2.data", 30, 3, ROAD_DASHED2_TEXTURE},
    {"road_zigzag.data", 256, 256, ROAD_ZIGZAG_TEXTURE},
    {"road_widenarrow.data", 256, 256, ROAD_WIDENARROW_TEXTURE},
    {"road_narrow.data", 30, 1, ROAD_NARROW_TEXTURE},
    {"road_narrowwide.data", 256, 256, ROAD_NARROWWIDE_TEXTURE},
    {"road_vcross.data", 256, 256, ROAD_VCROSSROAD_TEXTURE},
    {"road_across.data", 256, 256, ROAD_ACROSSROAD_TEXTURE}
};

/* Constructor.
   Parameters:
     * segments: sequence of segments to build the track. The last one must be
                 SEGMENT_NULL.
*/
VirtualTrack::VirtualTrack(const Options& options):
    bb_min(INT_MAX, INT_MAX, 0), bb_max(INT_MIN, INT_MIN, WALLS_H)
{
    init_segments(options);
    init_gl_program(options);
    init_gl_buffers();
    init_gl_textures(options);
    init_segments_geometry();
    compute_bounding_box();
    init_geometry();
    init_gl_lights();
}

VirtualTrack::~VirtualTrack()
{
    destroy_segments();
    destroy_gl();
}

/* Given a position and an orientation and normal vectors, correct
   them to make sure that they are over the current segment.
   Parameters:
     * position: position to correct.
     * orientation: orientation to correct.
     * normal: normal to correct.
*/
void
VirtualTrack::correct_position(glm::vec3& position, glm::vec3& orientation,
    glm::vec3& normal) const
{
    vector<TrackSegment *>::const_iterator it;
    bool found = false;

    for (it = segments.begin(); it != segments.end() && !found; it++) {
        if ((*it)->contains(position)) {
            (*it)->correct_position(position, orientation, normal);
            found = true;
        }
    }

    // Case where the point is outside the track
    if (!found) {
        // The position is the same as at the input but with Z = 0
        position[2] = 0;
        // Make sure the orientation vector is in the XY plane (and its normal)
        orientation[2] = 0;
        orientation = normalize(orientation);
        // The normal vector is in the Z axis
        normal = glm::vec3(0, 0, 1);
    }
}

// Return the scene's bounding sphere
void
VirtualTrack::get_bounding_sphere(glm::vec3& center, float& radius) const
{
    center = bs_center;
    radius = bs_radius;
}

/* Get the starting position for the mobile.
   Parameters:
     * position: returned starting position.
     * orientation: returned starting orientation.
     * normal: returned starting normal.
*/
void
VirtualTrack::get_start_position(
    glm::vec3& position, glm::vec3& orientation, glm::vec3& normal) const
{
    // Set the start position to (0, 1, 0) instead of (0, 0, 0) to avoid that
    // the car is outside the track if the track is open.
    position = glm::vec3(0, 1, 0);
    orientation = glm::vec3(0, 1, 0);
    normal = glm::vec3(0, 0, 1);
    correct_position(position, orientation, normal);
}

// Render the scene
void
VirtualTrack::render()
{
    vector<TrackSegment *>::const_iterator it;

    glBindBuffer(GL_ARRAY_BUFFER, context.vertex_buffer);

    // Position
    glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
        sizeof(gl_vertex_t), (void *)offsetof(gl_vertex_t, position));
    glEnableVertexAttribArray(ATTR_POSITION);

    // Normal
    glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,
        sizeof(gl_vertex_t), (void *)offsetof(gl_vertex_t, normal));
    glEnableVertexAttribArray(ATTR_NORMAL);

    // Texture coordinates
    glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
        sizeof(gl_vertex_t), (void *)offsetof(gl_vertex_t, texcoord));
    glEnableVertexAttribArray(ATTR_TEXCOORD);

    // Draw the primitives
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    // Draw the floor
    glUniform1i(context.u_texture, CARPET_TEXTURE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void *)0);
    // Draw the walls
    glUniform1i(context.u_texture, WALL_TEXTURE);
    glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_SHORT,
        (void *)(4 * sizeof(GLushort)));

    // Render the segments
    for (it = segments.begin(); it != segments.end(); it++) {
        (*it)->render();
    }
}

/* Set the projection transformation.
   Parameters:
     * fovh: horizontal Field Of View.
     * fovv: vertical Field Of View.
     * znear: near clip distance.
     * zfar: far clip distance.
*/
void
VirtualTrack::set_projection(float fovh, float fovv, float znear, float zfar)
{
    // Compute the projection matrix
    projection_matrix = glm::perspective(
        glm::radians(fovv), fovh/fovv, znear, zfar);
}

/* Set the point of view.
   Parameters:
     * eye: the position of the eye.
     * center: the position where the eye looks at.
     * up: vector that points to the up direction.
*/
void
VirtualTrack::set_view(
    const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
    // Compute the modelview-projection matrix
    glm::mat4 mvp = projection_matrix * glm::lookAt(eye, center, up);

    // Update the uniforms
    glUniformMatrix4fv(context.u_mvprojection, 1, GL_FALSE, &mvp[0][0]);
}

//// PRIVATE FUNCTIONS

// Compile a vertex/fragment shader
GLuint
VirtualTrack::compile_shader(GLenum type, const char *shader_src)
{
    GLuint shader;
    GLint compiled, infolen = 0;
    char *infolog;
    string errmsg;

    // Create the shader object
    shader = glCreateShader(type);

    // Load the shader source
    glShaderSource(shader, 1, &shader_src, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
        if(infolen > 1) {
            infolog = new char[infolen];
            glGetShaderInfoLog(shader, infolen, NULL, infolog);
            errmsg = string("error compiling shader:\n") + infolog;
            delete[] infolog;
        }
        glDeleteShader(shader);
        throw FollowException(errmsg);
    }
    return shader;
}

// Compute the bounding box and sphere of the scene
void
VirtualTrack::compute_bounding_box()
{
    vector<TrackSegment *>::iterator it;
    glm::vec2 bb_segment_min, bb_segment_max;

    int i = 0;
    for (it = segments.begin(); it != segments.end(); it++) {
        (*it)->get_bounding_box(bb_segment_min, bb_segment_max);
        i++;
        if (bb_segment_min[0] < bb_min[0]) bb_min[0] = bb_segment_min[0];
        if (bb_segment_min[1] < bb_min[1]) bb_min[1] = bb_segment_min[1];
        if (bb_segment_max[0] > bb_max[0]) bb_max[0] = bb_segment_max[0];
        if (bb_segment_max[1] > bb_max[1]) bb_max[1] = bb_segment_max[1];
    }

    // Add a margin to the bounding box
    bb_min -= glm::vec3(MARGIN, MARGIN, 0);
    bb_max += glm::vec3(MARGIN, MARGIN, 0);

    // Compute the bounding sphere
    bs_center = (bb_min + bb_max) * 0.5f;
    bs_radius = length(bb_max - bb_min) * 0.5;
}

// Creates a texture from an array
void
VirtualTrack::create_texture_from_array(GLint texid, GLsizei width,
    GLsizei height, const GLvoid *data)
{
    glActiveTexture(texid + GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context.tex_index[texid]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

// Creates a texture from a file
void
VirtualTrack::create_texture_from_file(GLint texid, GLsizei width,
    GLsizei height, const char *file)
{
    int e;

    const char *data = loadfile(file);
    if (!data) {
        e = errno;
        throw FollowException(
            string("error loading file '") + file + "': " + strerror(e));
    }
    create_texture_from_array(texid, width, height, data);
    delete[] data;
}

// Destroy the track segments.
void
VirtualTrack::destroy_segments()
{
    vector<TrackSegment *>::iterator it;

    for (it = segments.begin(); it != segments.end(); it++) {
        delete *it;
    }
}

// Initialize the geometry.
void
VirtualTrack::init_geometry()
{
    float maxu = (bb_max.x - bb_min.x)/TEXTURE_COORDS_MULT;
    float maxv = (bb_max.y - bb_min.y)/TEXTURE_COORDS_MULT;

    // Some vertex are repeated to use different normals
    const gl_vertex_t vertices[] = {
        // The floor
        {{bb_min.x, bb_min.y, 0}, {0, 0, 1}, {0, 0}},
        {{bb_max.x, bb_min.y, 0}, {0, 0, 1}, {maxu, 0}},
        {{bb_min.x, bb_max.y, 0}, {0, 0, 1}, {0, maxv}},
        {{bb_max.x, bb_max.y, 0}, {0, 0, 1}, {maxu, maxv}},
        // The front wall
        {{bb_max.x, bb_min.y, 0}, {0, 1, 0}, {0, 0}},
        {{bb_min.x, bb_min.y, 0}, {0, 1, 0}, {0, 0}},
        {{bb_max.x, bb_min.y, WALLS_H}, {0, 1, 0}, {0, 0}},
        {{bb_min.x, bb_min.y, WALLS_H}, {0, 1, 0}, {0, 0}},
        // The right wall
        {{bb_max.x, bb_max.y, 0}, {-1, 0, 0}, {0, 0}},
        {{bb_max.x, bb_min.y, 0}, {-1, 0, 0}, {0, 0}},
        {{bb_max.x, bb_max.y, WALLS_H}, {-1, 0, 0}, {0, 0}},
        {{bb_max.x, bb_min.y, WALLS_H}, {-1, 0, 0}, {0, 0}},
        // The back wall
        {{bb_min.x, bb_max.y, 0}, {0, -1, 0}, {0, 0}},
        {{bb_max.x, bb_max.y, 0}, {0, -1, 0}, {0, 0}},
        {{bb_min.x, bb_max.y, WALLS_H}, {0, -1, 0}, {0, 0}},
        {{bb_max.x, bb_max.y, WALLS_H}, {0, -1, 0}, {0, 0}},
        // The left wall
        {{bb_min.x, bb_min.y, 0}, {1, 0, 0}, {0, 0}},
        {{bb_min.x, bb_max.y, 0}, {1, 0, 0}, {0, 0}},
        {{bb_min.x, bb_min.y, WALLS_H}, {1, 0, 0}, {0, 0}},
        {{bb_min.x, bb_max.y, WALLS_H}, {1, 0, 0}, {0, 0}},
        // The ceiling
        {{bb_min.x, bb_max.y, WALLS_H}, {0, 0, -1}, {0, 0}},
        {{bb_max.x, bb_max.y, WALLS_H}, {0, 0, -1}, {0, 0}},
        {{bb_min.x, bb_min.y, WALLS_H}, {0, 0, -1}, {0, 0}},
        {{bb_max.x, bb_min.y, WALLS_H}, {0, 0, -1}, {0, 0}}
    };
    // DT = Degenerate Triangles
    const GLushort indices[] = {
        0, 1, 2, 3,
        4, 5, 6, 6, 5, 7,
        8, 9, 10, 10, 9, 11,
        12, 13, 14, 14, 13, 15,
        16, 17, 18, 18, 17, 19,
        20, 21, 22, 22, 21, 23
    };

    glBindBuffer(GL_ARRAY_BUFFER, context.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
}

// Initialize OpenGL Vertex and Index buffers.
void
VirtualTrack::init_gl_buffers()
{
    vector<TrackSegment *>::const_iterator it;
    size_t total_vertices = ROOM_NUM_VERTICES;
    size_t total_indices = ROOM_NUM_INDICES;

    // Get the total number of vertices and indexes to store in the buffers
    for (it = segments.begin(); it != segments.end(); it++) {
        total_vertices += (*it)->get_num_vertices();
        total_indices += (*it)->get_num_indices();
    }

    // Generate the buffers
    glGenBuffers(1, &context.vertex_buffer);
    glGenBuffers(1, &context.index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, context.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, total_vertices * sizeof(gl_vertex_t), 0,
        GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_indices * sizeof(GLushort), 0,
        GL_STATIC_DRAW);
}

// Initialize OpenGL lights.
void
VirtualTrack::init_gl_lights()
{
    GLfloat lights_sep_x = (bb_max.x - bb_min.x)/3;
    GLfloat lights_sep_y = (bb_max.y - bb_min.y)/3;
    GLint u_light, u_attenuation;

    // Lighting setup
    u_light = glGetUniformLocation(context.program, "u_light0");
    glUniform3f(u_light, bb_min.x + lights_sep_x, bb_min.y + lights_sep_y,
        WALLS_H);
    u_light = glGetUniformLocation(context.program, "u_light1");
    glUniform3f(u_light, bb_min.x + 2 * lights_sep_x, bb_min.y + lights_sep_y,
        WALLS_H);
    u_light = glGetUniformLocation(context.program, "u_light2");
    glUniform3f(u_light, bb_min.x + lights_sep_x, bb_min.y + 2 * lights_sep_y,
        WALLS_H);
    u_light = glGetUniformLocation(context.program, "u_light3");
    glUniform3f(u_light, bb_min.x + 2 * lights_sep_x,
        bb_min.y + 2 * lights_sep_y, WALLS_H);
    u_attenuation = glGetUniformLocation(context.program, "u_attenuation");
    glUniform1f(u_attenuation, ATTENUATION);
}

// Initialize OpenGL shader program.
void
VirtualTrack::init_gl_program(const Options& options)
{
    GLuint vertex_shader, fragment_shader;
    GLint linked, infolen;
    char *infolog;
    int e;
    string errmsg;

    // Load the vertex and fragment shaders source files
    const char *vertex_shader_src = loadfile(
        options.get_string("VertexShader").c_str());
    if (!vertex_shader_src) {
        e = errno;
        throw FollowException(string("error loading file '")
            + options.get_string("VertexShader") + "': " + strerror(e));
    }
    const char *fragment_shader_src = loadfile(
        options.get_string("FragmentShader").c_str());
    if (!fragment_shader_src) {
        e = errno;
        throw FollowException(string("error loading file '")
            + options.get_string("FragmentShader") + "': " + strerror(e));
    }

    // Compile the shaders
    vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

    // Create the program
    context.program = glCreateProgram();
    glAttachShader(context.program, vertex_shader);
    glAttachShader(context.program, fragment_shader);
    glBindAttribLocation(context.program, ATTR_POSITION, "a_position");
    glBindAttribLocation(context.program, ATTR_NORMAL, "a_normal");
    glBindAttribLocation(context.program, ATTR_TEXCOORD, "a_texcoord");
    glLinkProgram(context.program);
    glGetProgramiv(context.program, GL_LINK_STATUS, &linked);
    if (!linked) {
        glGetProgramiv(context.program, GL_INFO_LOG_LENGTH, &infolen);
        if (infolen > 1) {
            infolog = new char[infolen];
            glGetProgramInfoLog(context.program, infolen, NULL, infolog);
            errmsg = string("error linking program:\n") + infolog;
            delete[] infolog;
        }
        glDeleteProgram(context.program);
        throw FollowException(errmsg);
    }
    
    // Use the program object
    glUseProgram(context.program);

    // Enable face culling
    glEnable(GL_CULL_FACE);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    context.u_mvprojection = glGetUniformLocation(
        context.program, "u_mvprojection");
}

// Initialize textures
void
VirtualTrack::init_gl_textures(const Options& options)
{
    // General textures state
    glGenTextures(NUM_TEXTURES, context.tex_index);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    context.u_texture = glGetUniformLocation(context.program, "u_texture");

    // Create the textures
    for (int i = 0; i < NUM_TEXTURES; i++) {
        string texfile = options.get_string("TexturesPath") + "/"
            + texture_info[i].filename;
        create_texture_from_file(texture_info[i].index, texture_info[i].w,
            texture_info[i].h, texfile.c_str());
    }
}

// Build the track segments.
void
VirtualTrack::init_segments(const Options& options)
{
    vector<segment_t> segments;
    vector<segment_t>::const_iterator it;
    TrackSegment *s;
    glm::vec3 pos(0, 0, 0);
    float orient = M_PI/2;
    segment_t seg;

    // Load the track file
    try {
        load_track_file(options.get_string("TrackFile"), segments);
    } catch (out_of_range) {
        throw FollowException("track file not specified");
    }
    // Build the segments
    for (it = segments.begin(); it != segments.end(); it++) {
        seg = *it;
        // Get the position where to put the segment
        if (seg.prev < 0) {
            seg.prev += this->segments.size();
        }
        if (seg.prev >= 0) {
            this->segments[seg.prev]->get_output(seg.output, pos, orient);
        }
        switch ((*it).type) {
            case SEGMENT_STRAIGHT:
                s = new StraightSegment(pos, orient, seg.input);
                break;
            case SEGMENT_TURNLEFT:
                s = new TurnLeftSegment(pos, orient, seg.input);
                break;
            case SEGMENT_TURNRIGHT:
                s = new TurnRightSegment(pos, orient, seg.input);
                break;
            case SEGMENT_DASHED1:
                s = new DashedLine1Segment(pos, orient, seg.input);
                break;
            case SEGMENT_DASHED2:
                s = new DashedLine2Segment(pos, orient, seg.input);
                break;
            case SEGMENT_ZIGZAG:
                s = new ZigZagLineSegment(pos, orient, seg.input);
                break;
            case SEGMENT_WIDENARROW:
                s = new WideNarrowSegment(pos, orient, seg.input);
                break;
            case SEGMENT_NARROW:
                s = new NarrowSegment(pos, orient, seg.input);
                break;
            case SEGMENT_NARROWWIDE:
                s = new NarrowWideSegment(pos, orient, seg.input);
                break;
            case SEGMENT_VCROSSROAD:
                s = new VCrossroadSegment(pos, orient, seg.input);
                break;
            case SEGMENT_ACROSSROAD:
                s = new ACrossroadSegment(pos, orient, seg.input);
                break;
            case SEGMENT_DOUBLETURNLEFT:
                s = new DoubleTurnLeftSegment(pos, orient, seg.input);
                break;
            case SEGMENT_DOUBLETURNRIGHT:
                s = new DoubleTurnRightSegment(pos, orient, seg.input);
                break;
            default: break;
        }
        this->segments.push_back(s);
    }
}

// Define the segments geometry.
void
VirtualTrack::init_segments_geometry()
{
    vector<TrackSegment *>::const_iterator it;
    size_t vertices_i = ROOM_NUM_VERTICES;
    size_t indices_i = ROOM_NUM_INDICES;

    for (it = segments.begin(); it != segments.end(); it++) {
        (*it)->init_geometry(vertices_i, indices_i, context);
        vertices_i += (*it)->get_num_vertices();
        indices_i += (*it)->get_num_indices();
    }
}

// Load a track file (for the virtual camera)
void
VirtualTrack::load_track_file(const string& track_file,
    vector<segment_t>& segments)
{
    string line;
    size_t linenum = 1, sep;
    ifstream f(track_file);
    segment_t s;

    if (f.fail()) {
        throw FollowException("cannot open file " + track_file + ": "
            + strerror(errno));
    } else {
        while (getline(f, line)) {
            // Ignore comments
            if (line[0] == '#') continue;
            // Get the elements of the line
            stringstream ss(line);
            string stype, sinput, soutput;
            ss >> stype >> sinput >> soutput;
            // Get the type of segment
            s.type = SEGMENT_NULL;
            for (size_t i = 0; segments_ids[i].type != SEGMENT_NULL; i++) {
                if (stype == segments_ids[i].str_id) {
                    s.type = segments_ids[i].type;
                }
            }
            if (s.type == SEGMENT_NULL) {
                throw FollowException(track_file + ":"
                    + std::to_string(linenum) + ": wrong track segment '"
                    + line + "'");
            }
            // Get the input
            s.input = (sinput != "") ? atoi(sinput.c_str()) : 0;
            // Get the output
            if (soutput != "") {
                sep = soutput.find(':');
                if (sep != soutput.npos) {
                    s.prev = atoi(soutput.substr(0, sep).c_str());
                    s.output = atoi(soutput.substr(sep + 1).c_str());
                } else {
                    s.prev = atoi(soutput.c_str());
                    s.output = 0;
                }
            } else {
                s.prev = -1;
                s.output = 0;
            }
            segments.push_back(s);
            linenum++;
        }
    }
}

// Destroy the opengl resources
void
VirtualTrack::destroy_gl()
{
    glDeleteTextures(NUM_TEXTURES, context.tex_index);
    glDeleteBuffers(1, &context.vertex_buffer);
    glDeleteBuffers(1, &context.index_buffer);
}

