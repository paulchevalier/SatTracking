#ifndef SATTRACKMAINWINDOW_H
#define SATTRACKMAINWINDOW_H

/*
 * This is sattrackmainwindow.h file of SatTracking project, a Qt application for tracking satelitte
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

#include <QMainWindow>
#include "spaceobject.h"
#include "satmapdialog.h"
#include <QFile>
#include <QMessageBox>

namespace Ui {
class SatTrackMainWindow;
}

class SatTrackMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SatTrackMainWindow(QWidget *parent = 0);
    ~SatTrackMainWindow();
    void updateMFE(const double& value);
    void updateAltitude(const double& value);
    void updateLongitude(const double& value);
    void updateLatitude(const double& value);
    void updateDistance(const double& value);
    void updateAzimuth(const double& value);
    void updateElevation(const double& value);
    void adaptString(QString& line);
    void computeSatPosition(const QDateTime& datetime,double& altitude,double& latitude, double& longitude,double& elevation,double& azimuth, double& distance);
    void clearMapDialog();
    double getStationLatitude() const;
    double getStationLongitude() const;
    double getStationAltitude() const;
private slots:
    void useCurrentDate();
    void computeCourse();
    void openTLE();
    void loadTLE();
    void showMap();
    void refreshLongitude();
    void refreshLatitude();
    void refreshAltitude();
    void predictPasses();
private:
    Ui::SatTrackMainWindow *ui;
    QString tleFile;
    SpaceObject* sat;
    SpaceContext* context;
    SatMapDialog * mapdialog;
    double stationAltitude;
    double stationLongitude;
    double stationLatitude;
};

#endif // SATTRACKMAINWINDOW_H
