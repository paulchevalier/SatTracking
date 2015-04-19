/*
 * This is sattrackmainwindow.cpp file of SatTracking project, a Qt application for tracking satelitte
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

#include "sattrackmainwindow.h"
#include "ui_sattrackmainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QLabel>
#include <QDateTime>
#include <iostream>
#include <cstdio>

#include "form.h"

//TODO live tracking
//TODO position presets et fichier de config
SatTrackMainWindow::SatTrackMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SatTrackMainWindow)
{
    ui->setupUi(this);
    refreshAltitude();
    refreshLatitude();
    refreshLongitude();
    connect(ui->buttonCompute,SIGNAL(clicked()),this,SLOT(computeCourse()));
    connect(ui->buttonOpen,SIGNAL(clicked()),this,SLOT(openTLE()));
    connect(ui->buttonQuit,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->buttonLoadTLE,SIGNAL(clicked()),this,SLOT(loadTLE()));
    connect(ui->buttonRefresh,SIGNAL(clicked()),this,SLOT(useCurrentDate()));
    connect(ui->buttonCarte,SIGNAL(clicked()),this,SLOT(showMap()));
    connect(ui->spinBoxAltitude,SIGNAL(valueChanged(int)),SLOT(refreshAltitude()));
    connect(ui->spinBoxLatitudeDeg,SIGNAL(valueChanged(int)),SLOT(refreshLatitude()));
    connect(ui->spinBoxLatitudeMin,SIGNAL(valueChanged(int)),SLOT(refreshLatitude()));
    connect(ui->spinBoxLatitudeSec,SIGNAL(valueChanged(int)),SLOT(refreshLatitude()));
    connect(ui->spinBoxLongitudeDeg,SIGNAL(valueChanged(int)),SLOT(refreshLongitude()));
    connect(ui->spinBoxLongitudeMin,SIGNAL(valueChanged(int)),SLOT(refreshLongitude()));
    connect(ui->spinBoxLongitudeSec,SIGNAL(valueChanged(int)),SLOT(refreshLongitude()));
    connect(ui->buttonPredict,SIGNAL(clicked()),this,SLOT(predictPasses()));
    tleFile="";
    sat=NULL;
    context=NULL;
    mapdialog=NULL;
    ui->tleFileLine->setText(QApplication::applicationDirPath()  +"/issTest.tle");
    ui->dateTimeEditTarget->setDateTime( QDateTime::currentDateTimeUtc());
}

SatTrackMainWindow::~SatTrackMainWindow()
{
    delete ui;

    if(sat!=NULL){
        delete sat;
    }
    if(context!=NULL){
        delete context;
    }
}

void SatTrackMainWindow::clearMapDialog(){
    mapdialog=NULL;
}

void SatTrackMainWindow::openTLE(){
    QString folder= QDir::homePath();
    //TODO dossier par défaut
    if(tleFile.length()!=0){
        QFile file(tleFile);
        if(file.exists()){
            QFileInfo finfo(file);
            folder= finfo.path();
        }
    }


    tleFile = QFileDialog::getOpenFileName(this,"Ouvrir un TLE",folder,"TLE (*.tle)");
    ui->tleFileLine->setText(tleFile);
    loadTLE();
}

void SatTrackMainWindow::useCurrentDate(){
    ui->dateTimeEditTarget->setDateTime( QDateTime::currentDateTimeUtc());
    computeCourse();
}

//deprecated and useless
void SatTrackMainWindow::adaptString(QString &line){
    line.replace('.',",");
}

void SatTrackMainWindow::loadTLE(){
    if(tleFile.length()==0 && ui->tleFileLine->text().length()!=0){ //check if user didn't type the filename himself
        tleFile=ui->tleFileLine->text();
    }

    if(tleFile.length()!=0){
        QFile file(tleFile);
        if(file.exists()){
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QString line1;
            QString line2;
            if(file.bytesAvailable()>0){
                line1=file.readLine();
                if(file.bytesAvailable()>0){
                    line2=file.readLine();
                    //removed because good locale is used now
                    //adaptString(line1);
                    //adaptString(line2);
                    context = new SpaceContext(84,'a');
                    sat = new SpaceObject("ISS",line1.toStdString(),line2.toStdString(),context);
                    ui->dateTimeEditEpoch->setDateTime(sat->getEpochDayTime());
                    ui->editTLE->clear();
                    ui->editTLE->append(line1+line2);
                    useCurrentDate();
                }
            }
        }
    }
    else{
        QMessageBox::critical(this,"Impossible de charger le fichier","Veuillez vérifier le chemin du fichier");
        //TODO message d'erreur
    }

}

void SatTrackMainWindow::computeCourse(){


    if(sat!=NULL){
        double latitude,longitude,altitude,elevation,azimuth,distance;
        computeSatPosition(ui->dateTimeEditTarget->dateTime(),altitude,latitude,longitude,elevation,azimuth,distance);
        updateAltitude(altitude);
        updateLatitude(latitude);
        updateLongitude(longitude);
        updateElevation(elevation);
        updateAzimuth(azimuth);
        updateDistance(distance);
    }
    else{
        loadTLE();
        computeCourse();
    }

    //std::string line1="1 25544U 98067A   13360.23316093  .00009151  00000-0  16778-3 0  3240";
    //std::string line2="2 25544 051.6499 236.8020 0004513 314.0886 143.0863 15.50055431864495";
    //TODO set parameters
    /*
     * for debug purpose
    std::cout << "ro[0] (km) : " << ro[0] << std::endl;
    std::cout << "ro[1] (km) : " << ro[1] << std::endl;
    std::cout << "ro[2] (km) : " << ro[2] << std::endl;
    std::cout << "Altitude (km) : " << position.getAltitude() << std::endl;
    std::cout << "Longitude (deg) : " << position.getLongitudeDeg() << std::endl;
    std::cout << "Latitude (deg) : " << position.getLatitudeDeg() << std::endl;
    */
}

