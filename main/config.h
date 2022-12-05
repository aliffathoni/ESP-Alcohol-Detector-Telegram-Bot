#include <Arduino.h>

#include "telegram.cpp"
#include "display.cpp"
#include "sensor.cpp"
#include "gps.cpp"

class Telegram{
    public:
        Telegram();
        void begin();
        void send(String msg);
};

class Display{
    public:
        Display();
        void begin();
        void print_status();
        void print_warning();
        void print_location();
};

class Sensor{
    public:
        Sensor();
        void begin();
        float get();
};

class GPS{
    public:
        GPS();
};

class IO{
    public:
        IO(uint8_t buttonPin, );

}