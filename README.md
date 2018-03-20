
# linefollower

`linefollower` is an application to govern a line follower robot. Instead of
using reflectance sensors, this robot uses a camera to see what's in front
of him. The current implementation supports
**sysfs controlled GPIO/PWM motors**, a **`v4l` camera** and implements a
simulator using **OpenGL** that is useful to develop the application itself or
to test new algorithms.

## Getting Started

### Prerequisites

`linefollower` has the following dependencies:

* Library **rfsgpio**, also from the project *Robots From Scratch!*
* Library **OpenCV**, with support for `v4l`. Acticate as well support for
  `gtk` in this library to be able to show windows that display the captured
  frame (when the robot is connected to a screen).
* If support for the virtual camera is desired then **Mesa** is required, with
  support for **GLESv2** and **EGL**, as well as **libX**.

### Installing

To install this package, just clone the repository (or download the release
tarball) and execute the three known steps:

```
./configure
make
make install
```

By default, support for everything is chosen. To remove support for the virtual
camera set the option `--without-gles2` in the `./configure` command. To remove
support for displaying the captured frame set the option `--without-gtk` in
the `./configure` command.

### Using `linefollower`

The `linefollower` application is a daemon that analyzes the image captured
from the camera to determine where the line is and then gives orders to the
motors to move the robot in the correct direction.

The application can be in two different modes: **piloted** or **autonomous**.
In **piloted** mode, the application only analyzes the image but doesn't pilot
the motors by itself. It is the user that sends remote commands to move the
robot (like an RC car).

In **autonomous** mode, the application pilots the motors. In this mode, the
user commands to pilot won't work.

The application listens to an UDP socket where the commands to pilot the robot
or change from one mode to another are sent. By default, this is the 10101
port. A remote client can send a subscription request to the application using
this socket to receive events from the robot (like confirmation of change
of speed or mode). Also, the robot sends the geometry of the estimated line
to the subscriptors. The package provides the `Python` script
`follow-monitor.py` and the `Python` module `follow.py` to communicate with the
application from a remote machine.

The application is configured using a configuration file. Its path can be
given through the command line arguments, but by default usually it will be
`/etc/follow.conf`. The configuration file that comes with the package has
comments on it that are self explanatory.

`linefollower` can use a real camera or a virtual one (if support for this is
set in configure time, of course). In real camera mode, the image is captured
from a camera using the library **OpenCV+v4l**. In virtual camera mode, a
virtual track is rendered using **OpenGL ES 2**, and then the Color Buffer
image used as if it were the image from a camera. The virtual track is
configurable, it is formed by segments one after the other, more like an
*Scalextric*. The file where the virtual track is defined is given in the
application's configuration file.

### Using the real camera

Using the real camera is the default mode (used if no option is given):

```
Camera=real
```

You can set the camera image resolution, in pixels. Default is 640 x 480 if
none is given. The resolution must be a resolution supported by the camera:

```
CameraWidth=640
CameraHeight=480
```

Two important parameters are the camera's **Field Of View** (horizontal and
vertical), in degrees. These are used either for the real and the virtual
camera:

```
CameraFovh=62.2
CameraFovv=48.8
```

Also two important parameters that define the position and orientation of the
camera in space are the camera's distance to the floor and the camera's angle
with the horizontal plane. The application assumes that the robot is rolling
in a flat track, so with these two parameters and the FOV the robot position
can be stablished. The distance to the floor is given in cm and the angle
is given in degrees under the horizontal plane:

```
CameraZ=2.57856
CameraAngle=25
```

### Using the virtual camera

To use the virtual camera set the option:

```
Camera=virtual
```

In this mode, the `CameraWidth` and `CameraHeight` parameters give the size
of the **OpenGL** used viewport. The parameters `CameraFovh`, `CameraFovv`,
`CameraZ` and `CameraAngle` are equally used.

The option `TrackFile` gives the path to a file with the virtual track
description:

```
TrackFile=track1
```

This file contains a list of track segments, like this:

```
Straight
Straight
TurnLeft
Straight
TurnLeft
Straight
```

It gives the sequence of track segments. A segment must be seen as a node in
a graph with one or more inputs and one or more outputs. Inputs are the points
of the segment that can be connected with one or more previous segments.
Outputs are the points in the segment that connect with one or more subsequent
segments. Usually the segments have one input and one output, but there's some
segments, like crossroads, that can connect several roads, and then have
several inputs and outputs.

