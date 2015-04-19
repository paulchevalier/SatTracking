#ifndef SPACEOBJECT_H
#define SPACEOBJECT_H

#include "sgp4unit.h"
#include "sgp4io.h"
#include "sgp4ext.h"
#include <string>
#include <QDateTime>

//PI defined already in sgp4unit.h
const double TWOPI        = 2.0 * PI;
const double deg2rad  =   PI/ 180.0;         //   0.0174532925199433
const double xpdotp   =  1440.0 / (2.0 *PI);  // 229.1831180523293
const double RADS_PER_DEG = PI / 180.0;
const double EPOCH_JAN0_12H_1900 = 2415020.0; // Dec 31.5 1899 = Dec 31 1899 12h UTC
const double EPOCH_JAN1_00H_1900 = 2415020.5; // Jan  1.0 1900 = Jan  1 1900 00h UTC
const double EPOCH_JAN1_12H_2000 = 2451545.0; // Jan  1.5 2000 = Jan  1 2000 12h UTC
const double HR_PER_DAY   = 24.0;          // Hours per day   (solar)
const double MIN_PER_DAY  = 1440.0;        // Minutes per day (solar)
const double SEC_PER_DAY  = 86400.0;       // Seconds per day (solar)
const double OMEGA_E      = 1.00273790934; // earth rotation per sideral day

// Utility functions
double sqr   (const double x);
double AcTan (const double sinx, const double cosx);
double toGMST(const double& date);



class SpaceContext
{
public:
    SpaceContext(const int& wgs, const char &opsmode='i');
    gravconsttype getGravity() const;
    char getMode() const;
    double getTumin() const;
    double getRadius() const;
    double getFlattening()const;
    ~SpaceContext();
private:
    gravconsttype  whichconst;
    char typerun;
    char typeinput;
    char opsmode;
    double tumin,mu,radiusearthkm,xke,j2,j3,j4,j3oj2;
};


class SpacePosition{
public:
    SpacePosition(SpaceContext* context, double ro[3], const double& jday, const double &staLatitude, const double &staLongitude, const double &staAltitude);
    ~SpacePosition();
    double getLatitude() const;
    double getLongitude() const;
    double getAltitude() const;
    double getLatitudeDeg() const;
    double getLongitudeDeg() const;
    double getElevation() const;
    double getAzimuth() const;
    double getDistance() const;
    double getAzimuthDeg() const;
    double getElevationDeg() const;
private:
    double latitude;
    double longitude;
    double altitude;
    double elevation;
    double distance;
    double azimuth;
};

class SpaceObject
{
public:
    SpaceObject(const std::string &name, std::string line1, std::string line2, SpaceContext *context);
    double getMinutesFromEpoch(const int& year, const int& mon,const int& day, const int& hr, const int& minute, const double& sec) const;
    double getJDay(const int& year, const int& mon,const int& day, const int& hr, const int& minute, const double& sec) const;
    void getPositionAtTime(const double& tSinceEpoch, double ro[3], double vo[3]);
    double getEpochJDay()const;
    QDateTime getEpochDayTime()const;
    ~SpaceObject();
private:
    elsetrec satrec;
    char classification, intldesg[11];
    bool isInit;
    SpaceContext* context;
    //TODO pas forcément garder ici ?
    std::string name;
};


#endif // SPACEOBJECT_H
