// BrightBall code
// written by Mike Ellison (devellison)
// Public domain.
// 
// Thanks to:
// LadyAda for the LED strip and tutorial at http://www.ladyada.net/products/rgbledstrip/

#include <Wire.h>

#define USE_ADXL335  1

#if USE_ADXL335
  #define X_PIN A5
  #define Y_PIN A4
  #define Z_PIN A3
#endif

#define RED_PIN   3
#define GREEN_PIN 5
#define BLUE_PIN  6

#define DEBUGPRINTS 1

#define BUFFERSIZE 10
#define ACTIVETHRESHOLD 8
#define ACTIVEDELAY     1000

unsigned char rBuf[BUFFERSIZE];
unsigned char gBuf[BUFFERSIZE];
unsigned char bBuf[BUFFERSIZE];

long rAccum = 0;
long gAccum = 0;
long bAccum = 0;
long bufferPos = 0;

long activeCount = 0;
bool active = false;
char lastX = 0;
char lastY = 0;
char lastZ = 0;

int minX = -1;
int maxX = -1;
int minY = -1;
int maxY = -1;
int minZ = -1;
int maxZ = -1;

void setup()
{  
#if DEBUGPRINTS
  Serial.begin(9600);
#endif

  // Clear our filter buffers
  memset(&rBuf[0],0,BUFFERSIZE);
  memset(&gBuf[0],0,BUFFERSIZE);
  memset(&bBuf[0],0,BUFFERSIZE);

#if USE_ADXL335
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(Z_PIN, INPUT);
  digitalWrite(X_PIN,LOW);
  digitalWrite(Y_PIN,LOW);
  digitalWrite(Z_PIN,LOW);
#endif

  
  // Setup PWM pins for LED strip
  pinMode(RED_PIN,   OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN,  OUTPUT);
}


void loop()
{
  // Read the accelerometer
#if USE_LIS302DL
  char x = readReg( LIS302DL_ADDRESS, X_REG);
  char y = readReg( LIS302DL_ADDRESS, Y_REG);
  char z = readReg( LIS302DL_ADDRESS, Z_REG);
#endif

#if USE_ADXL335
  // 
  long ax = analogRead(X_PIN);
  long ay = analogRead(Y_PIN);
  long az = analogRead(Z_PIN);
  
  if ((minX == -1) || (ax < minX))
    minX = ax;
  if ((minY == -1) || (ax < minY))
    minY = ay;
  if ((minZ == -1) || (ax < minZ))
    minZ = az;

  if ((maxX == -1) || (ax > maxX))
    maxX = ax;
  if ((maxY == -1) || (ay > maxY))
    maxY = ay;
  if ((maxZ == -1) || (az > maxZ))
    maxZ = az;

  char x = (((ax - minX)*256) / (maxX - minX));
  char y = (((ay - minY)*256) / (maxY - minY));
  char z = (((az - minZ)*256) / (maxZ - minZ));
#endif
  // If we don't see much motion, increment
  // activeCount until we hit the delay size
  // then go inactive.
  if ((abs(x-lastX) < ACTIVETHRESHOLD) &&
      (abs(y-lastY) < ACTIVETHRESHOLD) &&
      (abs(z-lastZ) < ACTIVETHRESHOLD))
  {
     activeCount++;
     if (activeCount > ACTIVEDELAY)
     {
       activeCount--;
       active = false;
     }
  }
  else
  {
    activeCount = 0;
    active = true;
  }
  
  lastX = x;
  lastY = y;
  lastZ = z;

 
  unsigned char xVal;
  unsigned char yVal;
  unsigned char zVal;

  if (active)
  {
    // Original value is -128 to 128, but that makes for nasty jumps at the edges.
    //     abs(val) gives us 128 to 0 to 128 for a smooth curve.
    //     *2 gives us the whole range of colors back.
    xVal = abs(x)*2;
    yVal = abs(y)*2;
    zVal = abs(z)*2;
  }
  else
  {
    // If not active, slowly fade existing values from buffer to 0
    xVal = rBuf[bufferPos]; 
    yVal = gBuf[bufferPos]; 
    zVal = bBuf[bufferPos]; 
    
    if (xVal) {xVal--;}
    if (yVal) {yVal--;}
    if (zVal) {zVal--;}
  }

  // Remove old value from accumulators
  rAccum -= rBuf[bufferPos];
  gAccum -= gBuf[bufferPos];
  bAccum -= bBuf[bufferPos];

  // Add new value in and store
  rAccum += rBuf[bufferPos] = xVal;
  gAccum += gBuf[bufferPos] = yVal;
  bAccum += bBuf[bufferPos] = zVal;
  
  // move to next position in circular buffers
  bufferPos++;
  bufferPos %= BUFFERSIZE;

  // r/g/b are the average of all values in each buffer currently
  unsigned char r = rAccum/BUFFERSIZE;
  unsigned char g = gAccum/BUFFERSIZE;
  unsigned char b = bAccum/BUFFERSIZE;

  // send values to the pwm pins to light the LEDs
  analogWrite(RED_PIN,   r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN,  b);
}

