
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

// TODO: Don't use gettimeofday
// TODO: remove writefile
// TODO: remove readfile

// Variables to calculate the fps
static struct timeval t_prev = {0, 0}, t_current;
static float total_time = 0.0;
static unsigned int frames = 0;

/* Calculate the absolute value of the elements in an array.
   Parameters:
     * src: the input array.
     * size: the size of the input array.
     * dst: the destination array.
*/
void
utilities::abs(int *src, size_t size, int *dst)
{
    for (size_t i = 0; i < size; i++) {
        dst[i] = (src[i] >= 0) ? src[i] : -src[i];
    }
}

/* Compute the distance between to colors.
   Parameters:
     * a: first color.
     * b: second color.
*/
unsigned int
utilities::colordistance(const Scalar& a, const Scalar& b)
{
    int d0 = a[0] - b[0], d1 = a[1] - b[1], d2 = a[2] - b[2];
    return d0*d0 + d1*d1 + d2*d2;
}

/* Calculate the derivative of an array.
   Parameters:
     * x: the array.
     * size: size of the vector.
     * dx: the pointer where to put the result (must be previously allocated
           with at least size bytes).
*/
void
utilities::derivative(uchar *x, size_t size, int *dx)
{
    dx[0] = 0;
    for (size_t i = 1; i < size; i++) {
        dx[i] = (int)(x[i]) - (int)(x[i - 1]);
    }
}

/* Set to zero all the elements in the array lower than value.
   Parameters:
     * src: the input array.
     * value: the threshold value.
     * dst: the output array.
*/
void
utilities::filterlt(int *src, size_t size, int value, int *dst)
{
    for (size_t i = 0; i < size; i++) {
        dst[i] = (src[i] >= value) ? src[i] : 0;
    }
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

/* Find the local maximums of the non-zero contiguous values in an array.
   Parameters:
     * src: the input array.
     * value: the threshold value.
     * max_points: maximum number of local maximums to be found.
     * max: the list of indexes of the local maximums in the array.
   Return the number of local maximums found.
*/
size_t
utilities::localmax(int *src, size_t size, size_t max_points, int* max)
{
    int localmax = INT_MIN;
    size_t localmax_pos = 0;
    size_t num = 0;

    if (!max_points) {
        return 0;
    }
    for (size_t i = 0; i < size; i++) {
        if (src[i] > 0) {
            if (src[i] > localmax) {
                localmax = src[i];
                localmax_pos = i;
            }
        } else if (localmax_pos > 0) {
            max[num] = localmax_pos;
            num++;
            if (num >= max_points) {
                break;
            }
            localmax = INT_MIN;
            localmax_pos = 0;
        }
    }
    return num;
}

/* Compute the mean color of an array of pixels.
   Parameters:
     * ptr: pointer to a pixel in a Mat.
     * size: number of pixels to use for the mean.
     * channels: number of color channels.
     * mean_color: output mean color.
*/
void
utilities::meancolor(
    uchar *ptr, size_t size, size_t channels, Scalar& mean_color)
{
    unsigned int b = 0, g = 0, r = 0;
    for (uchar *p = ptr; p < ptr + (size * channels); p += channels) {
        b += p[0];
        g += p[1];
        r += p[2];
    }
    mean_color = Scalar(b/size, g/size, r/size);
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
    float deltatime;

    if (t_prev.tv_sec == 0) {
        // The first time just get t_prev and don't count any frames
        gettimeofday(&t_prev, 0);
    } else {
        // Compute the elapsed time and increment the frames counter
        gettimeofday(&t_current, 0);
        deltatime = (float)(t_current.tv_sec - t_prev.tv_sec
            + (t_current.tv_usec - t_prev.tv_usec) * 1e-6);
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

/* Read a value from a file.
   Parameters:
     * file: path to the file to read from.
     * value: contains the read string.
*/
void
utilities::readfile(const string& file, string& value)
{
    int fd, e, r;
    char buffer[MAX_STATIC_READ];

    fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
        throw FollowException(
            string("error opening ") + file + ": " + strerror(errno));
    }
    if ((r = read(fd, buffer, MAX_STATIC_READ)) < 0) {
        e = errno;
        close(fd);
        throw FollowException(
            string("error reading from ") + file + ": " + strerror(e));
    }
    buffer[r] = '\0';
    value = buffer;
    close(fd);
}

/* Write a value to a file.
   Parameters:
     * file: path to the file to write to.
     * value: string to write.
*/
void
utilities::writefile(const string& file, const string& value)
{
    int fd, e;

    fd = open(file.c_str(), O_WRONLY);
    if (fd < 0) {
        throw FollowException(
            string("error opening ") + file + ": " + strerror(errno));
    }
    if (write(fd, value.c_str(), value.length()) < 0) {
        e = errno;
        close(fd);
        throw FollowException(string("error writing '") + value + "' to " + file
            + ": " + strerror(e));
    }
    close(fd);
}

