#include <Camera.h>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/time_facet.hpp>

#include <chrono>
#include <iostream>

#include "Helper.h"
#include "ImageAnalysis.h"
#include <CCfits>

#include "Logging.h"

extern Helper helper;
using namespace CCfits;

using namespace std;
using namespace cv;

/* 
const std::string currentDateTime()
{
    char fmt[64], buf[64];
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%06u", tm);
    snprintf(buf, sizeof buf, fmt, tv.tv_usec);
    //cout << buf << endl;
    return buf;
}

std::string currentDateTime()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

    char currentTime[84] = "";
    sprintf(currentTime, "%s:%03d", buffer, milli);
    //printf("%s \n", currentTime);
    return currentTime;
}

std::string currentDateTime()
{
    namespace pt = boost::posix_time;
    pt::ptime current_date_microseconds = pt::microsec_clock::universal_time();
    long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
    pt::time_duration current_time_milliseconds = pt::milliseconds(milliseconds);
    pt::ptime current_date_milliseconds(current_date_microseconds.date(),
                                        current_time_milliseconds);

    pt::time_facet *facet = new pt::time_facet("%Y-%m-%d %H:%M:%s");
    cout.imbue(locale(cout.getloc(), facet));
    //std::cout << "Microseconds: " << current_date_microseconds
    //         << " Milliseconds: " << current_date_milliseconds << std::endl;
    const std::string str_time = pt::to_simple_string(current_date_milliseconds);
    //std::cout << pt::to_iso_string(current_date_milliseconds) << std::endl;
    //std::cout << pt::to_iso_extended_string(current_date_milliseconds) << std::endl;

    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_date_milliseconds;
    return stream.str();
}

std::string currentDateTime()
{
    std::string isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    std::string date = isoString.substr(0, 8);
    std::string time = isoString.substr(9, 20);
    //cout << isoString << endl;
    //cout << date << " " << time << endl;
    return date + " " +  time;
}
 */

