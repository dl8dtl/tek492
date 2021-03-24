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

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QTime>
#include <QPrinter>
#include <QPrintDialog>
#include <QtXml/qdom.h>
#include <qwt_plot_renderer.h>

#include <stdio.h>

#include "tek492.h"
#include "ui_tek492.h"

#include "usb_492/usb492.hpp"

using namespace std;
static USB_Device *dev = 0;

static const QString spanNames[] =
{
    "FULL",
    "200 MHz", "100 MHz", "50 MHz", "20 MHz", "10 MHz",
    "5 MHz", "2 MHz", "1 MHz", "500 kHz", "200 kHz", "100 kHz",
    "50 kHz", "20 kHz", "10 kHz", "5 kHz", "2 kHz", "1 kHz",
    "500 Hz"
};

static const double spanValues[] =
{
    0, // Full
    200E6, 100E6, 50E6, 20E6, 10E6,
    5E6, 2E6, 1E6, 500E3, 200E3, 100E3,
    50E3, 20E3, 10E3, 5E3, 2E3, 1E3,
    500
};

static const int autoBW[] =
{
    1000000, // Full
    1000000, 1000000, 1000000, 1000000, 1000000,
    1000000, 100000, 100000, 100000, 10000, 10000,
    10000, 10000, 1000, 1000, 1000, 100,
    100
};

static const int autoAtten[] =
{
    0, // -120 dBm
    0, // -110 dBm
    0, // -100 dBm
    0, // -90 dBm
    0, // -80 dBm
    0, // -70 dBm
    0, // -60 dBm
    0, // -50 dBm
    0, // -40 dBm
    0, // -30 dBm
    10, // -20 dBm
    20, // -10 dBm
    30, // 0 dBm
    40, // +10 dBm
    50, // +20 dBm
    60, // +30 dBm
};

static USB492 *logger_open(void)
{
    vector<USB_Device *> devs;
    USB_Device::find_all(devs, 0x3141, 0x0005);

    if (devs.empty())
    {
        throw("USB492 device not found\n");
        return NULL;
    }

    int n = 0;
    dev = devs[n];
    if (!dev->open())
    {
        throw("Can't open\n");
        return NULL;
    }
    if (!dev->set_default())
    {
        throw("Can't set default config\n");
        return NULL;
    }

    USB492 *usb492 = new USB492(dev);

    return usb492;
}

Tek492::Tek492(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Tek492)
{
    ui->setupUi(this);
    QApplication::setWindowIcon(QIcon(":/resource/images/icon"));

    try
    {
        usb492 = logger_open();
        ui->actionConnect->setEnabled(true);
    }
    catch (const char *e)
    {
        QMessageBox::warning(this, "USB error", e);
        usb492 = 0;
    }

    ui->fBand->addItem("1: 100 kHz … 1.8 GHz", 1);
    ui->fBand->addItem("2: 1.7 … 5.5 GHz", 2);
    ui->fBand->addItem("3: 3.0 … 7.1 GHz", 3);
    ui->fBand->addItem("4: 5.4 … 18.0 GHz", 4);
    ui->fBand->addItem("5: 15.0 … 21.0 GHz", 5);

    // File menu
    connect(ui->actionConnect, SIGNAL(triggered()), SLOT(connectToDevice()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(saveAs()));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(loadFile()));
    connect(ui->actionPrint, SIGNAL(triggered()), SLOT(print()));

    // Help menu
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(about()));
    connect(ui->actionUsage, SIGNAL(triggered()), SLOT(info()));

    // Main widgeet
    connect(ui->acquire, SIGNAL(pressed()), SLOT(acquire()));
    connect(ui->frequencyDial, SIGNAL(valueChanged(int)), SLOT(frequencyDial(int)));
    connect(ui->spanWheel, SIGNAL(valueChanged(double)), SLOT(spanWheelChanged(double)));
    connect(ui->refLevelWheel, SIGNAL(valueChanged(double)), SLOT(refLevelChanged(double)));
    connect(ui->fBand, SIGNAL(currentIndexChanged(int)), SLOT(bandChanged(int)));
    connect(ui->atten0, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten10, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten20, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten30, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten40, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten50, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->atten60, SIGNAL(toggled(bool)), SLOT(attenChanged(bool)));
    connect(ui->rbw100, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->rbw1k, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->rbw10k, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->rbw100k, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->rbw1M, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->vfNone, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->vfWide, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->vfNarrow, SIGNAL(toggled(bool)), SLOT(bwChanged(bool)));
    connect(ui->mAEnabled, SIGNAL(toggled(bool)), SLOT(markerAchecked(bool)));
    connect(ui->mAWheel, SIGNAL(valueChanged(double)), SLOT(markerAmoved(double)));
    connect(ui->mBEnabled, SIGNAL(toggled(bool)), SLOT(markerBchecked(bool)));
    connect(ui->mBWheel, SIGNAL(valueChanged(double)), SLOT(markerBmoved(double)));

    grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(ui->plotArea);

    data = new QVector<QPointF>();
    for (int i = 0; i < 1015; i++)
    {
        data->append(QPointF(i, 1000 - i));
    }

    curve = new TekPlotCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setPen(QPen(Qt::green, 0.0, Qt::SolidLine));
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
    curve->setSamples(*data);
    curve->attach(ui->plotArea);

    titleText = new QwtText("Tek492");
    titleText->setRenderFlags(Qt::AlignLeft | Qt::AlignTop);
    titleText->setColor("yellow");

    bLeftText = new QwtText("BL");
    bLeftText->setRenderFlags(Qt::AlignLeft | Qt::AlignBottom);
    bLeftText->setColor("yellow");

    bCenterText = new QwtText("BC");
    bCenterText->setRenderFlags(Qt::AlignHCenter | Qt::AlignBottom);
    bCenterText->setColor("yellow");

    bRightText = new QwtText("BR");
    bRightText->setRenderFlags(Qt::AlignRight | Qt::AlignBottom);
    bRightText->setColor("yellow");

    QFont font;
    font.setBold(true);
    titleText->setFont(font);

    title = new QwtPlotTextLabel();
    title->setText(*titleText);
    title->attach(ui->plotArea);

    marker = new QwtPlotTextLabel();
    marker->attach(ui->plotArea);

    bLeft = new QwtPlotTextLabel();
    bLeft->setText(*bLeftText);
    bLeft->attach(ui->plotArea);

    bCenter = new QwtPlotTextLabel();
    bCenter->setText(*bCenterText);
    bCenter->attach(ui->plotArea);

    bRight = new QwtPlotTextLabel();
    bRight->setText(*bRightText);
    bRight->attach(ui->plotArea);

    mAvalue = new QString();
    QColor markerColor("yellow");
    mASymbol = new QwtSymbol(QwtSymbol::DTriangle, QBrush(), QPen(markerColor), QSize(10,10));
    markerA = new QwtPlotMarker();
    markerA->setSymbol(mASymbol);
    mAText = new QwtText("A");
    mAText->setColor("yellow");
    markerA->setLabelAlignment(Qt::AlignHCenter | Qt::AlignTop);
    markerA->setLabel(*mAText);

    mBvalue = new QString();
    mBSymbol = new QwtSymbol(QwtSymbol::Diamond, QBrush(), QPen(markerColor), QSize(10,10));
    markerB = new QwtPlotMarker();
    markerB->setSymbol(mBSymbol);
    mBText = new QwtText("B");
    mBText->setColor("yellow");
    markerB->setLabelAlignment(Qt::AlignHCenter | Qt::AlignTop);
    markerB->setLabel(*mBText);

    ui->plotArea->replot();

    refLevelChanged(ui->refLevelWheel->value());
    bandChanged(0);
    attenChanged(true);
    bwChanged(true);
    spanWheelChanged(ui->spanWheel->value());
}

