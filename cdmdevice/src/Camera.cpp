#include <Camera.h>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/time_facet.hpp>

#include <chrono>
#include <string>
#include <thread> //for sleep, can be removed after debug
#include <iostream>

#include "Helper.h"
#include "ImageAnalysis.h"
#include <CCfits>

#include "Logging.h"

extern Helper helper;
using namespace CCfits;

using namespace std;
using namespace cv;


const std::map<std::string, std::string> Camera::pixelFormatMap = {
    {"IS_CM_SENSOR_RAW16", "Mono12"},
    {"IS_CM_MONO8", "Mono8"},
}; 

// Recherche inverse (GenICam -> interface)
static const std::unordered_map<std::string, PixelFormatInfo> pixelFormatByGenICam = {
    {"Mono12", {"Mono12", "IS_CM_SENSOR_RAW16", 16}},
    {"Mono8",  {"Mono8",  "IS_CM_MONO8",        8}},
};
std::string currentDateTime()
{//TODO move to helper?
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%S");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentDateTimeMs()
{//TODO move to helper?
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%s");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentDateTimeMsFilename()
{//TODO move to helper?
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y%m%d_%H%M%s");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentEpochTime()
{//TODO move to helper?
    unsigned long int now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    unsigned long int part1 = now / 1000;
    unsigned long int part2 = now % 1000;

    std::string result = to_string(part1) + "." + to_string(part2);
    return result;
}

string UTC_date_short()
{//TODO move to helper?
    char buffer[80];
    // current date/time based on current system
    time_t now = time(0);
    // convert now to tm struct for UTC
    tm *gmtm = gmtime(&now);
    strftime(buffer, 80, "%Y%m%d", gmtm);
    //puts(buffer);

    return buffer;
}


vector<vector<double>> transpose(vector<vector<double>> &A)
{
    //LOG_TRACE << "Camera::transpose()"<<endl;

    int rows = A.size();
    if (rows == 0)
        return {{}};
    int cols = A[0].size();
    vector<vector<double>> r(cols, vector<double>(rows));
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            r[j][i] = A[i][j];
        }
    }
    return r;
}

double calculateStdDev(vector<double> v)
{
    double sum = std::accumulate(std::begin(v), std::end(v), 0.0);
    double m = sum / v.size();

    double accum = 0.0;
    std::for_each(std::begin(v), std::end(v), [&](const double d) {
        accum += (d - m) * (d - m);
    });

    double stdev = sqrt(accum / (v.size() - 1));
    return stdev;
}

Camera::Camera() {
  //initialize IDS library
  peak::Library::Initialize();
}

Camera::~Camera() {
  //close IDS library
  peak::Library::Close();
}


bool Camera::setPixelFormat(const std::string &pixel_format)
{
    LOG_INFO << "Camera::setPixelFormat(): requested '" << pixel_format << "'" << std::endl;

    auto it = pixelFormatMap.find(pixel_format);
    if (it == pixelFormatMap.end())
    {
        LOG_ERROR << "Camera::setPixelFormat(): unsupported pixel_format '"
                   << pixel_format << "'" << std::endl;
        return false;
    }

    const std::string &genICamFormat = it->second;

    try
    {
        auto pixelFormatNode =
            m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("PixelFormat");

        pixelFormatNode->SetCurrentEntry(genICamFormat);

        LOG_INFO << "Camera::setPixelFormat(): set to '" << genICamFormat << "'" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Camera::setPixelFormat(): failed to set '" << genICamFormat
                   << "': " << e.what() << std::endl;
        return false;
    }
}

std::string Camera::writeFITSImage(Mat image, int n_stack,DataPointFinder* finder)
{
  LOG_INFO << "Camera::writeFITSImage()"<<std::endl;

  //TODO: should also send image data type to this method, now assume 8bit
  
  // if( ePixelFormat == VmbPixelFormatMono14)
  //     image=4*image; // For converting 14bit images to 16bit
  
  flip(image, image, 0); // Vertical flipping of image so it is displayed nicely in DS9.

  // Create a FITS primary array containing a 2-D image
  // declare axis arrays.
  long naxis = 2;
  long naxes[2] = {image.cols, image.rows};
  
  // declare auto-pointer to FITS at function scope. Ensures no resources
  // leaked if something fails in dynamic allocation.
  std::auto_ptr<FITS> pFits(0);
  
  std::ostringstream streamObj;
  std::ostringstream stream_fitsPath;
  std::ostringstream stream_remoteImagePath;
  
  stream_fitsPath << std::fixed << std::setprecision(4) << helper.get_fitsPath();
  stream_remoteImagePath << std::fixed << std::setprecision(4) << helper.get_remoteImagePathPrefix();

  double Az_deg=0,Alt_deg=0,RA_target=0,Dec_target=0,RA_telescope=0,Dec_telescope=0,Az_off=0,Alt_off=0;
  bool LEDs_state=0,OARL_state=0,parking_position=0,parked=0,in_Motion=0,tracking_In_Progress=0;
  
  if (finder != NULL){
    finder->getDatapointL1("azimuth_position", Az_deg);
    finder->getDatapointL1("zenithangle_position", Alt_deg);
    finder->getDatapointL1("RA_Target", RA_target);
    finder->getDatapointL1("Dec_Target", Dec_target);
    finder->getDatapointL1("RA_Telescope", RA_telescope);
    finder->getDatapointL1("Dec_Telescope", Dec_telescope);

    finder->getDatapointL1("Azimuth_Offset", Az_off);
    finder->getDatapointL1("ZenithAngle_Offset", Alt_off);

    finder->getDatapointL1("LEDPositions", LEDs_state);
    finder->getDatapointL1("OARLRelay_Status", OARL_state);
    finder->getDatapointL1("Status_In_Parking_Position", parking_position);
    finder->getDatapointL1("Status_Parked", parked);
    finder->getDatapointL1("Status_In_Motion", in_Motion);
    finder->getDatapointL1("Status_Tracking_In_Progress", tracking_In_Progress);
  }

  streamObj << std::fixed;
  streamObj << std::setprecision(4);
  streamObj << helper.unix_timestamp();
  streamObj << "_";
  streamObj << currentDateTimeMsFilename();
  streamObj << "-TARGET=";
  streamObj << helper.get_StarName();
  streamObj << "-EXP=";
  streamObj << std::setprecision(0);
  streamObj << Camera::get_exposure();
  streamObj << "-GAIN=";
  streamObj << Camera::get_master_gain();
  streamObj << "-ZD=";
  streamObj << 90-Alt_deg;
  streamObj << "-AZ=";
  streamObj << Az_deg;
  streamObj << "-LED=";
  streamObj << LEDs_state;
  streamObj << "-OARL=";
  streamObj << OARL_state;
  streamObj << "-parked=";
  streamObj << parked;
  streamObj << "-parkingPos=";
  streamObj << parking_position;
  streamObj << "-inMotion=";
  streamObj << in_Motion;
  streamObj << "-tracking=";
  streamObj << tracking_In_Progress;
  


  if (n_stack > 1)
    streamObj << "-stack=" << n_stack;
  streamObj << ".fits.gz";

  stream_fitsPath << streamObj.str();
  stream_remoteImagePath << streamObj.str();
  std::string fileName = streamObj.str();
  std::string filePath = stream_fitsPath.str();
  std::string remoteImagePath = stream_remoteImagePath.str();
  
  COND_LOG_DEBUG << "filePath: " << filePath << std::endl;
  COND_LOG_DEBUG << "remoteImagePath: " << remoteImagePath << std::endl;


LOG_TRACE << image.depth() << std::endl;

if (image.depth() == CV_8U)       // 0
    iBitsPerPixel = 8;
else if (image.depth() == CV_16U) // 2
    iBitsPerPixel = 16;
else
{
    LOG_ERROR << "writeFITSImage: unsupported Mat depth" << std::endl;
    return "-1";
}

double minVal, maxVal;
cv::minMaxLoc(image, &minVal, &maxVal);
LOG_INFO << "Camera::writeFITSImage(): pixel min=" << minVal 
          << " max=" << maxVal << std::endl;
  try
    {
      if ((iBitsPerPixel == 16) || (iBitsPerPixel == 12) || (iBitsPerPixel == 10))
	pFits.reset(new FITS(filePath, USHORT_IMG, naxis, naxes)); //BYTE_IMG for 8bit, USHORT_IMG for 16bit
      else if (iBitsPerPixel == 8)
	pFits.reset(new FITS(filePath, BYTE_IMG, naxis, naxes));
      
      else
	LOG_ERROR << "writeFITSImage Error: invalid bitdepth value for saving!" << endl;
    }
  catch (FITS::CantCreate)
    {
      // ... or not, as the case may be.
      return "-1"; //TODO: KLUDGE, should return just -1?
    }
    
 
  
  long &vectorLength = naxes[0];
  long &numberOfRows = naxes[1];
  long nelements(1);
  long fpixel(1);

  nelements = std::accumulate(&naxes[0], &naxes[naxis], 1, std::multiplies<long>());


  if ((iBitsPerPixel == 16) || (iBitsPerPixel == 12) || (iBitsPerPixel == 10))
    {
      // Mat to array 16bit
      std::vector<uint16_t> array;
      if (image.isContinuous())
        {
	  array.assign((uint16_t *)image.data, (uint16_t *)image.data + image.total()*image.channels());
        }
      else
        {
	  for (int i = 0; i < image.rows; ++i)
            {
	      //array.insert(array.end(), image.ptr<uchar>(i), image.ptr<uchar>(i)+image.cols);
	      array.insert(array.end(), image.ptr<uint16_t>(i), image.ptr<uint16_t>(i) + image.cols);
            }
        }
      
      // Convert array to valarray
      valarray<uint16_t> myVala(array.data(), array.size());
      pFits->pHDU().write(fpixel, nelements, myVala);
    }
  
  else if (iBitsPerPixel == 8)
    {
      // Mat to array 8bit
      std::vector<uchar> array;
      if (image.isContinuous())
        {
	  // array.assign(mat.datastart, mat.dataend); // <- has problems for sub-matrix like mat = big_mat.row(i)
	  array.assign(image.data, image.data + image.total());
        }
      else
        {
	  for (int i = 0; i < image.rows; ++i)
            {
	      array.insert(array.end(), image.ptr<uchar>(i), image.ptr<uchar>(i) + image.cols);
	      //array.insert(array.end(), image.ptr<uint16_t>(i), image.ptr<uint16_t>(i) + image.cols);
            }
        }
      
      // Convert array to valarray
      valarray<uchar> myVala(array.data(), array.size());
      pFits->pHDU().write(fpixel, nelements, myVala);
    }
  
  else
    LOG_ERROR << "Check pixel format" << endl;
  

  pFits->pHDU().addKey("RA_LST", RA_telescope, "Drive Right Ascension");
  pFits->pHDU().addKey("DEC_LST", Dec_telescope, "Drive Declination");
  pFits->pHDU().addKey("RA_TRGT", RA_target, "Target Right Ascension");
  pFits->pHDU().addKey("DEC_TRGT", Dec_target, "Target Declination");
  pFits->pHDU().addKey("EPOCH", "2000.0", "Epoch");
  pFits->pHDU().addKey("EQUINOX", "2000.0", "Equinox");
  //pFits->pHDU().addKey("SECPIX_SG", 18.56, "Arcsec per pixel"); TODO: Add this information for CDM
  
  pFits->pHDU().addKey("EXPOSURE", Camera::get_exposure(), "Total Exposure Time in miliseconds");
  pFits->pHDU().addKey("UNIXTIME", helper.unix_timestamp(), "Unix epoch time in seconds");
  pFits->pHDU().addKey("DATETIME", currentDateTime(), "UTC time");
  
  pFits->pHDU().addKey("LAT", 28.7573, "Latitude: Location:ORM");
  pFits->pHDU().addKey("LONG", 17.8850, "Longitude: Location:ORM");
  pFits->pHDU().addKey("ZENITH", 90-Alt_deg, "Zenith, in degrees");
  pFits->pHDU().addKey("AZIMUTH", Az_deg, "Azimuth, in degrees");
  
  pFits->pHDU().addKey("OFFZEN", Alt_off, "Offset of Zenith, in degrees");
  pFits->pHDU().addKey("OFFAZ", Az_off, "Offset of Azimuth, in degrees");
  pFits->pHDU().addKey("OBJECT", helper.get_StarName(), "Star name");
  pFits->pHDU().addKey("LEDS", LEDs_state, "LEDs state");
  //pFits->pHDU().addKey("LED01", helper.get_LED01_intensity(), "LED01 intensity");
  pFits->pHDU().addKey("OARL", OARL_state, "OARL status");
//  pFits->pHDU().addKey("SHUTTER", helper.get_Shutter_state(), "Shutter status");
//  pFits->pHDU().addKey("SIS", helper.get_SIS_state(), "SIS status");
  pFits->pHDU().addKey("INMOTION", in_Motion, "Drive status - In Motion");
  pFits->pHDU().addKey("PARKED", parked, "Drive status - Parked");
  pFits->pHDU().addKey("PARKINGP", parking_position, "Drive status - In Parking Position");
  pFits->pHDU().addKey("TRACKING", tracking_In_Progress, "Drive status - Tracking In Progress");
  
  //     pFits->pHDU().addKey("GAMMA", gamma_value, "Gamma");
  pFits->pHDU().addKey("GAIN", Camera::get_master_gain(), "Gain");
  pFits->pHDU().addKey("INFO", helper.get_Comment(), "Additional image info");
  pFits->pHDU().addKey("CAMTVAL", Camera::get_temperature_value(), "Camera temperature value");
  pFits->pHDU().addKey("CAMTSTAT", Camera::get_temperature_status(), "Camera temperature status");
  pFits->pHDU().addKey("STACK", n_stack, "Number of stacked images");
  
  //pFits->pHDU().addKey("DM_E_bot", helper.get_Aux_status_DM_East_Bottom(), "DM East Bottom");
  //pFits->pHDU().addKey("DM_E_top", helper.get_Aux_status_DM_East_Top(), "DM East Top");
  //pFits->pHDU().addKey("DM_W_bot", helper.get_Aux_status_DM_West_Bottom(), "DM West Bottom");
  //pFits->pHDU().addKey("DM_W_top", helper.get_Aux_status_DM_West_Top(), "DM West Top");
  
  //LOG_DEBUG << pFits->pHDU() << std::endl;
  LOG_INFO << "End of Camera::writeFITSImage() wrote "<<fileName<<std::endl;
  //return remoteImagePath;
  return fileName;
}

int Camera::Connect()
{
  //RR: move most LOG_INFO to COND_LOG_DEBUG
    LOG_INFO << "Camera::Connect(): Start"<<endl;
    //get a ref to the IDS device manager
    m_DeviceManagerPtr=&peak::DeviceManager::Instance();
    //update list of installed libraries
    m_DeviceManagerPtr->Update();
    //TODO: read serial no from config
    COND_LOG_DEBUG<<"Camera::Connect() : Connection to camera "<<m_config["ids_serial_no"]<<endl;
    std::string serial_no =m_config["ids_serial_no"]; //"4108904530";
    //enumerate all attached IDS devices, look for the correct SN
    for (const auto& descriptor: m_DeviceManagerPtr->Devices()) {
	if (descriptor->SerialNumber() == serial_no)
	  {
	    m_DevicePtr = descriptor->OpenDevice(peak::core::DeviceAccessType::Control);
	    break;
	  }
    }
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Could not access IDS camera "<<serial_no<<std::endl;
      return 1;
    }

    LOG_INFO << "Camera "<<serial_no<<" succesfully connected"<< std::endl;

    //get a nodemap object to access functions and parameters for current device
    m_NodemapPtr=m_DevicePtr->RemoteDevice()->NodeMaps().at(0);

    //set camera defaults
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("UserSetSelector")->SetCurrentEntry("Default");
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")->Execute();

    //datastreams for image taking
    auto datastreams=m_DevicePtr->DataStreams();
    //if (datastreams.empty()) ...
    m_DatastreamPtr=datastreams.at(0)->OpenDataStream();
    
    //retrieve sensor information
    //camera and sensor info
    LOG_INFO<<"Camera model: "
	    <<m_NodemapPtr->FindNode<peak::core::nodes::StringNode>("DeviceManufacturerInfo")->Value()
	    <<std::endl;
    LOG_INFO<<"Sensor model: "
	    <<m_NodemapPtr->FindNode<peak::core::nodes::StringNode>("SensorName")->Value()
	    <<std::endl;
    int64_t iWidth = m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("SensorWidth")->Value();
    int64_t iHeight = m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("SensorHeight")->Value();
    LOG_INFO<<"Image size "<<iWidth<<"x"<<iHeight<<std::endl;

    //link speed
    int64_t lspeed = m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("DeviceLinkSpeed")->Value();
    LOG_INFO<<"Camera and network negotiated link speed as "<<lspeed<<std::endl;
  
    
    //check temperature sensor
    LOG_INFO<<"Camera temperature sensor is "
	    <<m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("DeviceTemperatureSelector")->CurrentEntry()->SymbolicValue()
	    <<std::endl;
    LOG_INFO<<"T sensor reports temperature: "
	    <<m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("DeviceTemperature")->Value()
	    <<" deg"<<std::endl;

    //setup bit depth / exp / gain for now
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("PixelFormat")->SetCurrentEntry("Mono12");
    m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(20000.0);
    m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("Gain")->SetValue(1.0);
    
    //setup ROI
    //this should go to ::Configure?
    //check minimum ROI and report
    int64_t roi_w_min = m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Width")->Minimum();
    int64_t roi_h_min = m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Height")->Minimum();
    LOG_INFO<<"Minimum ROI is "<<roi_w_min<<"x"<<roi_h_min<<std::endl;
    //set ROI/offset to desired value
    m_roi_width=stoll(m_config["ids_roi_width"]);
    m_roi_height=stoll(m_config["ids_roi_height"]);
    int64_t offset_x=stoll(m_config["ids_roi_offset_x"]);
    int64_t offset_y=stoll(m_config["ids_roi_offset_y"]);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Width")->SetValue(m_roi_width);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Height")->SetValue(m_roi_height);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("OffsetX")->SetValue(offset_x);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("OffsetY")->SetValue(offset_y);
    //report
    m_roi_width=m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Width")->Value();
    m_roi_height=m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("Height")->Value();
    offset_x=m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("OffsetX")->Value();
    offset_y=m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("OffsetY")->Value();
    LOG_INFO<<"ROI is set to "<<m_roi_width<<"x"<<m_roi_height<<std::endl;
    LOG_INFO<<"ROI offset is set to "<<offset_x<<"x"<<offset_y<<std::endl;
    
    return 0;
}