std::string currentDateTime()
{
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%S");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentDateTimeMs()
{
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%s");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentDateTimeMsFilename()
{
    using namespace boost::posix_time;
    ptime current_time = boost::posix_time::microsec_clock::universal_time();
    time_facet *facet = new time_facet("%Y%m%d_%H%M%s");
    std::stringstream stream;
    stream.imbue(std::locale(std::locale::classic(), facet));
    stream << current_time;
    return stream.str();
}

std::string currentEpochTime()
{
    unsigned long int now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    unsigned long int part1 = now / 1000;
    unsigned long int part2 = now % 1000;

    std::string result = to_string(part1) + "." + to_string(part2);
    return result;
}

string UTC_date_short()
{
    char buffer[80];
    // current date/time based on current system
    time_t now = time(0);
    // convert now to tm struct for UTC
    tm *gmtm = gmtime(&now);
    strftime(buffer, 80, "%Y%m%d", gmtm);
    //puts(buffer);

    return buffer;
}

string UTC_time_short()
{
    char buffer[80];
    // current date/time based on current system
    time_t now = time(0);
    // convert now to tm struct for UTC
    tm *gmtm = gmtime(&now);
    strftime(buffer, 80, "%H%M%S", gmtm);
    //puts(buffer);

    return buffer;
}

vector<vector<double>> transpose(vector<vector<double>> &A)
{
    LOG_TRACE << "Camera::transpose()";

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

std::string Camera::writeFITSImage(Mat image, int n_stack)
{
    LOG_TRACE << "Camera::writeFITSImage()";

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
    streamObj << helper.get_Zenith();
    streamObj << "-AZ=";
    streamObj << helper.get_Azimuth();
    streamObj << "-LED=";
    streamObj << helper.get_LED01_intensity();
    streamObj << "-OARL=";
    streamObj << helper.get_OARL_state();
    streamObj << "-parked=";
    streamObj << helper.get_Drive_status_parked();
    streamObj << "-parkingPos=";
    streamObj << helper.get_Drive_status_in_parking_position();
    streamObj << "-inMotion=";
    streamObj << helper.get_Drive_status_in_motion();
    streamObj << "-tracking=";
    streamObj << helper.get_Drive_status_tracking_in_progress();
    streamObj << "-DM=";
    streamObj << helper.get_Aux_status_DM_East_Bottom();
    streamObj << helper.get_Aux_status_DM_East_Top();
    streamObj << helper.get_Aux_status_DM_West_Bottom();
    streamObj << helper.get_Aux_status_DM_West_Top();

    if (n_stack > 1)
        streamObj << "-stack=" << n_stack;
    streamObj << ".fits.gz";

    stream_fitsPath << streamObj.str();
    stream_remoteImagePath << streamObj.str();
    std::string fileName = streamObj.str();
    std::string filePath = stream_fitsPath.str();
    std::string remoteImagePath = stream_remoteImagePath.str();

    LOG_DEBUG << "filePath: " << filePath << std::endl;
    LOG_DEBUG << "remoteImagePath: " << remoteImagePath << std::endl;

    try
    {
        if ((iBitsPerPixel == 16) || (iBitsPerPixel == 12) || (iBitsPerPixel == 10))
            pFits.reset(new FITS(filePath, USHORT_IMG, naxis, naxes)); //BYTE_IMG for 8bit, USHORT_IMG for 16bit
        else if (iBitsPerPixel == 8)
            pFits.reset(new FITS(filePath, BYTE_IMG, naxis, naxes));

        else
            LOG_ERROR << "Error invalid bitdepth value for saving!" << endl;
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

    /* 	// Mat to array 8bit
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
			//array.insert(array.end(), image.ptr<uchar>(i), image.ptr<uchar>(i)+image.cols);
			array.insert(array.end(), image.ptr<uint16_t>(i), image.ptr<uint16_t>(i) + image.cols);
		}
	} */

    if ((iBitsPerPixel == 16) || (iBitsPerPixel == 12) || (iBitsPerPixel == 10))
    {
        // Mat to array 16bit
        std::vector<uint16_t> array;
        if (image.isContinuous())
        {
            array.assign((uint16_t *)image.data, (uint16_t *)image.data + image.total());
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

    pFits->pHDU().addKey("RA_LST", helper.get_Ra_drive(), "Drive Right Ascension");
    pFits->pHDU().addKey("DEC_LST", helper.get_Dec_drive(), "Drive Declination");
    pFits->pHDU().addKey("RA_TRGT", helper.get_Ra_target(), "Target Right Ascension");
    pFits->pHDU().addKey("DEC_TRGT", helper.get_Dec_target(), "Target Declination");
    pFits->pHDU().addKey("EPOCH", "2000.0", "Epoch");
    pFits->pHDU().addKey("EQUINOX", "2000.0", "Equinox");
    //pFits->pHDU().addKey("SECPIX_SG", 18.56, "Arcsec per pixel"); TODO: Add this information for CDM

    pFits->pHDU().addKey("EXPOSURE", Camera::get_exposure(), "Total Exposure Time in miliseconds");
    pFits->pHDU().addKey("UNIXTIME", helper.unix_timestamp(), "Unix epoch time in seconds");
    pFits->pHDU().addKey("DATETIME", currentDateTime(), "UTC time");

    pFits->pHDU().addKey("LAT", 28.7573, "Latitude: Location:ORM");
    pFits->pHDU().addKey("LONG", 17.8850, "Longitude: Location:ORM");
    pFits->pHDU().addKey("ZENITH", helper.get_Zenith(), "Zenith, in degrees");
    pFits->pHDU().addKey("AZIMUTH", helper.get_Azimuth(), "Azimuth, in degrees");

    pFits->pHDU().addKey("OFFZEN", helper.get_OffsetZenith(), "Offset of Zenith, in degrees");
    pFits->pHDU().addKey("OFFAZ", helper.get_OffsetAzimuth(), "Offset of Azimuth, in degrees");
    pFits->pHDU().addKey("OBJECT", helper.get_StarName(), "Star name");
    pFits->pHDU().addKey("LEDS", helper.get_LEDs_state(), "LEDs state");
    pFits->pHDU().addKey("LED01", helper.get_LED01_intensity(), "LED01 intensity");
    pFits->pHDU().addKey("OARL", helper.get_OARL_state(), "OARL status");
    pFits->pHDU().addKey("SHUTTER", helper.get_Shutter_state(), "Shutter status");
    pFits->pHDU().addKey("SIS", helper.get_SIS_state(), "SIS status");
    pFits->pHDU().addKey("INMOTION", helper.get_Drive_status_in_motion(), "Drive status - In Motion");
    pFits->pHDU().addKey("PARKED", helper.get_Drive_status_parked(), "Drive status - Parked");
    pFits->pHDU().addKey("PARKINGP", helper.get_Drive_status_in_parking_position(), "Drive status - In Parking Position");
    pFits->pHDU().addKey("TRACKING", helper.get_Drive_status_tracking_in_progress(), "Drive status - Tracking In Progress");

    //     pFits->pHDU().addKey("GAMMA", gamma_value, "Gamma");
    pFits->pHDU().addKey("GAIN", Camera::get_master_gain(), "Gain");
    pFits->pHDU().addKey("INFO", helper.get_Comment(), "Additional image info");
    pFits->pHDU().addKey("CAMTVAL", Camera::get_temperature_value(), "Camera temperature value");
    pFits->pHDU().addKey("CAMTSTAT", Camera::get_temperature_status(), "Camera temperature status");
    pFits->pHDU().addKey("STACK", n_stack, "Number of stacked images");

    pFits->pHDU().addKey("DM_E_bot", helper.get_Aux_status_DM_East_Bottom(), "DM East Bottom");
    pFits->pHDU().addKey("DM_E_top", helper.get_Aux_status_DM_East_Top(), "DM East Top");
    pFits->pHDU().addKey("DM_W_bot", helper.get_Aux_status_DM_West_Bottom(), "DM West Bottom");
    pFits->pHDU().addKey("DM_W_top", helper.get_Aux_status_DM_West_Top(), "DM West Top");

    LOG_DEBUG << pFits->pHDU() << std::endl;

    //return remoteImagePath;
    return fileName;
}

int Camera::Connect()
{
    LOG_TRACE << "Camera::Connect()";

    nRet = is_InitCamera(&hCam, NULL);
    LOG_DEBUG << "InitCamera returned " << nRet << std::endl;
    if (nRet != IS_SUCCESS)
    {
        LOG_ERROR << "Failed to open camera." << std::endl;
        return 1;
    }

    is_SetErrorReport(hCam, IS_ENABLE_ERR_REP);
    LOG_INFO << "Set Error Report result: " << nRet << endl;

    nRet = is_ResetToDefault(hCam); //Resets to default values
    if (nRet != IS_SUCCESS)
    {
        LOG_ERROR << "Failed to reset to default values." << std::endl;
        return 1;
    }

    nRet = is_GetCameraInfo(hCam, &camerainfo);
    if (nRet != IS_SUCCESS)
    {
        LOG_ERROR << "Failed to retrieve camera info." << std::endl;
    }

    nRet = is_GetSensorInfo(hCam, &sensorinfo);
    if (nRet != IS_SUCCESS)
    {
        LOG_ERROR << "Failed to retrieve sensor info." << std::endl;
    }
    LOG_INFO << "Sensor model " << sensorinfo.strSensorName << ". Camera serial no " << camerainfo.SerNo << std::endl;

    nRet = is_AOI(hCam, IS_AOI_IMAGE_GET_AOI, (void *)&rectAOI, sizeof(rectAOI));
    if (nRet != IS_SUCCESS)
    {
        LOG_ERROR << "Failed to retrieve AOI info." << std::endl;
    }
    iWidth = rectAOI.s32Width;
    iHeight = rectAOI.s32Height;
    LOG_INFO << "Image size is " << iWidth << "x" << iHeight << std::endl;

    // Check does the camera support reporting temperature status
    INT nFeatures = 0;
    is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_GET_SUPPORTED_FEATURES, &nFeatures, sizeof(nFeatures));
    if ((nFeatures & IS_DEVICE_FEATURE_CAP_TEMPERATURE_STATUS) == IS_DEVICE_FEATURE_CAP_TEMPERATURE_STATUS)
    {
        LOG_INFO << "Camera supports monitoring of camera temperature status";
    }
}

int Camera::Disconnect()
{
    LOG_TRACE << "Camera::Disconnect()";

    // You should release the reserved images in memory here. Like OpenCV Mat and IDS images

    // Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
    is_ExitCamera(hCam);
    hCam = NULL;
}

// TODO: merge GetMultipleImages, GetMultipleImagesStacked and StartCDM into one function?

int Camera::StartCDM(DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::StartCDM()";

    /*   //
    // Temporary used for testing purposes. Delete later.
    //

    //std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/1604897067-STAR=Mothallah-EXP=49.9945-ZD=62.9105-AZ=290.7250-OFFZD=0.0000-OFFAZ=0.0000-LED=20-OARL=1.fits", Read, true));
    //std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/1608676057-STAR=-EXP=1000-ZD=62-AZ=33-LED=20-OARL=1-parked=0-parkingPos=0-inMotion=1-tracking=0.fits", Read, true));
    std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/Fake_camera_image_8bit.fits", Read, true));

    Mat m1;
    vector<uchar> published_image;
    PHDU &image = pInfile->pHDU();

    if (iBitsPerPixel == 16)
    {
        LOG_DEBUG << "Bits per pixel = 16" << endl;
        std::valarray<uint16_t> contents;
        // read all user-specifed, coordinate, and checksum keys in the image
        image.readAllKeys();
        image.read(contents);
        //std::cout << image << std::endl; // this doesn't print the data, just header info.

        long ax1(image.axis(0));
        long ax2(image.axis(1));
        LOG_DEBUG << "Ax1: " << ax1 << endl;
        LOG_DEBUG << "Ax2: " << ax2 << endl;

        std::vector<uint16_t> myvec(begin(contents), end(contents));
        Mat m2 = cv::Mat(ax2, ax1, CV_16UC1, myvec.data());
        m2.copyTo(m1);
    }

    else if (iBitsPerPixel == 8)
    {
        LOG_DEBUG << "Bits per pixel = 8" << endl;
        std::valarray<uchar> contents;
        // read all user-specifed, coordinate, and checksum keys in the image
        image.readAllKeys();
        image.read(contents);
        //std::cout << image << std::endl; // this doesn't print the data, just header info.

        long ax1(image.axis(0));
        long ax2(image.axis(1));
        LOG_DEBUG << "Ax1: " << ax1 << endl;
        LOG_DEBUG << "Ax2: " << ax2 << endl;

        std::vector<uchar> myvec(begin(contents), end(contents));
        Mat m2 = cv::Mat(ax2, ax1, CV_8UC1, myvec.data());
        m2.copyTo(m1);
    }
    else
    {
        LOG_ERROR << "Check bits per pixel. Current value not 8 or 16." << endl;
    }

    //cv::imwrite("/home/lstoperator/CDM/images/mytest.png", m1);
    LOG_DEBUG << "Rows: " << m1.rows << endl;
    LOG_DEBUG << "Cols: " << m1.cols << endl;

    // ImageAnalysis myimage(m1);
    // myimage.CalculateCircle();
    // myimage.Draw();
    // //myimage.StoreResults();
    // myimage.SaveImage("/home/lstoperator/CDM/images/mytest_8bit.png");
    // return 0;

    //
    // End of temporary block
    // */

    b_keep_taking = 1;
    int array_size = 10;

    unsigned long int i_images_taken = 0;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        LOG_DEBUG << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;
        is_AddToSequence(hCam, pcImageMemory, nMemoryId);

        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    LOG_DEBUG << "is_CaptureVideo returned " << nRet << std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    Mat m1;
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

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);
        std::chrono::steady_clock::time_point begin_loop_after_image = std::chrono::steady_clock::now();

        if (nRet == IS_SUCCESS)
        {
            timestamp_UTC.push_back(currentDateTimeMs());
            timestamp_epoch.push_back(currentEpochTime());

            auto tp_start = std::chrono::high_resolution_clock::now();

            // Mat src, dst;
            // if (iBitsPerPixel == 8)
            //     src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);
            // else if (iBitsPerPixel == 16)
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            // else if (iBitsPerPixel == 12)
            // {
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            //     src = 16 * src;
            // }
            // else if (iBitsPerPixel == 10)
            // {
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            //     src = 64 * src;
            // }
            // else
            // {
            //     cout << "Check bitdepth!" << endl;
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            // }

            // Vertical flipping of image so it is upright when read from stored old fits files.
            //ImageAnalysis myimage(m1, "Vertical", 0);

            // Flipping=Horizontal + Transpose=1 -> Rotating 90 deg clockwise
            // This is to be done for incoming camera image or Fake camera image from fits file.
            /* 
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            ImageAnalysis myimage(m1, "Horizontal", 1, iBitsPerPixel);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            LOG_INFO << "Time difference [ImageInitalisation] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl; 
            */

            if (iBitsPerPixel == 8)
                m1 = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);

            else if (iBitsPerPixel == 16)
                m1 = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);

            else
            {
                LOG_ERROR << "Check bitdepth!" << endl;
                m1 = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            }

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            // Flipping=Horizontal + Transpose=1 -> Rotating 90 deg clockwise
            // This is to be done for incoming camera image or Fake camera image from fits file.
            ImageAnalysis myimage(m1, "Horizontal", 1, iBitsPerPixel);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            LOG_INFO << "Time difference [ImageInitalisation] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

            //ImageAnalysis myimage(src);
            begin = std::chrono::steady_clock::now();
            myimage.CalculateImage();
            end = std::chrono::steady_clock::now();
            LOG_INFO << "Time difference [CalculateImage] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

            begin = std::chrono::steady_clock::now();

            circle_results = myimage.GetCircleResults();
            led_x_results = myimage.GetLEDxResults();
            led_y_results = myimage.GetLEDyResults();
            oarl_x_results = myimage.GetOARLxResults();
            oarl_y_results = myimage.GetOARLyResults();
            displacement_results = myimage.GetDisplacementResults();
            oarl_mean_results = myimage.GetOARLmeanResults();

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
            LOG_INFO << "Time difference [Getting image results] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

            // Mat src, dst;

            // if (iBitsPerPixel == 8)
            //     src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);

            // else if (iBitsPerPixel == 16)
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);

            // else if (iBitsPerPixel == 12)
            // {
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            //     src = 16 * src;
            // }

            // else if (iBitsPerPixel == 10)
            // {
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            //     src = 64 * src;
            // }

            // else
            // {
            //     cout << "Check bitdepth!" << endl;
            //     src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            // }

            // // Transpose + Flip = 90 deg rotation
            // transpose(src, src);
            // flip(src, src, 1);

            // std::vector<int> compression_params;
            // compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            // compression_params.push_back(0);
            // resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);

            // vector<unsigned char> data;
            // cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint

            // int m_nameSpace = 2;
            // string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
            // //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
            // SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

            // SetDatapointThread *m_SetDatapointThread_nImages = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, i_images_taken + 1); //Updates the number of images taken

            // std::string imageName = writeFITSImage(src);
            // std::string filePath = helper.get_fitsPath() + imageName;
            // std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

            // char exec[300];
            // sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
            // cout << "Command is: " << exec << endl;
            // int scp_result = system(exec);
            // cout << "Output of scp is: " << scp_result << endl;
            // if (scp_result == 0)
            // {
            //     std::remove(filePath.c_str()); // deletes the file from the NUC if the file was copied succesfuly
            // }
            // else
            // {
            //     cout << "There was a problem while copying the image!" << endl;
            //     remoteImagePath = "Error";
            // }

            // v_image_paths.push_back(remoteImagePath);
            // SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName); //Updates the imageName

            auto tp_stop = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
            duration_count += ms.count();

            if (++loop_image_count == 100)
            {
                LOG_INFO << "Duration: " << duration_count / loop_image_count << std::endl;
                loop_image_count = 0;
                duration_count = 0;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;

            // TODO: Optimize this?
            LOG_DATA
                //cout
                << setprecision(10)
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
                << endl
                << endl; //

            //TODO: Time this process!
            if (i_images_taken % array_size == 0)
            {
                std::chrono::steady_clock::time_point begin_publish = std::chrono::steady_clock::now();

                LOG_INFO << "Gathered " << array_size << " images:" << endl;

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
                }

                for (int i = 0; i < nOARL; i++)
                {
                    myclient->setDatapoint(datapointName_OARL_x_arrays[i], m_nameSpace, OARL_x[i]);
                    myclient->setDatapoint(datapointName_OARL_y_arrays[i], m_nameSpace, OARL_y[i]);
                }

                /* No need to calculate the OARL mean values here. 
                   They are calculated in inside the ImageAnalysis class

                // Adding values from the second vector to the first (inplace)
                std::transform(OARL_x[0].begin(), OARL_x[0].end(), OARL_x[1].begin(), OARL_x[0].begin(), std::plus<double>());
                std::transform(OARL_y[0].begin(), OARL_y[0].end(), OARL_y[1].begin(), OARL_y[0].begin(), std::plus<double>());
                // Taking an average of the previously computed value
                std::transform(OARL_x[0].begin(), OARL_x[0].end(), OARL_x[0].begin(), std::bind(std::divides<double>(), std::placeholders::_1, 2.0));
                std::transform(OARL_y[0].begin(), OARL_y[0].end(), OARL_y[0].begin(), std::bind(std::divides<double>(), std::placeholders::_1, 2.0));
                 */

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
                published_image = myimage.GetImageToPublish(currentDateTime());
                myclient->setDatapoint("Unit_CDM.AuxControl.CDM.image.image_v", m_nameSpace, published_image);
                myclient->setDatapoint("Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", m_nameSpace, (int)i_images_taken);
                std::chrono::steady_clock::time_point end_getimage = std::chrono::steady_clock::now();
                LOG_INFO << "Time difference [Get image for publishing] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_getimage - begin_getimage).count() << "[ms]" << std::endl;

                //SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, m_datapointName, m_nameSpace, circle_x);
                //delete m_SetDatapointThread; // crashes
                //SetDatapointThread m_SetDatapointThread(myclient, m_datapointName, m_nameSpace, circle_x); //Causes first 2 vector cells to have weird values! TODO: Ask JeanLuc about this.

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
                LOG_INFO << "Time difference [Publishing results] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_publish - begin_publish).count() << "[ms]" << std::endl;

                // Write settings information to log file.
                LOG_SETTINGS
                    << helper.get_Zenith() << " "
                    << helper.get_Azimuth() << " "
                    << helper.get_LED01_intensity() << " "
                    << helper.get_OARL_state() << " "
                    << helper.get_Shutter_state() << " "
                    << helper.get_SIS_state() << " "
                    << helper.get_Drive_status_in_motion() << " "
                    << helper.get_Drive_status_parked() << " "
                    << helper.get_Drive_status_in_parking_position() << " "
                    << helper.get_Drive_status_tracking_in_progress() << " "
                    << helper.get_StarName() << " "

                    << Camera::get_exposure() << " "
                    << Camera::get_master_gain() << " "
                    << Camera::get_temperature_value() << " "
                    << Camera::get_temperature_status() << " "
                    // Add FPS, Pixel format etc. here

                    << helper.get_Aux_status_DM_East_Bottom() << " "
                    << helper.get_Aux_status_DM_East_Top() << " "
                    << helper.get_Aux_status_DM_West_Bottom() << " "
                    << helper.get_Aux_status_DM_West_Top() << " "

                    //<< helper.get_Comment() << " "

                    << endl
                    << endl; //
            }
        }

        else if (nRet == IS_CAPTURE_STATUS)
        {
            LOG_WARNING << "Camera::StartCDM() / IS_CAPTURE_STATUS";

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            LOG_WARNING << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            LOG_WARNING << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            LOG_WARNING << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            LOG_WARNING << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            LOG_WARNING << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {
                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            LOG_WARNING << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }

        std::chrono::steady_clock::time_point end_loop = std::chrono::steady_clock::now();
        //LOG_INFO << "Time difference [One loop] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop).count() << "[ms]" << std::endl;

        LOG_INFO << "Time difference [One loop after image] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop_after_image).count() << "[ms]" << std::endl;

    } // while (b_keep_taking == 1)

    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    LOG_DEBUG << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    LOG_DEBUG << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    LOG_DEBUG << "is_ClearSequence: " << nRet << endl;

    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
        LOG_DEBUG << "is_FreeImageMem: " << nRet << endl;
    }

    LOG_TRACE << "Finished StartCDM" << endl;
}

