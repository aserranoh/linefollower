
#include "command.hpp"

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "followexception.hpp"
#include "log.hpp"
#include "utilities.hpp"

#define EVT_ECHO_LENGTH 64
#define MAX_EVT_LENGTH  (sizeof(float)*4 + 1)
#define MAX_DATA_LENGTH 512
#define MAX_POINTS      32

using namespace utilities;

/* Encode a float to be sent through the network.
   Parameters:
     * f: the float to encode.
     * buf: buffer where to put it.
*/
void
encf(float f, char *buf) {
    uint32_t *i = (uint32_t*)&f;

    *(uint32_t*)buf = htobe32(*i);
}

/* Decode a float received throught the network.
   Parameters:
     * buf: buffer where the float is.
*/
float
decf(char *buf) {
    float f;
    uint32_t *i = (uint32_t*)&f;

    *i = be32toh(*(uint32_t*)buf);
    return f;
}

const ssize_t Command::msg_lengths[] = {
    0, 1, 1, 1, 1, 1, 1 + 2*sizeof(float), 1,
    EVT_ECHO_LENGTH, 1, 1, 1, 1, 1 + 2*sizeof(float), 0, 1 + 4*sizeof(float)
};

char Command::echo_event[EVT_ECHO_LENGTH];

Command::Command()
{}

/* Constructor.
   Parameters:
     * options: application options.
*/
Command::Command(const Options& options)
{
    struct sockaddr_in addr;
    char hostname[HOST_NAME_MAX + 1];

    // Create the socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw FollowException(string("cannot create socket: ")
            + strerror(errno));
    }
    // Make the socket non blocking
    fcntl(fd, F_SETFL, O_NONBLOCK);
    // Bind the socket
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)options.get_int("Port"));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        throw FollowException(string("cannot bind socket: ")
            + strerror(errno));
    }

    // Fill the contents of the echo event
    gethostname(hostname, HOST_NAME_MAX + 1);
    memset(echo_event, 0, msg_lengths[EVT_ECHO]);
    echo_event[0] = EVT_ECHO;
    snprintf(echo_event + 1, msg_lengths[EVT_ECHO] - 1,
        "%s, camera: %s, motors: %s", hostname,
        options.get_string("Camera").c_str(),
        options.get_string("Motors").c_str());

    // Keep some information to be sent when asked for
    wheel_axis_offset = options.get_float("WheelAxisOffset");
    wheel_distance = options.get_float("WheelDistance");
    wheel_diameter = options.get_float("WheelDiameter");
    front_wheel_offset = options.get_float("FrontWheelOffset");

    // Get the timeout to remove subscriptors after inactivity
    inactivity_timeout = options.get_int("InactivityTimeout");
}

Command::~Command()
{}

// Close the Command manager.
void
Command::close()
{
    if (fd >= 0) {
        ::close(fd);
    }
}

/* Receive a new command and return it.
   Parameters:
     * msg: the command received.
   Return 1 if a command was received, 0 otherwise.
*/
int
Command::get_command(msg_t& command)
{
    ssize_t rcvlen;
    char buf[msg_lengths[CMD_MOVE]];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    unsigned char cmd;
    struct timespec ts;

    // Receive a message
    rcvlen = recvfrom(fd, buf, msg_lengths[CMD_MOVE], 0, (sockaddr*)&addr,
        &addrlen);
    // If recvfrom returned error, check if it would block (return 0) or
    // if really is an error (exit with error)
    if (rcvlen == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Cleanup quiet subscriptors
            clock_gettime(CLOCK_MONOTONIC, &current_timestamp);
            remove_subscriptors_timeout();
            return 0;
        } else {
            throw FollowException(string("error receiving data: ")
                + strerror(errno));
        }
    }
    // If empty data was received, give a warning and return
    if (!rcvlen) {
        log_warn("received empty data");
        return 0;
    }
    // Check that the received command is valid
    cmd = buf[0];
    if (cmd < CMD_ECHO || cmd > CMD_INFO) {
        log_warn("received unknown command %d", cmd);
        return 0;
    }
    // Check that the size of the command corresponds to the command type
    if (rcvlen != msg_lengths[cmd]) {
        log_warn("wrong size (%zd) for command type %d (expected %zd)",
            rcvlen, cmd, msg_lengths[cmd]);
        return 0;
    }
    // Copy the command and do some treatement for some type of commands
    command.type = (msg_type_t)buf[0];
    switch (cmd) {
        case CMD_ECHO: echo(addr); break;
        case CMD_SUBSCRIBE: subscribe(addr); break;
        case CMD_UNSUBSCRIBE: unsubscribe(addr); break;
        case CMD_START:
        case CMD_STOP:
            break;
        case CMD_MOVE:
            command.speed = decf(buf + 1);
            command.turn = decf(buf + 1 + sizeof(float));
            break;
        case CMD_INFO: send_info(addr); break;
        default: break;
    }
    update_subscriptor_timestamp(addr);
    return 1;
}

