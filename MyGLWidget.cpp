// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Custom subclass of a QGLWidget to hold and render our beautiful scene.
//
// Author:  Sonny Chan
// Date:    June 2009
// Updated: February 2012
// --------------------------------------------------------------------------

#include "MyGLWidget.h"
#include <QtGui>
#include <complex>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace Eigen;

// --------------------------------------------------------------------------

MyGLWidget::MyGLWidget(const QGLFormat &format, QWidget *parent, 
                       const QGLWidget *shareWidget, Qt::WindowFlags f)
    : QGLWidget(format, parent, shareWidget, f)
{
    m_timeElapsed = m_timeDelta = 0.0;
    m_fpsEstimate = 0.0;

    m_flying = m_tracking = m_metaKey = false;
    m_windowStatus = 0;

    m_integrating = false;

    m_juliaX = Vector2f(-1.5f, .5f);
    m_juliaY = Vector2f(-1.f, 1.f);

    // populate the environment names (for loading & menu selection)
    m_environmentNames.append("St. Peter's Cathedral");
    m_environmentFiles.append(":/resources/stpeters_cross.png");
    m_environmentNames.append("Grace Cathedral");
    m_environmentFiles.append(":/resources/grace_cross.png");
    m_environmentNames.append("Eucalyptus Grove");
    m_environmentFiles.append(":/resources/rnl_cross.png");
    m_environmentNames.append("Uffizi Gallery");
    m_environmentFiles.append(":/resources/uffizi_cross.png");

    setMinimumSize(800, 600);
}

// --------------------------------------------------------------------------

void MyGLWidget::initializeGL()
{
    // initialize GLEW if we're using it
    #if defined(GLEW_VERSION)
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            QMessageBox::critical(this, tr("Initialization Error"), 
                                        tr("Error initializing GLEW!"));
        }
    #endif


    // generate environment map textures and load from files
    glGenTextures(CMAPS, m_cubeMaps);
    for (int i = 0; i < CMAPS; ++i)
        loadCubeMap(m_environmentFiles.value(i), m_cubeMaps[i]);
    m_cubeMap = m_cubeMaps[3];

    // load vertex and fragment programs to attach to the shader
    m_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/shader.vert");
    m_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/shader.frag");

    // set range for camera
    m_camera.setRange(0.4f, 10.f);

    // set up animation of julia fractal coordinates
    m_animation.setTargetObject(this);
    m_animation.setPropertyName("juliaCoord");
    m_animation.setDuration(3000);
    m_animation.setStartValue(randomVector2(m_juliaX, m_juliaY));
    m_animation.setEndValue(randomVector2(m_juliaX, m_juliaY));
    m_animation.setEasingCurve(QEasingCurve::InOutQuad);
    connect(&m_animation, SIGNAL(finished()), this, SLOT(restartAnimation()));
    m_animation.start();

    // set parameters for spring simulations
    for (int i = 0; i < k_springCount; ++i) {
        m_springs[i].setStiffness(200.0);
        m_springs[i].setDamping(1.0);
        m_springs[i].setInitialPosition(.25);
    }
    double dt = 0.005;
    m_springs[0].setIntegrator(new ExplicitEulerIntegrator<SimpleSpring::StateType>(&m_springs[0], dt));
    m_springs[1].setIntegrator(new ModifiedMidpointIntegrator<SimpleSpring::StateType>(&m_springs[1], dt));
    m_springs[2].setIntegrator(new RungeKutta4Integrator<SimpleSpring::StateType>(&m_springs[2], dt));
    m_springs[3].setIntegrator(new ImplicitEulerIntegrator<SimpleSpring::StateType,
                               SimpleSpring::MatrixType>(&m_springs[3], dt));
    resetSprings();

    // start a timer with 15ms period (roughly 60 fps)
    startTimer(15);
}

// --------------------------------------------------------------------------

void MyGLWidget::paintGL()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // first draw the sky box
    drawSkyBox();

    // move the spherical camera to where it should be
    m_camera.poseCamera();

    glEnable(GL_DEPTH_TEST);
