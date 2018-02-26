
#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include "opencv2/opencv.hpp"
#include <string>

using namespace cv;
using namespace std;

namespace utilities {

    // Calculate the absolute value of the elements in an array.
    void abs(int *src, size_t size, int *dst);

    /* Compute the distance between to colors.
       Parameters:
         * a: first color.
         * b: second color.
    */
    unsigned int colordistance(const Scalar& a, const Scalar& b);

    // Calculate the derivative of an array.
    void derivative(uchar *x, size_t size, int *dx);

    // Set to zero all the elements in the array lower than value.
    void filterlt(int *src, size_t size, int value, int *dst);

    /* Load a file into a memory buffer.
       This memory buffer must be freed by the user.
       Parameters:
         * filename: file to open.
    */
    const char *loadfile(const char *filename);

    // Find the local maximums of the non-zero contiguous values in an array.
    size_t localmax(int *src, size_t size, size_t max_points, int* max);

    /* Compute the mean color of an array of pixels.
       Parameters:
         * ptr: pointer to a pixel in a Mat.
         * size: number of pixels to use for the mean.
         * channels: number of color channels.
         * mean_color: output mean color.
    */
    void meancolor(
        uchar *ptr, size_t size, size_t channels, Scalar& mean_color);

    // Plot a 1D vector.
    void plot(int *x, size_t size, int scaley, Mat& plt);

    /* Print the FPS.
       Parameters:
         * nframes: number of frames elapsed since the last call to printfps.
    */
    void printfps(unsigned int nframes);

}

#endif

