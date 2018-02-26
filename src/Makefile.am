bin_PROGRAMS = follow
follow_SOURCES = acrossroadsegment.cpp acrossroadsegment.hpp camera.cpp \
                 camera.hpp camparams.hpp command.cpp command.hpp \
                 dashedline1segment.cpp dashedline1segment.hpp \
                 dashedline2segment.cpp dashedline2segment.hpp \
                 differentialroadfinder.cpp differentialroadfinder.hpp \
                 doubleturnleftsegment.cpp doubleturnleftsegment.hpp \
                 doubleturnrightsegment.cpp doubleturnrightsegment.hpp \
                 doubleturnsegment.cpp doubleturnsegment.hpp \
                 followexception.hpp framecapture.cpp framecapture.hpp \
                 gl.hpp gpiomotors.cpp gpiomotors.hpp linefollowerapp.cpp \
                 linefollowerapp.hpp main.cpp motors.cpp motors.hpp \
                 narrowsegment.cpp narrowsegment.hpp narrowwidesegment.cpp \
                 narrowwidesegment.hpp options.cpp options.hpp pathfinder.hpp \
                 pilot.cpp pilot.hpp realcamera.cpp realcamera.hpp road.cpp \
                 roadfinder.hpp road.hpp squaresegment.cpp squaresegment.hpp \
                 ssfapathfinder.cpp ssfapathfinder.hpp straightsegment.cpp \
                 straightsegment.hpp tracksegment.cpp tracksegment.hpp \
                 turnleftsegment.cpp turnleftsegment.hpp turnrightsegment.cpp \
                 turnrightsegment.hpp turnsegment.cpp turnsegment.hpp \
                 utilities.cpp utilities.hpp vcrossroadsegment.cpp \
                 vcrossroadsegment.hpp virtualcamera.cpp virtualcamera.hpp \
                 virtualmotors.cpp virtualmotors.hpp virtualtrack.cpp \
                 virtualtrack.hpp widenarrowsegment.cpp widenarrowsegment.hpp \
                 zigzaglinesegment.cpp zigzaglinesegment.hpp
follow_CFLAGS = ${glesv2_CFLAGS} ${egl_CFLAGS} ${x11_CFLAGS} ${opencv_CFLAGS}
follow_LDADD = -lrfsgpio -lpthread ${glesv2_LIBS} ${egl_LIBS} ${x11_LIBS} \
               ${opencv_LIBS}

python_PYTHON = follow.py
dist_bin_SCRIPTS = follow-monitor.py