int Camera::StartStream(DataAccessClientOPCUA *myclient)
{
    /*  
   LOG_TRACE << "Camera::StartStream()";

    //
    // Temporary used for testing purposes. Delete later.
    //

    //std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/1604897067-STAR=Mothallah-EXP=49.9945-ZD=62.9105-AZ=290.7250-OFFZD=0.0000-OFFAZ=0.0000-LED=20-OARL=1.fits", Read, true));
    //std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/1608676057-STAR=-EXP=1000-ZD=62-AZ=33-LED=20-OARL=1-parked=0-parkingPos=0-inMotion=1-tracking=0.fits", Read, true));
    std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/test/Fake_camera_image_8bit.fits", Read, true));

    Mat m1;
    vector<uchar> published_image;
    PHDU &image = pInfile->pHDU();

    if (iBitsPerPixel == 16)
    {
        LOG_DEBUG << "Bits per pixel = 16" << endl;
        std::valarray<uint16_t> contents;
        // read all user-specifed, coordinate, and checksum keys in the image
        image.readAllKeys();
        image.read(contents);
        //std::cout << image << std::endl; // this doesn't print the data, just header info.

        long ax1(image.axis(0));
        long ax2(image.axis(1));
        LOG_DEBUG << "Ax1: " << ax1 << endl;
        LOG_DEBUG << "Ax2: " << ax2 << endl;

        std::vector<uint16_t> myvec(begin(contents), end(contents));
        Mat m2 = cv::Mat(ax2, ax1, CV_16UC1, myvec.data());
        m2.copyTo(m1);
    }

    else if (iBitsPerPixel == 8)
    {
        LOG_DEBUG << "Bits per pixel = 8" << endl;
        std::valarray<uchar> contents;
        // read all user-specifed, coordinate, and checksum keys in the image
        image.readAllKeys();
        image.read(contents);
        //std::cout << image << std::endl; // this doesn't print the data, just header info.

        long ax1(image.axis(0));
        long ax2(image.axis(1));
        LOG_DEBUG << "Ax1: " << ax1 << endl;
        LOG_DEBUG << "Ax2: " << ax2 << endl;

        std::vector<uchar> myvec(begin(contents), end(contents));
        Mat m2 = cv::Mat(ax2, ax1, CV_8UC1, myvec.data());
        m2.copyTo(m1);
    }
    else
    {
        LOG_ERROR << "Check bits per pixel. Current value not 8 or 16." << endl;
    }

    //cv::imwrite("/home/lstoperator/CDM/images/mytest.png", m1);
    LOG_DEBUG << "Rows: " << m1.rows << endl;
    LOG_DEBUG << "Cols: " << m1.cols << endl;

    // ImageAnalysis myimage(m1);
    // myimage.CalculateCircle();
    // myimage.Draw();
    // //myimage.StoreResults();
    // myimage.SaveImage("/home/lstoperator/CDM/images/mytest_8bit.png");
    // return 0;

    //
    // End of temporary block
    // 
    */

    b_keep_taking = 1;

    unsigned long int i_images_taken = 0;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        LOG_DEBUG << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;
        is_AddToSequence(hCam, pcImageMemory, nMemoryId);

        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    LOG_DEBUG << "is_CaptureVideo returned " << nRet << std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    Mat m1;
    vector<uchar> published_image;

    while (b_keep_taking == 1)
    {

        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);
        std::chrono::steady_clock::time_point begin_loop_after_image = std::chrono::steady_clock::now();

        if (nRet == IS_SUCCESS)
        {
            // Vertical flipping of image so it is upright when read from stored old fits files.
            //ImageAnalysis myimage(m1, "Vertical", 0);

            // Flipping=Horizontal + Transpose=1 -> Rotating 90 deg clockwise
            // This is to be done for incoming camera image or Fake camera image from fits file.
            //ImageAnalysis myimage(m1, "Horizontal", 1, iBitsPerPixel);

            if (iBitsPerPixel == 8)
                m1 = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);

            else if (iBitsPerPixel == 16)
                m1 = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);

            else
            {
                LOG_ERROR << "Check bitdepth!" << endl;
                m1 = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
            }

            // Flipping=Horizontal + Transpose=1 -> Rotating 90 deg clockwise
            // This is to be done for incoming camera image or Fake camera image from fits file.
            ImageAnalysis myimage(m1, "Horizontal", 1, iBitsPerPixel);

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;

            int m_nameSpace = 2;
            published_image = myimage.GetImageToPublish(currentDateTime());
            myclient->setDatapoint("Unit_CDM.AuxControl.CDM.image.image_v", m_nameSpace, published_image);
        }

        else if (nRet == IS_CAPTURE_STATUS)
        {
            LOG_WARNING << "Camera::StartCDM() / IS_CAPTURE_STATUS";

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            LOG_WARNING << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            LOG_WARNING << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            LOG_WARNING << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            LOG_WARNING << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            LOG_WARNING << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {
                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            LOG_WARNING << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }

        std::chrono::steady_clock::time_point end_loop = std::chrono::steady_clock::now();
        //LOG_INFO << "Time difference [One loop] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop).count() << "[ms]" << std::endl;

        LOG_INFO << "Time difference [One loop after image] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_loop - begin_loop_after_image).count() << "[ms]" << std::endl;

    } // while (b_keep_taking == 1)

    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    LOG_DEBUG << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    LOG_DEBUG << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    LOG_DEBUG << "is_ClearSequence: " << nRet << endl;

    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
        LOG_DEBUG << "is_FreeImageMem: " << nRet << endl;
    }

    LOG_TRACE << "Finished StartStream" << endl;
}