/* Send the tracked line to the subscriptors.
   Parameters:
     * line: the line.
*/
void
Command::send_data(const Line& line)
{
    char buf[MAX_DATA_LENGTH];
    char *ptr;

    // Code of event
    buf[0] = EVT_DATA;
    // Number of points in the line
    buf[1] = (unsigned char)line.size();
    // Fill the buffer with the line points (up to MAX_POINTS)
    ptr = buf + 2*sizeof(unsigned char);
    for (size_t i = 0; i < line.size() && i < MAX_POINTS; i++) {
        const line_point_t& p = line.get_point(i);
        // X
        encf(p.x, ptr);
        // Y
        encf(p.y, ptr + sizeof(float));
        ptr += 2*sizeof(float);
    }
    // Send the data to all the subscribers
    send_event_data(buf, ptr - buf);
}

/* Send an event to the subscriptors.
   Parameters:
     * event: the event to send.
*/
void
Command::send_event(const msg_t& event)
{
    char buf[MAX_EVT_LENGTH];
    size_t size = sizeof(unsigned char);

    buf[0] = (char)event.type;
    if (event.type == EVT_MOVED) {
        encf(event.speed, buf + 1);
        encf(event.turn, buf + 1 + sizeof(float));
        size = sizeof(unsigned char) + sizeof(float) * 2;
    }
    send_event_data(buf, size);
}

// PRIVATE FUNCTIONS

/* Send an echo event.
   Parameters:
     * addr: the recipient's address.
*/
void
Command::echo(struct sockaddr_in& addr)
{
    if (sendto(fd, echo_event, msg_lengths[EVT_ECHO], 0, (sockaddr*)&addr,
        sizeof(addr)) < 0)
    {
        throw FollowException(string("error sending data: ")
            + strerror(errno));
    }
}

/* Get a subscriptor given its address.
   Parameters:
     * addr: IP address of the subscriptor.
*/
list<subscriptor_t>::iterator
Command::get_subscriptor(struct sockaddr_in& addr)
{
    list<subscriptor_t>::iterator it;
    bool equal;

    for (it = subscriptors.begin(); it != subscriptors.end(); it++) {
        equal = it->addr.sin_port == addr.sin_port
            && it->addr.sin_addr.s_addr == addr.sin_addr.s_addr;
        if (equal) {
            return it;
        }
    }
    return subscriptors.end();
}

// Remove the subscriptors with an old timestamp
void
Command::remove_subscriptors_timeout()
{
    list<subscriptor_t>::iterator it = subscriptors.begin();
    double ts, cur;

    while (it != subscriptors.end()) {
        ts = timespec2double(it->timestamp);
        cur = timespec2double(current_timestamp);
        if (cur - ts > inactivity_timeout) {
            // Timeout reached, unsubscribe
            unsubscribe_iterator(it++);
        } else {
            it++;
        }
    }
}

/* Send data to all the subscriptors.
   Parameters:
     * data: the data to send.
     * size: the size of the data to send.
*/
void
Command::send_event_data(const char *data, size_t size)
{
    list<subscriptor_t>::iterator it;

    for (it = subscriptors.begin(); it != subscriptors.end(); it++) {
        sendto(fd, data, size, 0, (sockaddr*)&(*it), sizeof(*it));
    }
}

/* Send robot geometric info to a client.
   Parameters:
     * addr: the recipient's address.
*/
void
Command::send_info(struct sockaddr_in& addr)
{
    char buf[MAX_EVT_LENGTH];

    buf[0] = EVT_INFO;
    encf(wheel_axis_offset, buf + 1);
    encf(wheel_distance, buf + 1 + sizeof(float));
    encf(wheel_diameter, buf + 1 + 2*sizeof(float));
    encf(front_wheel_offset, buf + 1 + 3*sizeof(float));
    sendto(fd, buf, msg_lengths[EVT_INFO], 0, (sockaddr*)&addr, sizeof(addr));
}

/* Add a subscriber.
   Parameters:
     * addr: Internet address of the subscriber.
*/
void
Command::subscribe(struct sockaddr_in& addr)
{
    char c = EVT_SUBSCRIBED;
    subscriptor_t s;
    struct timespec ts;

    // Create the subscriptor and add it to the list
    s.addr = addr;
    subscriptors.push_back(s);
    // Send an event back to inform that the subscription has taken place
    sendto(fd, &c, msg_lengths[EVT_SUBSCRIBED], 0, (sockaddr*)&addr,
        sizeof(addr));
}

/* Remove a subscriber.
   Parameters:
     * addr: Internet address of the subscriber.
*/
void
Command::unsubscribe(struct sockaddr_in& addr)
{
    list<subscriptor_t>::iterator it;

    it = get_subscriptor(addr);
    if (it != subscriptors.end()) {
        unsubscribe_iterator(it);
    }
}

/* Removes a subscriptor given an iterator that poinst to it.
   Parameters:
     * it: the iterator that points to the subscriptor.
*/
void
Command::unsubscribe_iterator(list<subscriptor_t>::iterator it)
{
    char c = EVT_UNSUBSCRIBED;

    // Send an event back to inform that the unsubscription has taken place
    sendto(fd, &c, msg_lengths[EVT_UNSUBSCRIBED], 0, (sockaddr*)&(it->addr),
        sizeof(it->addr));
    subscriptors.erase(it);
}

/* Set the timestamp of a subscriptor to the current timestamp.
   Parameters:
     * addr: address of the subscriptor.
*/
void
Command::update_subscriptor_timestamp(struct sockaddr_in& addr)
{
    list<subscriptor_t>::iterator it = get_subscriptor(addr);

    if (it != subscriptors.end()) {
        it->timestamp = current_timestamp;
    }
}