int Camera::Disconnect()
{
    LOG_INFO << "Camera::Disconnect(): Start"<<endl;

    // You should release the reserved images in memory here. Like OpenCV Mat and IDS images

    // when last IDS last device shared pointer is deleted, sensor is released. Nothing explicit to do about that
    m_DatastreamPtr=nullptr;
    m_NodemapPtr=nullptr;
    m_DevicePtr=nullptr;
    m_DeviceManagerPtr=nullptr;
    
    LOG_INFO << "Camera::Disconnect(): End"<< endl;
    return 0;
}

// TODO: merge GetMultipleImages, GetMultipleImagesStacked and StartCDM into one function?

int Camera::StartCDM(DataAccessClientOPCUA *myclient, DataPointFinder* finder)
{
    LOG_TRACE << "Camera::StartCDM(): Start"<<endl;

    //is camera connected
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Camera::StartCDM camera is not connected"<<std::endl;
      return -1;
    }
    
    b_keep_taking = 1;
    int array_size = 10;

    unsigned long int i_images_taken = 0;
    ////char *pcImageMemory_arr[n_allocated_memories];
    //int nMemoryId_arr[n_allocated_memories];

    // Setup for freerun configuration
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")->SetCurrentEntry("Continuous");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("Off");
    int payload_size =m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();
    LOG_INFO<<"Camera::StartCDM payload size: "<<payload_size<<std::endl;
    size_t num_buf_min=m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
    //check requested number of images
    if (num_buf_min>n_allocated_memories) {
      LOG_ERROR<<"Camera::StartCDM: number of allocated memories less than number of buffers needed ("
	       <<num_buf_min
	       <<") so increasing number to min"<<std::endl;
      n_allocated_memories=num_buf_min;
    } //if (num_buf_min>n_allocated_memories)
  
    LOG_INFO<<"Camera::StartCDM auto allocating data buffers"<<std::endl;
    //automatically alloc raw buffers
    for (size_t count=0; count<n_allocated_memories; count++) {
      auto buffer=m_DatastreamPtr->AllocAndAnnounceBuffer(static_cast<size_t>(payload_size),nullptr);
      m_DatastreamPtr->QueueBuffer(buffer);
    }

    LOG_INFO<<"Camera::StartCDM prepare for main loop"<<std::endl;
    
    int loop_image_count = 0;
    int64_t duration_count = 0;

    Mat m1;
    // April 2025 Remove the publication of the image to save bandwidth
    // TODO REMOVE ME
     vector<uchar> published_image;

    vector<double> circle_x;
    vector<double> circle_y;
    vector<double> circle_R;
    vector<double> circle_RMS;

    vector<double> displacement_x;
    vector<double> displacement_y;
    vector<double> rotation;

    vector<vector<double>> LED_x;
    vector<vector<double>> LED_y;
    vector<vector<double>> OARL_x;
    vector<vector<double>> OARL_y;

    vector<double> OARL_mean_x;
    vector<double> OARL_mean_y;

    vector<string> timestamp_UTC;
    vector<string> timestamp_epoch;

    LOG_INFO<<"Camera::StartCDM start freerun acquisition"<<std::endl;
    m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default, peak::core::DataStream::INFINITE_NUMBER);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();
    
    
    while (b_keep_taking == 1)
    {
        std::chrono::steady_clock::time_point begin_loop = std::chrono::steady_clock::now();

        double circle_stddev_R;
        double circle_stddev_RMS;
        double circle_stddev_x;
        double circle_stddev_y;

        vector<double> circle_results;
        vector<double> displacement_results;
        vector<double> led_x_results;
        vector<double> led_y_results;
        vector<double> oarl_x_results;
        vector<double> oarl_y_results;
        vector<double> oarl_mean_results;

        // Use is_LockSeqBuf when processing image?
	LOG_INFO<<"Camera::StartCDM loop: wait for image and process, image "<<loop_image_count<<std::endl;

	try {
	  m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(1000);
	  LOG_INFO<<"Camera::StartCDM loop: image acquired, start process "<<std::endl;
	}
	
	// RR TODO manage exceptions
	catch (const peak::core::TimeoutException& e)
	  {
	    LOG_ERROR<<"Camera::startCDM timeout in WaitForFinishedBuffer"<<std::endl;
	  LOG_ERROR<<e.what()<<std::endl;
	  }
	catch (std::exception& e)
	{
	  LOG_ERROR<<"Camera::startCDM exception in WaitForFinishedBuffer"<<std::endl;
	  LOG_ERROR<<e.what()<<std::endl;
	  //RR: terminate? at least while debugging
	}

	std::chrono::steady_clock::time_point begin_loop_after_image = std::chrono::steady_clock::now();
	
	timestamp_UTC.push_back(currentDateTimeMs());
	timestamp_epoch.push_back(currentEpochTime());
	
	auto tp_start = std::chrono::high_resolution_clock::now();
	
	if (iBitsPerPixel == 8) {
	  m1 = cv::Mat(m_roi_height, m_roi_width, CV_8UC1, (uchar *)m_ImgbufferPtr->BasePtr());
	  
	} else if (iBitsPerPixel == 16) {
	  m1 = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	  
	} else  {
	  LOG_ERROR << "Camera::StartCDM(): Check bitdepth!" << endl;
	  m1 = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	} // if (iBitsPerPixel == 8) 
	
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	// Flipping=Horizontal + Transpose=1 -> Rotating 90 deg clockwise
	// This is to be done for incoming camera image or Fake camera image from fits file.
	LOG_INFO<<"Camera::StartCDM loop: ImageAnalysis create"<<std::endl;
	ImageAnalysis myimage(m1, m_config, "Horizontal", 1, iBitsPerPixel);

	
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	LOG_IMAGE << "Camera::StartCDM(): Time difference [ImageInitalisation] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

	begin = std::chrono::steady_clock::now();
	LOG_INFO<<"Camera::StartCDM loop: ImageAnalysis CalculateImage"<<std::endl;
	myimage.CalculateImage();
	std::this_thread::sleep_for(std::chrono::milliseconds(3)); //RR in place of CalculateImage

	end = std::chrono::steady_clock::now();
	LOG_IMAGE << "Camera::StartCDM(): Time difference [CalculateImage] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
	
	LOG_INFO<<"Camera::StartCDM loop: free buffer after ImageAnalysis"<<std::endl;
	// queue buffer so that it can be used again
	m_DatastreamPtr->QueueBuffer(m_ImgbufferPtr);

	begin = std::chrono::steady_clock::now();

	
	circle_results = myimage.GetCircleResults();
	led_x_results = myimage.GetLEDxResults();
	led_y_results = myimage.GetLEDyResults();
	oarl_x_results = myimage.GetOARLxResults();
	oarl_y_results = myimage.GetOARLyResults();
	displacement_results = myimage.GetDisplacementResults();
	oarl_mean_results =myimage.GetOARLmeanResults();
	
	
	circle_x.push_back(circle_results[0]);
	circle_y.push_back(circle_results[1]);
	circle_R.push_back(circle_results[2]);
	circle_RMS.push_back(circle_results[3]);
	
	displacement_x.push_back(displacement_results[0]);
	displacement_y.push_back(displacement_results[1]);
	rotation.push_back(displacement_results[2]);
	
	LED_x.push_back(led_x_results);
	LED_y.push_back(led_y_results);
	OARL_x.push_back(oarl_x_results);
	OARL_y.push_back(oarl_y_results);
	
	OARL_mean_x.push_back(oarl_mean_results[0]);
	OARL_mean_y.push_back(oarl_mean_results[1]);
	
	end = std::chrono::steady_clock::now();
	LOG_IMAGE << "Camera::StartCDM(): Time difference [Getting image results] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

	auto tp_stop = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
	duration_count += ms.count();
	
	if (++loop_image_count == 100)
	  {
	    LOG_INFO << "Camera::StartCDM(): Duration: " << duration_count / loop_image_count << std::endl;
	    loop_image_count = 0;
	    duration_count = 0;
	  } //if (++loop_image_count == 100)
	
            //is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
	i_images_taken++;
	

  double Alt_deg=0,Az_deg=0;
  // TODO: Optimize this?
	LOG_DATA
	  //cout

	  << setprecision(10) //TODO same as in getmultipleimage
	  << 90-Alt_deg << " "
	  << Az_deg << " "
	  << circle_results[0] * px2arcsec << " "
	  << circle_results[1] * px2arcsec << " "
	  << circle_results[2] * px2arcsec << " "
	  << circle_results[3] * px2arcsec << " "
	  << oarl_mean_results[0] * px2arcsec << " "
	  << oarl_mean_results[1] * px2arcsec << " "
	  << displacement_results[0] * px2arcsec << " "
	  << displacement_results[1] * px2arcsec << " "
	  << displacement_results[2] * px2arcsec << " "
	  << led_x_results[0] * px2arcsec << " "
	  << led_x_results[1] * px2arcsec << " "
	  << led_x_results[2] * px2arcsec << " "
	  << led_x_results[3] * px2arcsec << " "
	  << led_x_results[4] * px2arcsec << " "
	  << led_x_results[5] * px2arcsec << " "
	  << led_x_results[6] * px2arcsec << " "
	  << led_x_results[7] * px2arcsec << " "
	  << led_x_results[8] * px2arcsec << " "
	  << led_x_results[9] * px2arcsec << " "
	  << led_x_results[10] * px2arcsec << " "
	  << led_x_results[11] * px2arcsec << " "
	  << led_y_results[0] * px2arcsec << " "
	  << led_y_results[1] * px2arcsec << " "
	  << led_y_results[2] * px2arcsec << " "
	  << led_y_results[3] * px2arcsec << " "
	  << led_y_results[4] * px2arcsec << " "
	  << led_y_results[5] * px2arcsec << " "
	  << led_y_results[6] * px2arcsec << " "
	  << led_y_results[7] * px2arcsec << " "
	  << led_y_results[8] * px2arcsec << " "
	  << led_y_results[9] * px2arcsec << " "
	  << led_y_results[10] * px2arcsec << " "
	  << led_y_results[11] * px2arcsec << " "
	  << oarl_x_results[0] * px2arcsec << " "
	  << oarl_x_results[1] * px2arcsec << " "
	  << oarl_y_results[0] * px2arcsec << " "
	  << oarl_y_results[1] * px2arcsec
	  << endl; //
	
	//TODO: Time this process!
	if (i_images_taken % array_size == 0)
	  {
	    std::chrono::steady_clock::time_point begin_publish = std::chrono::steady_clock::now();

	    LOG_IMAGE << "Camera::StartCDM(): Gathered " << array_size << " images:" << endl;
	    
	    //TODO: Time this transpose!
	    // We transpose the arrays so instead of grouping it by time first we group it by LEDs/OARLs first.
	    // So before we had a rows for one timeslice containing different LED information and after the transpose we have rows for each LED containing information for different timeslices.
	    
	    LED_x = transpose(LED_x);
	    LED_y = transpose(LED_y);
	    OARL_x = transpose(OARL_x);
	    OARL_y = transpose(OARL_y);
	    
	    int m_nameSpace = 2;
	    
	    //TODO: Time this publishing!
	    myclient->setDatapoint(datapointName_circle_x, m_nameSpace, circle_x);
	    myclient->setDatapoint(datapointName_circle_y, m_nameSpace, circle_y);
	    myclient->setDatapoint(datapointName_circle_R, m_nameSpace, circle_R);
	    myclient->setDatapoint(datapointName_circle_RMS, m_nameSpace, circle_RMS);
	    
	    myclient->setDatapoint(datapointName_displacement_x, m_nameSpace, displacement_x);
	    myclient->setDatapoint(datapointName_displacement_y, m_nameSpace, displacement_y);
	    myclient->setDatapoint(datapointName_rotation, m_nameSpace, rotation);
	    
	    for (int i = 0; i < nLED; i++)
	      {
		myclient->setDatapoint(datapointName_LED_x_arrays[i], m_nameSpace, LED_x[i]);
		myclient->setDatapoint(datapointName_LED_y_arrays[i], m_nameSpace, LED_y[i]);
	      } // for (int i = 0; i < nLED; i++)
	    
	    for (int i = 0; i < nOARL; i++)
	      {
		myclient->setDatapoint(datapointName_OARL_x_arrays[i], m_nameSpace, OARL_x[i]);
		myclient->setDatapoint(datapointName_OARL_y_arrays[i], m_nameSpace, OARL_y[i]);
	      } // for (int i = 0; i < nOARL; i++)
	    
                // Publish the value
	    myclient->setDatapoint(datapointName_OARL_x_mean, m_nameSpace, OARL_mean_x);
	    myclient->setDatapoint(datapointName_OARL_y_mean, m_nameSpace, OARL_mean_y);
	    
	    myclient->setDatapoint(datapointName_timestamp_UTC, m_nameSpace, timestamp_UTC);
	    myclient->setDatapoint(datapointName_timestamp_epoch, m_nameSpace, timestamp_epoch);
	    
	    // Publish stddev values
	    myclient->setDatapoint(datapointName_circle_x_stddev, m_nameSpace, calculateStdDev(circle_x));
	    myclient->setDatapoint(datapointName_circle_y_stddev, m_nameSpace, calculateStdDev(circle_y));
	    myclient->setDatapoint(datapointName_circle_R_stddev, m_nameSpace, calculateStdDev(circle_R));
	    myclient->setDatapoint(datapointName_circle_RMS_stddev, m_nameSpace, calculateStdDev(circle_RMS));
	    
	    // Push the image here
	    std::chrono::steady_clock::time_point begin_getimage = std::chrono::steady_clock::now();
	    // April 2025 Remove the publication of the image to save bandwidth
	    //TODO REMOVE ME Jul 2027
      published_image = myimage.GetImageToPublish(currentDateTime());
	    myclient->setDatapoint(datapointName_image, m_nameSpace, published_image);
	    myclient->setDatapoint(datapointName_nImagesGet, m_nameSpace, (int)i_images_taken);
	    std::chrono::steady_clock::time_point end_getimage = std::chrono::steady_clock::now();
	    LOG_IMAGE << "Camera::StartCDM(): Time difference [Get image for publishing] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_getimage - begin_getimage).count() << "[ms]" << std::endl;
	    
	    // TODO: Delete DatapointThreads or make an object on stack! Or just use setDatapoint if it is quick enough.
	    
	    circle_x.clear();
	    circle_y.clear();
	    circle_R.clear();
	    circle_RMS.clear();
	    
	    displacement_x.clear();
	    displacement_y.clear();
	    rotation.clear();
	    
	    LED_x.clear();
	    LED_y.clear();
	    OARL_x.clear();
	    OARL_y.clear();
	    
	    OARL_mean_x.clear();
	    OARL_mean_y.clear();
	    
	    timestamp_UTC.clear();
	    timestamp_epoch.clear();
	    
	    std::chrono::steady_clock::time_point end_publish = std::chrono::steady_clock::now();
	    LOG_IMAGE << "Camera::StartCDM(): Time difference [Publishing results] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_publish - begin_publish).count() << "[ms]" << std::endl;
	    
	    // Write settings information to log file.
	    LOG_SETTINGS	      
	      << Camera::get_exposure() << " "
	      << Camera::get_master_gain() << " "
	      << Camera::get_temperature_value() << " "
	      << Camera::get_temperature_status() << " "
        //TODO add more information
	      << endl
	      << endl; //
	  } //if i_images_taken % array_size...
	//} //IS_SUCCESS

	
        std::chrono::steady_clock::time_point end_loop = std::chrono::steady_clock::now();
        //LOG_INFO << "Time difference [One loop] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop).count() << "[ms]" << std::endl;
	
        LOG_IMAGE << "Camera::StartCDM(): Time difference [One loop after image] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop_after_image).count() << "[ms]" << std::endl;
	
	
    } // while (b_keep_taking == 1)

    //stop acquisition
    LOG_INFO<<"Camera::startCDM exited loop, stopping acquisition"<<std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);
    m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);
    
    LOG_INFO<<"Camera::startCDM flush buffers and release them"<<std::endl;
    //Flush and delete data buffers
    m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
    for (const auto& buffer : m_DatastreamPtr->AnnouncedBuffers())
      m_DatastreamPtr->RevokeBuffer(buffer);
    
    // Free the OpenCV memory?
    // Free the allocated memories

    LOG_TRACE << "Camera::StartCDM(): End"<< endl;
    return 0;
}