vector<std::string> Camera::GetMultipleImages(int n_images, DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::GetMultipleImages()";

    b_keep_taking = 1;

    vector<std::string> v_image_paths;
    int i_images_taken = 0;
    int n_allocated_memories = 20;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        LOG_DEBUG << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;

        is_AddToSequence(hCam, pcImageMemory, nMemoryId);
        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    LOG_DEBUG << "is_CaptureVideo returned " << nRet << std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while ((i_images_taken < n_images) && (b_keep_taking == 1))
    {
        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);

        if (nRet == IS_SUCCESS)
        {
            {
                auto tp_start = std::chrono::high_resolution_clock::now();
                Mat src, dst;

                if (iBitsPerPixel == 8)
                    src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);

                else if (iBitsPerPixel == 16)
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);

                else if (iBitsPerPixel == 12)
                {
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                    src = 16 * src;
                }

                else if (iBitsPerPixel == 10)
                {
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                    src = 64 * src;
                }

                else
                {
                    LOG_ERROR << "Check bitdepth!" << endl;
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                }

                // Transpose + Flip = 90 deg rotation
                transpose(src, src);
                flip(src, src, 1);

                std::vector<int> compression_params;
                compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
                compression_params.push_back(0);
                resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);

                vector<unsigned char> data;
                cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint

                int m_nameSpace = 2;
                string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
                //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
                SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

                SetDatapointThread *m_SetDatapointThread_nImages = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, i_images_taken + 1); //Updates the number of images taken

                std::string imageName = writeFITSImage(src);
                std::string filePath = helper.get_fitsPath() + imageName;
                std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

                char exec[300];
                sprintf(exec, "scp -o StrictHostKeyChecking=no %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
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

                v_image_paths.push_back(remoteImagePath);
                SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName.c_str()); //Updates the imageName

                auto tp_stop = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
                duration_count += ms.count();

                if (++loop_image_count == 100)
                {
                    LOG_INFO << "Duration: " << duration_count / loop_image_count << std::endl;
                    loop_image_count = 0;
                    duration_count = 0;
                }
            }
            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;
        }

        else if (nRet == IS_CAPTURE_STATUS)
        {
            LOG_WARNING << "Camera::GetMultipleImages() / IS_CAPTURE_STATUS";

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            LOG_WARNING << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            LOG_WARNING << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            LOG_WARNING << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            LOG_WARNING << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            LOG_WARNING << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {
                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            LOG_WARNING << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }
    }

    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    LOG_DEBUG << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    LOG_DEBUG << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    LOG_DEBUG << "is_ClearSequence: " << nRet << endl;

    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
        LOG_DEBUG << "is_FreeImageMem: " << nRet << endl;
    }

    LOG_DEBUG << "Finished GetMultipleImages" << endl;

    // TODO: also publish the images without saving to disk first
    // TODO: also publish the vector of image paths

    return v_image_paths;
}