void SatTrackMainWindow::computeSatPosition(const QDateTime& datetime,double& altitude,double& latitude, double& longitude,double & elevation, double& azimuth,double& distance){
    if(sat!=NULL){//todo erreur
        QTime time = datetime.time();
        QDate date = datetime.date();
        double tSinceEpoch = sat->getMinutesFromEpoch(date.year(),date.month(),date.day(),time.hour(),time.minute(),time.second());
        //std::cout << "Minutes since Epoch : " << tSinceEpoch << std::endl;
        updateMFE(tSinceEpoch);
        double ro[3];
        double vo[3];
        double jday = sat->getJDay(date.year(),date.month(),date.day(),time.hour(),time.minute(),time.second());
        sat->getPositionAtTime(tSinceEpoch,ro,vo);

        SpacePosition position(context,ro,jday,getStationLatitude(),getStationLongitude(),getStationAltitude());
        latitude = position.getLatitudeDeg();
        longitude = position.getLongitudeDeg();
        altitude = position.getAltitude();
        azimuth = position.getAzimuthDeg();
        elevation = position.getElevationDeg();
        distance = position.getDistance();

    }
}

void SatTrackMainWindow::predictPasses(){
    ui->textEditPredict->clear();
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    int dayLimit = ui->spinBoxDuration->value();
    const int stepTime = 30; // pas de temps pour la résolution de la trajectoire grossière
    const int stepTimePrecise = 1; // pas de temps pour la recherche précise des bornes
    int cumulatedTime = 0;
    int elevationThreshold=ui->spinBoxThreshold->value();
    double latitude,longitude,altitude;
    double elevation,azimuth,distance;
    double elevationMax=0;

    bool passStarted=false;
    QString passStart="";
    QString passEnd="";
    QDateTime maxEl;

    while(cumulatedTime <= dayLimit*86400){
        computeSatPosition(datetime,altitude,latitude,longitude,elevation,azimuth,distance);
        if(elevation>elevationThreshold && !passStarted){
            QDateTime tempdt = datetime;
            while(elevation>elevationThreshold){
                computeSatPosition(tempdt,altitude,latitude,longitude,elevation,azimuth,distance);
                tempdt = tempdt.addSecs(-stepTimePrecise); //reculer d'une seconde jusqu'à arriver à la limite, quand on sort de la boucle, elevation <= elevationThreshold
            }
            passStart= "On " + tempdt.date().toString()  + ", pass starts at " + tempdt.time().toString() + " for Az=" + QString::number(azimuth,'f',1) +"°";//  + " and El=" + QString::number(elevation,'f',1) +"°";
            passStarted=true;
            computeSatPosition(datetime,altitude,latitude,longitude,elevation,azimuth,distance);
        }



        if(elevation<elevationThreshold &&  passStarted){
            QDateTime tempdt = datetime.addSecs(-stepTime); //reculer de stepTime pour passer sous la limite
            computeSatPosition(tempdt,altitude,latitude,longitude,elevation,azimuth,distance);
            while(elevation>elevationThreshold){
                computeSatPosition(tempdt,altitude,latitude,longitude,elevation,azimuth,distance);
                tempdt = tempdt.addSecs(stepTimePrecise); //avancer d'une seconde jusqu'à arriver à la limite, quand on sort de la boucle, elevation = elevationThreshold
            }
            passEnd = " and ends at "+ tempdt.time().toString() + ", for Az=" + QString::number(azimuth,'f',1) +"°";// + " and El=" + QString::number(elevation,'f',1) +"°";
            int direction = 1;
            QDateTime eltempdt=maxEl;
            do {
                eltempdt=eltempdt.addSecs(-stepTimePrecise);
                computeSatPosition(eltempdt,altitude,latitude,longitude,elevation,azimuth,distance);
            }while(elevation==elevationMax);
            //détermine la direction du maximum
            if(elevation>elevationMax){
                direction = -1;
            }
            computeSatPosition(maxEl,altitude,latitude,longitude,elevation,azimuth,distance);
            //recherche le maximum
            while(elevation>=elevationMax){
                elevationMax=elevation;
                maxEl=maxEl.addSecs(stepTimePrecise*direction);
                computeSatPosition(maxEl,altitude,latitude,longitude,elevation,azimuth,distance);
            }
            ui->textEditPredict->append(passStart + passEnd + " with maximum elevation reached at " + maxEl.time().toString() + " for Az=" + QString::number(azimuth,'f',1) +"°"+" and El=" + QString::number(elevation,'f',1) +"°\n");
            passStarted=false;
            elevationMax=elevationThreshold;
            computeSatPosition(datetime,altitude,latitude,longitude,elevation,azimuth,distance);
        }
        if(passStarted && elevation> elevationMax){
            elevationMax=elevation;
            maxEl = datetime;
        }
        datetime=datetime.addSecs(stepTime);
        cumulatedTime+=stepTime;
    }

}