int Camera::StartSG(DataAccessClientOPCUA *myclient) {
   LOG_TRACE << "Camera::StartSG(): Start" << endl;

  // is camera connected
  if (m_DevicePtr == nullptr) {
    LOG_ERROR << "Camera::StartSG camera is not connected" << std::endl;
    return -1;
  }

  b_keep_taking = 1;

  unsigned long int i_images_taken = 0;
  ////char *pcImageMemory_arr[n_allocated_memories];
  // int nMemoryId_arr[n_allocated_memories];

  // retrieve sleep duration from config
  int delay_ms=std::stoi(m_config["sg_image_delay_ms"]);

  // Setup for sw trigger configuration
  m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")
      ->SetCurrentEntry("Continuous");
  m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")
      ->SetCurrentEntry("ExposureStart");
  m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")
      ->SetCurrentEntry("On");
  m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")
      ->SetCurrentEntry("Software");

  int payload_size =
      m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")
          ->Value();
  LOG_INFO << "Camera::StartSG payload size: " << payload_size << std::endl;
  size_t num_buf_min = m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
  // check requested number of images
  if (num_buf_min > n_allocated_memories) {
    LOG_ERROR << "Camera::StartSG: number of allocated memories less than "
                 "number of buffers needed ("
              << num_buf_min << ") so increasing number to min" << std::endl;
    n_allocated_memories = num_buf_min;
  } // if (num_buf_min>n_allocated_memories)

  LOG_INFO << "Camera::StartSG auto allocating data buffers" << std::endl;
  // automatically alloc raw buffers
  for (size_t count = 0; count < n_allocated_memories; count++) {
    auto buffer = m_DatastreamPtr->AllocAndAnnounceBuffer(
        static_cast<size_t>(payload_size), nullptr);
    m_DatastreamPtr->QueueBuffer(buffer);
  }

  LOG_INFO << "Camera::StartSG prepare for main loop" << std::endl;

  int loop_image_count = 0;
  int64_t duration_count = 0;

  // April 2025 Remove the publication of the image to save bandwidth
  // vector<uchar> published_image;

  /*
  vector<double> circle_x;
  vector<double> circle_y;
  vector<double> circle_R;
  vector<double> circle_RMS;

  vector<double> displacement_x;
  vector<double> displacement_y;
  vector<double> rotation;

  vector<vector<double>> LED_x;
  vector<vector<double>> LED_y;
  vector<vector<double>> OARL_x;
  vector<vector<double>> OARL_y;

  vector<double> OARL_mean_x;
  vector<double> OARL_mean_y;
  */

  vector<string> timestamp_UTC;
  vector<string> timestamp_epoch;

  LOG_INFO << "Camera::StartSG start SW trigger acquisition" << std::endl;
  m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default,
                                    PEAK_INFINITE_NUMBER);
  m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(1);
  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")
      ->Execute();

  while (b_keep_taking == 1) {
    std::chrono::steady_clock::time_point begin_loop =
        std::chrono::steady_clock::now();

    /*
    double circle_stddev_R;
    double circle_stddev_RMS;
    double circle_stddev_x;
    double circle_stddev_y;

    vector<double> circle_results;
    vector<double> displacement_results;
    vector<double> led_x_results;
    vector<double> led_y_results;
    vector<double> oarl_x_results;
    vector<double> oarl_y_results;
    vector<double> oarl_mean_results;
    */

    // Use is_LockSeqBuf when processing image?
    LOG_INFO << "Camera::StartSG loop: wait for image and process, image "
             << loop_image_count << std::endl;

    // trigger
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->WaitUntilDone();
    
    m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(10000);
    LOG_INFO << "Camera::StartCDM loop: image acquired, start process "
	     << std::endl;
    

    std::chrono::steady_clock::time_point begin_loop_after_image =
        std::chrono::steady_clock::now();

    timestamp_UTC.push_back(currentDateTimeMs());
    timestamp_epoch.push_back(currentEpochTime());

    auto tp_start = std::chrono::high_resolution_clock::now();

    // process data ...
    cv::Mat src, dst;
    if (iBitsPerPixel == 8)
      src = cv::Mat(m_roi_height, m_roi_width, CV_8UC1, static_cast<uint8_t*>(m_ImgbufferPtr->BasePtr()));
    else
      src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, static_cast<uint16_t *>(m_ImgbufferPtr->BasePtr()));

    //rotate image, camera is upside down
    rotate(src, src, ROTATE_180);


    LOG_INFO<<"Camera::StartSG Prepare FITS file"<<std::endl;
    std::string imageName = writeFITSImage(src);
    LOG_INFO<<"Camera::StartSG FITS file ready in "<<imageName<<std::endl;

    // move to remote location
    // RR: legacy code, replace with DataBroker
    char exec[300];
    sprintf(exec,
            "scp %s "
            "ccddev@10.200.100.102:/fefs/onsite/data/aux/lst1/cdm/SG_images/",
            imageName.c_str());
    int scp_result = system(exec);
    if (scp_result == 0) {
      LOG_INFO<<"Camera::StartSG FITS file succesfully transferred"<<std::endl;
      std::remove(imageName.c_str()); // deletes the file
    } else {
      LOG_ERROR<<"Camera::StartSG could not transfer FITS file"<<std::endl;      
    }
  

    auto tp_stop = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop -
                                                                    tp_start);
    duration_count += ms.count();

    if (++loop_image_count == 100) {
      LOG_INFO << "Camera::StartSG(): Duration: "
               << duration_count / loop_image_count << std::endl;
      loop_image_count = 0;
      duration_count = 0;
    } // if (++loop_image_count == 100)

    // is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
    i_images_taken++;

    // TODO: Optimize this?
    /*
    LOG_DATA
      //cout
      << setprecision(10)
      << helper.get_Zenith() << " "
      << helper.get_Azimuth() << " "
      << circle_results[0] * px2arcsec << " "
      << circle_results[1] * px2arcsec << " "
      << circle_results[2] * px2arcsec << " "
      << circle_results[3] * px2arcsec << " "
      << oarl_mean_results[0] * px2arcsec << " "
      << oarl_mean_results[1] * px2arcsec << " "
      << displacement_results[0] * px2arcsec << " "
      << displacement_results[1] * px2arcsec << " "
      << displacement_results[2] * px2arcsec << " "
      << led_x_results[0] * px2arcsec << " "
      << led_x_results[1] * px2arcsec << " "
      << led_x_results[2] * px2arcsec << " "
      << led_x_results[3] * px2arcsec << " "
      << led_x_results[4] * px2arcsec << " "
      << led_x_results[5] * px2arcsec << " "
      << led_x_results[6] * px2arcsec << " "
      << led_x_results[7] * px2arcsec << " "
      << led_x_results[8] * px2arcsec << " "
      << led_x_results[9] * px2arcsec << " "
      << led_x_results[10] * px2arcsec << " "
      << led_x_results[11] * px2arcsec << " "
      << led_y_results[0] * px2arcsec << " "
      << led_y_results[1] * px2arcsec << " "
      << led_y_results[2] * px2arcsec << " "
      << led_y_results[3] * px2arcsec << " "
      << led_y_results[4] * px2arcsec << " "
      << led_y_results[5] * px2arcsec << " "
      << led_y_results[6] * px2arcsec << " "
      << led_y_results[7] * px2arcsec << " "
      << led_y_results[8] * px2arcsec << " "
      << led_y_results[9] * px2arcsec << " "
      << led_y_results[10] * px2arcsec << " "
      << led_y_results[11] * px2arcsec << " "
      << oarl_x_results[0] * px2arcsec << " "
      << oarl_x_results[1] * px2arcsec << " "
      << oarl_y_results[0] * px2arcsec << " "
      << oarl_y_results[1] * px2arcsec
      << endl; //
    */

    /*
    // TODO: Time this process!
    if (i_images_taken % array_size == 0) {
      std::chrono::steady_clock::time_point begin_publish =
          std::chrono::steady_clock::now();

      LOG_IMAGE << "Camera::StartCDM(): Gathered " << array_size
                << " images:" << endl;

      // TODO: Time this transpose!
      //  We transpose the arrays so instead of grouping it by time first we
      //  group it by LEDs/OARLs first. So before we had a rows for one
      //  timeslice containing different LED information and after the transpose
      //  we have rows for each LED containing information for different
      //  timeslices.

      LED_x = transpose(LED_x);
      LED_y = transpose(LED_y);
      OARL_x = transpose(OARL_x);
      OARL_y = transpose(OARL_y);

      int m_nameSpace = 2;

      // TODO: Time this publishing!
      myclient->setDatapoint(datapointName_circle_x, m_nameSpace, circle_x);
      myclient->setDatapoint(datapointName_circle_y, m_nameSpace, circle_y);
      myclient->setDatapoint(datapointName_circle_R, m_nameSpace, circle_R);
      myclient->setDatapoint(datapointName_circle_RMS, m_nameSpace, circle_RMS);

      myclient->setDatapoint(datapointName_displacement_x, m_nameSpace,
                             displacement_x);
      myclient->setDatapoint(datapointName_displacement_y, m_nameSpace,
                             displacement_y);
      myclient->setDatapoint(datapointName_rotation, m_nameSpace, rotation);

      for (int i = 0; i < nLED; i++) {
        myclient->setDatapoint(datapointName_LED_x_arrays[i], m_nameSpace,
                               LED_x[i]);
        myclient->setDatapoint(datapointName_LED_y_arrays[i], m_nameSpace,
                               LED_y[i]);
      } // for (int i = 0; i < nLED; i++)

      for (int i = 0; i < nOARL; i++) {
        myclient->setDatapoint(datapointName_OARL_x_arrays[i], m_nameSpace,
                               OARL_x[i]);
        myclient->setDatapoint(datapointName_OARL_y_arrays[i], m_nameSpace,
                               OARL_y[i]);
      } // for (int i = 0; i < nOARL; i++)

      // Publish the value
      myclient->setDatapoint(datapointName_OARL_x_mean, m_nameSpace,
                             OARL_mean_x);
      myclient->setDatapoint(datapointName_OARL_y_mean, m_nameSpace,
                             OARL_mean_y);

      myclient->setDatapoint(datapointName_timestamp_UTC, m_nameSpace,
                             timestamp_UTC);
      myclient->setDatapoint(datapointName_timestamp_epoch, m_nameSpace,
                             timestamp_epoch);

      // Publish stddev values
      myclient->setDatapoint(datapointName_circle_x_stddev, m_nameSpace,
                             calculateStdDev(circle_x));
      myclient->setDatapoint(datapointName_circle_y_stddev, m_nameSpace,
                             calculateStdDev(circle_y));
      myclient->setDatapoint(datapointName_circle_R_stddev, m_nameSpace,
                             calculateStdDev(circle_R));
      myclient->setDatapoint(datapointName_circle_RMS_stddev, m_nameSpace,
                             calculateStdDev(circle_RMS));

      // Push the image here
      std::chrono::steady_clock::time_point begin_getimage =
          std::chrono::steady_clock::now();
      // April 2025 Remove the publication of the image to save bandwidth
      // published_image = myimage.GetImageToPublish(currentDateTime());
      // myclient->setDatapoint(datapointName_image, m_nameSpace,
      // published_image);
      myclient->setDatapoint(datapointName_nImagesGet, m_nameSpace,
                             (int)i_images_taken);
      std::chrono::steady_clock::time_point end_getimage =
          std::chrono::steady_clock::now();
      LOG_IMAGE
          << "Camera::StartCDM(): Time difference [Get image for publishing] = "
          << std::chrono::duration_cast<std::chrono::milliseconds>(
                 end_getimage - begin_getimage)
                 .count()
          << "[ms]" << std::endl;

      // TODO: Delete DatapointThreads or make an object on stack! Or just use
      // setDatapoint if it is quick enough.

      circle_x.clear();
      circle_y.clear();
      circle_R.clear();
      circle_RMS.clear();

      displacement_x.clear();
      displacement_y.clear();
      rotation.clear();

      LED_x.clear();
      LED_y.clear();
      OARL_x.clear();
      OARL_y.clear();

      OARL_mean_x.clear();
      OARL_mean_y.clear();

      timestamp_UTC.clear();
      timestamp_epoch.clear();

      std::chrono::steady_clock::time_point end_publish =
          std::chrono::steady_clock::now();
      LOG_IMAGE << "Camera::StartCDM(): Time difference [Publishing results] = "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       end_publish - begin_publish)
                       .count()
                << "[ms]" << std::endl;

      // Write settings information to log file.
      LOG_SETTINGS << helper.get_Zenith() << " " << helper.get_Azimuth() << " "
                   << helper.get_LEDs_state() << " " << helper.get_OARL_state()
                   << " " << helper.get_Shutter_state() << " "
                   << helper.get_SIS_state() << " "
                   << helper.get_Drive_status_in_motion() << " "
                   << helper.get_Drive_status_parked() << " "
                   << helper.get_Drive_status_in_parking_position() << " "
                   << helper.get_Drive_status_tracking_in_progress() << " "
                   << helper.get_StarName() << " "

                   << Camera::get_exposure() << " " << Camera::get_master_gain()
                   << " " << Camera::get_temperature_value() << " "
                   << Camera::get_temperature_status()
                   << " "
                   // Add FPS, Pixel format etc. here

                   << helper.get_Aux_status_DM_East_Bottom() << " "
                   << helper.get_Aux_status_DM_East_Top() << " "
                   << helper.get_Aux_status_DM_West_Bottom() << " "
                   << helper.get_Aux_status_DM_West_Top()
                   << " "

                   //<< helper.get_Comment() << " "

                   << endl
                   << endl; //
    } // if i_images_taken % array_size...
    //} //IS_SUCCESS
    */

    std::chrono::steady_clock::time_point end_loop =
        std::chrono::steady_clock::now();
    // LOG_INFO << "Time difference [One loop] = " <<
    // std::chrono::duration_cast<std::chrono::milliseconds>(end_loop -
    // begin_loop).count() << "[ms]" << std::endl;

    LOG_IMAGE << "Camera::StartSG(): Time difference [One loop after image] = "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_loop - begin_loop_after_image)
                     .count()
              << "[ms]" << std::endl;

    // sleep delay
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  } // while (b_keep_taking == 1)

  // stop acquisition
  LOG_INFO << "Camera::startSG exited loop, stopping acquisition" << std::endl;
  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")
      ->Execute();
  m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(0);
  m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);

  LOG_INFO << "Camera::startSG flush buffers and release them" << std::endl;
  // Flush and delete data buffers
  m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
  for (const auto &buffer : m_DatastreamPtr->AnnouncedBuffers())
    m_DatastreamPtr->RevokeBuffer(buffer);

  // Free the OpenCV memory?
  // Free the allocated memories

  LOG_TRACE << "Camera::StartSG(): End" << endl;
  return 0;
}

