// #include <Camera.h>

// Camera::Camera()
// {
//     hCam = (HIDS)0;/*  */
//     hWndDisplay = NULL; //DIB mode will be used for display
//     // Need to find out the memory size of the pixel and the colour mode
//     iColorMode = IS_CM_MONO8; //IS_CM_SENSOR_RAW16;
//     iBitsPerPixel = 8;        //16;

//     is_ResetToDefault(hCam); //This should go after camera initalisation
// }

// int Camera::Connect()
// {
//     nRet = is_InitCamera(&hCam, NULL);
//     std::cout << "InitCamera returned " << nRet << std::endl;
//     if (nRet != IS_SUCCESS)
//     {
//         std::cout << "Failed to open camera." << std::endl;
//         return 1;
//     }

//     BOOST_LOG_TRIVIAL(fatal) << "test";




// }