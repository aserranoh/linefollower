
import select
import socket
import struct
import time

# Events codes
EVT_ECHO = 8
EVT_SUBSCRIBED = 9
EVT_UNSUBSCRIBED = 10
EVT_STARTED = 11
EVT_STOPPED = 12
EVT_MOVED = 13
EVT_DATA = 14
EVT_INFO = 15

# Private constants
# Simple commands to send (the whole stream of bytes is the constant)
_CMD_ECHO = b'\x01'
_CMD_SUBSCRIBE = b'\x02'
_CMD_UNSUBSCRIBE = b'\x03'
_CMD_START = b'\x04'
_CMD_STOP = b'\x05'
_CMD_INFO = b'\x07'

# Move command code
_CMD_MOVE = 6

# Devaults values for scan function
_DEFAULT_TIMEOUT = 5
DEFAULT_PORT = 10101

# Constants to decode the data event
_FLOATS_PER_LINE_POINT = 2

class Event(object):
    '''An event received from the line follower robot.

    Every class that specializes this event must have two static attributes:
      * _LEN: size of the event, in bytes.
      * _CODE: code that identifies the event in the protocol.

    Constructor parameters:
      * bytes: the stream of bytes that contains the event.
    '''

    def __init__(self, bytes):
        # Check the length of the stream for this type of event
        if self._LEN is not None and len(bytes) != self._LEN:
            raise ValueError('wrong message size (code {})'.format(self._CODE))
        # Check that the event code is correct
        if bytes[0] != self._CODE:
            raise ValueError('wrong message code')
        # Create an attribute type with the code
        self.type = self._CODE

class EchoEvent(Event):
    '''An echo event sent by the line follower robot after an echo command.

    Attributes:
      * string: the string encoded in the echo event.

    Constructor parameters:
      * bytes: message bytes.
    '''

    _CODE = EVT_ECHO
    _LEN = 64

    def __init__(self, bytes):
        Event.__init__(self, bytes)
        self.string = bytes[1:bytes.find(b'\0', 1)].decode('utf-8')

class StartedEvent(Event):
    '''An start event, sent after the line follower robot has gone into
    autonomous mode.
    '''
    _CODE = EVT_STARTED
    _LEN = 1

class StoppedEvent(Event):
    '''An stopped event, sent after the line follower robot has gone into
    piloted mode.
    '''
    _CODE = EVT_STOPPED
    _LEN = 1

class SubscribedEvent(Event):
    '''A subscribed event, sent after the line follower robot has received
    a subscription command. This event is only sent to the subscriptor that
    sent the subscription command.
    '''
    _CODE = EVT_SUBSCRIBED
    _LEN = 1

class UnsubscribedEvent(Event):
    '''A unsubscribed event, sent after the line follower robot has received
    an unsubscribe command. This event is only sent to the subscriptor that
    sent the unsubscribe command.
    '''
    _CODE = EVT_UNSUBSCRIBED
    _LEN = 1

class MovedEvent(Event):
    '''A moved event, sent when the line follower robot moves the motors.

    Attributes:
      * speed: the speed used by the robot.
      * turn: the turn value used by the robot.

    Constructor parameters:
      * bytes: message bytes.
    '''

    _CODE = EVT_MOVED
    _LEN = 9

    def __init__(self, bytes):
        Event.__init__(self, bytes)
        self.speed, self.turn = struct.unpack('>xff', bytes)

class DataEvent(Event):
    '''The data event, that contains the geometry of the road and the path.

    Attributes:
      * sections: the sections of the road. This is a list of tuples
          (left_x, line_x, right_x, y).
      * path: the points of the path. This is a list of tuples (x, y).

    Constructor parameters:
      * bytes: message bytes.
    '''

    _CODE = EVT_DATA
    _LEN = None

    def __init__(self, bytes):
        Event.__init__(self, bytes)
        # Extract the number of points in the line
        num_p = bytes[1]
        # Extract the floating point values encoded in the event
        floats = struct.unpack('>xx{}f'.format(
            num_p*_FLOATS_PER_LINE_POINT), bytes)
        self.line = []
        next = 0
        # Get the line points
        for i in range(num_p):
            self.line.append((floats[next], floats[next+1]))
            next += _FLOATS_PER_LINE_POINT

