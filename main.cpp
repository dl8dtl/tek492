/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <dl8dtl@s09.sax.de> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/*
 * As this application uses Qt, any redistribution is bound to the
 * terms of either the GNU Lesser General Public License v. 3 ("LGPL"),
 * or to any commercial Qt license.
 */

/*
 * This application uses the Qwt extensions to Qt, which are also
 * provided under the terms of LGPL v2.1.  However, the Qwt license
 * explicitly disclaims many common use cases as not constituting a
 * "derived work" (in the LGPL sense), so it is generally less
 * demanding about the redistribution policy than Qt itself.
 */

/* $Id$ */

#include "tek492.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Tek492 w;
    w.setStyleSheet("QRadioButton::indicator {\n"
                    "   width: 14px;\n"
                    "   height: 14px;\n"
                    "}\n"
                    "QRadioButton::indicator::unchecked {\n"
                    "   image: url(:/resource/images/button-inactive);\n"
                    "}\n"
                    "QRadioButton::indicator::unchecked:hover {\n"
                    "   image: url(:/resource/images/button-inactive);\n"
                    "}\n"
                    "QRadioButton::indicator::unchecked:pressed {\n"
                    "   image: url(:/resource/images/button-pressed);\n"
                    "}\n"
                    "QRadioButton::indicator::checked {\n"
                    "   image: url(:/resource/images/button-active);\n"
                    "}\n"
                    "QRadioButton::indicator::checked:hover {\n"
                    "   image: url(:/resource/images/button-active);\n"
                    "}\n"
                    "QRadioButton::indicator::checked:pressed {\n"
                    "   image: url(:/resource/images/button-pressed);\n"
                    "}\n");
    w.show();

    return a.exec();
}
