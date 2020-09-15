#ifndef Camera_H_
#define Camera_H_

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/opencv.hpp>

#include <ueye.h> // Camera library (IDS)

#include <boost/bimap.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>

class Camera
{
public:
    //Need a constructor first if using a camera
    Camera()
    {
        hCam = (HIDS)0;
    };

    int Connect();
    int Disconnect();
    std::vector<boost::any> Configure(int nPixelClock=216, double exposure=1000, double fps=1, int gain=0, std::string pixel_format="IS_CM_MONO8");
    std::vector<unsigned char> GetImage();
    int GetMultipleImages(int n_images);
    int Start();
    int Stop();
    

//private:
    // Camera stuff
    HIDS hCam; //= (HIDS)0;
    SENSORINFO sensorinfo;
    CAMINFO camerainfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char *pcImageMemory = NULL;
    int nMemoryId = 0;

    uint formatID = 36;

    
    // Need to find out the memory size of the pixel and the colour mode
    int iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
    int iBitsPerPixel = 8;        //16;
    int iWidth = 0;  // will be properly initialized with the sensor info struct information
    int iHeight = 0; // will be properly initialized with the sensor info struct information
    double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
    IS_RECT rectAOI;
    int nRet;

    int m_active=0;

    typedef boost::bimap< std::string, int > bimap;
    const bimap pixel_formats = boost::assign::list_of< bimap::relation >
    ( "IS_CM_MONO8", IS_CM_MONO8 )
    ( "IS_CM_SENSOR_RAW8", IS_CM_SENSOR_RAW8 )
    ( "IS_CM_SENSOR_RAW16", IS_CM_SENSOR_RAW16 );


};

#endif //  Camera_H_