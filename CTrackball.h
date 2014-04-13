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

#ifndef CTRACKBALL_H
#define CTRACKBALL_H

#include "Eigen/Core";

class cTrackball
{
    Eigen::Vector2f m_center;           // viewport centre of trackball
    float           m_radius;           // viewport radius of trackball
    int             m_height;           // viewport height
    bool            m_invertY;          // whetherto invert Y screen coord
    bool            m_rolling;          // whether the trackball is rolling
    
    Eigen::Vector3f m_startVector;      // vector saved on mouse down
    Eigen::Matrix4f m_startMatrix;      // matrix saved on mouse down

    Eigen::Matrix4f m_viewMatrix;       // current orientation
    Eigen::Matrix3f m_inverseCamera;

    // Converts a 2D viewport coordinate into a 3D unit vector representing
    // the same point as if it were on the surface of the trackball.
    Eigen::Vector3f ballVector(Eigen::Vector2f screen) const;

    // Adjusts the orientation (view matrix) given an initial and a final
    // vector on the surface of the trackball.
    void            rotateView(Eigen::Vector3f start, Eigen::Vector3f finish);

public:
    cTrackball();
    ~cTrackball();

    void            setInvertY(bool invert) { m_invertY = invert; }
    void            resetView()             { m_viewMatrix.setIdentity(); }
    void            resize(int width = -1, int height = -1);
    void            applyTransform();

    void            setCameraOrientation(Eigen::Matrix3f camera)
                        { m_inverseCamera = camera.transpose(); }

    // call these functions when mouse input is captured
    void            mouseDown(int x, int y);
    void            mouseUp(int x, int y);
    void            mouseMove(int x, int y);
};

// --------------------------------------------------------------------------
#endif
