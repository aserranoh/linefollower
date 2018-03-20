
/*
NOTES ABOUT THE MATHS IN THE CHANGE OF COORDINATES SCREEN-WORLD
===============================================================

The purpose of this class is also change the coordinates of points from screen
reference frame to world reference frame. This note explains the mathematics
involved and justifies the attributes k1, k2, k3, k4, k5 and the functions
get_screen_x, get_screen_y, get_world_point, get_world_x and get_world_y.

Let C be the camera position and Cz be the camera height. The camera is at the
    origin for X and Y, so C = (0, 0, Cz).
Let fovv be the angle vertical Field of View of the camera.
Let fovh be the angle horizontal Field of View of the camera.
Let P be a visible point (for the camera) with Pz = 0. For simplicity, let Px
    also be 0, so P = (0, Py, 0).
Let K be the plane perpendicular to the camera visual and that goes through P.
Let dc the distance K-C.
Let Ck the pojection of the point C in the plane K in the direction of K's
    normal, so the closest point in K to C.
Let dp the distance P-Ck.
Let phi the angle between C-Ck and C-P.

Then:
                  dp                 dp
    tan (phi) = ------  ->  dc = -----------
                  dc              tan (phi)

Let H be a point H = (0, Hy, Hz) in the plane K and dh the distance H-Ck so
    that:

        fovv     dh                   dh
    tan ---- = ------   ->  dc = -------------
         2       dc               tan(fovv/2)

Then:

         dp               dh
    ------------- = ----------------
       tan (phi)      tan (fovv/2)

Let Psx, Psy the coordinates of the point P in the screen, in pixels, where the
    screen origin is in the center of the screen.
Let h the screen height, in pixels. Then:

     Psy     dp      tan (phi)                     2 * Psy * tan(fovv/2)
    ----- = ---- = --------------  -> tan (phi) = -----------------------
     h/2     dh     tan (fovv/2)                            h

Let a the angle between the line C-Ck and the vector (0, 1, 0).
Let b the angle between the line C-P and the vector (0, 1, 0). Then:

    b = a - phi

Beware, because phi is negative if the point P is under the center of the
screen.

Also:
                Cz                          Cz
    tan (b) = ------  ->  tan (a - phi) = ------
                Py                          Py

                              tan x - tan y
Remember that tan(x - y) = -------------------
                            1 + tan x * tan y

Then:

       tan a - tan phi       Cz             Cz (1 + tan a * tan phi)
    --------------------- = ----  ->  Py = --------------------------
     1 + tan a * tan phi     Py                  tan a - tan phi

And if we replace tan phi:

                         2 * Psy * tan(fovv/2)
          Cz (1 + tan a ----------------------)
                                   h
    Py = ------------------------------------
                     2 * Psy * tan(fovv/2)
            tan a - -----------------------
                               h

We can multiply and divide by (h / (2 * tan(fovv/2))), then:

                 h
      Cz (---------------- + tan a * Psy)
           2 * tan(fovv/2)
Py = -------------------------------------
                tan a * h
            ---------------- - Psy
             2 * tan(fovv/2)

For simplicity, let k1 = (Cz * h / (2 * tan(fovv/2))), k2 = Cz * tan a and
k3 = (tan a * h / (2 * tan(fovv/2))). Then:

          k1 + k2 * Psy
    Py = ---------------
            k3 - Psy

Now consider another point Q defined by Ck + (dq, 0, 0), so dq is the distance
Ck-Q. By definition, Ckx = 0, so Qx = dq.
Let W be the a point defined by Ck + (0, dw, 0) so that:

        fovh     dw                        fovh
    tan ---- = ------   ->  dw = dc * tan ------
         2       dc                          2

Let Qsx, Qsy the coordinates of the point Q in the screen, in pixels.
Let w the screen width, in pixels. Then:

     Qsx     dq             Qsx * dw     2 * Qsx * dc * tan(fovh/2)
    ----- = ----  ->  dq = ---------- = ----------------------------
     w/2     dw               w/2                    w

Now, remember that dc is the distance C-K. We can do as explained in
https://mathinsight.org/distance_point_plane to calculate this distance.
We define the plane K as the plane that passes through the point P (that we
have previously computed and was P = (0, Py, 0)) and has normal vector
n = (0, cos a, -sin a) (where the camera looks). Note that n is already
normalized. The plane equation would be:

    cos a * y - sin a * z - cos a * Py = 0

Then, according to the website, the distance C-K is:

    dc = |-sin a * Cz - cos a * Py|

Assuming that Cz is always positive, as well as Py (otherwise the point P
wouldn't be visible to the camera) and that a is in the range [0, PI/2]:

    dc = sin a * Cz + cos a * Py

Then:

          2 * Qsx * (sin a * Cz + cos a * Py) * tan(fovh/2)
    dq = --------------------------------------------------- = kpy * Qsx
                                 w


Where kpy is:

       2 * sin a * Cz * tan(fovh/2)     2 * cos a * tan(fovh/2)
kpy = ------------------------------ + ------------------------- * Py
                     w                             w

For simplicity, let k4 = (2 * sin a * Cz * tan(fovh/2) / w) and
k5 = (2 * cos a * tan(fovh/2) / w). Then:

    kpy = k4 + k5 * Py

And then:

    dq = (k4 + k5 * Py) * Qsx

*/

