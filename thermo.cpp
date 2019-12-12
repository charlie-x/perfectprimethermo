#include <iostream>
#include <thread>
#include <chrono>

#include "CSerial.h"

// ofset 11
enum eThermoType {
    T1 = 0 ,
    T2 = 64,
    T1_T2 = 192 /// T1-T2
};

enum eThermoMode {
    MAX = 193,
    MIN = 194,
    AVERAGE = 195,
    MEMORY = 0
};

// offset 10, low nybble?
enum eThermoSystem {
    CELCIUS = 1,
    FARENHEIT = 2,
    KELVIN = 3
};

// is meter in hold or free running state
// offset 10, high nybble
enum eThermoState {
    HOLD = 192,
    FREE = 128
};

typedef struct thermo_tag {
    // time
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    // modes etc
    eThermoState state;
    eThermoType type;
    eThermoMode mode;
    eThermoSystem system;

    // mine has two inputs
    float temperatureT1;
    float temperatureT2;

} thermoData ;

char getSystem(eThermoSystem s)
{
    switch (s) {
        case KELVIN: return 'K';
        case CELCIUS: return 'C';
        case FARENHEIT: return 'F';
    }

    return 'U';
}

void dump(thermoData t) 
{
    printf("HMS %02d:%02d:%02d,  Temp %f(T1) %f(T2) (%c)\n",
        t.hour, t.minute, t.seconds, 
        t.temperatureT1, t.temperatureT2, getSystem(t.system)
    );
}

int main()
{
    CSerial com;
    uint8_t buffer[18];
    thermoData thermo;

    if (ERROR_SUCCESS == com.Open("\\\\.\\COM14")) {

        com.Setup(CSerial::EBaud9600, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
        
        DWORD dataRead = 0;

        int count = 10;
        
        com.Purge();

        while (count--) {

            if (ERROR_SUCCESS == com.Read(buffer, sizeof(buffer), &dataRead, 0, 0)) {

                if (dataRead == sizeof(buffer)) {

                    thermo.system = (eThermoSystem)(buffer[10] & 0x0f);
                    thermo.state = (eThermoState)(buffer[10] >> 4);
                    thermo.type = (eThermoType)(buffer[11] & 0x0f);
                    thermo.mode = (eThermoMode)buffer[12];

                    thermo.hour = (uint8_t)buffer[13];
                    thermo.minute = (uint8_t)buffer[14];
                    thermo.seconds = (uint8_t)buffer[15];

                    // construct temperature
                    thermo.temperatureT1 = (float)(((uint16_t)(buffer[5]) << 8) + (uint16_t)(buffer[6])) / 10 ;
                    thermo.temperatureT2 = (float)(((uint16_t)(buffer[7]) << 8) + (uint16_t)(buffer[8])) / 10 ;

                    dump(thermo);
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        }

        com.Close();
    }
}