vector<std::string> Camera::GetMultipleImagesStacked(int n_images, DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::GetMultipleImagesStacked()";

    b_keep_taking = 1;
    cv::Mat accumulated_images = cv::Mat::zeros(iWidth, iHeight, CV_64FC1); // contains accumulated images. Height and width are reversed as the camera images are rotated 90 deg after taking.

    vector<std::string> v_image_paths;
    int i_images_taken = 0;
    int n_allocated_memories = 10;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        LOG_DEBUG << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;

        is_AddToSequence(hCam, pcImageMemory, nMemoryId);
        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    LOG_DEBUG << "is_CaptureVideo returned " << nRet << std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while ((i_images_taken < n_images) && (b_keep_taking == 1))
    {
        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);

        if (nRet == IS_SUCCESS)
        {
            {
                auto tp_start = std::chrono::high_resolution_clock::now();
                Mat src, dst;

                if (iBitsPerPixel == 8)
                    src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer);

                else if (iBitsPerPixel == 16)
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);

                else if (iBitsPerPixel == 12)
                {
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                    src = 16 * src;
                }

                else if (iBitsPerPixel == 10)
                {
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                    src = 64 * src;
                }

                else
                {
                    LOG_ERROR << "Check bitdepth!" << endl;
                    src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pBuffer);
                }

                // Transpose + Flip = 90 deg rotation
                transpose(src, src);
                flip(src, src, 1);

                std::vector<int> compression_params;
                compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
                compression_params.push_back(0);
                resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);

                vector<unsigned char> data;
                cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint

                int m_nameSpace = 2;
                string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
                //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
                SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

                SetDatapointThread *m_SetDatapointThread_nImages = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, i_images_taken + 1); //Updates the number of images taken

                // Accumulate images. In OpenCV_v2 input has to be 8bit or 32bit?
                //Conversion from CV_32 to CV_64 should be automatic (TODO: verify)
                //src.convertTo(src, CV_32FC1);
                cv::accumulate(src, accumulated_images);

                auto tp_stop = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
                duration_count += ms.count();

                if (++loop_image_count == 100)
                {
                    LOG_INFO << "Duration: " << duration_count / loop_image_count << std::endl;
                    loop_image_count = 0;
                    duration_count = 0;
                }
            }
            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;
        }

        else if (nRet == IS_CAPTURE_STATUS)
        {
            LOG_WARNING << "Camera::GetMultipleImagesStacked() / IS_CAPTURE_STATUS";

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            LOG_WARNING << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            LOG_WARNING << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            LOG_WARNING << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            LOG_WARNING << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            LOG_WARNING << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {
                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            LOG_WARNING << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            LOG_WARNING << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }

        LOG_INFO << "Images taken: " << i_images_taken << endl;
    }

    // Now convert, save and publish the image
    if (iBitsPerPixel == 16)
        accumulated_images.convertTo(accumulated_images, CV_16UC1, 1. / i_images_taken);
    else if (iBitsPerPixel == 8)
        accumulated_images.convertTo(accumulated_images, CV_8UC1, 1. / i_images_taken);

    // Publish the final image
    Mat accumulated_images_dst;
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0);
    resize(accumulated_images, accumulated_images_dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);
    vector<unsigned char> data;
    cv::imencode(".png", accumulated_images_dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
    int m_nameSpace = 2;
    string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

    // Make a FITS image
    std::string imageName = writeFITSImage(accumulated_images, i_images_taken);
    std::string filePath = helper.get_fitsPath() + imageName;
    std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

    char exec[300];
    sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
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

    v_image_paths.push_back(remoteImagePath);
    SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName.c_str()); //Updates the imageName

    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    LOG_DEBUG << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    LOG_DEBUG << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    LOG_DEBUG << "is_ClearSequence: " << nRet << endl;

    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
        LOG_DEBUG << "is_FreeImageMem: " << nRet << endl;
    }

    LOG_DEBUG << "Finished GetMultipleImages" << endl;

    return v_image_paths;
}

