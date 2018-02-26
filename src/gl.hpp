
#ifndef GLVERTEX_HPP
#define GLVERTEX_HPP

#include <GLES2/gl2.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#define NUM_TEXTURES            12
#define CARPET_TEXTURE          0
#define WALL_TEXTURE            1
#define ROAD_TEXTURE            2
#define WOOD_TEXTURE            3
#define ROAD_DASHED1_TEXTURE    4
#define ROAD_DASHED2_TEXTURE    5
#define ROAD_ZIGZAG_TEXTURE     6
#define ROAD_WIDENARROW_TEXTURE 7
#define ROAD_NARROW_TEXTURE     8
#define ROAD_NARROWWIDE_TEXTURE 9
#define ROAD_VCROSSROAD_TEXTURE 10
#define ROAD_ACROSSROAD_TEXTURE 11

typedef struct {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
} gl_vertex_t;

typedef struct {
    GLuint program;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLint u_mvprojection;
    GLint u_texture;
    GLuint tex_index[NUM_TEXTURES];
} gl_context_t;

#endif

