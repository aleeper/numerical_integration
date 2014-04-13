// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Spherical camera class allows for controlling a "spherical" camera using
// the mouse.  This camera always looks at the origin, and its position on a
// sphere can be controlling by moving the mouse.  The sphere's radius is
// adjusted using the mouse wheel.
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

#include "CSphericalCamera.h"
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

cSphericalCamera::cSphericalCamera(float radius)
: m_radius(radius), m_origin(0,0,0), m_azimuth(0), m_altitude(15)
{
    m_minRadius = 0.1f;
    m_maxRadius = 10.f;
    m_altitudeLimit = 75.f;
    m_lastX = m_lastY = 0;

    buildOrientationMatrix();
}

cSphericalCamera::~cSphericalCamera()
{
}

// --------------------------------------------------------------------------

void cSphericalCamera::buildOrientationMatrix()
{
    m_orientation.setIdentity();
    Matrix3f rotation;
    rotation = AngleAxisf( M_PI * m_altitude / 180.f, Vector3f::UnitX())
             * AngleAxisf( M_PI * m_azimuth / 180.f, Vector3f::UnitY());
    m_orientation.block<3,3>(0,0) = rotation;
}

// --------------------------------------------------------------------------

Matrix3f cSphericalCamera::orientation3x3()
{
    return m_orientation.block<3,3>(0,0);
}

// --------------------------------------------------------------------------

void cSphericalCamera::mouseDown(int x, int y)
{
    m_lastX = x;
    m_lastY = y;
}

void cSphericalCamera::mouseUp(int x, int y)
{
    m_lastX = x;
    m_lastY = y;
}

void cSphericalCamera::mouseMove(int x, int y)
{
    int dx = x - m_lastX;
    int dy = y - m_lastY;

    // update azimuth (longitude) based on x movement
    m_azimuth += 0.2f * dx;
    m_azimuth = fmodf(m_azimuth+360.0f, 360.0f);

    // update altitude (latitude) based on y movement
    m_altitude += 0.2f * dy;
    m_altitude = min(max(-m_altitudeLimit, m_altitude), m_altitudeLimit);

    // build the camera orientation matrix
    buildOrientationMatrix();

    m_lastX = x;
    m_lastY = y;
}

void cSphericalCamera::mouseScroll(int delta)
{
    m_radius *= expf(0.001f * -delta);
    m_radius = min(max(m_minRadius, m_radius), m_maxRadius);
}

// --------------------------------------------------------------------------

void cSphericalCamera::poseCamera()
{
    // this transform assumes right-handed, OpenGL coordinate system
    // modifications should be made if other coordinate system is used
    glTranslatef(m_origin[0], m_origin[1], m_origin[2]);
    glTranslatef(0, 0, -m_radius);
    glMultMatrixf(m_orientation.data());
}

// --------------------------------------------------------------------------

void cSphericalCamera::orientTexture()
{
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    Matrix4f inverse = m_orientation.transpose();
    glMultMatrixf(inverse.data());
//    glMultMatrixf(transpose(m_orientation).data());
    glPopAttrib();
}

void cSphericalCamera::resetTexture()
{
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glPopAttrib();
}
// --------------------------------------------------------------------------