/*
    // draw our scene object (int this case, the torus)
    m_camera.orientTexture();
    glPushMatrix();
        m_trackball.setCameraOrientation(m_camera.orientation3x3());
        m_trackball.applyTransform();
        m_shader.bind();
        m_shader.setUniformValue("c", m_juliaCoord);
        m_shader.setUniformValue("environment", 0);
        m_shader.bindAttributeLocation("tangent", 1);
        drawTorus(.5f, .25f);
        m_shader.release();
    glPopMatrix();
    m_camera.resetTexture();
*/

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT)
            ;
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);
    m_camera.orientTexture();

    // set up the combiner to apply 30% reflectivity
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
    const GLfloat reflectivity[] = { 0.f, 0.f, 0.f, .3f };
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reflectivity);

    glPushMatrix();
        m_trackball.setCameraOrientation(m_camera.orientation3x3());
        m_trackball.applyTransform();

        Vector3f colours[] = {
            Vector3f( .8f, .2f, .2f ),
            Vector3f( .2f, .8f, .2f ),
            Vector3f( .2f, .2f, .8f ),
            Vector3f( .7f, .7f, .2f )
        };
        glTranslatef(-0.9f, 0.f, 0.f);
        for (int i = 0; i < 4; ++i) {
            drawSpringSystem(m_springs[i], colours[i]);
            glTranslatef(.6f, 0.f, 0.f);
        }
    glPopMatrix();

    m_camera.resetTexture();
    glDisable(GL_TEXTURE_CUBE_MAP);
    glPopAttrib();

    // draw a 4 by 4 grid on the XZ plane for orientation
    glTranslatef(0.f, -.75f, 0.f);
    drawGrid(4.f);

    // render a text message into the viewport
    glColor4f(1.f, 1.f, 1.f, .6f);
    if (!m_messageText.isEmpty())
        renderText(8, 16, m_messageText);
}

// --------------------------------------------------------------------------

void MyGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, GLsizei(width), GLsizei(height));
    m_trackball.resize();
    m_trackball.setInvertY(true);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLdouble aspect = GLdouble(width) / GLdouble(height);
    gluPerspective(60.0, aspect, 0.1, 100.0);
}

// --------------------------------------------------------------------------

void MyGLWidget::timerEvent(QTimerEvent *event)
{
    static int fdelta = 0, frames = 0;
    static QTime last = QTime::currentTime();


    QTime current = QTime::currentTime();
    int delta_ms = last.msecsTo(current);
    last = current;

    // update the simulations
    if (m_integrating) {
        double delta_t = min(double(delta_ms) / 1000.0, 0.1);
        for (int i = 0; i < k_springCount; ++i)
            m_springs[i].update(delta_t);
    }

    // call updateGL to redraw the frame
    updateGL();

    // count an estimate of the current FPS, updating every second
    ++frames;
    fdelta += delta_ms;
    if (fdelta >= 1000) {
        m_fpsEstimate = frames;
        fdelta -= 1000;
        frames = 0;
        if (m_windowStatus)
            m_windowStatus->showMessage(QString("FPS: %1").arg(m_fpsEstimate));
    }
}

// --------------------------------------------------------------------------

void MyGLWidget::restartAnimation()
{
    // restart the animation with the endpoint as the new start point
    m_animation.setStartValue(m_animation.endValue());
    m_animation.setEndValue(randomVector2(m_juliaX, m_juliaY));
    m_animation.start();
}

void MyGLWidget::resetSprings()
{
    for (int i = 0; i < k_springCount; ++i)
        m_springs[i].reset();
}

void MyGLWidget::stepSprings()
{
    for (int i = 0; i < k_springCount; ++i)
        m_springs[i].update();
}

void MyGLWidget::setSpringParameter(int index, double value)
{
    for (int i = 0; i < k_springCount; ++i)
    {
        switch (index) {
        case 0: m_springs[i].setMass(value);        break;
        case 1: m_springs[i].setStiffness(value);   break;
        case 2: m_springs[i].setDamping(value);     break;
        case 3: m_springs[i].setGravity(value);     break;
        case 4: m_springs[i].setTimeStep(value);    break;
        default:                                    break;
        }
    }
}

