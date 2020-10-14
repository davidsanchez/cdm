/*********************************************************************
 *
 *  You can find more information on our web site:
 *   Yocto-Meteo documentation:
 *      https://www.yoctopuce.com/EN/products/yocto-meteo/doc.html
 *   C++ API Reference:
 *      https://www.yoctopuce.com/EN/doc/reference/yoctolib-cpp-EN.html
 *
 *********************************************************************/

#include "yocto_api.h"
#include "yocto_humidity.h"
#include "yocto_pressure.h"
#include "yocto_temperature.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

class Meteo
{
public:
    Meteo()
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
    };

    ~Meteo()
    {
        yFreeAPI();
    };

    vector<float> Update()
    {


        if (hsensor == NULL || tsensor == NULL || psensor == NULL)
        {
            cout << "No module connected (check USB cable)" << endl;    
            val = {0, 0, 0};
            return val;
        }

        if (hsensor->isOnline())
        {
            temperature = tsensor->get_currentValue();
            humidity = hsensor->get_currentValue();
            pressure = psensor->get_currentValue();
            val = {temperature, humidity, pressure};

            //cout << "Current temperature: " << temperature << " C" << endl;
            //cout << "Current humidity: " << humidity << " %RH" << endl;
            //cout << "Current pressure: " << pressure << " hPa" << endl;
        }
        else
        {
            cout << "Module not connected (check identification and USB cable)";
            val = {0, 0, 0 };
        }
        return val;
    };

    float get_temperature() { return tsensor->get_currentValue(); }
    float get_humidity() { return hsensor->get_currentValue(); }
    float get_pressure() { return psensor->get_currentValue(); }

private:
    string errmsg, target;
    YHumidity *hsensor;
    YTemperature *tsensor;
    YPressure *psensor;
    float temperature, humidity, pressure;
    vector<float> val{0, 0, 0 };
};