Tek492::~Tek492()
{
    delete markerA;
    //delete mASymbol; // this crashes; is the marker symbol deleted when deleting marker?
    delete mAvalue;
    delete markerB;
    delete mBvalue;
    delete mAText;
    delete mBText;
    delete titleText;
    delete bLeftText;
    delete bCenterText;
    delete bRightText;
    delete title;
    delete marker;
    delete bLeft;
    delete bCenter;
    delete bRight;
    delete ui;
    if (usb492)
        delete usb492;
    delete grid;
    delete data;
    delete curve;
}

static void delay(int milliseconds)
{
    QTime endTime = QTime::currentTime().addMSecs(milliseconds);
    while (QTime::currentTime() < endTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

static uint8_t memConfig(Ui::Tek492 *ui)
{
    uint8_t confval = 0x60; // unknown meaning, always set

    if (ui->maxHold->isChecked())
        confval |= 0x10;
    if (ui->viewB_minus_saveA->isChecked())
        confval |= 0x08;
    if (ui->viewB->isChecked())
        confval |= 0x04;
    if (ui->viewA->isChecked())
        confval |= 0x02;
    if (ui->saveA->isChecked())
        confval |= 0x01;

    return confval;
}

#define XML_ROOT "tek492"

/* root attributes */
#define XMLNS "http://uriah.heep.sax.de/tek492/xml"
#define XSI   "http://www.w3.org/2001/XMLSchema-instance"
#define SCHEMALOC "http://uriah.heep.sax.de/tek492/xml http://uriah.heep.sax.de/tek492/xml/tek492.xsd"
#define VERSION "0.1"

void Tek492::readFile(const QString fName)
{
    QDomDocument doc;
    QFile file(fName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "File error", "Error opening file");
        return;
    }
    if (!doc.setContent(&file))
    {
        QMessageBox::information(this, "File error", "File has no contents");
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    for (QDomNode node = docElem.firstChild();
         !node.isNull();
         node = node.nextSibling())
    {
        QDomElement ele = node.toElement();
        if (!ele.isNull())
        {
            if (ele.tagName() == "header")
            {
                for (QDomNode hnode = node.firstChild();
                     !hnode.isNull();
                     hnode = hnode.nextSibling())
                {
                    QDomElement hele = hnode.toElement();
                    if (!hele.isNull())
                    {
                        // parse header tags
                        if (hele.tagName() == "comment")
                        {
                            QString text = hele.text();
                            if (text != QString())
                                QMessageBox::information(this, "File comment",
                                                         QString("File comment is: ") + text);
                        }
                        else if (hele.tagName() == "band")
                        {
                            int index = hele.text().toInt();
                            ui->fBand->setCurrentIndex(index);
                            // triggers
                            //bandChanged(index);
                        }
                        else if (hele.tagName() == "centerFrequency")
                        {
                            ui->frequencySpinBox->setValue(hele.text().toDouble());
                        }
                        else if (hele.tagName() == "memoryConfiguration")
                        {
                            uint8_t confval = hele.text().toInt();
                            ui->maxHold->setChecked((confval & 0x10) != 0);
                            ui->viewB_minus_saveA->setChecked((confval & 0x08) != 0);
                            ui->viewB->setChecked((confval & 0x04) != 0);
                            ui->viewA->setChecked((confval & 0x02) != 0);
                            ui->saveA->setChecked((confval & 0x01) != 0);
                        }
                        else if (hele.tagName() == "videoFilter")
                        {
                            int vf = hele.text().toInt();
                            switch (vf)
                            {
                            case 0:
                                ui->vfNone->setChecked(true);
                                break;

                            case 1:
                                ui->vfWide->setChecked(true);
                                break;

                            case 2:
                                ui->vfNarrow->setChecked(true);
                                break;
                            }
                        }
                        else if (hele.tagName() == "spanIdx")
                        {
                            double val = hele.text().toDouble();
                            ui->spanWheel->setValue(18 - val);
                            // triggers
                            //spanWheelChanged(val);
                        }
                        else if (hele.tagName() == "rbw")
                        {
                            int rbw = hele.text().toInt();
                            switch (rbw)
                            {
                            case 100:
                                ui->rbw100->setChecked(true);
                                break;

                            case 1000:
                                ui->rbw1k->setChecked(true);
                                break;

                            case 10000:
                                ui->rbw10k->setChecked(true);
                                break;

                            case 100000:
                                ui->rbw100k->setChecked(true);
                                break;

                            case 1000000:
                                ui->rbw1M->setChecked(true);
                                break;
                            }
                        }
                        else if (hele.tagName() == "referenceLevel")
                        {
                            double val = hele.text().toDouble();
                            ui->refLevelWheel->setValue(val);
                            // triggers.
                            // refLevelChanged(val);
                        }
                        else if (hele.tagName() == "attenuation")
                        {
                            int atten = hele.text().toInt();
                            switch (atten)
                            {
                            case 0:
                                ui->atten0->setChecked(true);
                                break;

                            case 10:
                                ui->atten10->setChecked(true);
                                break;

                            case 20:
                                ui->atten20->setChecked(true);
                                break;

                            case 30:
                                ui->atten30->setChecked(true);
                                break;

                            case 40:
                                ui->atten40->setChecked(true);
                                break;

                            case 50:
                                ui->atten50->setChecked(true);
                                break;

                            case 60:
                                ui->atten60->setChecked(true);
                                break;
                            }
                        }
                        else
                        {
                            qDebug() << "Unknown header tag:" << hele.tagName();
                        }
                    }
                }
            }
            else if (ele.tagName() == "data")
            {
                int idx = 0;
                for (QDomNode dnode = node.firstChild();
                     !dnode.isNull() && idx < 1024;
                     dnode = dnode.nextSibling(), idx++)
                {
                    QDomElement dele = dnode.toElement();
                    if (!dele.isNull())
                    {
                        if (dele.tagName() == "ele")
                        {
                            rawData[idx] = dele.text().toInt();
                        }
                    }
                }
            }
            else
            {
                qDebug() << "Unknown tag:" << ele.tagName();
            }
        }

        plotData();
    }
}

void Tek492::writeFile(const QString fName, const QString comment)
{
    QDomDocument doc;
    doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    QDomElement root = doc.createElement(XML_ROOT);
    doc.appendChild(root);
    root.setAttribute("xmlns", XMLNS);
    root.setAttribute("xmlns:xsi", XSI);
    root.setAttribute("xsi:schemaLocation", SCHEMALOC);
    root.setAttribute("version", VERSION);

    QDomElement rootChildHeader = doc.createElement("header");
    root.appendChild(rootChildHeader);

    QDomElement headerComment = doc.createElement("comment");
    if (comment != QString())
    {
        QDomText txtComment = doc.createTextNode(comment);
        headerComment.appendChild(txtComment);
    }
    rootChildHeader.appendChild(headerComment);

    QDomElement headerBand = doc.createElement("band");
    QDomText txtBand = doc.createTextNode(QString::number(ui->fBand->currentIndex()));
    headerBand.appendChild(txtBand);
    rootChildHeader.appendChild(headerBand);

    QDomElement headerCF = doc.createElement("centerFrequency");
    QDomText txtCF = doc.createTextNode(QString::number(ui->frequencySpinBox->value()));
    headerCF.appendChild(txtCF);
    rootChildHeader.appendChild(headerCF);

    QDomElement memConf = doc.createElement("memoryConfiguration");
    QDomText txtMemConf = doc.createTextNode(QString::number(memConfig(ui), 10));
    memConf.appendChild(txtMemConf);
    rootChildHeader.appendChild(memConf);

    QDomElement videoFilter = doc.createElement("videoFilter");
    int vf = 0;
    if (ui->vfWide->isChecked())
        vf = 1;
    else if (ui->vfNarrow->isChecked())
        vf = 2;
    QDomText txtVF = doc.createTextNode(QString::number(vf));
    videoFilter.appendChild(txtVF);
    rootChildHeader.appendChild(videoFilter);

    QDomElement span = doc.createElement("spanIdx");
    QDomText txtSpan = doc.createTextNode(QString::number(spanIdx));
    span.appendChild(txtSpan);
    rootChildHeader.appendChild(span);

    QDomElement rbwEle = doc.createElement("rbw");
    uint rbw = 1000000;
    if (ui->rbw100->isChecked())
        rbw = 100;
    else if (ui->rbw1k->isChecked())
        rbw = 1000;
    else if (ui->rbw10k->isChecked())
        rbw = 10000;
    else if (ui->rbw100k->isChecked())
        rbw = 100000;
    QDomText txtRbw = doc.createTextNode(QString::number(rbw));
    rbwEle.appendChild(txtRbw);
    rootChildHeader.appendChild(rbwEle);

    QDomElement refLev = doc.createElement("referenceLevel");
    QDomText txtRef = doc.createTextNode(QString::number(refLevel));
    refLev.appendChild(txtRef);
    rootChildHeader.appendChild(refLev);

    QDomElement attenEle = doc.createElement("attenuation");
    int atten = 0;
    if (ui->atten10->isChecked())
        atten = 10;
    else if (ui->atten20->isChecked())
        atten = 20;
    else if (ui->atten30->isChecked())
        atten = 30;
    else if (ui->atten40->isChecked())
        atten = 40;
    else if (ui->atten50->isChecked())
        atten = 50;
    else if (ui->atten60->isChecked())
        atten = 60;
    QDomText txtAtten = doc.createTextNode(QString::number(atten));
    attenEle.appendChild(txtAtten);
    rootChildHeader.appendChild(attenEle);

    QDomElement data = doc.createElement("data");
    for (int i = 0; i < 1024; i++)
    {
        QDomElement dataEle = doc.createElement("ele");
        QDomText txtEle = doc.createTextNode(QString::number(rawData[i]));
        dataEle.appendChild(txtEle);
        data.appendChild(dataEle);
    }
    root.appendChild(data);

    QString fname = fName;
    if(fname.right(4) != ".xml") {fname += ".xml";}
    QFile file(fname);
    if(QFile::exists(fname)) {QFile::remove(fname);}
    if(file.open(QFile::WriteOnly))
    {
        file.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n").toUtf8());
        file.write(doc.toByteArray());
        file.close();
    }
}

void Tek492::plotData(void)
{
    qreal cf, span;

    // full span
    if (spanIdx == 0)
    {
        // f in MHz
        if (lowBound == 0)
            // band 1 is actually 0 ... 4.2 GHz
            span = 4200;
        else
            span = highBound - lowBound;
        cf = span / 2 + lowBound;
    }
    else
    {
        cf = ui->frequencySpinBox->value();
        span = 10 * spanValues[spanIdx];
        span /= 1E6; // in MHz
    }

    data->clear();

    // Discard bogus samples at the beginning and end
    qreal rl = refLevel;
    for (int i = 4; i <= 1018; ++i)
    {
        qreal f = cf + ((i - 507) / 1015.0) * span;
        qreal v = (static_cast<qreal>(rawData[i]) - 24) / 220.0;
        if (v > 1)
            v = 1;
        else if (v < 0)
            v = 0;
        qreal val = rl + 80 * (v - 1);
        data->append(QPointF(f, val));
    }
    curve->setBoundingRect(cf - 0.5 * span, rl - 80, span, 80.0);
    curve->setSamples(*data);
    ui->plotArea->replot();

    ui->mAEnabled->setCheckable(true);
    ui->mAEnabled->setToolTip("");
    ui->mBEnabled->setCheckable(true);
    ui->mBEnabled->setToolTip("");
    ui->actionSave->setEnabled(true);
    ui->actionPrint->setEnabled(true);}

// SLOTs
void Tek492::connectToDevice(void)
{
    ui->acquire->setEnabled(true);
}

void Tek492::saveAs(void)
{
    QString fName = QFileDialog::getSaveFileName(this, "Save file to");
    if (fName != QString())
    {
        QString comment = QInputDialog::getText(this, "File comment", "Enter an optional file comment");
        writeFile(fName, comment);
    }
}

void Tek492::loadFile(void)
{
    QString fName = QFileDialog::getOpenFileName(this, "Load file from");
    if (fName != QString())
    {
        readFile(fName);
    }
}

void Tek492::acquire(void)
{
    uint8_t crt_control;

    // full span
    if (spanIdx == 0)
    {
        crt_control = 0x08; // maxspan dot
    }
    else
    {
        crt_control = 0;
    }

    // Halt acquisition, turn off readout
    uint8_t confval = memConfig(ui);
    usb492->write_reg(USB492::CRT_Control, crt_control);
    usb492->write_reg(USB492::Storage_Control, confval);
    delay(50);

    for (int i = 0; i < 1024; i += 64)
        dev->control(USB_Device::Control_In | USB_Device::Control_Vendor,
                     0x01, 0, USB492::Storage_Data,
                     rawData + i, 64);

    // Resume acquisition, re-enable readout
    usb492->write_reg(USB492::Storage_Control, confval | 0x80);
    usb492->write_reg(USB492::CRT_Control, crt_control | 0x01);

    plotData();
}

void Tek492::frequencyDial(int value)
{
    static int oldValue;

    double cf = ui->frequencySpinBox->value();
    int delta = value - oldValue;
    oldValue = value;
    if (delta > 50)
        // wrap counterclockwise, e.g. 1 to 99 = 98; means -1
        delta -= 99;
    else if (delta < -50)
        // wrap clockwise, e.g. 99 to 1 = -98; means +1
        delta += 99;
    cf += delta;
    if (cf >= lowBound && cf <= highBound)
    {
        ui->frequencySpinBox->setValue(cf);
        qreal span;
        if (spanIdx == 0)
        {
            if (lowBound == 0)
            {
                // band 1 is actually 0 ... 4.2 GHz
                span = 4200;
                cf = span / 2;
            }
            else
                span = highBound - lowBound;
        }
        else
        {
            span = 10 * spanValues[spanIdx];
            span /= 1E6; // in MHz
        }
        ui->mAWheel->setMinimum(cf - span / 2);
        ui->mAWheel->setMaximum(cf + span / 2);
        ui->mAWheel->setSingleStep(span / 1015.0);
        if (ui->mAWheel->value() > cf + span / 2)
        {
            ui->mAWheel->setValue(cf + span / 2);
            markerAmoved(cf + span / 2);
        }
        else if (ui->mAWheel->value() < cf - span / 2)
        {
            ui->mAWheel->setValue(cf - span / 2);
            markerAmoved(cf - span / 2);
        }
        ui->mBWheel->setMinimum(cf - span / 2);
        ui->mBWheel->setMaximum(cf + span / 2);
        ui->mBWheel->setSingleStep(span / 1015.0);
        if (ui->mBWheel->value() > cf + span / 2)
        {
            ui->mBWheel->setValue(cf + span / 2);
            markerAmoved(cf + span / 2);
        }
        else if (ui->mBWheel->value() < cf - span / 2)
        {
            ui->mBWheel->setValue(cf - span / 2);
            markerAmoved(cf - span / 2);
        }
        ui->plotArea->replot();
    }
}

void Tek492::spanWheelChanged(double value)
{
    spanIdx = 18 - int(value);
    const QString &curspan = spanNames[spanIdx];
    ui->spanValue->setText(curspan);
    switch (autoBW[spanIdx])
    {
    case 1000000:
        ui->rbw1M->setChecked(true);
        break;

    case 100000:
        ui->rbw100k->setChecked(true);
        break;

    case 10000:
        ui->rbw10k->setChecked(true);
        break;

    case 1000:
        ui->rbw1k->setChecked(true);
        break;

    case 100:
        ui->rbw100->setChecked(true);
        break;
    }
    qreal cf = ui->frequencySpinBox->value();
    qreal span;
    if (spanIdx == 0)
    {
        if (lowBound == 0)
        {
            // band 1 is actually 0 ... 4.2 GHz
            span = 4200;
            cf = span / 2;
        }
        else
            span = highBound - lowBound;
    }
    else
    {
        span = 10 * spanValues[spanIdx];
        span /= 1E6; // in MHz
    }
    ui->mAWheel->setMinimum(cf - span / 2);
    ui->mAWheel->setMaximum(cf + span / 2);
    ui->mAWheel->setSingleStep(span / 1015.0);
    if (ui->mAWheel->value() > cf + span / 2)
    {
        ui->mAWheel->setValue(cf + span / 2);
        markerAmoved(cf + span / 2);
    }
    else if (ui->mAWheel->value() < cf - span / 2)
    {
        ui->mAWheel->setValue(cf - span / 2);
        markerAmoved(cf - span / 2);
    }
    ui->mBWheel->setMinimum(cf - span / 2);
    ui->mBWheel->setMaximum(cf + span / 2);
    ui->mBWheel->setSingleStep(span / 1015.0);
    if (ui->mBWheel->value() > cf + span / 2)
    {
        ui->mBWheel->setValue(cf + span / 2);
        markerAmoved(cf + span / 2);
    }
    else if (ui->mBWheel->value() < cf - span / 2)
    {
        ui->mBWheel->setValue(cf - span / 2);
        markerAmoved(cf - span / 2);
    }
    ui->plotArea->replot();
}

void Tek492::refLevelChanged(double value)
{
    refLevel = int(value);
    char buf[20];
    sprintf(buf, "%+3d dBm", refLevel);
    ui->refLevValue->setText(buf);
    int idx = (value + 120) / 10; // -120 dBm => index 0
    switch (autoAtten[idx])
    {
    case 0:
        ui->atten0->setChecked(true);
        break;

    case 10:
        ui->atten10->setChecked(true);
        break;

    case 20:
        ui->atten20->setChecked(true);
        break;

    case 30:
        ui->atten30->setChecked(true);
        break;

    case 40:
        ui->atten40->setChecked(true);
        break;

    case 50:
        ui->atten50->setChecked(true);
        break;

    case 60:
        ui->atten60->setChecked(true);
        break;
    }
}

void Tek492::bandChanged(int index)
{
    switch (index)
    {
    case 0: // 1: 100 kHz ... 1.8 GHz (actually 4.2)
        lowBound = 0;
        highBound = 1800;
        bCenterText->setText("0 … 1.8");
        break;

    case 1: // 2: 1.7 ... 5.5 GHz
        lowBound = 1700;
        highBound = 5500;
        bCenterText->setText("1.7 … 5.5");
        break;

    case 2: // 3: 3.0 ... 7.1 GHz
        lowBound = 3000;
        highBound = 7100;
        bCenterText->setText("3.0 … 7.1");
        break;

    case 3: // 4: 5.4 ... 18.0 GHz
        lowBound = 5400;
        highBound = 18000;
        bCenterText->setText("5.4 … 18.0");
        break;

    case 4: // 5: 15.0 ... 21.0 GHz
        lowBound = 15E3;
        highBound = 21E3;
        bCenterText->setText("15 … 21");
        break;
    }
    bCenter->setText(*bCenterText);
    ui->plotArea->replot();
    ui->frequencySpinBox->setMinimum(lowBound);
    ui->frequencySpinBox->setMaximum(highBound);
    if (ui->frequencySpinBox->value() < lowBound)
        ui->frequencySpinBox->setValue(lowBound);
    else if (ui->frequencySpinBox->value() > highBound)
        ui->frequencySpinBox->setValue(highBound);
    qreal span;
    qreal cf = ui->frequencySpinBox->value();
    if (spanIdx == 0)
    {
        if (lowBound == 0)
        {
            // band 1 is actually 0 ... 4.2 GHz
            span = 4200;
            cf = span / 2;
        }
        else
            span = highBound - lowBound;
    }
    else
    {
        span = 10 * spanValues[spanIdx];
        span /= 1E6; // in MHz
    }
    ui->mAWheel->setMinimum(cf - span / 2);
    ui->mAWheel->setMaximum(cf + span / 2);
    ui->mAWheel->setSingleStep(span / 1015.0);
    if (ui->mAWheel->value() > cf + span / 2)
    {
        ui->mAWheel->setValue(cf + span / 2);
        markerAmoved(cf + span / 2);
    }
    else if (ui->mAWheel->value() < cf - span / 2)
    {
        ui->mAWheel->setValue(cf - span / 2);
        markerAmoved(cf - span / 2);
    }
    ui->mBWheel->setMinimum(cf - span / 2);
    ui->mBWheel->setMaximum(cf + span / 2);
    ui->mBWheel->setSingleStep(span / 1015.0);
    if (ui->mBWheel->value() > cf + span / 2)
    {
        ui->mBWheel->setValue(cf + span / 2);
        markerAmoved(cf + span / 2);
    }
    else if (ui->mBWheel->value() < cf - span / 2)
    {
        ui->mBWheel->setValue(cf - span / 2);
        markerAmoved(cf - span / 2);
    }
    ui->plotArea->replot();
}

void Tek492::attenChanged(bool)
{
    if (ui->atten0->isChecked())
        bLeftText->setText("ATT: 0");
    else if (ui->atten10->isChecked())
        bLeftText->setText("ATT: 10 dB");
    else if (ui->atten20->isChecked())
        bLeftText->setText("ATT: 20 dB");
    else if (ui->atten30->isChecked())
        bLeftText->setText("ATT: 30 dB");
    else if (ui->atten40->isChecked())
        bLeftText->setText("ATT: 40 dB");
    else if (ui->atten50->isChecked())
        bLeftText->setText("ATT: 50 dB");
    else
        bLeftText->setText("ATT: 60 dB");
    bLeft->setText(*bLeftText);
    ui->plotArea->replot();
}

void Tek492::bwChanged(bool)
{
    int vf = 1;
    if (ui->vfNarrow->isChecked())
        vf = 300;
    else if (ui->vfWide->isChecked())
        vf = 30;

    QString bw;
    if (ui->rbw100->isChecked())
        switch (vf)
        {
        case 1:
            bw = QString("RBW: 100 Hz");
            break;

        case 30:
            bw = QString("RBW: 100 Hz\nVBW: 3.3 Hz");
            break;

        case 300:
            bw = QString("RBW: 100 Hz\nVBW: 0.33 Hz");
            break;
        }
    else if (ui->rbw1k->isChecked())
        switch (vf)
        {
        case 1:
            bw = QString("RBW: 1 kHz");
            break;

        case 30:
            bw = QString("RBW: 1 kHz\nVBW: 33 Hz");
            break;

        case 300:
            bw = QString("RBW: 1 kHz\nVBW: 3.3 Hz");
            break;
        }
    else if (ui->rbw10k->isChecked())
        switch (vf)
        {
        case 1:
            bw = QString("RBW: 10 kHz");
            break;

        case 30:
            bw = QString("RBW: 10 kHz\nVBW: 333 Hz");
            break;

        case 300:
            bw = QString("RBW: 10 kHz\nVBW: 33 Hz");
            break;
        }
    else if (ui->rbw100k->isChecked())
        switch (vf)
        {
        case 1:
            bw = QString("RBW: 100 kHz");
            break;

        case 30:
            bw = QString("RBW: 100 kHz\nVBW: 3.3 kHz");
            break;

        case 300:
            bw = QString("RBW: 100 kHz\nVBW: 333 Hz");
            break;
        }
    else
        switch (vf)
        {
        case 1:
            bw = QString("RBW: 1 MHz");
            break;

        case 30:
            bw = QString("RBW: 1 MHz\nVBW: 33 kHz");
            break;

        case 300:
            bw = QString("RBW: 1 MHz\nVBW: 3.3 kHz");
            break;
        }
    bRightText->setText(bw);
    bRight->setText(*bRightText);
    ui->plotArea->replot();
}

void Tek492::plotMarkerLabel(QColor color)
{
    if (ui->mAEnabled->isChecked() &&
            ui->mBEnabled->isChecked())
    {
        QwtText l(*mAvalue + "\n" + *mBvalue);
        l.setRenderFlags(Qt::AlignRight | Qt::AlignTop);
        l.setColor(color);
        marker->setText(l);
        marker->attach(ui->plotArea);
    }
    else if (ui->mAEnabled->isChecked())
    {
        QwtText l(*mAvalue);
        l.setRenderFlags(Qt::AlignRight | Qt::AlignTop);
        l.setColor(color);
        marker->setText(l);
        marker->attach(ui->plotArea);
    }
    else if (ui->mBEnabled->isChecked())
    {
        QwtText l(*mBvalue);
        l.setRenderFlags(Qt::AlignRight | Qt::AlignTop);
        l.setColor(color);
        marker->setText(l);
        marker->attach(ui->plotArea);
    }
    else
    {
        marker->detach();
    }
}

void Tek492::markerAchecked(bool ischecked)
{
    if (ischecked)
    {
        ui->mAWheel->setEnabled(true);
        markerA->attach(ui->plotArea);
        markerAmoved(ui->mAWheel->value());
    }
    else
    {
        ui->mAWheel->setEnabled(false);
        markerA->detach();
    }

    plotMarkerLabel(QColor("yellow"));
    ui->plotArea->replot();
}

void Tek492::markerAmoved(double val)
{
    qreal cf, span;
    // full span
    if (spanIdx == 0)
    {
        // f in MHz
        if (lowBound == 0)
            // band 1 is actually 0 ... 4.2 GHz
            span = 4200;
        else
            span = highBound - lowBound;
        cf = span / 2 + lowBound;
    }
    else
    {
        cf = ui->frequencySpinBox->value();
        span = 10 * spanValues[spanIdx];
        span /= 1E6; // in MHz
    }
    int idx = int((val - cf + span / 2) / span * 1015.0);
    QPointF a = curve->sample(idx);
    if (rawData[idx + 4] > 220)
        markerA->setLabelAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    else
        markerA->setLabelAlignment(Qt::AlignHCenter | Qt::AlignTop);
    qreal dbm = a.ry();
    if (span < 0.2)
        mAvalue->sprintf("A: %.3f MHz, %.1f dBm", val, dbm);
    else if (span < 2)
        mAvalue->sprintf("A: %.2f MHz, %.1f dBm", val, dbm);
    else
        mAvalue->sprintf("A: %.1f MHz, %.1f dBm", val, dbm);
    markerA->setXValue(val);
    markerA->setYValue(dbm);
    plotMarkerLabel(QColor("yellow"));
    ui->plotArea->replot();
}

void Tek492::markerBchecked(bool ischecked)
{
    if (ischecked)
    {
        ui->mBWheel->setEnabled(true);
        markerB->attach(ui->plotArea);
        markerBmoved(ui->mBWheel->value());
    }
    else
    {
        ui->mBWheel->setEnabled(false);
        markerB->detach();
    }

    plotMarkerLabel(QColor("yellow"));
    ui->plotArea->replot();
}

void Tek492::markerBmoved(double val)
{
    qreal cf, span;
    // full span
    if (spanIdx == 0)
    {
        // f in MHz
        if (lowBound == 0)
            // band 1 is actually 0 ... 4.2 GHz
            span = 4200;
        else
            span = highBound - lowBound;
        cf = span / 2 + lowBound;
    }
    else
    {
        cf = ui->frequencySpinBox->value();
        span = 10 * spanValues[spanIdx];
        span /= 1E6; // in MHz
    }
    int idx = int((val - cf + span / 2) / span * 1015.0);
    QPointF a = curve->sample(idx);
    if (rawData[idx + 4] > 220)
        markerB->setLabelAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    else
        markerB->setLabelAlignment(Qt::AlignHCenter | Qt::AlignTop);
    qreal dbm = a.ry();
    if (span < 0.2)
        mBvalue->sprintf("B: %.3f MHz, %.1f dBm", val, dbm);
    else if (span < 2)
        mBvalue->sprintf("B: %.2f MHz, %.1f dBm", val, dbm);
    else
        mBvalue->sprintf("B: %.1f MHz, %.1f dBm", val, dbm);
    markerB->setXValue(val);
    markerB->setYValue(dbm);
    plotMarkerLabel(QColor("yellow"));
    ui->plotArea->replot();
}

void Tek492::about(void)
{
    QMessageBox::about(this,
                       "About Tek492",
                       "----------------------------------------------------------------------------\n"
                       "\"THE BEER-WARE LICENSE\" (Revision 42):\n"
                       "<dl8dtl@s09.sax.de> wrote this file.  As long as you retain this notice you "
                       "can do whatever you want with this stuff. If we meet some day, and you "
                       "think this stuff is worth it, you can buy me a beer in return.\n"
                       "                                                         Joerg Wunsch\n"
                       "----------------------------------------------------------------------------\n\n"
                       "As this application uses Qt, any redistribution is bound to the "
                       "terms of either the GNU Lesser General Public License v. 3 (\"LGPLv3\"), "
                       "or to any commercial Qt license.\n\n"
                       "This application uses the Qwt extensions to Qt, which are "
                       "provided under the terms of LGPL v2.1.  However, the Qwt license "
                       "explicitly disclaims many common use cases as not constituting a "
                       "\"derived work\" (in the LGPL sense), so it is generally less "
                       "demanding about the redistribution policy than Qt itself.");
}

void Tek492::info(void)
{
    QMessageBox::information(this,
                             "Info about Tek492",
                             "Tek492 uses circuitben's simple USB device to acquire "
                             "data through the accessory connector of the Tektronix 492 "
                             "analyzer:\n\n"
                             "http://www.circuitben.net/node/4\n\n"
                             "The various user interface elements in Tek492 feature the "
                             "controls of the real device. They are mostly used to maintain "
                             "consistent information to be recorded in the diagram only.\n"
                             "Make sure the memory configuration setup matches the device, "
                             "as these settings are re-applied after capturing data.\n"
                             "If the appropriate USB-492 device has been detected, use the "
                             "\"Connect\" menu entry (in the \"File\" menu) to actually "
                             "connect to the device; then, use the \"Acquire\" button to "
                             "capture data from the analyzer.\n"
                             "Two markers (A and B) can optionally be applied to the "
                             "diagram.",
                             QMessageBox::Ok);
}

void Tek492::print(void)
{
    QPrinter printer(QPrinter::HighResolution);

    QString docName = ui->plotArea->title().text();
    if (docName.isEmpty())
    {
        printer.setDocName("tek492-printout");
    }
    else
    {
        docName.replace(QRegExp(QString::fromLatin1("\n")), " -- ");
        printer.setDocName(docName);
    }

    printer.setCreator("Tek492");
    printer.setOrientation(QPrinter::Landscape);
    printer.setPageSize(QPrinter::A6);  // A6 yields best ratio between
                                        // curve and font size

    QPrintDialog dialog(&printer);
    if (dialog.exec())
    {
        QwtPlotRenderer renderer;

        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame);
        renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);

        if (printer.colorMode() == QPrinter::GrayScale)
        {
            QColor black("black");
            titleText->setColor(black);
            title->setText(*titleText);
            bLeftText->setColor(black);
            bLeft->setText(*bLeftText);
            bCenterText->setColor(black);
            bCenter->setText(*bCenterText);
            bRightText->setColor(black);
            bRight->setText(*bRightText);
            mAText->setColor(black);
            markerA->setLabel(*mAText);
            mASymbol->setPen(black);
            mBText->setColor(black);
            markerB->setLabel(*mBText);
            mBSymbol->setPen(black);
            curve->setPen(QPen(Qt::black, 0.0, Qt::SolidLine));
            plotMarkerLabel(black);
        }
        else
        {
            QColor tekBlue(0, 119, 209);
            titleText->setColor(tekBlue);
            title->setText(*titleText);
            bLeftText->setColor(tekBlue);
            bLeft->setText(*bLeftText);
            bCenterText->setColor(tekBlue);
            bCenter->setText(*bCenterText);
            bRightText->setColor(tekBlue);
            bRight->setText(*bRightText);
            QColor ray(49, 99, 53);
            curve->setPen(QPen(ray, 0.0, Qt::SolidLine));
            QColor brown(148, 14, 14); // dark red / brown
            mAText->setColor(brown);
            markerA->setLabel(*mAText);
            mASymbol->setPen(brown);
            mBText->setColor(brown);
            markerB->setLabel(*mBText);
            mBSymbol->setPen(brown);
            plotMarkerLabel(brown);
        }

        renderer.renderTo(ui->plotArea, printer);

        QColor yellow("yellow");
        titleText->setColor(yellow);
        title->setText(*titleText);
        bLeftText->setColor(yellow);
        bLeft->setText(*bLeftText);
        bCenterText->setColor(yellow);
        bCenter->setText(*bCenterText);
        bRightText->setColor(yellow);
        bRight->setText(*bRightText);
        curve->setPen(QPen(Qt::green, 0.0, Qt::SolidLine));
        mAText->setColor(yellow);
        markerA->setLabel(*mAText);
        mASymbol->setPen(yellow);
        mBText->setColor(yellow);
        markerB->setLabel(*mBText);
        mBSymbol->setPen(yellow);
        plotMarkerLabel(yellow);
    }
}
