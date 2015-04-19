/*
 * This is satmapdialog.cpp file of SatTracking project, a Qt application for tracking satelitte
 * Copyright 2014-2015 Paul Chevalier
 *
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "satmapdialog.h"
#include "ui_satmapdialog.h"
#include <QMessageBox>
#include <QImage>
#include <QDir>
#include <QApplication>
#include <iostream>
#include <QPainter>
#include <QPointF>
#include <QPen>
#include "sattrackmainwindow.h"


SatMapDialog::SatMapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SatMapDialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    mapwidget = new SatMapWidget();
    ui->verticalLayout->addWidget(mapwidget);
    connect(ui->buttonClose,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->buttonRefresh,SIGNAL(clicked()),this,SLOT(updateValues()));
}

SatMapDialog::~SatMapDialog()
{
    delete ui;
    ((SatTrackMainWindow*)parent())->clearMapDialog();
}

void SatMapDialog::updateValues(){
    this->update();
}

int SatMapDialog::getStepTime(){
    return ui->spinBoxStepTime->value();
}

int SatMapDialog::getTimeLimit(){
    return ui->spinBoxDuration->value()*3600;
}

int SatMapDialog::getIntermediateMark(){
    return ui->spinBoxMarker->value()*3600;
}



SatMapWidget::SatMapWidget(QWidget *parent) :
    QWidget(parent)
{



}

SatMapWidget::~SatMapWidget(){



}




void SatMapWidget::paintEvent(QPaintEvent *event){
   SatTrackMainWindow* main = ((SatTrackMainWindow*)(this->parent()->parent()));
   SatMapDialog* dialog = ((SatMapDialog*) this->parent());

   QPainter p;
   double latitude,longitude,altitude;
   double elevation,azimuth,distance;

   bool beforeMarker= true;
   int cumulatedTime=0;
   int timeLimit = dialog->getTimeLimit();
   int markerLimit = dialog->getIntermediateMark();
   int stepTime = dialog->getStepTime();
   int elevationThreshold=15;
   //TODO widget : elevation limit
   //TODO afficher debut de passe, heure minutes, seconde + azimuth et el
   //case visible passes
   QDateTime datetime = QDateTime::currentDateTimeUtc();
   QString s= QApplication::applicationDirPath() +"/rectangularproj.jpg";

   QPixmap background(s);
   p.begin(this);
   //p.drawText(QRectF(60,75,100,75),"Hello Qt");
   p.drawPixmap(0,0,background);

   QPen penHLines(QColor(233,242,43), 5, Qt::SolidLine);
   p.setPen(penHLines);

   main->computeSatPosition(datetime,altitude,latitude,longitude,elevation,azimuth,distance);
   QPointF point1 = degToPixel(latitude,longitude);

   QPointF marker;
   while(cumulatedTime <= timeLimit){
        datetime=datetime.addSecs(stepTime);
        main->computeSatPosition(datetime,altitude,latitude,longitude,elevation,azimuth,distance);
        QPointF point2  = degToPixel(latitude,longitude);
        if(elevation>elevationThreshold){
           QPen newpen = p.pen();
           newpen.setColor(QColor(180,10,50));
           p.setPen(newpen);
        }
        else{
           QPen newpen = p.pen();
           newpen.setColor(QColor(233,242,43));
           p.setPen(newpen);
        }


        if(!((point1.x()>1500 && point2.x()<100) || (point2.x()>1500 && point1.x()<100))){
            p.drawLine(point1,point2);
        }
        point1=point2;
        cumulatedTime+=stepTime;


        if(cumulatedTime>markerLimit && beforeMarker){
            QPen newpen = p.pen();
            newpen.setWidth(2);
            newpen.setStyle(Qt::DotLine);
            p.setPen(newpen);
            beforeMarker=false;
            marker=point2;
        }
   }

   QPointF station = degToPixel(main->getStationLatitude(),main->getStationLongitude());
   QPen penCircle(QColor(255,0,20), 5, Qt::SolidLine);
   p.setPen(penCircle);
   p.drawRect(station.x()-2,station.y()-2,2,2);
   p.end();

}


QPointF SatMapWidget::degToPixel(double latitude, double longitude){
    double longPix;
    double latPix;
    if(longitude>180){
        longitude=-360+longitude;
    }

    longPix = (((double)(longitude+180.0))*DEG30PIX)/15.0;
    latPix = (((double)(90-latitude))*DEG30PIX)/15.0;
    return QPointF(longPix,latPix);
}

