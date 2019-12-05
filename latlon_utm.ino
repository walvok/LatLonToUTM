/* Part of code is  from (c) Chris Veress 2014-2019 */
/*                                      MIT License */
#include <math.h>
const double FOURTHPI = PI / 4;
const double deg2rad = PI / 180;
const double rad2deg = 180.0 / PI;
const double equrad = 6377563;
const double squecc = 0.00667054;

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  while(1)
  {
  Serial.println(LLtoUTM(49.223248, 16.591355)); //can be chaneg to stream from GPS
  }
}

String LLtoUTM(const double Lat, const double Long)
{
    double a = 6378137;
    double eccSquared = 0.00669438;   //WGS-84, 6378137, 0.00669438
    double k0 = 0.9996;
    double LongOrigin;
    double eccPrimeSquared;
    double N, T, C, A, M;
    double UTMEasting;
    double UTMNorthing;
    
  //Make sure the longitude is between -180.00 .. 179.9
    double LongTemp = (Long+180)-int((Long+180)/360)*360-180; // -180.00 .. 179.9;
    double LatRad = Lat*deg2rad;
    double LongRad = LongTemp*deg2rad;
    double LongOriginRad;
    int    ZoneNumber;
    
  ZoneNumber = int((LongTemp + 180)/6) + 1;
  if( Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 )
    ZoneNumber = 32;
  // Special zones for Svalbard
  if( Lat >= 72.0 && Lat < 84.0 ) 
  {
    if(      LongTemp >= 0.0  && LongTemp <  9.0 ) ZoneNumber = 31;
    else if( LongTemp >= 9.0  && LongTemp < 21.0 ) ZoneNumber = 33;
    else if( LongTemp >= 21.0 && LongTemp < 33.0 ) ZoneNumber = 35;
    else if( LongTemp >= 33.0 && LongTemp < 42.0 ) ZoneNumber = 37;
   }
  LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone
  LongOriginRad = LongOrigin * deg2rad;
  //compute the UTM Zone from the latitude and longitude
  eccPrimeSquared = (eccSquared)/(1-eccSquared);
  N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
  T = tan(LatRad)*tan(LatRad);
  C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
  A = cos(LatRad)*(LongRad-LongOriginRad);
  M = a*((1 - eccSquared/4    - 3*eccSquared*eccSquared/64  - 5*eccSquared*eccSquared*eccSquared/256)*LatRad 
        - (3*eccSquared/8 + 3*eccSquared*eccSquared/32  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
                  + (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad) 
                  - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
  
  UTMEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
          + (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
          + 500000.0);
  UTMNorthing = (double)(k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
         + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
         
  if(Lat < 0)
    UTMNorthing += 10000000.0; //10000000 meter offset for southern hemisphere

  //Preparation of MGRS 100km chars  
  String UTMTwo = MGRSZoneDesignator(UTMEasting, UTMNorthing);
  int corrE = UTMEasting / 100000; //Correction from UTM to MGRS -> cutting first number
  UTMEasting = UTMEasting - (corrE * 100000); 
  int corrN = UTMNorthing / 100000; //Correction from UTM to MGRS -> cutting first two numbers
  UTMNorthing = UTMNorthing - (corrN * 100000);
   
  //Preparation to output
  String toUTM = String(ZoneNumber);
  toUTM += " ";
  toUTM += UTMLetterDesignator(Lat);
  toUTM += " ";
  toUTM += UTMTwo;
  toUTM += " E:";
  toUTM += UTMEasting;
  toUTM += " N:";
  toUTM += UTMNorthing;
  return toUTM;
}

String MGRSZoneDesignator(double UTMEasting, double UTMNorthing)
{
  String e100kLetters[] = {"S","T","U","V","W","X","Y","Z"};
  String n100kLetters[] = {"A","B","C","D","E","F","G","H","J","K","L","M","N","P","Q","R","S","T","U","V"};
  const int col = floor(UTMEasting /100000); //Cutting first number. Upside is more info
  const int row = int(floor(UTMNorthing /100000))%20; //Cutting first two numbers. Upside is more info
  String ZoneDesignator = e100kLetters[col-1];
  ZoneDesignator += n100kLetters[row];
  return ZoneDesignator;
}

char UTMLetterDesignator(double Lat)
{
  //Written by Chuck Gantz- chuck.gantz@globalstar.com
  char LetterDesignator;
  if((84 >= Lat) && (Lat >= 72)) LetterDesignator = 'X';
  else if((72 > Lat) && (Lat >= 64)) LetterDesignator = 'W';
  else if((64 > Lat) && (Lat >= 56)) LetterDesignator = 'V';
  else if((56 > Lat) && (Lat >= 48)) LetterDesignator = 'U';
  else if((48 > Lat) && (Lat >= 40)) LetterDesignator = 'T';
  else if((40 > Lat) && (Lat >= 32)) LetterDesignator = 'S';
  else if((32 > Lat) && (Lat >= 24)) LetterDesignator = 'R';
  else if((24 > Lat) && (Lat >= 16)) LetterDesignator = 'Q';
  else if((16 > Lat) && (Lat >= 8)) LetterDesignator = 'P';
  else if(( 8 > Lat) && (Lat >= 0)) LetterDesignator = 'N';
  else if(( 0 > Lat) && (Lat >= -8)) LetterDesignator = 'M';
  else if((-8> Lat) && (Lat >= -16)) LetterDesignator = 'L';
  else if((-16 > Lat) && (Lat >= -24)) LetterDesignator = 'K';
  else if((-24 > Lat) && (Lat >= -32)) LetterDesignator = 'J';
  else if((-32 > Lat) && (Lat >= -40)) LetterDesignator = 'H';
  else if((-40 > Lat) && (Lat >= -48)) LetterDesignator = 'G';
  else if((-48 > Lat) && (Lat >= -56)) LetterDesignator = 'F';
  else if((-56 > Lat) && (Lat >= -64)) LetterDesignator = 'E';
  else if((-64 > Lat) && (Lat >= -72)) LetterDesignator = 'D';
  else if((-72 > Lat) && (Lat >= -80)) LetterDesignator = 'C';
  else LetterDesignator = 'Z'; //This is here as an error flag to show that the Latitude is outside the UTM limits
  return LetterDesignator;
}
