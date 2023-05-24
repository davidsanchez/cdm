/*********************************************************************
 *
 *  You can find more information on our web site:
 *   Yocto-Meteo documentation:
 *      https://www.yoctopuce.com/EN/products/yocto-meteo/doc.html
 *   C++ API Reference:
 *      https://www.yoctopuce.com/EN/doc/reference/yoctolib-cpp-EN.html
 *
 *********************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split

#include "Logging.h"
#include "Meteo.h"


using namespace std;

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

Meteo::Meteo()
{

    // Setup the API to use local USB devices
    if (yRegisterHub("usb", errmsg) != YAPI_SUCCESS)
    {
        cerr << "RegisterHub error: " << errmsg << endl;
        //return 1;
    }

    hsensor = yFirstHumidity();
    tsensor = yFirstTemperature();
    psensor = yFirstPressure();

    // chunk.memory = (char *)malloc(1); /* will be grown as needed by the realloc above */
    // chunk.size = 0;                   /* no data at this point */


};

Meteo::~Meteo()
{
    yFreeAPI();

};

// Gets the data from the sensor inside the hood.
vector<float> Meteo::Update_sensor()
{
    if (hsensor == NULL || tsensor == NULL || psensor == NULL)
    {
        LOG_ERROR << "No module connected (check USB cable)";
        val = {0, 0, 0, 0};
        return val;
    }

    if (hsensor->isOnline())
    {
        temperature = tsensor->get_currentValue();
        humidity_rel = hsensor->get_currentValue();
        humidity_abs = hsensor->get_absHum();
        pressure = psensor->get_currentValue();
        val = {temperature, humidity_rel, humidity_abs, pressure};
    }
    else
    {
        LOG_ERROR << "Module not connected (check identification and USB cable)";
        val = {0, 0, 0, 0};
    }

    return val;
};


