#ifndef Camera_H_
#define Camera_H_

#include <iostream>

#include <ueye.h> // Camera library (IDS)

class Camera
{
public:
    //Need a constructor first if using a camera
    Camera();
    int Connect();

    //int query_cameras();

//private:
    // Camera stuff
    HIDS hCam = (HIDS)0;
    SENSORINFO sensorinfo;
    CAMINFO camerainfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char *pcImageMemory;
    int id = 0;
    
    // Need to find out the memory size of the pixel and the colour mode
    int iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
    int iBitsPerPixel = 8;        //16;
    int iWidth = 0;  // will be properly initialized with the sensor info struct information
    int iHeight = 0; // will be properly initialized with the sensor info struct information
    double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
    IS_RECT rectAOI;
    int nRet;
};

#endif //  Camera_H_