
#include <GLES2/gl2.h>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>

#include "followexception.hpp"
#include "virtualcamera.hpp"

#include "opencv2/opencv.hpp"

// Near value for the perspective cone (this NEVER must be <= 0!!)
#define Z_NEAR  0.1

#define X11_PIXMAP_DEPTH            24
#define X11_IMAGE_ALIGN             8
#define X11_IMAGE_BYTES_PER_PIXEL   4

/* Constructor.
   Parameters:
     * segments: description of the virtual track's segments.
     * cam_params: camera's parameters.
*/
VirtualCamera::VirtualCamera(const vector<segment_t>& segments,
        const cam_params_t& cam_params):
    Camera(), cam_params(cam_params),
    front_buffer(cam_params.height, cam_params.width, CV_8UC4),
    back_buffer(cam_params.height, cam_params.width, CV_8UC4)
{
    glm::vec3 bs_center;
    float bs_radius;

    // Set the buffers to use
    set_buffers(front_buffer, back_buffer);

    // Initialize the EGL/OpenGL machinery
    init_x11();
    init_egl();
    init_gl();

    // Initialize the track
    track = new VirtualTrack(segments);

    // Initialize camera position
    track->get_start_position(position, orientation, normal);

    // Set the camera projection. The Z-Far is set to the scene's bounding
    // sphere diameter plus the Z-Near. That way we assure that no matter where
    // the camera is, the whole scene is shown.
    track->get_bounding_sphere(bs_center, bs_radius);
    track->set_projection(
        cam_params.fovh, cam_params.fovv, Z_NEAR, bs_radius*2);
}

VirtualCamera::~VirtualCamera()
{
    delete track;

    // Finalize x11
    XFreePixmap(x11_display, x11_pixmap);
    XCloseDisplay(x11_display);
}

// Fetch the next frame
void
VirtualCamera::fetch()
{
    XImage *aux;

    // Set the camera position and orientation
    // The camera position (eye) is the current position in the track + the
    // camera Z position (added in the direction of the normal!)
    glm::vec3 eye = position + normal * cam_params.cam_z;

    // The camera orientation is obtained rotating the orientation vector the
    // camera angle through the orientation x normal vector
    // Beware, rotate needs the angle in radians
    glm::vec3 vx = glm::cross(normal, orientation);
    glm::vec3 o = glm::rotate(orientation, (float)(cam_params.cam_angle), vx);
    track->set_view(eye, eye + o, cross(o, vx));

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the scene
    track->render();
    glFinish();

    // Read back the pixels from the pixmap where the scene is rendered
    XGetSubImage(x11_display, x11_pixmap, 0, 0, cam_params.width,
        cam_params.height, AllPlanes, ZPixmap, x11_back_img, 0, 0);

    // Swap the back and front images
    swap_buffers();
    aux = x11_front_img;
    x11_front_img = x11_back_img;
    x11_back_img = aux;
}

// Return frame's height
size_t
VirtualCamera::get_height()
{
    return cam_params.height;
}

// Return frame's width
size_t
VirtualCamera::get_width()
{
    return cam_params.width;
}

// Return the position, orientation and normal of the camera
void
VirtualCamera::get_position(
    glm::vec3& position, glm::vec3& orientation, glm::vec3& normal) const
{
    position = this->position;
    orientation = this->orientation;
    normal = this->normal;
}

// Set the position, orientation and normal of the camera
void
VirtualCamera::set_position(
    glm::vec3& position, glm::vec3& orientation, glm::vec3& normal)
{
    this->position = position;
    this->orientation = orientation;
    this->normal = normal;
    track->correct_position(this->position, this->orientation, this->normal);
}

// PRIVATE FUNCTIONS

// Initialize EGL library
void
VirtualCamera::init_egl()
{
    EGLint majorVersion, minorVersion;
    EGLint config_attrs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 24,
        EGL_SURFACE_TYPE, EGL_PIXMAP_BIT,
        EGL_MATCH_NATIVE_PIXMAP, (EGLint)x11_pixmap,
        EGL_NONE
    };
    EGLConfig egl_config;
    EGLint nconfs;
    EGLContext context;
    const EGLint context_attrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    // Get the display and initialize EGL
    egl_display = eglGetDisplay(x11_display);
    if (egl_display == EGL_NO_DISPLAY) {
        throw FollowException("error in eglGetDisplay");
    }
    if (!eglInitialize(egl_display, &majorVersion, &minorVersion)) {
        throw FollowException("error in eglInitialize");
    }

    // Obtain the configuration
    if (!eglChooseConfig(egl_display, config_attrs, &egl_config, 1, &nconfs)) {
        throw FollowException("error in eglChooseConfig");
    }

    // Specify the native surface to render to
    if ((egl_surface = eglCreatePixmapSurface(
        egl_display, egl_config, x11_pixmap, 0)) == EGL_NO_SURFACE)
    {
        throw FollowException("error in eglCreatePixmapSurface");
    }

    // Create the rendering context
    if ((context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT,
        context_attrs)) == EGL_NO_CONTEXT)
    {
        throw FollowException("error in eglCreateContext");
    }
    if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, context)) {
        throw FollowException("error in eglMakeCurrent");
    }
}

// Initialize OpenGL stuff
void
VirtualCamera::init_gl()
{
    glViewport(0, 0, cam_params.width, cam_params.height);
}

// Initialize X11 stuff
void
VirtualCamera::init_x11()
{
    x11_display = XOpenDisplay(NULL);
    if (!x11_display)
        throw FollowException("error in XOpenDisplay");
    x11_pixmap = XCreatePixmap(x11_display, DefaultRootWindow(x11_display),
        cam_params.width, cam_params.height, X11_PIXMAP_DEPTH);
    // Create the images
    x11_front_img = XCreateImage(x11_display, DefaultVisual(x11_display, 0),
        X11_PIXMAP_DEPTH, ZPixmap, 0, (char *)front_buffer.data,
        cam_params.width, cam_params.height, X11_IMAGE_ALIGN,
        cam_params.width * X11_IMAGE_BYTES_PER_PIXEL);
    x11_back_img = XCreateImage(x11_display, DefaultVisual(x11_display, 0),
        X11_PIXMAP_DEPTH, ZPixmap, 0, (char *)back_buffer.data,
        cam_params.width, cam_params.height, X11_IMAGE_ALIGN,
        cam_params.width * X11_IMAGE_BYTES_PER_PIXEL);
}

