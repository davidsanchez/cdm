#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#ifndef Camera_H_
#define Camera_H_

#include "pluginsBase.h"
#include "lappThread.h" // needed for MOS
#include "Helper.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/opencv.hpp>

#include <peak/peak.hpp> // Camera library (IDS)

#include <boost/bimap.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>

#include <map>
#include "Config.h"
using namespace std;

#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"

extern Helper helper;

//RR: these from uEye that was removed
#define IS_CM_MONO8                 6
#define IS_CM_MONO10                34
#define IS_CM_MONO12                26
#define IS_CM_SENSOR_RAW8           11
#define IS_CM_SENSOR_RAW10          33
#define IS_CM_SENSOR_RAW12          27
#define IS_CM_SENSOR_RAW16          29
#define IS_GET_MASTER_GAIN                  0x8000
#define IS_IGNORE_PARAMETER                 (-1)

// Structure décrivant un format de pixel
struct PixelFormatInfo {
    std::string genICamName;
    std::string interfaceName;
    int bitsPerPixel;
};


class Camera
{
public:
  //Need a constructor first if using a camera
  Camera();
  ~Camera();
  
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
  int StartSG(DataAccessClientOPCUA* myclient);
  int StopSG();  
  int StartStream(DataAccessClientOPCUA* myclient);
  int StopStream();

  void SetConfig( map<std::string,std::string> config) {m_config=config;}

  double get_exposure() {return Camera::exposure_setting;}
  int get_master_gain() {return Camera::master_gain_setting;}
  std::string writeFITSImage(cv::Mat image, int n_stack = 1);  

  double get_temperature_value();
  std::string get_temperature_status();
    
  static const std::map<std::string, std::string> pixelFormatMap;

  //private:
  // Camera stuff
  peak::DeviceManager* m_DeviceManagerPtr; 
  std::shared_ptr<peak::core::Device> m_DevicePtr;
  std::shared_ptr<peak::core::NodeMap> m_NodemapPtr;
  std::shared_ptr<peak::core::DataStream> m_DatastreamPtr;
  std::shared_ptr<peak::core::Buffer> m_ImgbufferPtr;
  int64_t m_roi_width,m_roi_height;
  //HIDS hCam; //= (HIDS)0;
  //SENSORINFO sensorinfo;
  //CAMINFO camerainfo;
  //HWND hWndDisplay = NULL; //DIB mode will be used for display
  char *pcImageMemory = NULL;
  int nMemoryId = 0;
  size_t n_allocated_memories = 50;

  uint formatID = 36;

  double exposure_setting = 0;
  int master_gain_setting = 0;
    
  // Need to find out the memory size of the pixel and the colour mode
  int iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16; //IS_CM_MONO8; 
  int iBitsPerPixel = 8;        //8; //16;
  int iWidth = 0;  // will be properly initialized with the sensor info struct information
  int iHeight = 0; // will be properly initialized with the sensor info struct information
  double dblFrameRateToSet = 10.0; // if set to 0.0 the max possible fps will be set
  //IS_RECT rectAOI;
  int nRet;

  bool b_keep_taking=1;

  
  typedef boost::bimap< std::string, int > bimap;
  const bimap pixel_formats = boost::assign::list_of< bimap::relation >
    ( "IS_CM_MONO8", IS_CM_MONO8 )
    ( "IS_CM_SENSOR_RAW8", IS_CM_SENSOR_RAW8 )
    ( "IS_CM_SENSOR_RAW16", IS_CM_SENSOR_RAW16 );
  
  
  Config *cdm_config = new Config(CDM_CONFIGURATION_NAME,"");

  const std::string datapointName_circle_x = helper.searchDatapoint("circle_x",cdm_config);
  const std::string datapointName_circle_y = helper.searchDatapoint("circle_y",cdm_config);
  const std::string datapointName_circle_R = helper.searchDatapoint("circle_R",cdm_config);
  const std::string datapointName_circle_RMS =  helper.searchDatapoint("circle_RMS",cdm_config);
  const std::string datapointName_displacement_x =  helper.searchDatapoint("displacement_x",cdm_config);
  const std::string datapointName_displacement_y = helper.searchDatapoint("displacement_y",cdm_config);
  const std::string datapointName_rotation = helper.searchDatapoint("rotation",cdm_config);
    

