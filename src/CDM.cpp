#include "CDM.h"
#include "Camera.h"

using namespace std;
//using namespace cv;

Camera camera;

void init_logging()
{
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_file_log(
        keywords::file_name = "sample.log",
        keywords::auto_flush = true, //writes messages immediately to file. Should be used only for debug.
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
    //logging::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
    logging::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace);

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

    std::string resultCall;
    m_testThread = new TestAsynchroneThread(getDataAccessClientOPCUARef());
    ret = m_testThread->startRun();

    return ret;
}

int CDM::cmdAsynch(const std::string &command, int commandStringAck, const std::string &datapointName, int nameSpace, std::string &result)
{
    cout << "Datapoint name: " << datapointName << endl;
    // not use in this example
    int ret = 0;
    result = "";
    printf("CDM::cmdAsynch\n");
    if (command.compare("CloseShutters") == 0)
    {
        m_testThread->cmdCloseShutter(datapointName, nameSpace);
    }
    if (command.compare("OpenShutters") == 0)
    {
        m_testThread->cmdOpenShutter(datapointName, nameSpace);
    }

    if (command.compare("GetMultipleImages") == 0)
    {
        m_testThread->cmdGetMultipleImages(datapointName, nameSpace);

        std::vector<string> data(5, "mytest");
        SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, data);

    }

    if (command.compare("Start") == 0)
    {
        m_testThread->cmdStart(datapointName, nameSpace);
    }

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
            subChaine2.erase(0, pos + 1); // store the rest of the string (example the arguments of the instruction)

            if (subChaine1.compare("Connect") == 0)
            {
                BOOST_LOG_TRIVIAL(trace) << "In Connect"; //TODO: deleteme
                camera.Connect();
                camera.Configure(); // Sets default parameters

                /* Standby stuff and tests
				// Checks if stanby is supported. Return 1 because it is supported.
				ULONG ulValue = IS_GET_STATUS;
				ULONG nRetu = is_CameraStatus(hCam, IS_STANDBY_SUPPORTED, ulValue);
				cout << nRetu << endl;
				// Check the status of standby. Return 0 because currently not in standby.
				ulValue = IS_GET_STATUS;
				nRetu = is_CameraStatus(hCam, IS_STANDBY, ulValue);
				cout << nRetu << endl;
				// Activates standby. Returns 0 because command was successfully executed.
				ulValue = 1;
				nRetu = is_CameraStatus(hCam, IS_STANDBY, ulValue);
				cout << nRetu << endl;	
				// Check the status of standby. Return 1 because currently in standby.
				ulValue = IS_GET_STATUS;
				nRetu = is_CameraStatus(hCam, IS_STANDBY, ulValue);
				cout << nRetu << endl;			
				// Deactivates standby. Returns 0 because command was successfully executed.
				ulValue = 0;
				nRetu = is_CameraStatus(hCam, IS_STANDBY, ulValue);
				cout << nRetu << endl;
				// Check the status of standby. Return 0 because currently not in standby.
				ulValue = IS_GET_STATUS;
				nRetu = is_CameraStatus(hCam, IS_STANDBY, ulValue);
				cout << nRetu << endl; */
            }

            if (subChaine1.compare("Disconnect") == 0)
            {
                camera.Disconnect();
            }

            if (subChaine1.compare("Configure") == 0)
            {
                //TODO: check what will happen if some of the parameters missing. The code below assumes that you received everything! Need to implement some safety guard.
                std::vector<std::string> results;
                boost::split(results, subChaine2, [](char c) { return c == ' '; });

                // TODO:get the returning string value and return it to OPCUA
                //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");
                string config_message = camera.Configure(stoi(results[0]), stod(results[1]), stod(results[2]), stoi(results[3]), results[4]);
            
                //SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, nPixelClock);

                // Put here the rest Datapoint Threads or refactor
            }

            if (subChaine1.compare("Comment") == 0)
            {
                // TODO: Make some parsing/safety checks. best inside Comment function.
                CDM::Comment(subChaine2);
            }

            if (subChaine1.compare("GetImage") == 0)
            {
                camera.GetImage();

                int m_nameSpace = 2;
                string temString = "UnitCameraM.AuxControlCameraM.image.image_v";
                //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
                //SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, data); //pushes the image to the datapoint
            }

            if (subChaine1.compare("GetMultipleImages") == 0)
            {
                // TODO: Move to async part
                // CDM::GetMultipleImages(atoi(subChaine2.c_str()));
            }

            if (subChaine1.compare("Start") == 0)
            {
                // TODO: Move to async part
                //CDM::Start();
            }

            if (subChaine1.compare("Stop") == 0)
            {
                //CDM::Stop();
            }
        }
    }
    // example here do nothing but wait
    //sleep(3);

    return ret;
}

