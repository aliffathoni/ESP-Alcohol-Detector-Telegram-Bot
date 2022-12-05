#include <>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int RXPin = 12, TXPin = 13;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;
