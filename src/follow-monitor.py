#!/usr/bin/env python

import argparse
import ipaddress
import pygame
import time

import follow

# Window's size
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600

# Time between processing input events to update speed and turn
UPDATE_DELTA_TIME = 0.05

# Amount to increase/decrease turn if the button is pressed
TURN_DELTA = 0.01

# Amount to increase/decrease speed if the button is pressed
SPEED_DELTA = 0.01

# Button to accelerate (the B button of the SNES gamepad)
SPEED_BUTTON = 2

# Button to switch from autonomous to piloted mode (start button of the SNES
# gamepad)
START_BUTTON = 9

# Basic colors to render things
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
MAGENTA = (255, 0, 255)

# Values to scale the road to fit it in the screen
X_SCALE = 5
Y_SCALE = 5

# Initialization of pygame. This is done here because we need it initialized
# to create the fonts just down here
pygame.init()

# Font used to print some texts in the creen
FONT = pygame.font.SysFont("sans", 24)

# Texts for the speed and turn status widgets
TURN_TEXT = FONT.render('Turn', True, WHITE)
SPEED_TEXT = FONT.render('Speed', True, WHITE)


class FollowMonitorMain(object):
    '''follow-monitor main entry point.

    Constructor parameters:
      * network: network to scan for line follower robots.
    '''

    def __init__(self, network):
        # Scan for line followers
        self._scan_followers(network)
        self._init_pygame()
        # Stop flag
        self.done = False
        # Motion attributes
        self.turn = 0.0
        self.speed = 0.0
        self.feedback_turn = 0.0
        self.feedback_speed = 0.0
        self.wheel_axis_offset = None
        self.wheel_distance = None
        self.wheel_diameter = None
        self.front_wheel_offset = None
        self.started = False

    def _scan_followers(self, network):
        '''Scan for line followers.

        Parameters:
          * network: object of type IPv4Network that describes the network to
              scan for line follower robots.
        '''
        print('scanning {} for line followers...'.format(network))
        # Prepare the list of hosts to scan
        if network.num_addresses == 1:
            scan_hosts = [network.network_address]
        else:
            scan_hosts = network.hosts()
        self.follower = None
        # Show the list of line follower robots
        index = 0
        followers = []
        for addr, echo_string in follow.scan(scan_hosts):
            print('[{}] {}:{}: {}'.format(
                index, addr[0], addr[1], echo_string))
            followers.append(addr)
        if not followers:
            # No followers found, 
            print('none found')
        else:
            if len(followers) == 1:
                # One follower found, connect to that one
                index = 0
            else:
                # Several followers found, ask the user which one to connect to
                index = -1
                while index < 0 or index >= len(followers):
                    index = int(input('connect to follower: '))
            print('connecting to follower {}'.format(index))
            self.follower = follow.Follower(followers[index])
            # Subscribe for events
            self.follower.subscribe()
            # Send a message to get info from the robot. The info includes the
            # geometry of the robot.
            self.follower.get_info()

    def _init_pygame(self):
        '''Initialize pygame stuff.'''
        pygame.joystick.init()
        # Create the screen
        self.screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
        # Create the joystick
        if pygame.joystick.get_count():
            self.joystick = pygame.joystick.Joystick(0)
            self.joystick.init()
        else:
            print('no joystick found')

    def run(self):
        '''Execute the application's main loop.'''
        # This variable holds the next time at which the gamepad inputs must be
        # processed
        next_update_time = 0.0
        while not self.done:
            self._process_pygame_events()
            # Update speed and turn values
            now = time.time()
            if now > next_update_time:
                self._update_speed_and_turn()
                next_update_time = now + UPDATE_DELTA_TIME
            self._process_follower_events()
            self._render()
            # Switch front and back buffers
            pygame.display.flip()

    def _process_pygame_events(self):
        '''Process pygame events.'''
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                # Window closed
                self.done = True
            if event.type == pygame.JOYBUTTONDOWN:
                # Button of gamepad pressed (only START_BUTTON event is used)
                if event.button == START_BUTTON:
                    if self.started:
                        self.follower.stop()
                    else:
                        self.follower.start()

    def _update_speed_and_turn(self):
        '''Updates the speed and turn according to the buttons pressed and
        if there's new values send them to the robot.
        '''
        # Update speed
        # Keep the previous speed to check if different than new
        prev_speed = self.speed
        if self.joystick.get_button(SPEED_BUTTON):
            # Accelerate if the button was pressed (limit to 1.0)
            self.speed = min(self.speed + SPEED_DELTA, 1.0)
        else:
            # Decelerate if the button wasn't pressed (limit to 0.0)
            self.speed = max(self.speed - SPEED_DELTA, 0.0)
        # Update turn
        # Keep the previous turn to check for differences
        prev_turn = self.turn
        delta = self.joystick.get_axis(0)
        if delta != 0.0:
            turn = self.turn - delta * TURN_DELTA
        else:
            # If not turning, set it to 0.0 to stop turning immediately
            turn = 0.0
        # Set turn inside the limits
        self.turn = min(max(turn, -1.0), 1.0)
        # Send a command if the speed or turn where changed
        if prev_speed != self.speed or prev_turn != self.turn:
            self.follower.move(self.speed, self.turn)

    def _process_follower_events(self):
        '''Process the received events from the robot.'''
        evt = self.follower.get_event()
        while evt is not None:
            if evt.type == follow.EVT_SUBSCRIBED:
                print('subscribed')
            elif evt.type == follow.EVT_UNSUBSCRIBED:
                print('unsubscribed')
            elif evt.type == follow.EVT_STARTED:
                self.started = True
            elif evt.type == follow.EVT_STOPPED:
                self.started = False
            elif evt.type == follow.EVT_MOVED:
                # Got feedback of the movement
                self.feedback_turn = evt.turn
                self.feedback_speed = evt.speed
            elif evt.type == follow.EVT_DATA:
                # The road and path geometry data
                self.sections = evt.sections
                self.path = evt.path
            elif evt.type == follow.EVT_INFO:
                # Robot's geometric info
                self.wheel_axis_offset = evt.wheel_axis_offset
                self.wheel_distance = evt.wheel_distance
                self.wheel_diameter = evt.wheel_diameter
                self.front_wheel_offset = evt.front_wheel_offset
            evt = self.follower.get_event()

    def _render(self):
        '''Render things on screen.'''
        # Clear the screen
        self.screen.fill(BLACK)
        self._render_road()
        self._render_path()
        self._render_robot()
        self._render_status_widgets()

    def _render_road(self):
        '''Render the road.'''
        for i in range(1, len(self.sections)):
            s0 = self.sections[i - 1]
            s1 = self.sections[i]
            # Draw the left line
            pygame.draw.line(self.screen, BLUE, self._wtos((s0[0], s0[3])),
                self._wtos((s1[0], s1[3])))
            # Draw the right line
            pygame.draw.line(self.screen, BLUE, self._wtos((s0[2], s0[3])),
                self._wtos((s1[2], s1[3])))
            # Draw the central line. Check for errors because somtimes it is
            # invalid
            try:
                pygame.draw.line(self.screen, YELLOW,
                    self._wtos((s0[1], s0[3])), self._wtos((s1[1], s1[3])))
            except TypeError: pass

    def _render_path(self):
        '''Render the path that the line follower robot follows.'''
        for i in range(1, len(self.path)):
            p0 = self.path[i - 1]
            p1 = self.path[i]
            # Draw the path segment
            pygame.draw.line(
                self.screen, GREEN, self._wtos(p0), self._wtos(p1))

    def _wtos(self, wp):
        '''Transform world coordinates to screen coordinates.

        Parameters:
          * wp: world coordinates point.
        '''
        return (int(wp[0]*X_SCALE) + 400,
            WINDOW_HEIGHT - int(wp[1]*Y_SCALE) - 140)

    def _render_status_widgets(self):
        '''Render speed and turn status widgets.'''
        # Draw the turn status widget
        pygame.draw.rect(self.screen, WHITE, [150, 550, 500, 40], 1)
        self.screen.blit(TURN_TEXT, (75, 555))
        pygame.draw.rect(
            self.screen, RED, [400, 553, int(self.feedback_turn*250), 34], 0)
        pygame.draw.line(self.screen, WHITE, [400, 545], [400, 595], 1)
        # Draw the speed status widget
        pygame.draw.rect(self.screen, WHITE, [750, 100, 40, 400], 1)
        pygame.draw.rect(self.screen, BLUE,
            [753, 497, 34, int(-self.feedback_speed*400)], 0)
        self.screen.blit(SPEED_TEXT, (725, 50))

    def _render_robot(self):
        '''Render a simple representation of the robot.'''
        # Draw the triangle that represents the robot
        pygame.draw.line(self.screen, MAGENTA,
            self._wtos((0, self.front_wheel_offset)),
            self._wtos((-self.wheel_distance/2.0, self.wheel_axis_offset)), 1)
        pygame.draw.line(self.screen, MAGENTA,
            self._wtos((-self.wheel_distance/2.0, self.wheel_axis_offset)),
            self._wtos((self.wheel_distance/2.0, self.wheel_axis_offset)), 1)
        pygame.draw.line(self.screen, MAGENTA,
            self._wtos((self.wheel_distance/2.0, self.wheel_axis_offset)),
            self._wtos((0, self.front_wheel_offset)), 1)
        # Draw the wheels
        pygame.draw.line(self.screen, MAGENTA,
            self._wtos((-self.wheel_distance/2.0,
            self.wheel_axis_offset - self.wheel_diameter/2.0)),
            self._wtos((-self.wheel_distance/2.0,
            self.wheel_axis_offset + self.wheel_diameter/2.0)), 1)
        pygame.draw.line(self.screen, MAGENTA,
            self._wtos((self.wheel_distance/2.0,
            self.wheel_axis_offset - self.wheel_diameter/2.0)),
            self._wtos((self.wheel_distance/2.0,
            self.wheel_axis_offset + self.wheel_diameter/2.0)), 1)
        # Draw a small circle that represents the camera
        pygame.draw.circle(self.screen, MAGENTA, self._wtos((0, 0)), 5)

# Main entry point
if __name__ == '__main__':
    # Command line arguments description
    parser = argparse.ArgumentParser(
        description='Communication with line follower robots.')
    parser.add_argument('network', type=ipaddress.IPv4Network,
        help='network to scan for line follower robots')
    args = parser.parse_args()

    # Launch the main loop
    FollowMonitorMain(args.network).run()