class InfoEvent(Event):
    '''An info event, sent with some geometric information of the robot.

    Attributes:
      * wheel_axis_offset: distance of the wheels axis to the center of the
          robot (where the camera is).
      * wheel_distance: distance between wheels.
      * wheel_diameter: diameter of the wheels.
      * front_wheel_offset: distance from the front wheel to the center of the
          robot (where the camera is).

    Constructor parameters:
      * bytes: message bytes.
    '''

    _CODE = EVT_INFO
    _LEN = 17

    def __init__(self, bytes):
        Event.__init__(self, bytes)
        (self.wheel_axis_offset, self.wheel_distance, self.wheel_diameter,
            self.front_wheel_offset) = struct.unpack('>xffff', bytes)

class Follower(object):
    '''Implements the communication with a follower robot.

    Constructor parameters:
      * addr: address (ip, port) of the follower robot.
    '''

    # List that maps the event codes with the event constructors
    _EventFromCode = [None, None, None, None, None, None, None, None, None,
        SubscribedEvent,
        UnsubscribedEvent,
        StartedEvent,
        StoppedEvent,
        MovedEvent,
        DataEvent,
        InfoEvent,
    ]

    def __init__(self, addr):
        # Create the socket, set it as nonblocking
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(0)
        # Connect the socket to be able to use the send system call
        self.sock.connect(addr)

    def get_event(self):
        '''Read an event from the socket.

        Returns the read event or None if no data was available or there
        wasn't a valid event.
        '''
        evt = None
        try:
            bytes = self.sock.recv(1024)
            # Check that we didn't received an empty stream of bytes
            if not len(bytes):
                print('empty message received')
            else:
                # Try to build the event from the stream of bytes
                try:
                    evt = self._EventFromCode[bytes[0]](bytes)
                except (IndexError, ValueError) as e:
                    print(e)
        except BlockingIOError:
            # The socket would block, just return an empty event
            pass
        return evt

    def get_info(self):
        '''Send a info query command.

        This requests for info about the robot like its geometrics.
        '''
        self.sock.send(_CMD_INFO)

    def move(self, speed, turn):
        '''Send a move command to the line follower robot.

        Parameters:
          * speed: the speed of the movement.
          * turn: the turn value for the movement.
        '''
        bytes = struct.pack('>Bff', _CMD_MOVE, speed, turn)
        self.sock.send(bytes)

    def start(self):
        '''Send a start command to the line follower robot.'''
        self.sock.send(_CMD_START)

    def stop(self):
        '''Send a stop command to the line follower robot.'''
        self.sock.send(_CMD_STOP)

    def subscribe(self):
        '''Send a subscribe command to the line follower robot.'''
        self.sock.send(_CMD_SUBSCRIBE)

    def unsubscribe(self):
        '''Send a unsubscribe command to the line follower robot.'''
        self.sock.send(_CMD_UNSUBSCRIBE)

def scan(hosts, port=DEFAULT_PORT, timeout=_DEFAULT_TIMEOUT):
    '''Send an echo command to a list of hosts to determine who is listening.

    Parameters:
      * hosts: lists of hosts to scan.
      * port: port where to send the echo command (default is 10101).
      * timeout: maximum time to wait for answers.
    '''
    # Create the socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Create the poll object that will serve to do a timeouted wait
    poll = select.poll()
    poll.register(sock, select.POLLIN)
    # Send the ECHO command to all the hosts
    for h in hosts:
        sock.sendto(_CMD_ECHO, (str(h), port))
    # Wait for the commands to arrive
    target_time = time.time() + timeout
    while poll.poll((target_time - time.time())*1000):
        data, addr = sock.recvfrom(1024)
        try:
            # Check that the received event was an echo event
            cmd = EchoEvent(data)
            yield (addr, cmd.string)
        except ValueError: pass
    sock.close()

