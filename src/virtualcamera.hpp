
/* virtualcamera.hpp
   Represents a virtual camera, that is actually a way of getting images
   rendered from a virtual camera in an OpenGL scene.
*/

#ifndef VIRTUALCAMERA_HPP
#define VIRTUALCAMERA_HPP

#include <EGL/egl.h>
#include <glm/vec3.hpp>
#include <X11/Xlib.h>

#include "camera.hpp"
#include "camparams.hpp"
#include "virtualtrack.hpp"

class VirtualCamera: public Camera {

    public:

        /* Constructor.
           Parameters:
             * segments: description of the virtual track's segments.
             * cam_params: camera's parameters.
        */
        VirtualCamera(
            const vector<segment_t>& segments, const cam_params_t& cam_params);

        ~VirtualCamera();

        // Fetch the next frame
        virtual void fetch();

        // Return frame's height
        virtual size_t get_height();

        // Return frame's width
        virtual size_t get_width();

        // Return the position, orientation and normal of the camera
        void get_position(glm::vec3& position, glm::vec3& orientation,
            glm::vec3& normal) const;

        // Set the position, orientation and normal of the camera
        void set_position(
            glm::vec3& position, glm::vec3& orientation, glm::vec3& normal);

    private:

        // The scene from where the frames are rendered and retrieved
        VirtualTrack *track;

        // The camera's parameters
        cam_params_t cam_params;

        // The front and back buffers
        Mat front_buffer;
        Mat back_buffer;

        // X11 state and variables
        Display *x11_display;
        Pixmap x11_pixmap;
        XImage *x11_front_img;
        XImage *x11_back_img;

        // EGL state and variables
        EGLDisplay egl_display;
        EGLSurface egl_surface;

        // Current position, orientation and normal in the virtual track
        glm::vec3 position;
        glm::vec3 orientation;
        glm::vec3 normal;

        // Initialize EGL library
        void init_egl();

        // Initialize OpenGL stuff
        void init_gl();

        // Initialize X11 stuff
        void init_x11();

};

#endif

