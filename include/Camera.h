#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#ifndef Camera_H_
#define Camera_H_

#include "pluginsBase.h"
#include "lappThread.h" // needed for MOS

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/opencv.hpp>

#include <ueye.h> // Camera library (IDS)

#include <boost/bimap.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>

#include <map>
#include "Config.h"
using namespace std;

class Camera
{
public:
    //Need a constructor first if using a camera
    Camera()
    {
        hCam = (HIDS)0;
        // LoadCDMConfiguration(m_config);
    };

    int Connect();
    int Disconnect();
    std::vector<boost::any> Configure(int nPixelClock=216, double exposure=1000, double fps=1, int gain=0, std::string pixel_format="IS_CM_SENSOR_RAW16");
    //std::vector<unsigned char> GetImage();
    void GetImage(DataAccessClientOPCUA* myclient);
    //std::vector<std::string> GetMultipleImages(int n_images);
    std::vector<std::string> GetMultipleImages(int n_images, DataAccessClientOPCUA* myclient);
    std::vector<std::string> GetMultipleImagesStacked(int n_images, DataAccessClientOPCUA* myclient);
    void StopGetMultipleImages();
    int StartCDM(DataAccessClientOPCUA* myclient);
    int StopCDM();
    int StartStream(DataAccessClientOPCUA* myclient);
    int StopStream();

    void SetConfig( map<std::string,std::string> config) {m_config=config;}

    double get_exposure() {return Camera::exposure_setting;}
    int get_master_gain() {return Camera::master_gain_setting;}
    std::string writeFITSImage(cv::Mat image, int n_stack = 1);  

    double get_temperature_value();
    std::string get_temperature_status();
    

//private:
    // Camera stuff
    HIDS hCam; //= (HIDS)0;
    SENSORINFO sensorinfo;
    CAMINFO camerainfo;
    HWND hWndDisplay = NULL; //DIB mode will be used for display
    char *pcImageMemory = NULL;
    int nMemoryId = 0;
    int n_allocated_memories = 50;

    uint formatID = 36;

    double exposure_setting = 0;
    int master_gain_setting = 0;
    
    // Need to find out the memory size of the pixel and the colour mode
    int iColorMode = IS_CM_SENSOR_RAW16; //IS_CM_MONO8; 
    int iBitsPerPixel = 16;        //8;
    int iWidth = 0;  // will be properly initialized with the sensor info struct information
    int iHeight = 0; // will be properly initialized with the sensor info struct information
    double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
    IS_RECT rectAOI;
    int nRet;

    bool b_keep_taking=1;

    typedef boost::bimap< std::string, int > bimap;
    const bimap pixel_formats = boost::assign::list_of< bimap::relation >
    ( "IS_CM_MONO8", IS_CM_MONO8 )
    ( "IS_CM_SENSOR_RAW8", IS_CM_SENSOR_RAW8 )
    ( "IS_CM_SENSOR_RAW16", IS_CM_SENSOR_RAW16 );

    const std::string datapointName_circle_x = "Unit_CDM.AuxControl.CDM.Circle.circle_x.circle_x_v";
    const std::string datapointName_circle_y = "Unit_CDM.AuxControl.CDM.Circle.circle_y.circle_y_v";
    const std::string datapointName_circle_R = "Unit_CDM.AuxControl.CDM.Circle.circle_R.circle_R_v";
    const std::string datapointName_circle_RMS = "Unit_CDM.AuxControl.CDM.Circle.circle_RMS.circle_RMS_v";
    const std::string datapointName_displacement_x = "Unit_CDM.AuxControl.CDM.Displacement.displacement_x.displacement_x_v";
    const std::string datapointName_displacement_y = "Unit_CDM.AuxControl.CDM.Displacement.displacement_y.displacement_y_v";
    const std::string datapointName_rotation = "Unit_CDM.AuxControl.CDM.Displacement.rotation.rotation_v";

    const std::string datapointName_circle_x_stddev = "Unit_CDM.AuxControl.CDM.Circle.circle_x_stddev.circle_x_stddev_v";
    const std::string datapointName_circle_y_stddev = "Unit_CDM.AuxControl.CDM.Circle.circle_y_stddev.circle_y_stddev_v";
    const std::string datapointName_circle_R_stddev = "Unit_CDM.AuxControl.CDM.Circle.circle_R_stddev.circle_R_stddev_v";
    const std::string datapointName_circle_RMS_stddev = "Unit_CDM.AuxControl.CDM.Circle.circle_RMS_stddev.circle_RMS_stddev_v";


    const int nLED = 12;
    const int nOARL = 2;
    const double px2arcsec = 7.35;

    // TODO: initalize vectors with the proper size immediately.    
    std::vector<std::string> datapointName_LED_x_arrays =
    {
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_01.LED_x_01_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_02.LED_x_02_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_03.LED_x_03_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_04.LED_x_04_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_05.LED_x_05_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_06.LED_x_06_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_07.LED_x_07_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_08.LED_x_08_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_09.LED_x_09_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_10.LED_x_10_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_11.LED_x_11_v", 
        "Unit_CDM.AuxControl.CDM.LED_x.LED_x_12.LED_x_12_v", 
    };

    std::vector<std::string> datapointName_LED_y_arrays =
    {
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_01.LED_y_01_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_02.LED_y_02_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_03.LED_y_03_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_04.LED_y_04_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_05.LED_y_05_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_06.LED_y_06_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_07.LED_y_07_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_08.LED_y_08_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_09.LED_y_09_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_10.LED_y_10_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_11.LED_y_11_v", 
        "Unit_CDM.AuxControl.CDM.LED_y.LED_y_12.LED_y_12_v", 
    };

    std::vector<std::string> datapointName_OARL_x_arrays =
    {
        "Unit_CDM.AuxControl.CDM.OARL_x.OARL_x_1.OARL_x_1_v", 
        "Unit_CDM.AuxControl.CDM.OARL_x.OARL_x_2.OARL_x_2_v", 
    };

    std::vector<std::string> datapointName_OARL_y_arrays =
    {
        "Unit_CDM.AuxControl.CDM.OARL_y.OARL_y_1.OARL_y_1_v", 
        "Unit_CDM.AuxControl.CDM.OARL_y.OARL_y_2.OARL_y_2_v", 
    };

    std::string datapointName_OARL_x_mean = "Unit_CDM.AuxControl.CDM.OARL_x.OARL_x_mean.OARL_x_mean_v";
    std::string datapointName_OARL_y_mean = "Unit_CDM.AuxControl.CDM.OARL_y.OARL_y_mean.OARL_y_mean_v";

    std::string datapointName_timestamp_UTC = "Unit_CDM.AuxControl.CDM.timestamp.timestamp_UTC.timestamp_UTC_v";
    std::string datapointName_timestamp_epoch = "Unit_CDM.AuxControl.CDM.timestamp.timestamp_epoch.timestamp_epoch_v";

private:
    map<std::string,std::string> m_config;
   

};

#endif //  Camera_H_