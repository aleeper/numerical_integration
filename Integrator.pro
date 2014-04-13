# --------------------------------------------------------------------------
# Sonny's Qt+OpenGL Boilerplate Application
#
# Run qmake on this file to generate the platform-specific project files
# for building this application.  Written for Qt 4.5 Open Source edition,
# but should be compatible with most previous/future versions.
#
# Author:  Sonny Chan
# Date:    June 2009
# --------------------------------------------------------------------------

# Generate a Visual Studio application if building on Windows.
win32 {
    TEMPLATE  = vcapp
}

SOURCES  += main.cpp \
            MyMainWindow.cpp \
            MyGLWidget.cpp \
            CSphericalCamera.cpp \
            CTrackball.cpp \
            SimpleSpring.cpp \
    Integrators.cpp

HEADERS  += MyMainWindow.h \
            MyGLWidget.h \
            CSphericalCamera.h \
            CTrackball.h \
            SimpleSpring.h \
    Integrators.h

RESOURCES   += Integrator.qrc
            
# If we're not building on Mac OS X, we need to use GLEW to access OpenGL.
macx {
    LIBS    += -framework OpenGL
} else {
    SOURCES += GL/glew.c
    DEFINES += GLEW_STATIC
    LIBS    += -lGLU
}
            
QT       += opengl
