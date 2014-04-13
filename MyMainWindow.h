// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Main window consisting of an OpenGL widget, menu bar, and status bar.
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QPushButton>
#include <QDoubleSpinBox>
#include "MyGLWidget.h"

class MyMainWindow : public QMainWindow
{
    Q_OBJECT

    MyGLWidget      *m_openGLView;
    QMenuBar        *m_menuBar;
    QStatusBar      *m_statusBar;
    QSignalMapper   *m_signalMapper;

    QPushButton     *m_startButton;
    QList<QDoubleSpinBox *> m_spinners;

public:
    MyMainWindow();

public slots:
    void parameterChanged(double newValue);

protected:
    void createMenus();
    QWidget *createControlPanel();

private slots:
    void setEnvironment(int index)  { m_openGLView->setEnvironment(index); }
};

// --------------------------------------------------------------------------
#endif