// --------------------------------------------------------------------------

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->x(), y = event->y();

    // see which state we are in and pass to appropriate controller
    if (m_tracking) m_trackball.mouseMove(x, y);
    if (m_flying)   m_camera.mouseMove(x, y);

    QString stat = QString(": (%1, %2)").arg(event->x()).arg(event->y());
    m_messageText = tr("Mouse Moved") + stat;

    event->accept();
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    int x = event->x(), y = event->y();

    // condition on the button that caused this event
    if (event->modifiers() & Qt::ControlModifier)           m_metaKey = true;
    if (event->button() == Qt::RightButton || m_metaKey)    m_tracking = true;
    else if (event->button() == Qt::LeftButton)             m_flying = true;

    // pass the event coordinates to the camera or trackball
    if (m_tracking) m_trackball.mouseDown(x, y);
    if (m_flying)   m_camera.mouseDown(x, y);

    QString stat = QString(": B%1 (%2, %3) %4")
        .arg(event->button()).arg(event->x()).arg(event->y());
    m_messageText = tr("Mouse Pressed") + stat;

    event->accept();
}

void MyGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int x = event->x(), y = event->y();

    // condition on the button that caused this event
    if (m_tracking && (event->button() == Qt::RightButton || m_metaKey)) {
        m_trackball.mouseUp(x, y);
        m_tracking = m_metaKey = false;
    }
    else if (m_flying && event->button() == Qt::LeftButton) {
        m_camera.mouseUp(x, y);
        m_flying = false;
    }

    QString stat = QString(": B%1 (%2, %3)")
        .arg(event->button()).arg(event->x()).arg(event->y());
    m_messageText = tr("Mouse Released") + stat;

    event->accept();
}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
    // use mouse wheel event to zoom the camera
    m_camera.mouseScroll(event->delta());

    QString stat = QString(": %1").arg(event->delta());
    m_messageText = tr("Mouse Scrolled") + stat;

    event->accept();
}

// --------------------------------------------------------------------------

void MyGLWidget::drawSpringSystem(const SimpleSpring &spring, const Vector3f &colour)
{
    double y = spring.currentState()[1];

    // draw torii for springs
    double a = y + 0.2;
    double dy = (1.0 - a) / 8.0;
    glPushMatrix();
    glColor3f(.7f, .7f, .7f);
    glRotatef(90.f, 1.f, 0.f, 0.f);
    glTranslated(0.0, 0.0, -a);
    for (int i = 0; i < 8; ++i) {
        glTranslated(0.0, 0.0, -dy);
        drawTorus(.1f, .02f);
    }
    glPopMatrix();

    // draw ball for mass
    glPushMatrix();
    glColor3fv(colour.data());
    glTranslated(0.0, y, 0.0);
    drawSphere(.25f);
    glPopMatrix();
}

// --------------------------------------------------------------------------

void MyGLWidget::drawSkyBox()
{
    glPushAttrib(GL_ENABLE_BIT);
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_CUBE_MAP);
    
    glDisable(GL_DEPTH_TEST);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);

    m_camera.orientTexture();
    drawCube();
    m_camera.resetTexture();

    glDisable(GL_TEXTURE_CUBE_MAP);
    glPopAttrib();
}

// --------------------------------------------------------------------------

void MyGLWidget::drawGrid(float size)
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3f(.75f, .75f, .75f);
    float gspan = 0.5f * size;
    glBegin(GL_LINES);
    for (float f = -gspan; f < 1.05f * gspan; f += 0.1f * gspan) {
        glVertex3f(f, 0.f, -gspan); glVertex3f(f, 0.f, gspan);
        glVertex3f(-gspan, 0.f, f); glVertex3f(gspan, 0.f, f);
    }
    glEnd();
    glPopAttrib();
}

// --------------------------------------------------------------------------

void MyGLWidget::drawTorus(float major, float minor)
{
    static GLuint displayList = 0;
    const int segMajor = 48, segMinor = 24;
    const float pi = log(complex<float>(-1)).imag();

    // if we haven't cached a display list for this object, create it
    if (displayList == 0)
    {
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);
        for (int i = 0; i < segMajor; ++i) {
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= segMinor; ++j) {
                for (int k = 0; k < 2; ++k) {
                    float u = float(i+k) / segMajor;
                    float v = float(j) / segMinor;
                    glMultiTexCoord2f(GL_TEXTURE0, u, v);
                    glMultiTexCoord2f(GL_TEXTURE1, u, v);
                    float theta = 2.f * pi * u;
                    float phi = 2.f * pi * v;
                    float r = -cosf(phi);
                    float nx = r * cosf(theta);
                    float ny = r * sinf(theta);
                    float nz = sinf(phi);
                    glNormal3f(nx, ny, nz);
                    float tx = -sinf(theta);
                    float ty = cosf(theta);
                    glVertexAttrib3f(1, tx, ty, 0.f);
                    float x = (major + minor * r) * cosf(theta);
                    float y = (major + minor * r) * sinf(theta);
                    float z = minor * nz;
                    glVertex3f(x, y, z);
                }
            }
            glEnd();
        }
        glEndList();
    }

    glCallList(displayList);
}