The complete format of a line in the track file is:

```
<segment_type> <input> <previous>:<output>
```

Where:

* `segment_type`: The type of segment to use.
* `input`: The input in this segment to use. If is omitted, the first input is
  used. If the segment doesn't have the given input number, the first input is
  used.
* `previous`: The previous segment to connect to. If it is omitted, the
  previous segment in the file is used. If a value is given, it must be the
  index in the track file of the segment to connect to. A negative number means
  the Nth segment before the current one (for example, -1 would be the previous
  segment in the list).
* `output`: The output in the previous segment to connect to. If it is omitted,
  the first output is used.

The type of segments supported are:

* `Straight`: Straight segment.
* `TurnLeft`: Short turn left.
* `TurnRight`: Short turn right.
* `Dashed1`: Straight segment with dashed central line.
* `Dashed2`: Another straight segment with a different dashed pattern.
* `ZigZag`: Straight segment with a central line in zig-zag.
* `WideNarrow`: Straight segment where the external lines get closer at the
  end of the segment.
* `Narrow`: Straight segment where the external lines are closer.
* `NarrowWide`: Straight segment where the external lines start close to each
  other and then separate until the standard distance.
* `VCrossroad`: A segment where one road arives and two different paths depart.
* `ACrossroad`: A segment where two different paths are joined into one.
* `DoubleTurnLeft`: Long turn left.
* `DoubleTurnRight`: Long turn right.

Example:

```
Straight
VCrossroad
TurnRight
Straight
TurnRight
ACrossroad
TurnLeft 0 -5:1
Straight
TurnLeft
Straight 0 -4
```

Here, the 7th segment (`TurnLeft`) connects with the segment 5 positions before
it (`VCrossroad`), output number 1. The last segment (`Straight`) connects with
the segment 4 positions before it (`ACrossroad`). This one only has one output,
so it's not necessary to specify the output.

### Using the GPIO motors

By default, the real motors are used:

```
Motors=real
```

At the moment, only the type `gpio` real motors are implemented. These are the
default. The application can be extended to use other type of motors (for
example, I2C motors, etc.):

```
RealMotorsType=gpio
```

The GPIO motors are defined by 7 parameters:

```
GPIOMotorsPWMLeft=1
GPIOMotorsPWMRight=0
GPIOMotorsDirection0Left=24
GPIOMotorsDirection1Left=23
GPIOMotorsDirection0Right=4
GPIOMotorsDirection1Right=17
GPIOMotorsPWMFrequency=20000
```

`GPIOMotorsPWMLeft` and `GPIOMotorsPWMRight` are the PWM channels to use for
the left and right motors (as seen by the `sysfs` PWM driver).
`GPIOMotorsDirection*` are the GPIO pin numbers to use to give the direction
of the motor to the motor controller. Their use is resumed in the next table:

`GPIOMotorsDirection0*` | `GPIOMotorsDirection1*` | Motor direction
----------------------- | ----------------------- | ---------------
0                       | 0                       | Stopped
0                       | 1                       | Backwards
1                       | 0                       | Forwards
1                       | 1                       | Stopped

`GPIOMotorsPWMFrequency` is the frequency in Hz of the PWM signal.

### Using the virtual motors

To be able to move the "virtual robot" throught the virtual track, a couple
of virtual motors are used. It is just a mean to move the camera through the
**OpenGL** scene. To use the virtual motors set the option:

```
Motors=virtual
```

The virtual motors are defined by 4 parameters:

```
VirtualMotorsRpm=778
WheelDistance=7.18
WheelDiameter=4
WheelAxisOffset=-11.8
```

`VirtualMotorsRpm` is the maximum speed of the motors (in RPM), `WheelDistance`
is the distance between wheels, `WheelDiameter` is the diameter of the wheels
and `WheelAxisOffset` is the distance from the wheels axis to the center of
the robot (where the camera is). These 4 parameters are necessary to calculate
the positions of the robot in the virtual scene.

Note that the option `Motors` can be set to `both`, in that case if using a
virtual camera the real robot will move at the same time that the camera does
in the virtual world. In the other hand, the virtual motors can only be used
with a virtual camera.

### Using the Line Tracker algorithm

TBD

### Using the piloting algorithm

TBD

## Authors

**Antonio Serrano Hernandez**.

## License

This project is licensed under the GPLv3 License - see the `COPYING` file for
details.