  const std::string datapointName_circle_x_stddev = helper.searchDatapoint("circle_x_stddev",cdm_config);
  const std::string datapointName_circle_y_stddev = helper.searchDatapoint("circle_y_stddev",cdm_config);
  const std::string datapointName_circle_R_stddev = helper.searchDatapoint("circle_RMS_stddev",cdm_config);
  const std::string datapointName_circle_RMS_stddev = helper.searchDatapoint("circle_R_stddev",cdm_config);

  const std::string datapointName_image = helper.searchDatapoint("image",cdm_config);
  const std::string datapointName_nImagesGet = helper.searchDatapoint("nImagesGet",cdm_config);
  const std::string datapointName_imageName = helper.searchDatapoint("imageName",cdm_config);
  const std::string datapointName_imagePath = helper.searchDatapoint("imagePath",cdm_config);


  const int nLED = 12;
  const int nOARL = 2;
  const double px2arcsec = 1;//TODO DS. change7.35;

  // TODO: initalize vectors with the proper size immediately.    
  std::vector<std::string> datapointName_LED_x_arrays =
    {
      helper.searchDatapoint("LED_x_01",cdm_config),
      helper.searchDatapoint("LED_x_02",cdm_config),
      helper.searchDatapoint("LED_x_03",cdm_config),
      helper.searchDatapoint("LED_x_04",cdm_config),
      helper.searchDatapoint("LED_x_05",cdm_config),
      helper.searchDatapoint("LED_x_06",cdm_config),
      helper.searchDatapoint("LED_x_07",cdm_config),
      helper.searchDatapoint("LED_x_08",cdm_config),
      helper.searchDatapoint("LED_x_09",cdm_config),
      helper.searchDatapoint("LED_x_10",cdm_config),
      helper.searchDatapoint("LED_x_11",cdm_config),
      helper.searchDatapoint("LED_x_12",cdm_config),
    };

  std::vector<std::string> datapointName_LED_y_arrays =
    {
      helper.searchDatapoint("LED_y_01",cdm_config),
      helper.searchDatapoint("LED_y_02",cdm_config),
      helper.searchDatapoint("LED_y_03",cdm_config),
      helper.searchDatapoint("LED_y_04",cdm_config),
      helper.searchDatapoint("LED_y_05",cdm_config),
      helper.searchDatapoint("LED_y_06",cdm_config),
      helper.searchDatapoint("LED_y_07",cdm_config),
      helper.searchDatapoint("LED_y_08",cdm_config),
      helper.searchDatapoint("LED_y_09",cdm_config),
      helper.searchDatapoint("LED_y_10",cdm_config),
      helper.searchDatapoint("LED_y_11",cdm_config),
      helper.searchDatapoint("LED_y_12",cdm_config),
    };

  std::vector<std::string> datapointName_OARL_x_arrays =
    {
      helper.searchDatapoint("OARL_x_1",cdm_config),
      helper.searchDatapoint("OARL_x_2",cdm_config), 
    };

  std::vector<std::string> datapointName_OARL_y_arrays =
    {
      helper.searchDatapoint("OARL_y_1",cdm_config),
      helper.searchDatapoint("OARL_y_2",cdm_config), 
    };

  std::string datapointName_OARL_x_mean = helper.searchDatapoint("OARL_x_mean",cdm_config);
  std::string datapointName_OARL_y_mean = helper.searchDatapoint("OARL_y_mean",cdm_config);

  std::string datapointName_timestamp_UTC = helper.searchDatapoint("timestamp-UTC",cdm_config);
  std::string datapointName_timestamp_epoch = helper.searchDatapoint("timestamp-EPOCH",cdm_config);

private:
  map<std::string,std::string> m_config;
    static const std::unordered_map<std::string, std::string> pixelFormatMap;
    bool setPixelFormat(const std::string &pixel_format);
    // Nouveau membre temporaire pour transmettre l'exposure entre setExposure() et setFrameRate()
    double m_lastExposure = 0.0;

    // Nouvelles méthodes privées extraites de Configure()
    void setPixelClock(int nPixelClock, std::vector<boost::any> &return_values);
    void setExposure(double exposure, std::vector<boost::any> &return_values);
    void setAcquisitionMode();
    void setFrameRate(double fps, std::vector<boost::any> &return_values);
    void setGain(int gain, std::vector<boost::any> &return_values);
    void setPixelFormatAndReport(std::string pixel_format, std::vector<boost::any> &return_values);
};

#endif //  Camera_H_
