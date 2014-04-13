// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

#include <QApplication>
#include "MyMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    // initialize resources -- remember to change "Boilerplate" below!
    Q_INIT_RESOURCE(Integrator);

    MyMainWindow window;
    window.show();

    return application.exec();
}

// --------------------------------------------------------------------------
