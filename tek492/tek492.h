/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <dl8dtl@darc.de> wrote this file.  As long as you retain this notice you
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

#ifndef TEK492_H
#define TEK492_H

#include <QMainWindow>
#include <QVector>

#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_plot_textlabel.h>
#include <qwt_symbol.h>

#include "usb_492/usb492.hpp"

namespace Ui {
class Tek492;
}

class TekPlotCurve: public QwtPlotCurve
{
    QRectF d_boundingrect;

public:
    QRectF boundingRect() const
    {
        return d_boundingrect;
    }
    void setBoundingRect(qreal left, qreal top, qreal width, qreal height)
    {
        d_boundingrect = QRectF(left, top, width, height);
    }
};

class Tek492 : public QMainWindow
{
    Q_OBJECT

public slots:
    void acquire(void);
    void frequencyDial(int change);
    void spanWheelChanged(double change);
    void refLevelChanged(double change);
    void bandChanged(int index);
    void attenChanged(bool);
    void bwChanged(bool);
    void markerAchecked(bool ischecked);
    void markerAmoved(double val);
    void markerBchecked(bool ischecked);
    void markerBmoved(double val);
    void connectToDevice(void);
    void saveAs(void);
    void loadFile(void);
    void about(void);
    void info(void);
    void print(void);

public:
    explicit Tek492(QWidget *parent = 0);
    ~Tek492();

private:
    void plotMarkerLabel(QColor color);
    void writeFile(const QString fName, const QString comment);
    void readFile(const QString fName);
    void plotData(void);

    Ui::Tek492 *ui;
    USB492 *usb492;
    QwtPlotGrid *grid;
    QwtPlotMarker *markerA, *markerB;
    QwtSymbol *mASymbol, *mBSymbol;
    QwtText *mAText, *mBText;
    TekPlotCurve *curve;
    QVector<QPointF> *data;
    QwtText *titleText, *bLeftText, *bCenterText, *bRightText;
    QString *mAvalue, *mBvalue;
    QwtPlotTextLabel *title, *marker, *bLeft, *bCenter, *bRight;

    int spanIdx;
    int refLevel;
    double lowBound, highBound;
    uint8_t rawData[1024];
};

#endif // TEK492_H
