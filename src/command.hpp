
#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <netinet/in.h>
#include <list>
#include <vector>

#include "line.hpp"
#include "options.hpp"

// Enumeration of message types
typedef enum {
    MSG_NULL,
    CMD_ECHO,
    CMD_SUBSCRIBE,
    CMD_UNSUBSCRIBE,
    CMD_START,
    CMD_STOP,
    CMD_MOVE,
    CMD_INFO,
    EVT_ECHO,
    EVT_SUBSCRIBED,
    EVT_UNSUBSCRIBED,
    EVT_STARTED,
    EVT_STOPPED,
    EVT_MOVED,
    EVT_DATA,
    EVT_INFO
} msg_type_t;

// Contents of a message
typedef struct {
    // The message type
    msg_type_t type;
    union {
        // Move command/event
        struct {
            float speed;
            float turn;
        };
        // Info event
        struct {
            float wheel_axis_offset;
            float wheel_distance;
            float wheel_diameter;
            float front_wheel_offset;
        };
    };
} msg_t;

// Data from a subscriptor
typedef struct {
    // The subscriptor address
    struct sockaddr_in addr;

    // Timestamp of last sent command
    struct timespec timestamp;
} subscriptor_t;

class Command {

    public:

        Command();

        /* Constructor.
           Parameters:
             * options: application options.
        */
        Command(const Options& options);

        ~Command();

        // Close the Command manager.
        void close();

        /* Receive a new command and return it.
           Parameters:
             * command: the command received.
           Return 1 if a command was received, 0 otherwise.
        */
        int get_command(msg_t& command);

        /* Send the tracked line to the subscriptors.
           Parameters:
             * line: the line.
        */
        void send_data(const Line& line);

        /* Send an event to the subscriptors.
           Parameters:
             * event: the event to send.
        */
        void send_event(const msg_t& event);

    private:

        // File descriptor for the UDP socket
        int fd;

        // The subscriptors
        list<subscriptor_t> subscriptors;

        // Some info about the geometrics of the robot to be sent when asked
        // for
        float wheel_axis_offset;
        float wheel_distance;
        float wheel_diameter;
        float front_wheel_offset;

        // Lengths of the different types of messages
        static const ssize_t msg_lengths[];

        // Contents of the echo event
        static char echo_event[];

        // Current timestamp, to manage timeout subscriptors desconnection
        struct timespec current_timestamp;

        // Time of inactivity to remove subscriptors
        int inactivity_timeout;

        /* Send an echo event.
           Parameters:
             * addr: the recipient's address.
        */
        void echo(struct sockaddr_in& addr);

        /* Get a subscriptor given its address.
           Parameters:
             * addr: IP address of the subscriptor.
        */
        list<subscriptor_t>::iterator get_subscriptor(
            struct sockaddr_in& addr);

        // Remove the subscriptors with an old timestamp
        void remove_subscriptors_timeout();

        /* Send data to all the subscriptors.
           Parameters:
             * data: the data to send.
             * size: the size of the data to send.
        */
        void send_event_data(const char *data, size_t size);

        /* Send robot geometric info to a client.
           Parameters:
             * addr: the recipient's address.
        */
        void send_info(struct sockaddr_in& addr);

        /* Add a subscriber.
           Parameters:
             * addr: Internet address of the subscriber.
        */
        void subscribe(struct sockaddr_in& addr);

        /* Remove a subscriber.
           Parameters:
             * addr: Internet address of the subscriber.
        */
        void unsubscribe(struct sockaddr_in& addr);

        /* Removes a subscriptor given an iterator that poinst to it.
           Parameters:
             * it: the iterator that points to the subscriptor.
        */
        void unsubscribe_iterator(list<subscriptor_t>::iterator it);

        /* Set the timestamp of a subscriptor to the current timestamp.
           Parameters:
             * addr: address of the subscriptor.
        */
        void update_subscriptor_timestamp(struct sockaddr_in& addr);

};

#endif

