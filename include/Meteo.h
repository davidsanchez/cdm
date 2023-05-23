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
#ifndef Meteo_H_
#define Meteo_H_

#include "yocto_api.h"
#include "yocto_humidity.h"
#include "yocto_pressure.h"
#include "yocto_temperature.h"

#include <curl/curl.h>

#include "Config.h"

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
    vector<string> Update_WS();  // Gets the data from MAGIC Weather Station.
    
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

    // CURL stuff to get the MAGIC Weather Station data.
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    string MAGIC_WS_address;// = "http://www.magic.iac.es/site/weather/WSssdata.txt";
    int n_MAGIC_WS_datapoints;// = 16; // Number of datapoints on the MAGIC weather website

    map<std::string,std::string> m_config;
};

#endif //  Meteo_H_