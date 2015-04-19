#ifndef SATMAPDIALOG_H
#define SATMAPDIALOG_H

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

#include <QDialog>
#include <QPointF>
#include <QWindow>

#define DEG30PIX 70.0



class SatMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SatMapWidget(QWidget *parent = 0);
    ~SatMapWidget();
     QPointF degToPixel(double latitude, double longitude);
protected:
    void paintEvent(QPaintEvent *event);
};


namespace Ui {
class SatMapDialog;
}

class SatMapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SatMapDialog(QWidget *parent = 0);
    ~SatMapDialog();
    int getStepTime();
    int getTimeLimit();
    int getIntermediateMark();
private slots:
    void updateValues();

private:
    Ui::SatMapDialog *ui;
    SatMapWidget* mapwidget;

};







#endif // SATMAPDIALOG_H