int Camera::StartStream(DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::StartStream(): Start"<<endl;

    //is camera connected
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Camera::StartStream camera is not connected"<<std::endl;
      return -1;
    }
    
    b_keep_taking = 1;

    unsigned long int i_images_taken = 0;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];

    // Setup for freerun configuration
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")->SetCurrentEntry("Continuous");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("Off");
    int payload_size =m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();
    LOG_INFO<<"Camera::StartStream payload size: "<<payload_size<<std::endl;
    size_t num_buf_min=m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
    //check requested number of images
    if (num_buf_min>n_allocated_memories) {
      LOG_ERROR<<"Camera::StartStream: number of allocated memories less than number of buffers needed ("
	       <<num_buf_min
	       <<") so increasing number to min"<<std::endl;
      n_allocated_memories=num_buf_min;
    } //if (num_buf_min>n_allocated_memories)
  
    LOG_INFO<<"Camera::StartStream auto allocating data buffers"<<std::endl;
    //automatically alloc raw buffers
    for (size_t count=0; count<n_allocated_memories; count++) {
      auto buffer=m_DatastreamPtr->AllocAndAnnounceBuffer(static_cast<size_t>(payload_size),nullptr);
      m_DatastreamPtr->QueueBuffer(buffer);
    }

    LOG_INFO<<"Camera::StartStream prepare for main loop"<<std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    Mat m1;
    // April 2025 Remove the publication of the image to save bandwidth
    // vector<uchar> published_image;

    LOG_INFO<<"Camera::StartStream start freerun acquisition"<<std::endl;
    m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default, peak::core::DataStream::INFINITE_NUMBER);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();
    

    while (b_keep_taking == 1)
    {

      LOG_INFO<<"Camera::StartStream loop: wait for image and process, image "<<loop_image_count<<std::endl;

      std::chrono::steady_clock::time_point begin_loop = std::chrono::steady_clock::now();
      
      try {
	m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(1000);
	LOG_INFO<<"Camera::StartStream loop: image acquired, start process "<<std::endl;
      }
      // RR TODO manage exceptions
      catch (const peak::core::TimeoutException& e)
	{
	  LOG_ERROR<<"Camera::startStream timeout in WaitForFinishedBuffer"<<std::endl;
	  LOG_ERROR<<e.what()<<std::endl;
	}
      catch (std::exception& e)
	{
	  LOG_ERROR<<"Camera::startStream exception in WaitForFinishedBuffer"<<std::endl;
	  LOG_ERROR<<e.what()<<std::endl;
	  //RR: terminate? at least while debugging
	}
      std::chrono::steady_clock::time_point begin_loop_after_image = std::chrono::steady_clock::now();

      if (iBitsPerPixel == 8)
	m1 = cv::Mat(m_roi_height, m_roi_width, CV_8UC1, (uchar *)m_ImgbufferPtr->BasePtr());
      
      else if (iBitsPerPixel == 16)
	m1 = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
      
      else
	{
	  LOG_ERROR << "Camera::StartStream(): Check bitdepth!" << endl;
	  m1 = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	}

      ImageAnalysis myimage(m1, m_config, "Horizontal", 1, iBitsPerPixel);

      LOG_INFO<<"Camera::StartStream loop: free buffer after ImageAnalysis"<<std::endl;
      // queue buffer so that it can be used again
      m_DatastreamPtr->QueueBuffer(m_ImgbufferPtr);
      
      i_images_taken++;


      
      //int m_nameSpace = 2;
      // April 2025 Remove the publication of the image to save bandwidth
      // published_image = myimage.GetImageToPublish(currentDateTime());
      // myclient->setDatapoint(datapointName_image, m_nameSpace, published_image);

  
      std::chrono::steady_clock::time_point end_loop = std::chrono::steady_clock::now();
      
      LOG_IMAGE << "Camera::StartStream(): Time difference [One loop after image] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop_after_image).count() << "[ms]" << std::endl;

	    
    } // while (b_keep_taking == 1)

    // Free the OpenCV memory?
    // Free the allocated memories

    //stop acquisition
    LOG_INFO<<"Camera::startStream exited loop, stopping acquisition"<<std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);
    m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);
    
    LOG_INFO<<"Camera::startStream flush buffers and release them"<<std::endl;
    //Flush and delete data buffers
    m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
    for (const auto& buffer : m_DatastreamPtr->AnnouncedBuffers())
      m_DatastreamPtr->RevokeBuffer(buffer);
    
     LOG_TRACE << "Camera::StartStream(): End"<< endl;
     return 0;
}
  

