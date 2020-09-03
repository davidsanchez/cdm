#include "CDM.h"

using namespace std;

/* int CDM::Connect()
{
	// Starts the driver and establishes the connection to the camera
    is_InitCamera(&hCam, hWndDisplay);

    // You can query information about the sensor type used in the camera
    is_GetSensorInfo(hCam, &sInfo);

    // Saving the information about the max. image proportions in variables
    DisplayWidth = sInfo.nMaxWidth;
    DisplayHeight = sInfo.nMaxHeight;

	// if (sInfo.nColorMode == IS_COLORMODE_BAYER)
    // {
    //     // For color camera models use RGB24 mode
    //     nColorMode = IS_CM_SENSOR_RAW16;
    //     nBitsPerPixel = 16;
    // }
} */


void init_logging()
{
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_file_log
    (
        keywords::file_name = "sample.log",
		keywords::auto_flush = true, //writes messages immediately to file. Should be used only for debug.
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"
    );
	//logging::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
	logging::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");


    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::trace
    );

    logging::add_common_attributes();
}

int CDM::init(const std::string &chaine)
{ // You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
    // but becarefull, you have to call before doing  your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::init())
    // This method is automaticaly call by the program "MOS" after "MOS" server is launched but the "MOS" server is not really ready.
    // So don't use this method in ordr to communicate with the "MOS" Server.
    // you can use the afertStart() method if needed.
    int ret = 0;

    // Mandatory allways need
    printf("\n***********************************\nIn CDM::init\n***********************************\n");
    PluginsBase::init(chaine);

    init_logging();
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    return ret;
}

int CDM::afterStart()
{ // You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
    // but be careful, you have to call before doing your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::afterStart())
    // This method is automatically called by the program "MOS" after "MOS" server is launched and ready.
    int ret = 0;
    printf("\n***********************************\nIn CDM::afterStart\n***********************************\n");
    // Mandatory always need
    ret = PluginsBase::afterStart();

    // Here query the available cameras and get that information. Don't connect yet?

    //TODO: What is this doing exactly? Needed?
    if (ret != -1)
    {
        // here an example in order to call method as a client to the server
        // here call the method GetMonitoring() with 1 Input argument
        // and print the Output Argument of the method

        std::vector<std::string> *listElement = getListMonitoringRef();
        for (std::vector<std::string>::iterator it = listElement->begin(); it != listElement->end();
             it++)
        {
            printf("elementMonitoring = %s\n", it->c_str());
        }
        listElement = getListControlRef();
        for (std::vector<std::string>::iterator it = listElement->begin(); it != listElement->end();
             it++)
        {
            printf("elementControl = %s\n", it->c_str());
        } /*
 		for (std::vector<std::string>::iterator it = m_listControl.begin(); it != m_listControl.end();
                        it++) {
			printf("elementControl = %s\n",it->c_str());
 		}*/
    }

    return ret;
}

int CDM::cmdAsynch(const std::string &command, int commandStringAck, const std::string &datapointName, int nameSpace, std::string &result)
{
    // not use in this example
    int ret = 0;
    return ret;
}

int CDM::close()
{
    // here we do nothing
    int ret = 0;

    //Exit camera for example
    // ...

    // Close connection
    m_clientOpcUaRef->disconnect();

    return ret;
}