void SatTrackMainWindow::showMap(){

    if(mapdialog==NULL){
        mapdialog=new SatMapDialog(this);
        mapdialog->show();
    }

}

void SatTrackMainWindow::updateMFE(const double &value){
    ui->labelMFEO->setText(QString::number(value) + " mins");
}

void SatTrackMainWindow::updateLatitude(const double &value){
    ui->labelLatO->setText(QString::number(value,'f',2) + "°");
}

void SatTrackMainWindow::updateAltitude(const double &value){
    ui->labelAltiO->setText(QString::number(value,'f',2)+ " km");
}

void SatTrackMainWindow::updateLongitude(const double &value){
    ui->labelLonO->setText(QString::number(value,'f',2)+ "°");
}

void SatTrackMainWindow::updateElevation(const double &value){
    ui->labelElevationO->setText(QString::number(value,'f',1) + "°");
}

void SatTrackMainWindow::updateAzimuth(const double &value){
    ui->labelAzimuthO->setText(QString::number(value,'f',1) + "°");
}

void SatTrackMainWindow::updateDistance(const double &value){
    ui->labelDistanceO->setText(QString::number(value,'f',2)+ " km");
}

void SatTrackMainWindow::refreshLongitude(){
    int sign = ui->spinBoxLongitudeDeg->value() < 0? -1 : 1;
    stationLongitude=ui->spinBoxLongitudeDeg->value()+(1.0/60.0)*sign*ui->spinBoxLongitudeMin->value()+(1.0/3600.0)*sign*ui->spinBoxLongitudeSec->value();
    //std ::cout << "Longitude = " << stationLongitude << std::endl;
}

void SatTrackMainWindow::refreshLatitude(){
    int sign = ui->spinBoxLatitudeDeg->value() < 0? -1 : 1;
    stationLatitude=ui->spinBoxLatitudeDeg->value()+(1.0/60.0)*sign*ui->spinBoxLatitudeMin->value()+(1.0/3600.0)*sign*ui->spinBoxLatitudeSec->value();
   // std ::cout << "Latitude = " << stationLatitude << std::endl;;
}

void SatTrackMainWindow::refreshAltitude(){
    stationAltitude=ui->spinBoxAltitude->value();
    //std::cout << "Altitude = " << stationAltitude << std::endl;
}


double SatTrackMainWindow::getStationLatitude() const{
    return stationLatitude;
}
double SatTrackMainWindow::getStationLongitude() const{
    return stationLongitude;
}
double SatTrackMainWindow::getStationAltitude() const{
    return stationAltitude;
}