// --------------------------------------------------------------------------

void MyGLWidget::drawCube(float scale)
{
    static GLuint displayList = 0;

    // unit cube from Jim Blinn's Corner, Platonic Solids
    // IEEE Computer Graphics & Applications, 1987:7(11)
    GLfloat vc[8][3] = { 
        { 1.f, 1.f, 1.f }, { 1.f, 1.f,-1.f }, { 1.f,-1.f, 1.f }, { 1.f,-1.f,-1.f },
        {-1.f, 1.f, 1.f }, {-1.f, 1.f,-1.f }, {-1.f,-1.f, 1.f }, {-1.f,-1.f,-1.f }
    };
    GLfloat tc[8][3] = {
        { 1.f, 1.f, 1.f }, { 1.f, 1.f, 0.f }, { 1.f, 0.f, 1.f }, { 1.f, 0.f, 0.f },
        { 0.f, 1.f, 1.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f }
    };
    int faces[6][4] = {
        { 2, 1, 3, 4 }, { 5, 6, 8, 7 }, { 1, 2, 6, 5 },
        { 4, 3, 7, 8 }, { 3, 1, 5, 7 }, { 2, 4, 8, 6 }
    };

    // scale the vertex coordinates by the given parameter
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 3; ++j)
            vc[i][j] *= scale;

    // if we haven't cached a display list for this object, create it
    if (displayList == 0)
    {
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);
        glBegin(GL_QUADS);
            for (int i = 0; i < 6; ++i)
                for (int j = 0; j < 4; ++j) {
                    glTexCoord3fv(vc[faces[i][j]-1]);
                    glNormal3fv(vc[faces[i][j]-1]);
                    glVertex3fv(vc[faces[i][j]-1]);
                }
        glEnd();
        glEndList();
    }

    glCallList(displayList);    
}

// --------------------------------------------------------------------------

void MyGLWidget::drawSphere(float radius)
{
    static GLUquadric *quadric = 0;

    if (quadric == 0) {
        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
    }

    gluSphere(quadric, radius, 24, 24);
}

// --------------------------------------------------------------------------

bool MyGLWidget::loadCubeMap(const QString &name, GLuint textureID)
{
    // assumes a single cube map image with a cross format, as is on the
    // images on Paul Debevec's web site: http://www.debevec.org/Probes/
    QImage image(name);
    
    // assert that the image is 3 cells across and 4 cells down
    if (image.width() * 4 != image.height() * 3) {
        QMessageBox::critical(this, tr("Image Load Error"), 
                              QString("Could not load cube map from %1").arg(name));
        return false;
    }
    int size = image.width() / 3;

    GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    int coordinates[][2] = { {2,1}, {0,1}, {1,0}, {1,2}, {1,3}, {1,1} };
    bool mirrorh[] = { true,  true,  false, false, false, true };
    bool mirrorv[] = { false, false, true,  true,  true,  false };

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (int i = 0; i < 6; ++i) {
        QImage face = image.copy(coordinates[i][0]*size, coordinates[i][1]*size,
            size, size).mirrored(mirrorh[i], mirrorv[i]);
        glTexImage2D(targets[i], 0, GL_RGBA, size, size, 0,
            GL_BGRA, GL_UNSIGNED_BYTE, face.bits());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return true;
}

// --------------------------------------------------------------------------

QPointF MyGLWidget::randomVector2(const Vector2f &xrange, const Vector2f &yrange)
{
    static bool seeded = false;
    
    if (!seeded) {
        srand(time(0));
        seeded = true;
    }

    float x = (rand() % 10000) * (xrange[1]-xrange[0]) / 10000.f + xrange[0];
    float y = (rand() % 10000) * (yrange[1]-yrange[0]) / 10000.f + yrange[0];
    return QPointF(x, y*.1f);
}

// --------------------------------------------------------------------------