int CDM::cmd(const std::string &command, int commandStringAck, std::string &result)
{
    int ret = 0;
    printf("In CMD part: received command with the instruction: %s\n", command.c_str());
    std::string chaine = command + " ";
    std::string subChaine1 = chaine;
    std::string subChaine2 = chaine;
    int flag = 1;
    std::string::size_type pos;
    while (flag)
    {
        subChaine1 = subChaine2;
        pos = subChaine2.find(' ');   // find separator =' '
        if (pos == std::string::npos) // nothing to do ? -> exit
            flag = 0;
        else
        {
            subChaine1.erase(pos);        // find the pair name:value
            subChaine2.erase(0, pos + 1); // store the rest of the string (example the arguments of the instrution)
            if (subChaine1.compare("startAllDevice") == 0)
            {
                // userMethodStartAll(subChaine2); // name of your method who manage this action
            }
            if (subChaine1.compare("stopAllDevice") == 0)
            {
                // userMethodStopAll(); // name of your method who manage this action
            }
            if (subChaine1.compare("Connect") == 0)
            {
                BOOST_LOG_TRIVIAL(trace) << "In Connect"; //TODO: deleteme
				CDM::Connect();
            }
			if (subChaine1.compare("Disconnect") == 0)
            {
				CDM::Disconnect();
            }
			if (subChaine1.compare("Configure") == 0)
            {
				std::vector<std::string> results;
				boost::split(results, subChaine2, [](char c){return c == ' ';});		

				// TODO:get the returning string value and return it to OPCUA		
				string config_message = CDM::Configure(stoi(results[0]), stof(results[1]), stod(results[2]), stof(results[3]), stof(results[4]), results[5]);

            }
        }
    }
    // example here do nothing but wait
    //sleep(3);

    return ret;
}

string CDM::Configure(int nPixelClock, float exposure, double fps, float gain, float n_images_integrate, string pixel_format)
{
	// Set pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_SET returned " << nRet << ". tried to set pixel clock to = " << nPixelClock << std::endl;
	// Get current pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_GET returned " << nRet << ". The current pixel clock is = " << nPixelClock << std::endl;
	SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, nPixelClock);

	// Set frame rate
	double new_fps;
	nRet = is_SetFrameRate(hCam, fps, (double *)&new_fps);
	std::cout << "SetFrameRate returned " << nRet << ". New framerate = " << new_fps << std::endl;
	is_SetFrameRate(hCam, IS_GET_FRAMERATE, &fps);
    std::cout << "Applied framerate " << fps << " fps." << std::endl;
	SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, (float)fps);

	// Set exposure
	double current_exposure;
	is_Exposure (hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
    is_Exposure (hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&exposure, sizeof(exposure));
    std::cout << "Set exposure is: " << exposure << std::endl;
    is_Exposure (hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
	SetDatapointThread *m_SetDatapointThread_exposure = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, exposure);
	
	SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, pixel_format);

	//Call destructor?



	return "Message";
}

int CDM::Connect()
{
	nRet = is_InitCamera(&hCam, NULL);
    std::cout << "InitCamera returned " << nRet << std::endl;
    if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to open camera." << std::endl;
        return 1;
    }
	
	nRet = is_ResetToDefault(hCam); //Resets to default values
	if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to reset to default values." << std::endl;
        return 1;
    }

	nRet = is_GetCameraInfo(hCam, &camerainfo);
    if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to retrieve camera info." << std::endl;
    }

	nRet = is_GetSensorInfo(hCam, &sensorinfo);
    if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to retrieve sensor info." << std::endl;
    }
	std::cout << "Sensor model " << sensorinfo.strSensorName << ". Camera serial no " << camerainfo.SerNo << std::endl;

	nRet = is_AOI(hCam, IS_AOI_IMAGE_GET_AOI, (void *)&rectAOI, sizeof(rectAOI));
    if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to retrieve AOI info." << std::endl;
    }
    iWidth = rectAOI.s32Width;
    iHeight = rectAOI.s32Height;
    std::cout << "Image size is " << iWidth << "x" << iHeight << std::endl;

}

int CDM::Disconnect()
{
	// You should release the reserved images in memory here. Like OpenCV Mat and IDS images
	
	// Free the allocated buffer
	if( pcImageMemory != NULL )
		is_FreeImageMem( hCam, pcImageMemory, nMemoryId );
	pcImageMemory = NULL;

	// Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
	is_ExitCamera( hCam );
	hCam = NULL;

}

int CDM::get(const std::string &chaine, int commandStringAck, std::vector<boost::any> &tabValue)
{
    int ret = 0;
    return ret;
}

int CDM::set(const std::string &chaine, int commandStringAck, std::vector<boost::any> &tabValue)
{
    int ret = 0;
    return ret;
}

// Be careful, always need to allow to connect this Plugin with MOS
extern "C"
{
    CDM *CDM_plugin()
    {
        return new CDM();
    }
}