vector<std::string> Camera::GetMultipleImages(int n_images, DataAccessClientOPCUA *myclient,DataPointFinder* finder)
{
    LOG_TRACE << "Camera::GetMultipleImages(): Start"<<endl;
    LOG_TRACE << "Camera::GetMultipleImages(): Number of images to be taken "<<n_images<<endl;

    //is camera connected
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Camera::GetMultipleImages camera is not connected"<<std::endl;
      return {};
    }

    double Az_deg=0,Alt_deg=0;
    finder->getDatapointL1("azimuth_position", Az_deg);
    finder->getDatapointL1("zenithangle_position", Alt_deg);

    LOG_WARNING<< "Camera::GetMultipleImages(): Azimuth = "<<Az_deg<<std::endl;
    LOG_WARNING<< "Camera::GetMultipleImages(): Zenith = "<<Alt_deg<<std::endl;
    b_keep_taking = 1;

    vector<std::string> v_image_paths;
    int i_images_taken = 0;
    int n_allocated_memories = 100;

    // Setup for freerun configuration
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")
      ->SetCurrentEntry("Continuous");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")
      ->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")
      ->SetCurrentEntry("On");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")
      ->SetCurrentEntry("Software");

    int payload_size =
      m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")
      ->Value();
    LOG_INFO << "Camera::GetMultipleImages payload size: " << payload_size << std::endl;
    size_t num_buf_min = m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
    // check requested number of images
    if (num_buf_min > n_allocated_memories) {
      LOG_ERROR << "Camera::GetMultipleImages: number of allocated memories less than "
	"number of buffers needed ("
		<< num_buf_min << ") so increasing number to min" << std::endl;
      n_allocated_memories = num_buf_min;
    } // if (num_buf_min>n_allocated_memories)

    LOG_INFO << "Camera::GetMultipleImages auto allocating data buffers" << std::endl;
    // automatically alloc raw buffers
    for (size_t count = 0; count < n_allocated_memories; count++) {
      auto buffer = m_DatastreamPtr->AllocAndAnnounceBuffer(static_cast<size_t>(payload_size), nullptr);
      m_DatastreamPtr->QueueBuffer(buffer);
    }

    LOG_INFO<<"Camera::GetMultipleImages prepare for main loop"<<std::endl;

    m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default,
				      PEAK_INFINITE_NUMBER);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(1);
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")
      ->Execute();
    
    int loop_image_count = 0;
    int64_t duration_count = 0;

    while ((i_images_taken < n_images) && (b_keep_taking == 1))
    {
        // Use is_LockSeqBuf when processing image?
        LOG_TRACE << "Camera::GetMultipleImages(): Number of images taken "<<i_images_taken+1<<" over "<<n_images<<endl;
        try {
	  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->Execute();
	  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->WaitUntilDone();

          m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(3000);
	  LOG_INFO<<"Camera::GetMultipleImages loop: image acquired"<<std::endl;
	}
	// RR TODO manage exceptions
	catch (const peak::core::TimeoutException& e)
	  {
	    LOG_ERROR<<"Camera::GetMultipleImages timeout in WaitForFinishedBuffer"<<std::endl;
	    LOG_ERROR<<e.what()<<std::endl;
	  }
	catch (std::exception& e)
	  {
	    LOG_ERROR<<"Camera::GetMultipleImages exception in WaitForFinishedBuffer"<<std::endl;
	    LOG_ERROR<<e.what()<<std::endl;
	    //RR: terminate? at least while debugging
	  }
	
	auto tp_start = std::chrono::high_resolution_clock::now();
	Mat src, dst;

	if (iBitsPerPixel == 8)
	  src = cv::Mat(m_roi_height, m_roi_width, CV_8UC1, (uchar *)m_ImgbufferPtr->BasePtr());
	
	else if (iBitsPerPixel == 16)
	  src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	
	else if (iBitsPerPixel == 12)
	  {
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	    src = 16 * src;
	  }
	
	else if (iBitsPerPixel == 10)
	  {
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	    src = 64 * src;
	  }
	
	else
	  {
	    LOG_ERROR << "Camera::GetMultipleImages(): Check bitdepth!" << endl;
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	  }

	
	// Transpose + Flip = 90 deg rotation
	transpose(src, src);
	flip(src, src, 1);

	std::vector<int> compression_params;
	compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	resize(src, dst, cv::Size(0, 0), 0.15, 0.15, cv::INTER_AREA);
	
	vector<unsigned char> data;
	cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
	
	int m_nameSpace = 2;
	string temString = datapointName_image;
	//getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
	SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint
	
	SetDatapointThread *m_SetDatapointThread_nImages = new SetDatapointThread(myclient,datapointName_nImagesGet, 2, i_images_taken + 1); //Updates the number of images taken

	std::string imageName = writeFITSImage(src,1,finder);

	v_image_paths.push_back(imageName);
	SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient,datapointName_imageName, 2, imageName.c_str()); //Updates the imageName

	auto tp_stop = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
	duration_count += ms.count();
	
	if (++loop_image_count == 100)
	  {
	    LOG_INFO << "Camera::GetMultipleImages(): Duration: " << duration_count / loop_image_count << std::endl;
	    loop_image_count = 0;
	    duration_count = 0;
	  }
    

	i_images_taken++;

    } //while ((i_images_taken < n_images) && (b_keep_taking == 1))

    // Free the OpenCV memory?
    // Free the allocated memories

    LOG_INFO << "Camera::GetMultipleImages() exited loop, stopping acquisition" << std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")
      ->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(0);
    m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);
    
    LOG_INFO << "Camera::GetMultipleImages() flush buffers and release them" << std::endl;
    // Flush and delete data buffers
    m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
    for (const auto &buffer : m_DatastreamPtr->AnnouncedBuffers())
      m_DatastreamPtr->RevokeBuffer(buffer);       
    
    
    LOG_TRACE << "Camera::GetMultipleImages(): End"<<endl;

    // TODO: also publish the images without saving to disk first
    // TODO: also publish the vector of image paths

    return v_image_paths;
}

