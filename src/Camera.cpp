#include <Camera.h>

#include <chrono>
#include <iostream>

#include "Helper.h"
#include "ImageAnalysis.h"
#include <CCfits>

extern Helper helper;
using namespace CCfits;

using namespace std;
using namespace cv;

std::string Camera::writeFITSImage(Mat image, string img_info)
{
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
    streamObj << "-STAR=";
    streamObj << helper.get_StarName();
    streamObj << "-EXP=";
    streamObj << std::setprecision(0);
    streamObj << Camera::get_exposure();
    streamObj << "-ZD=";
    streamObj << helper.get_Zenith();
    streamObj << "-AZ=";
    streamObj << helper.get_Azimuth();
    // streamObj << "-OFFZD=";
    // streamObj << helper.get_OffsetZenith();
    // streamObj << "-OFFAZ=";
    // streamObj << helper.get_OffsetAzimuth();
    streamObj << "-LED=";
    streamObj << helper.get_LED_intensity();
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


    if (img_info != "")
        streamObj << "-" << img_info;
    streamObj << ".fits";

    stream_fitsPath << streamObj.str();
    stream_remoteImagePath << streamObj.str();
    std::string fileName = streamObj.str();
    std::string filePath = stream_fitsPath.str();
    std::string remoteImagePath = stream_remoteImagePath.str();

    std::cout << "filePath: " << filePath << std::endl;
    std::cout << "remoteImagePath: " << remoteImagePath << std::endl;

    try
    {
        if ((iBitsPerPixel == 16) || (iBitsPerPixel == 12) || (iBitsPerPixel == 10))
            pFits.reset(new FITS(filePath, USHORT_IMG, naxis, naxes)); //BYTE_IMG for 8bit, USHORT_IMG for 16bit
        else if (iBitsPerPixel == 8)
            pFits.reset(new FITS(filePath, BYTE_IMG, naxis, naxes));

        else
            cout << "Error invalid bitdepth value for saving!" << endl;
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
        cout << "Check pixel format" << endl;

    pFits->pHDU().addKey("RA", helper.get_RA(), "Right Ascension");
    pFits->pHDU().addKey("DEC", helper.get_DEC(), "Declination");
    pFits->pHDU().addKey("EPOCH", "2000.0", "Epoch");
    pFits->pHDU().addKey("EQUINOX", "2000.0", "Equinox");
    //pFits->pHDU().addKey("SECPIX_SG", 18.56, "Arcsec per pixel"); TODO: Add this information for CDM

    pFits->pHDU().addKey("EXPOSURE", Camera::get_exposure(), "Total Exposure Time in miliseconds");
    pFits->pHDU().addKey("TIME", helper.unix_timestamp(), "Unix epoch time in seconds");
    pFits->pHDU().addKey("UTC", helper.UTC_time(), "UTC time");

    pFits->pHDU().addKey("LAT", 28.7573, "Latitude: Location:ORM");
    pFits->pHDU().addKey("LONG", 17.8850, "Longitude: Location:ORM");
    pFits->pHDU().addKey("ZENITH", helper.get_Zenith(), "Zenith, in degrees");
    pFits->pHDU().addKey("AZIMUTH", helper.get_Azimuth(), "Azimuth, in degrees");

    //pFits->pHDU().addKey("OFFZEN", CDM::get_OffsetZenith(), "Offset of Zenith, in degrees");
    //pFits->pHDU().addKey("OFFAZ", CDM::get_OffsetAzimuth(), "Offset of Azimuth, in degrees");
    pFits->pHDU().addKey("OBJECT", helper.get_StarName(), "Star name");
    pFits->pHDU().addKey("LED", helper.get_LED_intensity(), "LED01 intensity");
    pFits->pHDU().addKey("OARL", helper.get_OARL_state(), "OARL status");
    pFits->pHDU().addKey("INFO", img_info, "Additional image info");
    pFits->pHDU().addKey("INMOTION", helper.get_Drive_status_in_motion(), "Drive status - In Motion");
    pFits->pHDU().addKey("PARKED", helper.get_Drive_status_parked(), "Drive status - Parked");
    pFits->pHDU().addKey("PARKINGP", helper.get_Drive_status_in_parking_position(), "Drive status - In Parking Position");
    pFits->pHDU().addKey("TRACKING", helper.get_Drive_status_tracking_in_progress(), "Drive status - Tracking In Progress");
    pFits->pHDU().addKey("GAIN", Camera::get_master_gain(), "Gain");
    //     pFits->pHDU().addKey("GAMMA", gamma_value, "Gamma");
    pFits->pHDU().addKey("COMMENT", helper.get_Comment(), "Gamma");

    std::cout << pFits->pHDU() << std::endl;

    //return remoteImagePath;
    return fileName;
}

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

// TODO: merge GetMultipleImages, GetMultipleImagesStacked and StartCDM into one function?

int Camera::StartCDM(DataAccessClientOPCUA* myclient)
{
    cout << "StartCDM command" << endl;

    // Temporary used for testing purposes. Delete later.
    std::auto_ptr<FITS> pInfile(new FITS("/home/lstoperator/CDM/fits/1604897067-STAR=Mothallah-EXP=49.9945-ZD=62.9105-AZ=290.7250-OFFZD=0.0000-OFFAZ=0.0000-LED=20-OARL=1.fits",Read,true));
    PHDU& image = pInfile->pHDU(); 
    //std::valarray<unsigned long>  contents;
    std::valarray<uint16_t>  contents;
    // read all user-specifed, coordinate, and checksum keys in the image
    image.readAllKeys();
    image.read(contents);
    // this doesn't print the data, just header info.
    //std::cout << image << std::endl;

    long ax1(image.axis(0));
    long ax2(image.axis(1));

    // for (long j = 0; j < ax2; j+=10)
    // {
    //     std::ostream_iterator<short> c(std::cout,"\t");
    //     std::copy(&contents[j*ax1],&contents[(j+1)*ax1-1],c);
    //     std::cout << '\n';       
    // }        
    // std::cout << std::endl;

    std::vector<uint16_t> myvec(begin(contents), end(contents));
    Mat m1(ax2, ax1, CV_16UC1, myvec.data()); 
    //cv::imwrite("/home/lstoperator/CDM/images/mytest.png", m1);

    ImageAnalysis myimage(m1);
    //myimage.Draw();
    myimage.CalculateCircle();

    
    // End of temporary block


    b_keep_taking = 1;

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

    int loop_image_count = 0;
    int64_t duration_count = 0;

    while (b_keep_taking == 1)
    {
        // Use is_LockSeqBuf when processing image?

        char *pBuffer = NULL;
        nRet = is_WaitForNextImage(hCam, 1500, &pBuffer, &nMemoryId);

        if (nRet == IS_SUCCESS)
        {
            // TODO: Why is this brace here?
            {
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

    cout << "Finished StartCDM" << endl;

}

vector<std::string> Camera::GetMultipleImages(int n_images, DataAccessClientOPCUA *myclient)
{
    b_keep_taking = 1;

    vector<std::string> v_image_paths;
    int i_images_taken = 0;
    int n_allocated_memories = 20;
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
                    cout << "Check bitdepth!" << endl;
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
                sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
                cout << "Command is: " << exec << endl;
                int scp_result = system(exec);
                cout << "Output of scp is: " << scp_result << endl;
                if (scp_result == 0)
                {
                    std::remove(filePath.c_str()); // deletes the file from the NUC if the file was copied succesfuly
                }
                else
                {
                    cout << "There was a problem while copying the image!" << endl;
                    remoteImagePath = "Error";
                }

                v_image_paths.push_back(remoteImagePath);
                SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName); //Updates the imageName

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

    cout << "Finished GetMultipleImages" << endl;

    // TODO: also publish the images without saving to disk first
    // TODO: also publish the vector of image paths

    //helper.publish_datapoint("Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, 4);

    return v_image_paths;
}

vector<std::string> Camera::GetMultipleImagesStacked(int n_images, DataAccessClientOPCUA *myclient)
{
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
        std::cout << "AllocImageMem returned " << nRet << " [pcImageMemory=" << pcImageMemory << " nMemoryId=" << nMemoryId << "]" << std::endl;

        is_AddToSequence(hCam, pcImageMemory, nMemoryId);
        pcImageMemory_arr[i] = pcImageMemory;
        nMemoryId_arr[i] = nMemoryId;
    }
    is_InitImageQueue(hCam, 0);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    std::cout << "is_CaptureVideo returned " << nRet << std::endl;

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
                    cout << "Check bitdepth!" << endl;
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

        cout << "Images taken: " << i_images_taken << endl;
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
    std:string string_average = "avg=" + std::to_string(i_images_taken);
    std::string imageName = writeFITSImage(accumulated_images, string_average);
    std::string filePath = helper.get_fitsPath() + imageName;
    std::string remoteImagePath = helper.get_remoteImagePathPrefix() + imageName;

    char exec[300];
    sprintf(exec, "scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
    cout << "Command is: " << exec << endl;
    int scp_result = system(exec);
    cout << "Output of scp is: " << scp_result << endl;
    if (scp_result == 0)
    {
        std::remove(filePath.c_str()); // deletes the file from the NUC if the file was copied succesfuly
    }
    else
    {
        cout << "There was a problem while copying the image!" << endl;
        remoteImagePath = "Error";
    }

    v_image_paths.push_back(remoteImagePath);
    SetDatapointThread *m_SetDatapointThread_imageName = new SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName); //Updates the imageName

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

    cout << "Finished GetMultipleImages" << endl;

    return v_image_paths;
}

// TODO: these 2 methods are redundant. Use only 1. 
void Camera::StopGetMultipleImages()
{
    b_keep_taking = 0;
}

int Camera::StopCDM()
{
    b_keep_taking = 0;
}

void Camera::GetImage(DataAccessClientOPCUA *myclient)
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
    cout << "Command is: " << exec << endl;
    int scp_result = system(exec);
    cout << "Output of scp is: " << scp_result << endl;
    if (scp_result == 0)
    {
        std::remove(filePath.c_str()); // deletes the file from the NUC if the file was copied succesfuly
    }
    else
    {
        cout << "There was a problem while copying the image!" << endl;
        remoteImagePath = "Error";
    }

    SetDatapointThread(myclient, "Unit_CDM.AuxControl.CDM.imageName.imageName_v", 2, imageName); //Updates the imageName

    // Free the allocated buffer
    if (pcImageMemory != NULL)
        is_FreeImageMem(hCam, pcImageMemory, nMemoryId);
    pcImageMemory = NULL;

    return;
}

std::vector<boost::any> Camera::Configure(int nPixelClock, double exposure, double fps, int gain, string pixel_format)
{
    std::vector<boost::any> return_values;

    // Set pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_SET returned " << nRet << ". tried to set pixel clock to = " << nPixelClock << std::endl;
    // Get current pixel clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&nPixelClock, sizeof(nPixelClock));
    std::cout << "IS_PIXELCLOCK_CMD_GET returned " << nRet << ". The current pixel clock is = " << nPixelClock << std::endl;
    //SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, nPixelClock);
    return_values.push_back(nPixelClock);

    // Set frame rate
    double new_fps;
    nRet = is_SetFrameRate(hCam, fps, (double *)&new_fps);
    std::cout << "SetFrameRate returned " << nRet << ". New framerate = " << new_fps << std::endl;
    is_SetFrameRate(hCam, IS_GET_FRAMERATE, &fps);
    std::cout << "Applied framerate " << fps << " fps." << std::endl;
    //SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, fps);
    return_values.push_back(fps);

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
    return_values.push_back(current_exposure);
    Camera::exposure_setting = current_exposure;

    // Set hardware gain
    std::cout << "Gain to be set is: " << gain << std::endl;
    is_SetHardwareGain(hCam, gain, 14, 0, 32); // Master, red, green, blue
    int master_gain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
    //SetDatapointThread *m_SetDatapointThread_gain = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.gain.gain_v", 2, master_gain);
    return_values.push_back(master_gain);
    Camera::master_gain_setting = master_gain;
    std::cout << "Gain set is: " << master_gain << std::endl;

    // Set Display Mode
    nRet = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    std::cout << "SetDisplayMode returned " << nRet << std::endl;

    // Set Color Mode
    //TODO: depending on the chosen pixel format the iBitsPerPixel should also change.
    nRet = is_SetColorMode(hCam, pixel_formats.left.at(pixel_format));
    std::cout << "SetColorMode returned " << nRet << std::endl;
    nRet = is_SetColorMode(hCam, IS_GET_COLOR_MODE);
    std::cout << "GetColorMode returned " << pixel_formats.right.at(nRet) << std::endl;
    //SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, pixel_formats.right.at(nRet));
    return_values.push_back(pixel_formats.right.at(nRet));

    // Setting image format
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, sizeof(formatID));
    printf("Status ImageFormat %d\n", nRet);

    //TODO: Check if the fps, exposure, pixel clock are still after pixel format setting.

    //Call destructors?

    //return "Message status"; //TODO: You should return errors here.
    return return_values;
}
