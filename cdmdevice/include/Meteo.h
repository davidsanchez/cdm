/*********************************************************************
 *
 *  You can find more information on our web site:
 *   Yocto-Meteo documentation:
 *      https://www.yoctopuce.com/EN/products/yocto-meteo/doc.html
 *   C++ API Reference:
 *      https://www.yoctopuce.com/EN/doc/reference/yoctolib-cpp-EN.html
 *
 *********************************************************************/
#pragma GCC diagnostic ignored "-Wunused-function"
#ifndef METEO_H_
#define METEO_H_

#include "yocto_api.h"
#include "yocto_humidity.h"
#include "yocto_pressure.h"
#include "yocto_temperature.h"


using namespace std;

// Used for cURL
struct MemoryStruct
{
    char *memory;
    size_t size;
};

// Used for cURL
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

class Meteo
{
public:
    Meteo();
    ~Meteo();

    vector<float> Update_sensor(); // Gets the data from the sensor.
    
    float get_temperature() { return tsensor->get_currentValue(); }
    float get_humidity_relative() { return hsensor->get_currentValue(); }
    float get_humidity_absolute() { return hsensor->get_absHum(); }
    float get_pressure() { return psensor->get_currentValue(); }

private:
    string errmsg, target;
    YHumidity *hsensor;
    YTemperature *tsensor;
    YPressure *psensor;
    float temperature, humidity_abs, humidity_rel, pressure;
    vector<float> val{0, 0, 0, 0};

};

#endif //  METEO_H_