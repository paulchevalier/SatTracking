/*
 * This is spaceobject.cpp file of SatTracking project, a Qt application for tracking satelitte
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

#include "spaceobject.h"
#include <iostream>

double sqr(const double x)
{
   return (x * x);
}

// AcTan()
// ArcTangent of sin(x) / cos(x). The advantage of this function over arctan()
// is that it returns the correct quadrant of the angle.
double AcTan(const double sinx, const double cosx)
{
   if (cosx == 0.0)
   {
      return (sinx > 0.0) ? (PI / 2.0) : (3.0 * PI / 2.0);
   }
   else
   {
      return (cosx > 0.0) ? (atan(sinx / cosx)) : (PI + atan(sinx / cosx));
   }
}

double toGMST(const double& date)
{
   const double UT = fmod(date + 0.5, 1.0);
   const double TU = (date - EPOCH_JAN1_12H_2000- UT) / 36525.0;

   double GMST = 24110.54841 + TU *
                 (8640184.812866 + TU * (0.093104 - TU * 6.2e-06));

   GMST = fmod(GMST + SEC_PER_DAY * OMEGA_E * UT, SEC_PER_DAY);

   if (GMST < 0.0)
      GMST += SEC_PER_DAY;  // "wrap" negative modulo value

   return  (TWOPI * (GMST / SEC_PER_DAY));
 }



SpaceObject::SpaceObject(const std::string &name, std::string line1, std::string line2, SpaceContext* context)
{
    //TODO cleaner
    this->name=name;
    this->context=context;
    isInit=true;

    std::cout << "SpaceObject : parsing lines" << std::endl;
    //TODO cleaner
    twoline2rv(line1,line2,'i',wgs84,satrec);

}


double SpaceObject::getMinutesFromEpoch(const int& year, const int& mon,const int& day, const int& hr, const int& minute, const double& sec) const{
    if(isInit){
        double jdayFE;
        jday( year,mon,day,hr,minute,sec, jdayFE );
        return (jdayFE - satrec.jdsatepoch) * 1440.0;
    }
    return 0;
}


double SpaceObject::getJDay(const int& year, const int& mon,const int& day, const int& hr, const int& minute, const double& sec) const{
    if(isInit){
        double jdayFE;
        jday(year,mon,day,hr,minute,sec, jdayFE);
        return (jdayFE);
    }
    return 0;
}

void SpaceObject::getPositionAtTime(const double& tSinceEpoch, double ro[3], double vo[3]){
    sgp4 (context->getGravity(), satrec,  tSinceEpoch, ro,  vo);
    //#include "debug7.h"

}

double SpaceObject::getEpochJDay()const{
    return satrec.jdsatepoch;
}


QDateTime SpaceObject::getEpochDayTime()const{
    int year,mon,day,hr,minute;
    double sec;
    invjday(satrec.jdsatepoch, year,mon,day,hr,minute,sec);
    return QDateTime(QDate(year,mon,day),QTime(hr,minute,sec));
}


SpaceObject::~SpaceObject(){


}


SpaceContext::SpaceContext(const int &wgs,const char& opsmode){
    whichconst = wgs84;
    this->opsmode=opsmode;
    std::cout<< "SpaceContext::SpaceContext : using opsmode " << opsmode << " and wgs" << wgs <<std::endl;

    //opsmode = 'a' best understanding of how afspc code works
    //opsmode = 'i' imporved sgp4 resulting in smoother behavior

    //opsmode='i' ou 'a'
    //default is WGS84
    if (wgs == 721) whichconst = wgs72old;
    if (wgs== 72) whichconst = wgs72;
    getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2 );
}

gravconsttype SpaceContext::getGravity()const{
    return whichconst;
}

char SpaceContext::getMode()const{
    return opsmode;
}

double SpaceContext::getTumin()const{
    return tumin;
}

double SpaceContext::getFlattening()const{
    return 1/298.257223563;
    //TODO different values for WGS72
}

double SpaceContext::getRadius()const{
    return radiusearthkm;

}
SpaceContext::~SpaceContext(){

}

SpacePosition::SpacePosition(SpaceContext *context, double ro[3], const double &jday,const double& staLatitude, const double& staLongitude,const double& staAltitude){

    //Calcul de theta
    double theta=fmod((AcTan(ro[1], ro[0]) - toGMST(jday)), TWOPI);

    theta = fmod(theta, TWOPI);

    if (theta  < 0.0)
    {
      theta += TWOPI;  // "wrap" negative modulo
    }

    //Calcul de lat :

    double kmSemiMaj = context->getRadius();
    double r   = sqrt(sqr(ro[0]) + sqr(ro[1]));
    double F = context->getFlattening();
    double e2  = F * (2.0 - F);
    double lat = AcTan(ro[2], r);

    const double delta = 1.0e-07;
    double phi;
    double c;
    //newton raphson algo for inverting problem
    do
    {
       phi = lat;
       c   = 1.0 / sqrt(1.0 - e2 * sqr(sin(phi)));
       lat = AcTan(ro[2] + kmSemiMaj * c * e2 * sin(phi), r);
    }
    while (fabs(lat - phi) > delta);

    latitude = lat;
    longitude = theta;
    altitude = r / cos(lat) - kmSemiMaj * c;


    //coordonées ecef du satellite à partir de lat/long
    double nPhiSat=context->getRadius()/sqrt(1.0-e2*sqr(sin(latitude)));
    double rXecef=(nPhiSat+altitude)*cos(latitude)*cos(longitude);
    double rYecef=(nPhiSat+altitude)*cos(latitude)*sin(longitude);
    double rZecef=(nPhiSat*(1.0-e2)+altitude)*sin(latitude);
    //calcul de x,y,z station en km


    double rStationPlane=(context->getRadius()/sqrt(1.0-e2*sqr(sin(deg2rad*staLatitude)))+ staAltitude/1000.0)*cos(deg2rad*staLatitude);

    double rhoX = rXecef-rStationPlane*cos(deg2rad*staLongitude);
    double rhoY = rYecef-rStationPlane*sin(deg2rad*staLongitude);
    double rhoZ =  rZecef-((context->getRadius()*sqr(1-F))/sqrt(1.0-e2*sqr(sin(deg2rad*staLatitude)))+ staAltitude/1000.0)*sin(deg2rad*staLatitude);


    double sinLat=sin(deg2rad*staLatitude); //sin lambda
    double cosLat=cos(deg2rad*staLatitude); //cos lambda
    double sinLon=sin(deg2rad*staLongitude); //sin phi
    double cosLon=cos(deg2rad*staLongitude); //cos phi
    double S = rhoX*sinLat*cosLon + rhoY*sinLat*sinLon-rhoZ*cosLat;
    double E = -rhoX*sinLon+rhoY*cosLon;
    double Z = rhoX*cosLat*cosLon+rhoY*cosLat*sinLon+rhoZ*sinLat;
    double rho2 = sqr(S)+sqr(E)+sqr(Z);


    distance = sqrt(rho2);
    azimuth=AcTan(E,-S);
    elevation=asin(Z/distance);
    if(azimuth<0){
        azimuth+=TWOPI;
    }
}

SpacePosition::~SpacePosition(){


}


double SpacePosition::getLatitude()const{
    return latitude;
}

double SpacePosition::getAltitude()const{
    return altitude;
}

double SpacePosition::getLongitude()const{
    return longitude;
}

double SpacePosition::getLatitudeDeg()const{
    return latitude/deg2rad;
}

double SpacePosition::getLongitudeDeg()const{
    return longitude/deg2rad;
}

double SpacePosition::getElevation()const{
    return elevation;
}

double SpacePosition::getAzimuth()const{
    return azimuth;
}

double SpacePosition::getDistance()const{
    return distance;
}

double SpacePosition::getElevationDeg()const{
    return elevation/deg2rad;
}

double SpacePosition::getAzimuthDeg()const{
    return azimuth/deg2rad;
}


