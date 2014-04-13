// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Main window consisting of an OpenGL widget, menu bar, and status bar.
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

#include "MyMainWindow.h"
#include <QtGui>

// --------------------------------------------------------------------------

MyMainWindow::MyMainWindow()
    : QMainWindow()
{
    setUnifiedTitleAndToolBarOnMac(true);

    // The default QGLFormat has the following properties:
    //  - Double buffer: Enabled.
    //  - Depth buffer: Enabled.
    //  - RGBA: Enabled (i.e., color index disabled).
    //  - Alpha channel: Disabled.
    //  - Accumulator buffer: Disabled.
    //  - Stencil buffer: Disabled.
    //  - Stereo: Disabled.
    //  - Direct rendering: Enabled.
    //  - Overlay: Disabled.
    //  - Plane: 0 (i.e., normal plane).
    //  - Multisample buffers: Disabled.
    // These can be changed here in the format variable if necessary.

    QGLFormat format;

    m_openGLView = new MyGLWidget(format);

    // create a menu for the window
    m_menuBar = new QMenuBar();
    this->setMenuBar(m_menuBar);
    m_signalMapper = new QSignalMapper(this);
    createMenus();

    // create a status bar for the window
    m_statusBar = new QStatusBar();
    this->setStatusBar(m_statusBar);

    // create a control panel widget
    QWidget *controlPanel = createControlPanel();

    // create a central widget to contain all this stuff
    QWidget *widget = new QWidget;
    QLayout *layout = new QHBoxLayout(widget);
    layout->setMargin(0);
    layout->addWidget(controlPanel);
    layout->addWidget(m_openGLView);

    this->setCentralWidget(widget);

    m_openGLView->setWindowStatusBar(m_statusBar);
}

// --------------------------------------------------------------------------

QDoubleSpinBox *createParameterSpinner(QFormLayout *form, QString name,
                                       double initial, double minval, double maxval,
                                       int decimals = 2, QString suffix = "")
{
    QDoubleSpinBox *spinner = new QDoubleSpinBox;
    spinner->setRange(minval, maxval);
    spinner->setSingleStep(maxval * 0.01);
    spinner->setDecimals(decimals);
    spinner->setValue(initial);
    spinner->setSuffix(suffix);
    form->addRow(name, spinner);
    return spinner;
}

QWidget *MyMainWindow::createControlPanel()
{
    // start, stop and reset buttons
    m_startButton = new QPushButton("Go!");
    m_startButton->setCheckable(true);
    connect(m_startButton, SIGNAL(toggled(bool)), m_openGLView, SLOT(setIntegrating(bool)));

    QPushButton *stepButton = new QPushButton("Step");
    connect(stepButton, SIGNAL(clicked()), m_openGLView, SLOT(stepSprings()));

    QPushButton *resetButton = new QPushButton("Reset");
    connect(resetButton, SIGNAL(clicked()), m_openGLView, SLOT(resetSprings()));

    QLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(stepButton);
    buttonLayout->addWidget(resetButton);

    // the parameters group box
    QGroupBox *parametersBox = new QGroupBox("Parameters");
    QFormLayout *parametersLayout = new QFormLayout(parametersBox);

    m_spinners.append(createParameterSpinner(parametersLayout, "Mass", 1.0, 0.1, 10.0, 1, " kg"));
    m_spinners.append(createParameterSpinner(parametersLayout, "Stiffness", 100.0, 10.0, 5000.0, 0, " N/m"));
    m_spinners.append(createParameterSpinner(parametersLayout, "Damping", 1.0, 0.0, 100.0, 1, " Ns/m"));
    m_spinners.append(createParameterSpinner(parametersLayout, "Gravity", 9.81, 0.0, 100.0, 2, " m/s2"));
    m_spinners.append(createParameterSpinner(parametersLayout, "Timestep", 0.005, 0.0001, 0.1, 4, " s"));

    foreach (QDoubleSpinBox *spinner, m_spinners) {
        connect(spinner, SIGNAL(valueChanged(double)),
                this, SLOT(parameterChanged(double)));
    }

    // create and return the widget
    QWidget *widget = new QWidget;
    QBoxLayout *widgetLayout = new QVBoxLayout(widget);
    widgetLayout->addWidget(parametersBox);
    widgetLayout->addStretch(2);
    widgetLayout->addLayout(buttonLayout);

    return widget;
}

void MyMainWindow::parameterChanged(double newValue)
{
    int i = m_spinners.indexOf(dynamic_cast<QDoubleSpinBox *>(sender()));
    if (i >= 0) m_openGLView->setSpringParameter(i, newValue);
}

// --------------------------------------------------------------------------

void MyMainWindow::createMenus()
{
    // file menu
    QMenu *fileMenu = m_menuBar->addMenu(tr("&File"));
    QAction *actionExit = fileMenu->addAction(tr("E&xit"));
    connect(actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    // environment menu
    QMenu *environmentMenu = m_menuBar->addMenu(tr("&Environment"));
    QActionGroup *actionGroup = new QActionGroup(this);
    QStringList envs = m_openGLView->availableEnvironments();
    for (int i = 0; i < envs.size(); ++i) {
        QAction *action = actionGroup->addAction(envs[i]);
        action->setCheckable(true);
        if (i == 3) action->setChecked(true);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(action, i);
    }
    environmentMenu->addActions(actionGroup->actions());
    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(setEnvironment(int)));

}

// --------------------------------------------------------------------------
