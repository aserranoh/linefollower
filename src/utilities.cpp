
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "followexception.hpp"
#include "utilities.hpp"

#define PLOT_ROWS           512
#define TIME_BETWEEN_FPS    2.0
#define BUFFER_SIZE         4096

#define MAX_STATIC_READ     1024

// Variables to calculate the fps
static struct timespec t_prev = {0, 0}, t_current;
static float total_time = 0.0;
static unsigned int frames = 0;

/* Calculate the derivative of an array.
   Parameters:
     * x: the array.
     * size: size of the x array.
     * dx: the pointer where to put the result. It must be previously
         allocated with at least size + 1 bytes (a sentinel value is added at
         the end). The first and the last positions of dx are always 0 at exit.
*/
void
utilities::derivative(uchar *x, size_t size, int *dx)
{
    dx[0] = 0;
    for (size_t i = 1; i < size; i++) {
        dx[i] = (int)(x[i]) - (int)(x[i - 1]);
    }
    // Sentinel value
    dx[size] = 0;
}

/* Load a file into a memory buffer.
   This memory buffer must be freed by the user.
   Parameters:
     * filename: file to open.
*/
const char *
utilities::loadfile(const char *filename)
{
    FILE *f;
    long size;
    char *buf;
    size_t total_read = 0, r;

    // Open the file
    f = fopen(filename, "r");
    if (!f) return 0;

    // Get the file size
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    // Allocate a buffer the size of the whole file
    buf = new char[size + 1];
    buf[size] = 0;

    // Copy the contents of the file to the buffer and return the buffer
    while ((r = fread(buf + total_read, 1, BUFFER_SIZE, f)) > 0) {
        total_read += r;
    }
    if (ferror(f)) {
        delete[] buf;
        buf = 0;
    }
    return buf;
}

/* Find the absolute minimum and maximum of an array.
   Parameters:
     * src: the input array.
     * size: size of the src array.
     * min: the minimum value.
     * minpos: position of the minimum in the array src.
     * max: the maximum value.
     * maxpos: position of the maximum in the array src.
*/
void
utilities::minmax(int *src, size_t size, int& min, int& minpos, int& max,
    int& maxpos)
{
    min = INT_MAX;
    max = INT_MIN;
    for (int i = 0; i < size; i++) {
        if (src[i] < min) {
            min = src[minpos = i];
        }
        if (src[i] > max) {
            max = src[maxpos = i];
        }
    }
}

/* Plot a 1D vector.
   Parameters:
     * x: the vector to plot.
     * size: size of the vector.
     * scaley: size of the y axis.
     * Mat: image where to plot.
*/
void
utilities::plot(int *x, size_t size, int scaley, Mat& plt)
{
    int min = INT_MAX, max = INT_MIN;
    int r0, r;

    plt = Mat(PLOT_ROWS, size, CV_8UC1, Scalar(255));
    // If scaley is lower than zero, autocalculate it from min and max
    if (scaley < 0) {
        // In the first pass get the min and max
        for (size_t i = 0; i < size; i++) {
            min = (x[i] < min) ? x[i] : min;
            max = (x[i] > max) ? x[i] : max;
        }
        scaley = max - min;
    }
    // If max and min are 0, exit
    if (min != 0 || max != 0) {
        // Plot the values scalling to the max and min
        r0 = (x[0] - min) * plt.rows / scaley;
        for (size_t i = 1; i < size; i++) {
            r = (x[i] - min) * plt.rows / scaley;
            line(plt, Point(i - 1, plt.rows - r0), Point(i, plt.rows - r),
                Scalar(0));
            r0 = r;
        }
    }
}

/* Print the FPS.
   Parameters:
     * nframes: number of frames elapsed since the last call to printfps.
*/
void
utilities::printfps(unsigned int nframes)
{
    float deltatime, t0, t1;

    if (t_prev.tv_sec == 0) {
        // The first time just get t_prev and don't count any frames
        clock_gettime(CLOCK_MONOTONIC, &t_prev);
    } else {
        // Compute the elapsed time and increment the frames counter
        clock_gettime(CLOCK_MONOTONIC, &t_current);
        t0 = timespec2double(t_prev);
        t1 = timespec2double(t_current);
        deltatime = t1 - t0;
        t_prev = t_current;
        total_time += deltatime;
        frames++;
        // Only print the fps when a certain amount of time has elapsed
        if (total_time > TIME_BETWEEN_FPS) {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n",
                frames, total_time, frames/total_time);
            total_time = 0.0;
            frames = 0;
        }
    }
}