// TODO: these multiple methods are redundant. Use only 1.
void Camera::StopGetMultipleImages()
{
    LOG_TRACE << "Camera::StopGetMultipleImages()";
    b_keep_taking = 0;
}

int Camera::StopCDM()
{
    LOG_TRACE << "Camera::StopCDM()";
    b_keep_taking = 0;
}

int Camera::StopStream()
{
    LOG_TRACE << "Camera::StopStream()";
    b_keep_taking = 0;
}

void Camera::GetImage(DataAccessClientOPCUA *myclient)
{
    LOG_TRACE << "Camera::GetImage()";

    nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
    LOG_DEBUG << "Status is_AllocImageMem" << nRet;
    //Activate memory for storing
    nRet = is_SetImageMem(hCam, pcImageMemory, nMemoryId);
    LOG_DEBUG << "Status is_SetImageMem" << nRet;
    int nRet = is_FreezeVideo(hCam, IS_WAIT);
    LOG_DEBUG << "Status is_FreezeVideo" << nRet;

    // TODO: Add pushing image to a datapoint and making a .fits file
    // Actually make a function that processes the image when it has been taken.y

    cv::Mat src, dst;
    if (iBitsPerPixel == 8)
        src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pcImageMemory);
    else if (iBitsPerPixel == 16)
        src = cv::Mat(iHeight, iWidth, CV_16UC1, (uint16_t *)pcImageMemory);

    // Transpose + Flip = 90 deg rotation
    transpose(src, src);
    flip(src, src, 1);

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0);
    resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);

    vector<unsigned char> data;
    cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint

    int m_nameSpace = 2;
    string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(myclient, temString, m_nameSpace, data); //pushes the image to the datapoint

    std::string imageName = writeFITSImage(src);
    std::string filePath = helper.get_fitsPath() + imageName;
    std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

    char exec[300];
    sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
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

    std::vector<std::string> publish_remoteImagePath; 
    publish_remoteImagePath.push_back(remoteImagePath.c_str());
    SetDatapointThread *m_SetDatapointThread_remote_path = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, publish_remoteImagePath); //Updates the imagePath
    SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName.c_str()); //Updates the imageName
    SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, remoteImagePath.c_str()); //Updates the imagePath_cat

    // Free the allocated buffer
    if (pcImageMemory != NULL)
        is_FreeImageMem(hCam, pcImageMemory, nMemoryId);
    pcImageMemory = NULL;

    return;
}