vector<std::string> Camera::GetMultipleImagesStacked(int n_images, DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::GetMultipleImagesStacked(): Start"<<endl;
    LOG_TRACE << "Camera::GetMultipleImagesStacked(): Number of images to be taken "<<n_images<<endl;

    //is camera connected
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Camera::MultipleImagesStacked camera is not connected"<<std::endl;
      return {};
    }
    
    b_keep_taking = 1;

    cv::Mat accumulated_images = cv::Mat::zeros(m_roi_width, m_roi_height, CV_64FC1); // contains accumulated images. Height and width are reversed as the camera images are rotated 90 deg after taking.

    vector<std::string> v_image_paths;
    int i_images_taken = 0;
    int n_allocated_memories = 10;

    // Setup for freerun configuration
        m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")
      ->SetCurrentEntry("Continuous");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")
      ->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")
      ->SetCurrentEntry("On");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")
      ->SetCurrentEntry("Software");
    
    int payload_size =m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();
    LOG_INFO<<"Camera::GetMultipleImagesStacked payload size: "<<payload_size<<std::endl;
    size_t num_buf_min=m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
    //check requested number of images
    if (num_buf_min>n_allocated_memories) {
      LOG_ERROR<<"Camera::GetMultipleImagesStacked: number of allocated memories less than number of buffers needed ("
	       <<num_buf_min
	       <<") so increasing number to min"<<std::endl;
      n_allocated_memories=num_buf_min;
    } //if (num_buf_min>n_allocated_memories)
  
    LOG_INFO<<"Camera::GetMultipleImagesStacked auto allocating data buffers"<<std::endl;
    //automatically alloc raw buffers
    for (size_t count=0; count<n_allocated_memories; count++) {
      auto buffer=m_DatastreamPtr->AllocAndAnnounceBuffer(static_cast<size_t>(payload_size),nullptr);
      m_DatastreamPtr->QueueBuffer(buffer);
    }

    LOG_INFO << "Camera::GetMultipleImagesStacked prepare for main loop"
             << std::endl;
    m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default,
				      PEAK_INFINITE_NUMBER);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(1);
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")
      ->Execute();    

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while ((i_images_taken < n_images) && (b_keep_taking == 1))
    {
        // Use is_LockSeqBuf when processing image?
        LOG_TRACE << "Camera::GetMultipleImagesStacked(): Number of images taken "<<i_images_taken+1<<" over "<<n_images<<endl;

        try {
	  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->Execute();
	  m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->WaitUntilDone();
          
	  m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(1000);
	  LOG_INFO<<"Camera::GetMultipleImagesStacked loop: image acquired, start process "<<std::endl;
	}
	// RR TODO manage exceptions
	catch (const peak::core::TimeoutException& e)
	  {
	    LOG_ERROR<<"Camera::GetMultipleImagesStacked timeout in WaitForFinishedBuffer"<<std::endl;
	    LOG_ERROR<<e.what()<<std::endl;
	  }
	catch (std::exception& e)
	  {
	    LOG_ERROR<<"Camera::GetMultipleImagesStacked exception in WaitForFinishedBuffer"<<std::endl;
	    LOG_ERROR<<e.what()<<std::endl;
	    //RR: terminate? at least while debugging
	  }
	auto tp_start = std::chrono::high_resolution_clock::now();
	Mat src, dst;
	
	if (iBitsPerPixel == 8)
	  src = cv::Mat(m_roi_height, m_roi_width, CV_8UC1, (uchar *)m_ImgbufferPtr->BasePtr());
	
	else if (iBitsPerPixel == 16)
	  src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	
	else if (iBitsPerPixel == 12)
	  {
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	    src = 16 * src;
	  }
	
	else if (iBitsPerPixel == 10)
	  {
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	    src = 64 * src;
	  }
	
	else
	  {
	    LOG_ERROR << "Camera::GetMultipleImagesStacked(): Check bitdepth!" << endl;
	    src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, (uint16_t *)m_ImgbufferPtr->BasePtr());
	  }
	
	LOG_INFO<<"Camera::GetMultipleImagesStacked loop: free buffer after cv::Mat"<<std::endl;
	// queue buffer so that it can be used again
	m_DatastreamPtr->QueueBuffer(m_ImgbufferPtr);
      
	
	// Transpose + Flip = 90 deg rotation
	transpose(src, src);
	flip(src, src, 1);

	std::vector<int> compression_params;
	compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	resize(src, dst, cv::Size(0, 0), 0.15, 0.15, cv::INTER_AREA);
	
	vector<unsigned char> data;
	cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
	
	int m_nameSpace = 2;
	string temString = datapointName_image;
	//getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
	SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint
	
	SetDatapointThread *m_SetDatapointThread_nImages = new SetDatapointThread(myclient, datapointName_nImagesGet, 2, i_images_taken + 1); //Updates the number of images taken
	
	// Accumulate images. In OpenCV_v2 input has to be 8bit or 32bit?
	//Conversion from CV_32 to CV_64 should be automatic (TODO: verify)
	//src.convertTo(src, CV_32FC1);
	cv::accumulate(src, accumulated_images);

	auto tp_stop = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
	duration_count += ms.count();

	if (++loop_image_count == 100)
	  {
	    LOG_INFO << "Camera::GetMultipleImagesStacked(): Duration: " << duration_count / loop_image_count << std::endl;
	    loop_image_count = 0;
	    duration_count = 0;
	  }

            i_images_taken++;

    } //while ((i_images_taken < n_images) && (b_keep_taking == 1))
	    
    LOG_INFO << "Camera::GetMultipleImagesStacked(): Images taken: " << i_images_taken << endl;
    
    LOG_INFO << "Camera::GetMultipleImages() exited loop, stopping acquisition" << std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")
      ->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
      ->SetValue(0);
    m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);

    
    // Now convert, save and publish the image
    if (iBitsPerPixel == 16)
        accumulated_images.convertTo(accumulated_images, CV_16UC1, 1. / i_images_taken);
    else if (iBitsPerPixel == 8)
        accumulated_images.convertTo(accumulated_images, CV_8UC1, 1. / i_images_taken);

    // Publish the final image
    Mat accumulated_images_dst;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0);
    resize(accumulated_images, accumulated_images_dst, cv::Size(0, 0), 0.15, 0.15, cv::INTER_AREA);
    vector<unsigned char> data;
    cv::imencode(".png", accumulated_images_dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
    int m_nameSpace = 2;
    string temString = datapointName_image;
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

    // Make a FITS image
    std::string imageName = writeFITSImage(accumulated_images, i_images_taken);


    v_image_paths.push_back(imageName);
    SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, datapointName_imageName, 2, imageName.c_str()); //Updates the imageName

    // Free the OpenCV memory?
    // Free the allocated memories

    LOG_INFO<<"Camera::GetMultipleImagesStacked flush buffers and release them"<<std::endl;
    //Flush and delete data buffers
    m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
    for (const auto& buffer : m_DatastreamPtr->AnnouncedBuffers())
      m_DatastreamPtr->RevokeBuffer(buffer);
 

    LOG_TRACE << "Camera::GetMultipleImagesStacked(): End" << endl;

    return v_image_paths;
}