/* int CDM::Start()
{
    cout << "Start command" << endl;

    int i_images_taken = 0;
    int n_allocated_memories = 10;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        std::cout << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;
        is_AddToSequence(hCam, pcImageMemory, nMemoryId);

        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    std::cout << "is_CaptureVideo returned " << nRet << std::endl;
    if(nRet == 0)
        m_active=1;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while (m_active==1)
    {
        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);

        if (nRet == IS_SUCCESS)
        {
            {
                auto tp_start = std::chrono::high_resolution_clock::now();

                //Mat src = cv::Mat(iHeight, iWidth, CV_8UC3, (uchar *)pBuffer); //DZ , 3*iWidth
                Mat src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer); //DZ , 3*iWidth

                auto tp_stop = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
                duration_count += ms.count();

                if (++loop_image_count == 100)
                {
                    std::cout << "Duration: " << duration_count / loop_image_count << std::endl;
                    loop_image_count = 0;
                    duration_count = 0;
                }
            }
            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;
        }
        else if (nRet == IS_CAPTURE_STATUS)
        {

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            std::cout << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            std::cout << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            std::cout << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            std::cout << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            std::cout << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {

                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                std::cout << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            std::cout << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            std::cout << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }
    }


}

int CDM::Stop()
{
    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    cout << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    cout << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    cout << "is_ClearSequence: " << nRet << endl;

    // TODO: You have to do this
    // for (int i = 0; i < n_allocated_memories; i++)
    // {
    //     nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
    //     cout << "is_FreeImageMem: " << nRet << endl;
    // }

}

int CDM::GetMultipleImages(int n_images)
{
    int i_images_taken = 0;
    int n_allocated_memories = 10;
    char *pcImageMemory_arr[n_allocated_memories];
    int nMemoryId_arr[n_allocated_memories];
    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
        std::cout << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;
        ;
        is_AddToSequence(hCam, pcImageMemory, nMemoryId);

        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    std::cout << "is_CaptureVideo returned " << nRet << std::endl;

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while (i_images_taken < n_images)
    {
        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);

        if (nRet == IS_SUCCESS)
        {
            {
                auto tp_start = std::chrono::high_resolution_clock::now();

                //Mat src = cv::Mat(iHeight, iWidth, CV_8UC3, (uchar *)pBuffer); //DZ , 3*iWidth
                Mat src = cv::Mat(iHeight, iWidth, CV_8UC1, (uchar *)pBuffer); //DZ , 3*iWidth

                auto tp_stop = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
                duration_count += ms.count();

                if (++loop_image_count == 100)
                {
                    std::cout << "Duration: " << duration_count / loop_image_count << std::endl;
                    loop_image_count = 0;
                    duration_count = 0;
                }
            }
            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
            i_images_taken++;
        }
        else if (nRet == IS_CAPTURE_STATUS)
        {

            UEYE_CAPTURE_STATUS_INFO CaptureStatusInfo;
            INT nRet2 = is_CaptureStatus(hCam, IS_CAPTURE_STATUS_INFO_CMD_GET, (void *)&CaptureStatusInfo, sizeof(CaptureStatusInfo));

            std::cout << "Total: " << CaptureStatusInfo.dwCapStatusCnt_Total << std::endl;
            std::cout << "\tDrvOutOfBuffers: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_DRV_OUT_OF_BUFFERS] << std::endl;
            std::cout << "\tApiNoDestMem:    " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_NO_DEST_MEM] << std::endl;
            std::cout << "\tApiImageLocked:  " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_API_IMAGE_LOCKED] << std::endl;
            std::cout << "\tUsbTransferFail: " << CaptureStatusInfo.adwCapStatusCnt_Detail[IS_CAP_STATUS_USB_TRANSFER_FAILED] << std::endl;

            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            if (nRet == IS_SUCCESS)

            {

                WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
                std::cout << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
            }

            is_UnlockSeqBuf(hCam, nMemoryId, pBuffer);
        }
        else
        {
            std::cout << "is_WaitForNextImage : " << nRet << std::endl;
            //	wLinkSpeed_Mb
            // The camera has the device ID 1

            UINT nDeviceId = 1;
            IS_DEVICE_INFO deviceInfo;
            memset(&deviceInfo, 0, sizeof(IS_DEVICE_INFO));
            nRet = is_DeviceInfo((HIDS)(nDeviceId | IS_USE_DEVICE_ID), IS_DEVICE_INFO_CMD_GET_DEVICE_INFO, (void *)&deviceInfo, sizeof(deviceInfo));

            WORD wLinkSpeed_Mb = deviceInfo.infoDevHeartbeat.wLinkSpeed_Mb;
            std::cout << "\twLinkSpeed_Mb: " << wLinkSpeed_Mb << std::endl;
        }
    }

    // Free the OpenCV memory?
    // Free the allocated memories

    nRet = is_StopLiveVideo(hCam, IS_FORCE_VIDEO_STOP);
    cout << "is_StopLiveVideo result: " << nRet << endl;

    nRet = is_ExitImageQueue(hCam);
    cout << "is_ExitImageQueue: " << nRet << endl;

    nRet = is_ClearSequence(hCam);
    cout << "is_ClearSequence: " << nRet << endl;

    for (int i = 0; i < n_allocated_memories; i++)
    {
        nRet = is_FreeImageMem(hCam, pcImageMemory_arr[i], nMemoryId_arr[i]);
        cout << "is_FreeImageMem: " << nRet << endl;
    }

    std::vector<string> data(5, "mytest");
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, data);
}

int CDM::GetImage()
{
    nRet = is_AllocImageMem(hCam, iWidth, iHeight, iBitsPerPixel, &pcImageMemory, &nMemoryId);
    printf("Status is_AllocImageMem %d\n", nRet);
    //Activate memory for storing
    nRet = is_SetImageMem(hCam, pcImageMemory, nMemoryId);
    printf("Status is_SetImageMem %d\n", nRet);
    int nRet = is_FreezeVideo(hCam, IS_WAIT);
    printf("Status is_FreezeVideo %d\n", nRet);

    // TODO: Add pushing image to a datapoint and making a .fits file
    // Actually make a function that processes the image when it has been taken.y

    cv::Mat src, dst;
    if (iBitsPerPixel == 8)
        src = cv::Mat(3684, 4912, CV_8UC1, (uchar *)pcImageMemory);

    // Transpose + Flip = 90 deg rotation
    transpose(src, src);
    flip(src, src, 1);

    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0);
    resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);

    vector<unsigned char> data;
    cv::imencode(".png", dst, data, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
    int m_nameSpace = 2;
    string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
    //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
    SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, data); //pushes the image to the datapoint

    // Free the allocated buffer
    if (pcImageMemory != NULL)
        is_FreeImageMem(hCam, pcImageMemory, nMemoryId);
    pcImageMemory = NULL;
}
 */
