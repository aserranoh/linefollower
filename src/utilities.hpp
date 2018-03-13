
#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <math.h>
#include "opencv2/opencv.hpp"
#include <string>

#define NS_PER_S    1000000000.0

using namespace cv;
using namespace std;

namespace utilities {

    /* Calculate the derivative of an array.
       Parameters:
         * x: the array.
         * size: size of the x array.
         * dx: the pointer where to put the result. It must be previously
             allocated with at least size + 1 bytes (a sentinel value is added
             at the end). The first and the last positions of dx are always 0
             at exit.
    */
    void derivative(uchar *x, size_t size, int *dx);

    /* Load a file into a memory buffer.
       This memory buffer must be freed by the user.
       Parameters:
         * filename: file to open.
    */
    const char *loadfile(const char *filename);

    /* Find the absolute minimum and maximum of an array.
       Parameters:
         * src: the input array.
         * size: size of the src array.
         * min: position of the minimum in the array src.
         * max: position of the maximum in the array src.
    */
    void minmax(int *src, size_t size, int &min, int &max);

    // Plot a 1D vector.
    void plot(int *x, size_t size, int scaley, Mat& plt);

    /* Print the FPS.
       Parameters:
         * nframes: number of frames elapsed since the last call to printfps.
    */
    void printfps(unsigned int nframes);

    /* Convert the struct timespec to a double timestamp.
       Parameters:
         * ts: the timespec struct.
    */
    inline double timespec2double(struct timespec ts) {
        return (double)ts.tv_sec + (double)(ts.tv_nsec)/NS_PER_S;
    }

    // Convert degrees to radians
    inline float to_deg(float deg) {
        return deg * 180.0 / M_PI;
    }

    // Convert degrees to radians
    inline float to_rad(float deg) {
        return deg * M_PI / 180.0;
    }

}

#endif

