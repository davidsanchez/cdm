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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split

#include "Logging.h"

using namespace std;

struct MemoryStruct
{
    char *memory;
    size_t size;
};

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

        const string MAGIC_WS_address = "http://www.magic.iac.es/site/weather/WSssdata.txt";

        chunk.memory = (char *)malloc(1); /* will be grown as needed by the realloc above */
        chunk.size = 0;                   /* no data at this point */

        curl_global_init(CURL_GLOBAL_ALL);

        /* init the curl session */
        curl_handle = curl_easy_init();

        /* specify URL to get */
        curl_easy_setopt(curl_handle, CURLOPT_URL, MAGIC_WS_address.c_str() );

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

        /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Makes curl fail on HTTP 4xx errors.
        curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, true);
    };

    ~Meteo()
    {
        yFreeAPI();

        /* cleanup curl stuff */
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);

        /* we're done with libcurl, so clean it up */
        curl_global_cleanup();
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
            val = {0, 0, 0};
        }

        return val;
    };

    // Gets the data from MAGIC Weather Station.
    vector<string> Update_WS()
    {
        int n_datapoints = 16;                                  // Number of datapoints on the MAGIC weather website
        std::vector<std::string> ws_data(n_datapoints, "-999"); // Default value if curl fails

        /* get data with CURL! */
        res = curl_easy_perform(curl_handle);

        /* check for errors */
        if (res != CURLE_OK)
        {
            LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }
        else
        {
            /* Now, our chunk.memory points to a memory block that is chunk.size
             bytes big and contains the remote file. Do something nice with it! */

            //printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
            //printf("%s\n", chunk.memory);

            boost::split(ws_data, chunk.memory, boost::is_any_of(","));

            // In most of this you don't need find() inside substr, you can just use substr(2).
            // Here it is left for clarity.

            // ws_data[0] // time
            ws_data[1] = ws_data[1].substr(1);                            // date
            ws_data[2] = ws_data[2].substr(ws_data[2].find("TE") + 2);    // temperature
            ws_data[3] = ws_data[3].substr(ws_data[3].find("DR") + 2);    // pressure
            ws_data[4] = ws_data[4].substr(ws_data[4].find("WR") + 2);    // average wind direction
            ws_data[5] = ws_data[5].substr(ws_data[5].find("FE") + 2);    // humidity
            ws_data[6] = ws_data[6].substr(ws_data[6].find("WG") + 2);    // wind speed
            ws_data[7] = ws_data[7].substr(ws_data[7].find("WS") + 2);    // wind gusts
            ws_data[8] = ws_data[8].substr(ws_data[8].find("WD") + 2);    // average wind speed
            ws_data[9] = ws_data[9].substr(ws_data[9].find("WV") + 2);    // current wind direction
            ws_data[10] = ws_data[10].substr(ws_data[10].find("TK") + 2); // Unknown. Maybe temperature.
            ws_data[11] = ws_data[11].substr(ws_data[11].find("TD") + 2); // TNG Dust
            ws_data[12] = ws_data[12].substr(ws_data[12].find("TS") + 2); // TNG Seeing
            ws_data[13] = ws_data[13].substr(ws_data[13].find("RA") + 2); // Rain
            ws_data[14] = ws_data[14].substr(ws_data[14].find("RS") + 2); // Rain statistics
            ws_data[15] = ws_data[15].substr(ws_data[15].find("ST") + 2); // Status
        }

        // Empty the chunk. If you don't empty it the data will be appended next time.
        chunk.memory = (char *)malloc(1);
        chunk.size = 0;

        return ws_data;
    }

    float get_temperature() { return tsensor->get_currentValue(); }
    float get_humidity() { return hsensor->get_currentValue(); }
    float get_pressure() { return psensor->get_currentValue(); }

private:
    string errmsg, target;
    YHumidity *hsensor;
    YTemperature *tsensor;
    YPressure *psensor;
    float temperature, humidity, pressure;
    vector<float> val{0, 0, 0};

    // CURL stuff to get the MAGIC Weather Station data.
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
};