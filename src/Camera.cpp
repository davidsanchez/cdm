#include <Camera.h>

#include <iostream>
#include <chrono>

using namespace std;
using namespace cv;


int Camera::Connect()
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

int Camera::Disconnect()
{
    // You should release the reserved images in memory here. Like OpenCV Mat and IDS images

    // Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
    is_ExitCamera(hCam);
    hCam = NULL;
}

int Camera::Start()
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

int Camera::Stop()
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

int Camera::GetMultipleImages(int n_images)
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
}

int Camera::GetImage()
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
    string temString = "UnitCameraM.AuxControlCameraM.image.image_v";
    //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
    //SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, data); //pushes the image to the datapoint

    // Free the allocated buffer
    if (pcImageMemory != NULL)
        is_FreeImageMem(hCam, pcImageMemory, nMemoryId);
    pcImageMemory = NULL;
}

string Camera::Configure(int nPixelClock, double exposure, double fps, int gain, string pixel_format)
{
    // Set pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_SET returned " << nRet << ". tried to set pixel clock to = " << nPixelClock << std::endl;
    // Get current pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_GET returned " << nRet << ". The current pixel clock is = " << nPixelClock << std::endl;
    //SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, nPixelClock);

    // Set frame rate
    double new_fps;
    nRet = is_SetFrameRate(hCam, fps, (double *)&new_fps);
    std::cout << "SetFrameRate returned " << nRet << ". New framerate = " << new_fps << std::endl;
    is_SetFrameRate(hCam, IS_GET_FRAMERATE, &fps);
    std::cout << "Applied framerate " << fps << " fps." << std::endl;
    //SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, fps);

    // Set exposure
    double current_exposure;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
    std::cout << "Value of exposure to be set is : " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&exposure, sizeof(current_exposure));
    std::cout << "Set exposure is: " << exposure << std::endl;
    is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&current_exposure, sizeof(current_exposure));
    std::cout << "Current exposure is: " << current_exposure << std::endl;
    //SetDatapointThread *m_SetDatapointThread_exposure = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, current_exposure);

    // Set hardware gain
    is_SetHardwareGain(hCam, gain, 14, 0, 32); // Master, red, green, blue
    int master_gain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
    //SetDatapointThread *m_SetDatapointThread_gain = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.gain.gain_v", 2, master_gain);

    // Set Display Mode
    nRet = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    std::cout << "SetDisplayMode returned " << nRet << std::endl;

    // Set Color Mode
    nRet = is_SetColorMode(hCam, pixel_formats.left.at(pixel_format));
    std::cout << "SetColorMode returned " << nRet << std::endl;
    nRet = is_SetColorMode(hCam, IS_GET_COLOR_MODE);
    std::cout << "GetColorMode returned " << pixel_formats.right.at(nRet) << std::endl;
    //SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, pixel_formats.right.at(nRet));

    // Setting image format
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, sizeof(formatID));
    printf("Status ImageFormat %d\n", nRet);

    //TODO: Check if the fps, exposure, pixel clock are still after pixel format setting.

    //Call destructors?

    return "Message status"; //TODO: You should return errors here.
}