// TODO: these multiple methods are redundant. Use only 1.
void Camera::StopGetMultipleImages()
{
    LOG_TRACE << "Camera::StopGetMultipleImages(): Start"<<endl;
    b_keep_taking = 0;
}

int Camera::StopCDM()
{
    LOG_TRACE << "Camera::StopCDM()"<<endl;
    b_keep_taking = 0;
    return 0;
}

int Camera::StopSG()
{
    LOG_TRACE << "Camera::StopSG()"<<endl;
    b_keep_taking = 0;
    return 0;
}

int Camera::StopStream()
{
    LOG_TRACE << "Camera::StopStream(): Start"<<endl;
    b_keep_taking = 0;
    return 0;
}

void Camera::GetImage(DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::GetImage(): Start"<<endl;

    //is camera connected
    if (m_DevicePtr==nullptr) {
      LOG_ERROR<<"Camera::GetImage camera is not connected"<<std::endl;
      return;
    }
    
    // configure everything to grab one single image, then cleanup
    // this will take order of 0.5 s
    LOG_INFO<<"Camera::GetImage setup acquisition, single frame"<<std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")->SetCurrentEntry("SingleFrame"); 
    // Here without ExposureStart trigger, replace it by software or hardware trigger if necessary
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("Off");  
    //std::shared_ptr<peak::core::NodeMap> nodemap_ds=datastream->NodeMaps().at(0);
    LOG_INFO<<"Camera::GetImage find payload size"<<std::endl;
    int payload_size =m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();
    LOG_INFO<<"Camera::GetImage find minimum number of buffers needed"<<std::endl;
    size_t num_buf_min=m_DatastreamPtr->NumBuffersAnnouncedMinRequired();
    LOG_INFO<<"Camera::GetImage payload size: "<<payload_size<<", min no of mem buffers: "<<num_buf_min<<std::endl;
  
    LOG_INFO<<"Camera::GetImage allocating data buffers"<<std::endl;
    //automatically alloc raw buffers
    for (size_t count=0; count<num_buf_min; count++) {
      auto buffer=m_DatastreamPtr->AllocAndAnnounceBuffer(static_cast<size_t>(payload_size),nullptr);
      m_DatastreamPtr->QueueBuffer(buffer);
    }

    LOG_INFO<<"Camera::GetImage acquiring image"<<std::endl;
    m_DatastreamPtr->StartAcquisition(peak::core::AcquisitionStartMode::Default, 1);
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();
    
    //wait for data for 1s
    m_ImgbufferPtr = m_DatastreamPtr->WaitForFinishedBuffer(1000);

    LOG_INFO<<"Camera::GetImage image ready, processing image size "<<m_roi_width<<"x"<<m_roi_height<<std::endl;
    // process data ...
    cv::Mat src, dst;
    if (iBitsPerPixel == 8)
      src = cv::Mat(m_roi_height, m_roi_width, CV_8UC1,
                    static_cast<uint8_t *>(m_ImgbufferPtr->BasePtr()));
    else
      src = cv::Mat(m_roi_height, m_roi_width, CV_16UC1, static_cast<uint16_t *>(m_ImgbufferPtr->BasePtr()));

    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0);
    resize(src, dst, cv::Size(0, 0), 0.15, 0.15, cv::INTER_AREA);
    vector<unsigned char> data;
    cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint

    //RR temp
    //cv::imwrite("./img.png",,compression_params);
    
    int m_nameSpace = 2;
    string temString = datapointName_image;
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

    LOG_INFO<<"Prepare FITS file"<<std::endl;
    std::string imageName = writeFITSImage(src);
    LOG_INFO<<"FITS file ready in "<<imageName<<std::endl;
    
    /*
    //REMOVE COPY BY CDM AND CHANGE THE FILE PUSH ON THE DATABROKER
    std::string filePath = helper.get_fitsPath() + imageName;
    std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

    char exec[300];
    sprintf(exec, "scp -o StrictHostKeyChecking=no %s %s", filePath.c_str(), m_config["OUT_FITS_BENDING"].c_str());
    // sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
    LOG_DEBUG << "Command is: " << exec << endl;
    int scp_result = system(exec);
    LOG_DEBUG << "Output of scp is: " << scp_result << endl;
    if (scp_result == 0)
    {
        std::remove(filePath.c_str()); // deletes the file from the NUC if the file was copied succesfuly
    }
    else
    {
        LOG_ERROR << "There was a problem while copying the image!" << endl;
        remoteImagePath = "Error";
    }
    */

    std::vector<std::string> publish_remoteImagePath; 
    publish_remoteImagePath.push_back(imageName.c_str());
    SetDatapointThread *m_SetDatapointThread_remote_path = new SetDatapointThread(myclient, datapointName_imagePath, 2, publish_remoteImagePath); //Updates the imagePath
    LOG_INFO<<"GetImage SetDatapointThread for "<<datapointName_imageName<< " at "<<datapointName_imagePath<<std::endl;
    SetDatapointThread(myclient, datapointName_imageName, 2, imageName.c_str()); //Updates the imageName
    SetDatapointThread(myclient, datapointName_imagePath, 2, imageName.c_str()); //Updates the imagePath_cat

    LOG_INFO<<"Camera::GetImage: relinquish buffer and stop acquisition "<<std::endl;
    // queue buffer so that it can be used again
    m_DatastreamPtr->QueueBuffer(m_ImgbufferPtr);

    //stop acquisition
    m_NodemapPtr->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")->Execute();
    m_NodemapPtr->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);
    m_DatastreamPtr->StopAcquisition(peak::core::AcquisitionStopMode::Default);

    LOG_INFO<<"Camera::GetImage: flush and revoke img buffers"<<std::endl;
    m_DatastreamPtr->Flush(peak::core::DataStreamFlushMode::DiscardAll);
    for (const auto& buffer : m_DatastreamPtr->AnnouncedBuffers())
      m_DatastreamPtr->RevokeBuffer(buffer);


    LOG_INFO << "Camera::GetImage end"<<endl;
}
void Camera::setPixelClock(int nPixelClock, std::vector<boost::any> &return_values)
{
    float pclock = nPixelClock * 1e6f;
    try
    {
        m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("DeviceClockFrequency")->SetValue(pclock);
    }
    catch (const peak::core::BadAccessException &e)
    {
        LOG_INFO << "Camera::setPixelClock(): DeviceClockFrequency not writable "
                    "(pixel clock not configurable on this model)" << std::endl;
    }

    pclock = m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("DeviceClockFrequency")->Value();
    LOG_INFO << "Camera::setPixelClock(): actual pixel clock = " << pclock / 1e6 << " MHz" << std::endl;
    return_values.push_back(static_cast<int>(pclock / 1e6));
}

