#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.setFixedSize(1200, 700);
    w.setWindowTitle("Control your Arduino UNO");
    w.setWindowFlags(Qt::Window); // Add buttons maximize and minimize
    QPixmap pix(":/image/IconArdu.png"); // +info: https://drslash.com/flat-osx-pack/
    w.setWindowIcon(QIcon(pix));
    w.show();

    return a.exec();
}

//Create executable WINDEPLOYQT.EXE + Info: https://www.youtube.com/watch?v=raueG9n6rC4


// BOARD IMAGE
// link: http://robotosh.blogspot.com.es/2012/07/arduino-jy-mcu-bluetooth.html

