// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Trackball class allows for rotating an object using the mouse via a
// virtual trackball.  It functions as if the trackball were a hemisphere
// protruding from the centre of the the screen.
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

#include "CTrackball.h"
#include "Eigen/Geometry"
#include <algorithm>
#include <cmath>

// use GLEW if we're not on Mac OS X
#if defined(__APPLE__)
#include <OpenGL/GL.h>
#else
#include "GL/glew.h"
#endif

using namespace std;
using namespace Eigen;

// --------------------------------------------------------------------------

cTrackball::cTrackball()
    : m_rolling(false), m_invertY(false), m_radius(1), m_height(1)
{
    m_center = Vector2f::Zero();
    m_startVector = Vector3f::UnitZ();
    m_startMatrix.setIdentity();
    m_viewMatrix.setIdentity();
    m_inverseCamera.setIdentity();
}

cTrackball::~cTrackball()
{
}

// --------------------------------------------------------------------------

Vector3f cTrackball::ballVector(Vector2f screen) const
{
    // normalize and centre the screen coordinates first
    screen -= m_center;
    screen /= m_radius;
    
    float lsqared = screen.squaredNorm();
    Vector3f ball;
    
    // if we are grabbing outside the bounds of the virtual hemisphere, 
    // take a point on the edge
    if (lsqared > 1.0)
    {
        screen.normalize();
        ball = Vector3f(screen[0], screen[1], 0.0f);
    }
    // otherwise we are on the protruding hemisphere
    else
    {
        float z = sqrtf(1.0f - lsqared);
        ball = Vector3f(screen[0], screen[1], z);
    }

    // return the ball vector, taking into account the camera's orientation
    return m_inverseCamera * ball;
}

// --------------------------------------------------------------------------

void cTrackball::rotateView(Vector3f start, Vector3f finish)
{
    // assume that start and finish are unit vectors
    Vector3f axis = start.cross(finish);
    if (axis.squaredNorm() > 0.f) {
        axis.normalize();
        float angle = acosf(start.dot(finish));
        Transform<float, 3, Affine, 0> t(AngleAxisf(angle, axis));
        m_viewMatrix = t * m_viewMatrix;
    }
}

// --------------------------------------------------------------------------

void cTrackball::resize(int width, int height)
{
    int x = 0, y = 0;

    // if viewport was not explicitly given, read from OpenGL state
    if (width < 0 || height < 0) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        x = viewport[0];    width  = viewport[2];
        y = viewport[1];    height = viewport[3];
    }

    // half the viewport width and height
    float hw = 0.5f * width, hh = 0.5f * height;

    m_center = Vector2f(x+hw, y+hh);
    m_radius = min(hw, hh);
    m_height = height;
}

// --------------------------------------------------------------------------

void cTrackball::applyTransform()
{
    glMultMatrixf(m_viewMatrix.data());
}

// --------------------------------------------------------------------------

void cTrackball::mouseDown(int x, int y)
{
    if (m_invertY) y = m_height - y;
    m_startVector = ballVector(Vector2f(x, y));
    m_startMatrix = m_viewMatrix;
    m_rolling = true;
}

void cTrackball::mouseUp(int x, int y)
{
    mouseMove(x, y);
    m_rolling = false;
}

void cTrackball::mouseMove(int x, int y)
{
    if (m_invertY) y = m_height - y;
    if (m_rolling) {
        Vector3f finish = ballVector(Vector2f(x, y));
        m_viewMatrix = m_startMatrix;
        rotateView(m_startVector, finish);
    }
}

// --------------------------------------------------------------------------
