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

#ifndef CSPHERICALCAMERA_H
#define CSPHERICALCAMERA_H

#include "Eigen/Core"

class cSphericalCamera
{
    float               m_radius;
    Eigen::Vector3f     m_origin;
    Eigen::Matrix4f     m_orientation;

    float       m_azimuth, m_altitude, m_altitudeLimit;
    float       m_minRadius, m_maxRadius;
    int         m_lastX, m_lastY;

    // sets the class orientation matrix from azimuth/altitude
    void        buildOrientationMatrix();

public:
    cSphericalCamera(float radius = 2.f);
    ~cSphericalCamera();

    // returns a 3x3 version of the camera orientation matrix
    Eigen::Matrix3f orientation3x3();

    // set the near/far range for the spherical camera
    void        setRange(float minr, float maxr)
                    { m_minRadius = minr; m_maxRadius = maxr; }

    // issues OpenGL transforms to place the camera (call after glIdentity)
    void        poseCamera();

    // load the camera's orientation into the texture matrix (for env-mapping)
    void        orientTexture();
    void        resetTexture();

    // call these functions when mouse input is captured
    void        mouseDown(int x, int y);
    void        mouseUp(int x, int y);
    void        mouseMove(int x, int y);
    void        mouseScroll(int delta);
};

// --------------------------------------------------------------------------
#endif
