
bin_PROGRAMS = follow

virtual_camera_SOURCES = acrossroadsegment.cpp acrossroadsegment.hpp \
    dashedline1segment.cpp dashedline1segment.hpp dashedline2segment.cpp \
    dashedline2segment.hpp doubleturnleftsegment.cpp doubleturnleftsegment.hpp\
    doubleturnrightsegment.cpp doubleturnrightsegment.hpp \
    doubleturnsegment.cpp doubleturnsegment.hpp gl.hpp narrowsegment.cpp \
    narrowsegment.hpp narrowwidesegment.cpp narrowwidesegment.hpp \
    squaresegment.cpp squaresegment.hpp straightsegment.cpp \
    straightsegment.hpp tracksegment.cpp tracksegment.hpp turnleftsegment.cpp \
    turnleftsegment.hpp turnrightsegment.cpp turnrightsegment.hpp \
    turnsegment.cpp turnsegment.hpp vcrossroadsegment.cpp \
    vcrossroadsegment.hpp virtualcamera.cpp virtualcamera.hpp \
    virtualmotors.cpp virtualmotors.hpp virtualtrack.cpp virtualtrack.hpp \
    widenarrowsegment.cpp widenarrowsegment.hpp zigzaglinesegment.cpp \
    zigzaglinesegment.hpp

follow_SOURCES = camera.cpp camera.hpp camparams.hpp command.cpp command.hpp \
    daemon.h differentialroadfinder.cpp differentialroadfinder.hpp \
    followexception.hpp framecapture.cpp framecapture.hpp gpiomotors.cpp \
    gpiomotors.hpp linefollowerapp.cpp linefollowerapp.hpp log.cpp log.hpp \
    main.cpp motors.cpp motors.hpp options.cpp options.hpp pathfinder.hpp \
    pilot.cpp pilot.hpp realcamera.cpp realcamera.hpp road.cpp roadfinder.hpp \
    road.hpp ssfapathfinder.cpp ssfapathfinder.hpp utilities.cpp utilities.hpp

follow_CFLAGS = ${opencv_CFLAGS}
follow_LDADD = -lrfsgpio -lpthread ${opencv_LIBS}

if GLES2
follow_SOURCES += $(virtual_camera_SOURCES)
follow_CFLAGS += ${glesv2_CFLAGS} ${egl_CFLAGS} ${x11_CFLAGS}
follow_LDADD += ${glesv2_LIBS} ${egl_LIBS} ${x11_LIBS}
endif

if PYTHON
python_PYTHON = follow.py
dist_bin_SCRIPTS = follow-monitor follow-command
endif