void Camera::setExposure(double exposure, std::vector<boost::any> &return_values)
{
    LOG_INFO << "Camera::setExposure(): requested = " << exposure << std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(exposure);

    double current_exposure = m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->Value();
    LOG_INFO << "Camera::setExposure(): actual = " << current_exposure << std::endl;

    Camera::exposure_setting = current_exposure;
    // Note: on pousse exposure plus tard, après fps, pour garder l'ordre original de return_values
    m_lastExposure = current_exposure; // stocker temporairement
}

void Camera::setAcquisitionMode()
{
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")->SetCurrentEntry("Continuous");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
    m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("Off");
}

void Camera::setFrameRate(double fps, std::vector<boost::any> &return_values)
{
    LOG_INFO << "Camera::setFrameRate(): requested = " << fps << std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")->SetValue(fps);

    double current_fps = m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")->Value();
    LOG_INFO << "Camera::setFrameRate(): actual = " << current_fps << std::endl;

    return_values.push_back(current_fps);
    return_values.push_back(m_lastExposure); // exposure pushed here to keep original order
}

void Camera::setGain(int gain, std::vector<boost::any> &return_values)
{
    LOG_INFO << "Camera::setGain(): requested = " << gain << std::endl;
    m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("Gain")->SetValue(gain);

    float current_gain = m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("Gain")->Value();
    LOG_INFO << "Camera::setGain(): actual = " << current_gain << std::endl;

    int gain_int = static_cast<int>(current_gain);
    return_values.push_back(gain_int);
    Camera::master_gain_setting = gain_int;
}

void Camera::setPixelFormatAndReport(std::string pixel_format, std::vector<boost::any> &return_values)
{
    if (!setPixelFormat(pixel_format))
    {
        LOG_ERROR << "Camera::setPixelFormatAndReport(): failed to configure pixel format '"
                   << pixel_format << "'" << std::endl;
        throw std::runtime_error("Camera::setPixelFormatAndReport(): unsupported pixel format: " + pixel_format);
    }

    std::string actualGenICamFormat =
        m_NodemapPtr->FindNode<peak::core::nodes::EnumerationNode>("PixelFormat")
                    ->CurrentEntry()->SymbolicValue();

    LOG_INFO << "Camera::setPixelFormatAndReport(): actual pixel format = "
             << actualGenICamFormat << std::endl;

    auto it = pixelFormatByGenICam.find(actualGenICamFormat);
    if (it == pixelFormatByGenICam.end())
    {
        LOG_ERROR << "Camera::setPixelFormatAndReport(): unknown GenICam format returned by camera: "
                   << actualGenICamFormat << std::endl;
        throw std::runtime_error("Unknown pixel format reported by camera: " + actualGenICamFormat);
    }

    return_values.push_back(it->second.interfaceName);
    iBitsPerPixel = it->second.bitsPerPixel;

    LOG_INFO << "Camera::setPixelFormatAndReport(): iBitsPerPixel = " << iBitsPerPixel << std::endl;
}

std::vector<boost::any> Camera::Configure(int nPixelClock, double exposure, double fps, int gain, string pixel_format)
{
    LOG_TRACE << "Camera::Configure(): Start" << endl;

    std::vector<boost::any> return_values;

    try
    {
        // --- Pixel clock ---
        //COND_LOG_DEBUG << "Camera::Configure(): setPixelClock " <<nPixelClock<< endl;
        //setPixelClock(nPixelClock, return_values);

        // --- Exposure ---
        COND_LOG_DEBUG << "Camera::Configure(): setExposure" << endl;
        setExposure(exposure, return_values);

        // --- Trigger / acquisition mode + FPS ---
        COND_LOG_DEBUG << "Camera::Configure():  Trigger / acquisition mode + FPS" << endl;
        setAcquisitionMode();
        setFrameRate(fps, return_values);

        // --- Gain ---
        COND_LOG_DEBUG << "Camera::Configure():  setGain" << endl;
        setGain(gain, return_values);

        // --- Pixel format ---
        COND_LOG_DEBUG << "Camera::Configure():  setPixelFormatAndReport" << endl;
        setPixelFormatAndReport(pixel_format, return_values);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Camera::Configure(): exception during configuration: " << e.what() << std::endl;
        // Selon la logique voulue : throw; ou return return_values partiel
        throw;
    }

    LOG_TRACE << "Camera::Configure(): End" << endl;
    return return_values;
}

double Camera::get_temperature_value()
{
  //LOG_INFO<<"Camera::get_temperature_value()"<<std::endl;
  // Checks if the camera is connected.
  if (m_DevicePtr==nullptr) {
    LOG_ERROR<<"Camera::get_temperature_value: camera is not connected"<<std::endl;
    return -1;
  }
  float fTemperature=m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("DeviceTemperature")->Value();
  return fTemperature;
}

string Camera::get_temperature_status()
{
  //LOG_INFO<<"Camera::get_temperature_status()"<<std::endl;
  // Checks if the camera is connected.
  if (m_DevicePtr==nullptr) {
    LOG_ERROR<<"Camera::get_temperature_status: camera is not connected"<<std::endl;
    return "Camera not connected";
  }
  
  //RR: set limits somewhere in config?

  float temp=m_NodemapPtr->FindNode<peak::core::nodes::FloatNode>("DeviceTemperature")->Value();
  string temperatureStatus = "Normal";
  if (temp>80.0) {
    temperatureStatus = "Critical";
    LOG_FATAL << "Camera: temperature status critical"<<endl;
  } else if (temp>70.0) {
    temperatureStatus = "Warning";
    LOG_WARNING << "Camera: temperature status warning"<<endl;
  } 
  return temperatureStatus;
  
}