std::vector<boost::any> Camera::Configure(int nPixelClock, double exposure, double fps, int gain, string pixel_format)
{
    LOG_TRACE << "Camera::Configure()";

    std::vector<boost::any> return_values;

    // Set pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&nPixelClock, sizeof(nPixelClock));
    LOG_INFO << "IS_PIXELCLOCK_CMD_SET returned " << nRet << ". tried to set pixel clock to = " << nPixelClock << std::endl;
    // Get current pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&nPixelClock, sizeof(nPixelClock));
    LOG_INFO << "IS_PIXELCLOCK_CMD_GET returned " << nRet << ". The current pixel clock is = " << nPixelClock << std::endl;
    return_values.push_back(nPixelClock);

    // Set frame rate
    double new_fps;
    nRet = is_SetFrameRate(hCam, fps, (double *)&new_fps);
    LOG_INFO << "SetFrameRate returned " << nRet << ". New framerate = " << new_fps << std::endl;
    is_SetFrameRate(hCam, IS_GET_FRAMERATE, &fps);
    LOG_INFO << "Applied framerate " << fps << " fps." << std::endl;
    return_values.push_back(fps);

    // Set exposure
    double current_exposure;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    LOG_INFO << "Current exposure is: " << current_exposure << std::endl;
    LOG_INFO << "Value of exposure to be set is : " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&exposure, sizeof(current_exposure));
    LOG_INFO << "Set exposure is: " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    LOG_INFO << "Current exposure is: " << current_exposure << std::endl;
    return_values.push_back(current_exposure);
    Camera::exposure_setting = current_exposure;

    // Set hardware gain
    LOG_INFO << "Gain to be set is: " << gain << std::endl;
    is_SetHardwareGain(hCam, gain, 14, 0, 32); // Master, red, green, blue
    int master_gain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
    return_values.push_back(master_gain);
    Camera::master_gain_setting = master_gain;
    LOG_INFO << "Gain set is: " << master_gain << std::endl;

    // Set Display Mode
    nRet = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    LOG_INFO << "SetDisplayMode returned " << nRet << std::endl;

    // Set Color Mode
    //TODO: depending on the chosen pixel format the iBitsPerPixel should also change.
    nRet = is_SetColorMode(hCam, pixel_formats.left.at(pixel_format));
    LOG_INFO << "SetColorMode returned " << nRet << std::endl;
    nRet = is_SetColorMode(hCam, IS_GET_COLOR_MODE);
    LOG_INFO << "GetColorMode returned " << pixel_formats.right.at(nRet) << std::endl;
    return_values.push_back(pixel_formats.right.at(nRet));

    if (pixel_format == "IS_CM_SENSOR_RAW16")
        iBitsPerPixel = 16;
    else if (pixel_format == "IS_CM_MONO8")
        iBitsPerPixel = 8;
    else
        LOG_ERROR << "Bad pixel format." << endl;

    // Setting image format
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, sizeof(formatID));
    LOG_INFO << "Status ImageFormat: " << nRet;

    //TODO: Check if the fps, exposure, pixel clock are still after pixel format setting.

    //Call destructors?

    //return "Message status"; //TODO: You should return errors here.
    return return_values;
}