int CDM::Comment(string comment)
{
    this->comment = comment;
    SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.comment.comment_v", 2, this->comment);
}

/* string CDM::Configure(int nPixelClock, double exposure, double fps, int gain, string pixel_format)
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
    SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, fps);

    // Set exposure
    double current_exposure;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
    std::cout << "Value of exposure to be set is : " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&exposure, sizeof(current_exposure));
    std::cout << "Set exposure is: " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
    SetDatapointThread *m_SetDatapointThread_exposure = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, current_exposure);

    // Set hardware gain
    is_SetHardwareGain(hCam, gain, 14, 0, 32); // Master, red, green, blue
    int master_gain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
    SetDatapointThread *m_SetDatapointThread_gain = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.gain.gain_v", 2, master_gain);

    // Set Display Mode
    nRet = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    std::cout << "SetDisplayMode returned " << nRet << std::endl;

    // Set Color Mode
    nRet = is_SetColorMode(hCam, pixel_formats.left.at(pixel_format));
    std::cout << "SetColorMode returned " << nRet << std::endl;
    nRet = is_SetColorMode(hCam, IS_GET_COLOR_MODE);
    std::cout << "GetColorMode returned " << pixel_formats.right.at(nRet) << std::endl;
    SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, pixel_formats.right.at(nRet));

    // Setting image format
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, sizeof(formatID));
    printf("Status ImageFormat %d\n", nRet);

    //TODO: Check if the fps, exposure, pixel clock are still after pixel format setting.

    //Call destructors?

    return "Message status"; //TODO: You should return errors here.
}
 */

/* int CDM::Connect()
{
    nRet = is_InitCamera(&hCam, NULL);
    std::cout << "InitCamera returned " << nRet << std::endl;
    if (nRet != IS_SUCCESS)
    {
        std::cout << "Failed to open camera." << std::endl;
        return 1;
    }

    is_SetErrorReport(hCam, IS_ENABLE_ERR_REP);
    cout << "Set Error Report result: " << nRet << endl;

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

    // Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
    is_ExitCamera(hCam);
    hCam = NULL;
}
 */

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