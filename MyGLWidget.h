// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Custom subclass of a QGLWidget to hold and render our beautiful scene.
//
// Author:  Sonny Chan
// Date:    June 2009
// Updated: February 2012
// --------------------------------------------------------------------------

#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

// use GLEW if we're not on Mac OS X
#if defined(__APPLE__)
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include "GL/glew.h"
#endif

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QPropertyAnimation>
#include <QStatusBar>

#include "CSphericalCamera.h"
#include "CTrackball.h"
#include "SimpleSpring.h"

// --------------------------------------------------------------------------

class MyGLWidget : public QGLWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF juliaCoord READ juliaCoord WRITE setJuliaCoord)

    int                 m_viewportWidth, m_viewportHeight;
    double              m_timeElapsed, m_timeDelta;
    double              m_fpsEstimate;

    cSphericalCamera    m_camera;
    cTrackball          m_trackball;
    bool                m_flying, m_tracking, m_metaKey;

    // our spring systems
    static const int    k_springCount = 4;
    SimpleSpring        m_springs[k_springCount];
    bool                m_integrating;


    Eigen::Vector2f     m_juliaX, m_juliaY;
    QPointF             m_juliaCoord;
    QPropertyAnimation  m_animation;

    QGLShaderProgram    m_shader;

    static const int    CMAPS = 4;
    GLuint              m_cubeMaps[CMAPS], m_cubeMap;
    QStringList         m_environmentNames, m_environmentFiles;

    QString             m_messageText;
    QStatusBar         *m_windowStatus;

public:
    MyGLWidget(const QGLFormat & format, QWidget *parent = 0, 
               const QGLWidget *shareWidget = 0, Qt::WindowFlags f = 0);

    void setWindowStatusBar(QStatusBar *statusBar)  { m_windowStatus = statusBar; }

    QStringList availableEnvironments()             { return m_environmentNames; }
    void setEnvironment(int index)                  { m_cubeMap = m_cubeMaps[index]; }

    QPointF juliaCoord() const                      { return m_juliaCoord; }
    void setJuliaCoord(const QPointF &c)            { m_juliaCoord = c; }

    void setSpringParameter(int index, double value);

public slots:
    // Gets called when the animation finishes, and restarts it with a new target.
    void restartAnimation();

    void resetSprings();
    void stepSprings();
    void setIntegrating(bool i)                     { m_integrating = i; }

protected:

    // Sets up the OpenGL rendering context, defines display lists, etc. 
    // Called once before the first time resizeGL() or paintGL() is called.
    virtual void initializeGL();

    // Renders the OpenGL scene. Called whenever the widget needs to be updated.
    virtual void paintGL();

    // Sets up the OpenGL viewport, projection, etc. Gets called whenever the 
    // widget has been resized
    virtual void resizeGL(int width, int height);

    // Called with the QTimerEvent event parameter class when a timer event occurs.
    virtual void timerEvent(QTimerEvent *event);

    // Capture mouse events with this widget.
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);


    void drawSpringSystem(const SimpleSpring &spring,
                          const Eigen::Vector3f &colour = Eigen::Vector3f(0,0,0));

    void drawSkyBox();

    // Draws a grey coloured grid on the XZ plane
    void drawGrid(float size);

    // Draws a torus on the Z axis, with UV texture coordinates
    void drawTorus(float major, float minor);

    void drawCube(float scale = 1.f);

    void drawSphere(float radius = 1.f);

    bool loadCubeMap(const QString &name, GLuint textureID);

    QPointF randomVector2(const Eigen::Vector2f &xrange,
                          const Eigen::Vector2f &yrange);
};

// --------------------------------------------------------------------------
#endif