double Camera::get_temperature_value()
{
    // Checks if the camera is connected.
    if (hCam != (HIDS)0)
    {
        // Get the camera temperature value
        double fTemperature = 0;
        nRet = is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_GET_TEMPERATURE,
                                (void *)&fTemperature, sizeof(fTemperature));
        LOG_INFO << "Camera temperature: " << fTemperature;
        return fTemperature;
    }

    else
    {
        LOG_DEBUG << "Camera not connected.";
        return 0;
    }
}

string Camera::get_temperature_status()
{
    // Checks if the camera is connected.
    if (hCam != (HIDS)0)
    {
        // Query the temperature status
        INT nTemperatureStatus = 0;
        string temperatureStatus = "";
        is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_GET_TEMPERATURE_STATUS, &nTemperatureStatus, sizeof(nTemperatureStatus));
        if (nTemperatureStatus == TEMPERATURE_CONTROL_STATUS_CRITICAL)
        {
            LOG_FATAL << "Temperature status: Critical";
            temperatureStatus = "Critical";
        }
        else if (nTemperatureStatus == TEMPERATURE_CONTROL_STATUS_WARNING)
        {
            LOG_WARNING << "Temperature status: Warning";
            temperatureStatus = "Warning";
        }
        else if (nTemperatureStatus == TEMPERATURE_CONTROL_STATUS_NORMAL)
        {
            LOG_INFO << "Temperature status: Normal";
            temperatureStatus = "Normal";
        }

        return temperatureStatus;
    }

    else
    {
        LOG_DEBUG << "Camera not connected.";
        return "Camera not connected";
    }
}