#include "cameraparameters.hpp"

#include <math.h>

#include "utilities.hpp"

using namespace utilities;

// Limits of camera angle
const float CameraParameters::min_angle = 0.0;
const float CameraParameters::max_angle = M_PI / 2.0;

// Limit of camera Z. Camera Z never can be 0.
const float CameraParameters::min_z = 1.0;

/* Constructor.
   Parameters:
     * width: camera's image width.
     * height: camera's image height.
     * fovh: horizontal Field Of View.
     * fovv: vertical Field Of View.
     * z: distance from the camera to the plane Z = 0.
     * angle: angle between the camera and the plane Z = 0 (always
         positive and always looking down).
*/
CameraParameters::CameraParameters(size_t width, size_t height, float fovh,
        float fovv, float z, float angle):
    width(width), height(height), fovh(fovh), fovv(fovv), z(z), angle(angle)
{
    // Initialize the constants for the coordinates transformations
    float tan_a = tan(angle);
    float kv = height / (2 * tan(to_rad(fovv/2.0)));
    k1 = z * kv;
    k2 = z * tan_a;
    k3 = tan_a * kv;
    float kh = 2 * tan(to_rad(fovh/2.0)) / width;
    k4 = sin(angle) * z * kh;
    k5 = cos(angle) * kh;
}

/* Return the X coordinate in screen reference frame from the X
   coordinate in world reference frame.
   Parameters:
     * wx: X coordinate in world reference frame.
     * wy: Y coordinate in world reference frame.
*/
int
CameraParameters::get_screen_x(float wx, float wy) const
{
    // Transform to world point
    int sx = wx / (k4 + k5 * wy);
    // Translate the screen point to the left origin
    return sx + (int)width/2;
}

/* Return the Y coordinate in screen reference frame from the Y
   coordinate in world reference frame.
   Parameters:
     * wy: Y coordinate in world reference frame.
*/
int
CameraParameters::get_screen_y(float wy) const
{
    // Transform to screen point
    int sy = (k3 * wy - k1) / (k2 + wy);
    // Translate the screen point to the top origin
    return (int)height/2 - sy;
}

/* Return a world point from a screen point.
   Parameters:
     * sx: screen x coordinate.
     * sy: screen y coordinate.
*/
glm::vec2
CameraParameters::get_world_point(int sx, int sy) const
{
    float wy = get_world_y(sy);
    return glm::vec2(get_world_x(sx, wy), wy);
}

/* Return the X coordinate in world reference frame from the X
   coordinate in screen reference frame. The y coordinate in world
   frame is also necessary for this transformation.
   Parameters:
     * sx: X coordinate in screen reference frame.
     * wy: Y coordinate in world reference frame.
*/
float
CameraParameters::get_world_x(int sx, float wy) const
{
    // Translate the screen point to the center of the screen origin
    sx = sx - (int)width/2;
    // Transform to world point
    return (k4 + k5 * wy) * sx;
}

/* Return the Y coordinate in world reference frame from the Y
   coordinate in screen reference frame.
   Parameters:
     * sy: Y coordinate in screen reference frame.
*/
float
CameraParameters::get_world_y(int sy) const
{
    // Translate the screen point to the center of the screen origin
    sy = (int)height/2 - sy;
    // Transform to world point
    return (k1 + k2 * sy)/(k3 - sy);
}

