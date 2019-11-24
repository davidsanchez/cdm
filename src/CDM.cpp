#include <ctime>
#include <iomanip>

#include "pluginsBase.h"
#include "CDM.h"

#include <CCfits>
#include <cmath>

#include <unistd.h>

#define ELEMENT "MOS_Server.CDM.Image.Image_v"

using namespace std;
using namespace CCfits;
using namespace cv;

long int unix_timestamp()
{
	time_t t = std::time(0);
	long int now = static_cast<long int>(t);
	return now;
}

string UTC_time()
{
	// current date/time based on current system
	time_t now = time(0);
	// convert now to string form
	char *dt = ctime(&now);
	cout << "The local date and time is: " << dt << endl;

	// convert now to tm struct for UTC
	tm *gmtm = gmtime(&now);
	dt = asctime(gmtm);
	cout << "The UTC date and time is: " << dt << endl;

	return dt;
}

std::string CDM::writeImage(Mat image) //, double exposure_value) //,  double azimuth, double zenith)
{
	//TODO: should also send image data type to this method, now assume 8bit

	// if( ePixelFormat == VmbPixelFormatMono14)
	//     image=4*image; // For converting 14bit images to 16bit

	flip(image, image, 0); //Do it or not?

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
	
/* 	streamObj << std::fixed;
	streamObj << std::setprecision(4);
	streamObj << CDM::fitsPath;
	streamObj << unix_timestamp();
	streamObj << "-STAR=";
	streamObj << CDM::get_StarName();
	streamObj << "-EXP=";
	streamObj << CDM::get_exposure();
	streamObj << "-ZD=";
	streamObj << CDM::get_Zenith();
	streamObj << "-AZ=";
	streamObj << CDM::get_Azimuth();
	streamObj << "-OFFZD=";
	streamObj << CDM::get_OffsetZenith();
	streamObj << "-OFFAZ=";
	streamObj << CDM::get_OffsetAzimuth();
	streamObj << ".fits";
	std::string fileName = streamObj.str(); */

	stream_fitsPath << std::fixed << std::setprecision(4) << CDM::fitsPath;
	stream_remoteImagePath << std::fixed << std::setprecision(4) << CDM::remoteImagePathPrefix;

	streamObj << std::fixed;
	streamObj << std::setprecision(4);
	streamObj << unix_timestamp();
	streamObj << "-STAR=";
	streamObj << CDM::get_StarName();
	streamObj << "-EXP=";
	streamObj << CDM::get_exposure();
	streamObj << "-ZD=";
	streamObj << CDM::get_Zenith();
	streamObj << "-AZ=";
	streamObj << CDM::get_Azimuth();
	streamObj << "-OFFZD=";
	streamObj << CDM::get_OffsetZenith();
	streamObj << "-OFFAZ=";
	streamObj << CDM::get_OffsetAzimuth();
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
		if ((bitdepth == 16) || (bitdepth == 12) || (bitdepth == 10))
			pFits.reset(new FITS(filePath, USHORT_IMG, naxis, naxes)); //BYTE_IMG for 8bit, USHORT_IMG for 16bit
		else if (bitdepth == 8)
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

	if ((bitdepth == 16) || (bitdepth == 12) || (bitdepth == 10))
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

	else if (bitdepth == 8)
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

	pFits->pHDU().addKey("RA", CDM::get_RA(), "Right Ascension");
	pFits->pHDU().addKey("DEC", CDM::get_DEC(), "Declination");
	pFits->pHDU().addKey("EPOCH", "2000.0", "Epoch");
	pFits->pHDU().addKey("EQUINOX", "2000.0", "Equinox");
	//pFits->pHDU().addKey("SECPIX", 18.56, "Arcsec per pixel"); TODO: Add this information for CDM

	pFits->pHDU().addKey("EXPOSURE", CDM::get_exposure(), "Total Exposure Time in miliseconds");
	pFits->pHDU().addKey("TIME", unix_timestamp(), "Unix epoch time in seconds");
	pFits->pHDU().addKey("UTC", UTC_time(), "UTC time");

	pFits->pHDU().addKey("LAT", 28.7573, "Latitude: Location:ORM");
	pFits->pHDU().addKey("LONG", 17.8850, "Longitude: Location:ORM");
	pFits->pHDU().addKey("ZENITH", CDM::get_Zenith(), "Zenith, in degrees");
	pFits->pHDU().addKey("AZIMUTH", CDM::get_Azimuth(), "Azimuth, in degrees");

	pFits->pHDU().addKey("OFFZEN", CDM::get_OffsetZenith(), "Offset of Zenith, in degrees");
	pFits->pHDU().addKey("OFFAZ", CDM::get_OffsetAzimuth(), "Offset of Azimuth, in degrees");
	//pFits->pHDU().addKey("ZENITH", 0, "In degrees");
	pFits->pHDU().addKey("OBJECT", CDM::get_StarName(), "Star name");

	//     pFits->pHDU().addKey("GAIN", gain_value, "Gain");
	//     pFits->pHDU().addKey("GAMMA", gamma_value, "Gamma");

	std::cout << pFits->pHDU() << std::endl;

	// TODO: make check if scp was succesful
/* 	char exec[300];
	sprintf(exec,"scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
	//sprintf(exec,"rsync -vh %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
	cout << "Command is: " << exec << endl;
	system(exec); */

	//return remoteImagePath;
	return fileName;
}

int CDM::connectOpcUa(std::string url)
{
	int ret = 0;

	std::string pluginClass = "ptr_Plugin";
	DynamicLoader *pluginsLoader;

	std::string pluginFile = API_LIB_PATH;
	char *pPath;
	pPath = getenv("MOS_PATH");
	if (pPath != NULL)
	{
		pluginFile = pPath;
		pluginFile += "/../lib/libDataAccessClientOPCUA.so";
	}

	pluginsLoader = new DynamicLoader(pluginFile, pluginClass);
	m_clientOpcUaRef = pluginsLoader->load();
	if (m_clientOpcUaRef == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			//ret = m_clientOpcUaRef->connect(url, NULL);
			ret = m_clientOpcUaRef->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

int CDM::get(std::string chaine, int commandStringAck, std::vector<boost::any> *tabValue)
{
	int ret = 0;
	//printf("sonde jl SG::get() --> boost::any()\n");
	cout << "Get chaine is: " << chaine << endl;
	tabValue->resize(0);
	float return_value = 0;
	string str_return_value = "None";

	// if(hsensor->isOnline()) {
	// 	if(chaine.compare("getTemperature")==0)
	// 		//cout << "Current temperature: " << tsensor->get_currentValue() << " C" << endl;
	// 		return_value = tsensor->get_currentValue();
	// 	else if(chaine.compare("getPressure")==0)
	// 		//cout << "Current pressure: " << psensor->get_currentValue() << " hPa" << endl;
	// 		return_value = psensor->get_currentValue();
	// 	else if(chaine.compare("getHumidity")==0)
	// 		//cout << "Current humidity: " << hsensor->get_currentValue() << " %RH" << endl;
	// 		return_value = hsensor->get_currentValue();

	if (chaine.find("getExposure") != std::string::npos)
	{
		cout << "Exposure is: " << exposure << endl;
		return_value = exposure;
		tabValue->push_back(return_value);
	}
	else if (chaine.find("getFPS") != std::string::npos)
	{
		cout << "Framerate is: " << framerate << endl;
		return_value = framerate;
		tabValue->push_back(return_value);
	}
	else if (chaine.find("getPixelClock") != std::string::npos)
	{
		cout << "PixelClock is: " << pixelClock << endl;
		return_value = pixelClock;
		tabValue->push_back(return_value);
	}
	else if (chaine.find("getStarName") != std::string::npos)
	{
		//cout << "StarName is: " << StarName << endl;
		cout << "StarName is: " << CDM::get_StarName() << endl;
		str_return_value = CDM::get_StarName();
		tabValue->push_back(str_return_value);
	}

	else if (chaine.find("getOffsetAzimuth") != std::string::npos)
	{
		cout << "Offset of Azimuth is: " << CDM::get_OffsetAzimuth() << endl;
		return_value = CDM::get_OffsetAzimuth();
		tabValue->push_back(return_value);
	}

	else if (chaine.find("getOffsetZenith") != std::string::npos)
	{
		cout << "Offset of Zenith is: " << CDM::get_OffsetZenith() << endl;
		return_value = CDM::get_OffsetZenith();
		tabValue->push_back(return_value);
	}

	else if (chaine.find("getRA") != std::string::npos)
	{
		std::string finalnode = "Drive.DriveControl.RA.RA_v";
		int nameSpace = 2;
		//short int element; //(change with the good type of the datapoint float/int/string/.....)
		float element = 0;
		if (m_clientOpcUaRef != NULL)
			CDM::m_clientOpcUaRef->getDatapoint(finalnode, nameSpace, &element);
		cout << "RA is: " << element << endl;

		return_value = element;
		tabValue->push_back(return_value);
		RA = element;
	}

	else if (chaine.find("getDEC") != std::string::npos)
	{
		std::string finalnode = "Drive.DriveControl.Dec.Dec_v";
		int nameSpace = 2;
		//short int element; //(change with the good type of the datapoint float/int/string/.....)
		float element = 0;
		if (m_clientOpcUaRef != NULL)
			CDM::m_clientOpcUaRef->getDatapoint(finalnode, nameSpace, &element);
		cout << "DEC is: " << element << endl;
		return_value = element;
		tabValue->push_back(return_value);
		DEC = element;
	}

	else if (chaine.find("getAzimuth") != std::string::npos)
	{
		std::string finalnode = "Drive.DriveControl.CurrentPosition.azimuth_position.azimuth_position_v";
		int nameSpace = 2;
		//short int element; //(change with the good type of the datapoint float/int/string/.....)
		float element = 0;
		if (m_clientOpcUaRef != NULL)
			CDM::m_clientOpcUaRef->getDatapoint(finalnode, nameSpace, &element);
		cout << "Azimuth is: " << element << endl;

		return_value = element;
		tabValue->push_back(return_value);
		azimuth = element;
	}

	else if (chaine.find("getZenith") != std::string::npos)
	{
		std::string finalnode = "Drive.DriveControl.CurrentPosition.elevation_position.elevation_position_v";
		int nameSpace = 2;
		//short int element; //(change with the good type of the datapoint float/int/string/.....)
		float element = 0;
		if (m_clientOpcUaRef != NULL)
			CDM::m_clientOpcUaRef->getDatapoint(finalnode, nameSpace, &element);
		cout << "Zenith is: " << element << endl;

		return_value = element;
		tabValue->push_back(return_value);
		zenith = element;
	}

	else if ((chaine.find("getImage") != std::string::npos) && camera_ready)
	{
		cout << "Getting image" << endl;

		std::string remoteImagePath = getImage();

		std::ostringstream filePathT;
		filePathT << std::setprecision(0);
		filePathT << imagePath;
		filePathT << "DisplayImage_resize.png";
		std::string filePath = filePathT.str();

		//std::string filePath = "/home/lstoperator/images/OpenCV_image_resize.png";

		// FILE *p_file = NULL;
		// p_file = fopen(filePath.c_str(),"rb");
		// fseek(p_file,0,SEEK_END);
		// int size = ftell(p_file);
		// fclose(p_file);

		// std::ifstream file(filePath.c_str(),std::ios::in | std::ios::binary);
		// tabValue.resize(size);
		// std::vector<Byte> data;
		// data.resize(size);
		// file.read((char *)&data[0],tabValue.size());

		// if(string *pstr = boost::any_cast<string>(&(tabValue)[0])){
		// }
		// if(Byte *pstr = boost::any_cast<Byte>(&(tabValue)[0])){

		// 	tabValue.resize(data.size());
		// 	for (int  j=0; j<tabValue.size(); j++ )
		// 	{
		// 		tabValue[j] = (Byte) boost::any_cast <Byte> (data[j]);
		// 	}

		FILE *p_file = NULL;
		p_file = fopen(filePath.c_str(), "rb");
		fseek(p_file, 0, SEEK_END);
		int size = ftell(p_file);
		fclose(p_file);

		std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
		std::vector<Byte> data;
		data.resize(size);
		file.read((char *)&data[0], size);
		int m_nameSpace = 2;
		string temString = ELEMENT;

		//getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace,true);
		SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, data);
	
		SetDatapointThread *m_SetDatapointThread2 = new SetDatapointThread(getDataAccessClientOPCUARef(), "MOS_Server.CDM.ImagePath.ImagePath_v", m_nameSpace, remoteImagePath);

		/*		
		Mat image = imread("images/OpenCV_image_resize.png", 0);
		int m_nameSpace=2;
		string temString=ELEMENT;
	
	//	typedef unsigned char byte;	
		int size = image.total()*image.elemSize();
		std::vector<Byte> data;
		data.resize(size);
		std::memcpy((char *)&data[0], image.data, size*sizeof(Byte));		
	//

		vector<Byte> data;
		for(int i=0; i<image.rows; i++)
			for(int j=0; j<image.cols; j++)
			{
				data.push_back(*(uchar*)(image.data+ i*image.step + j));	
			}		
	
		SetDatapointThread *m_SetDatapointThread= new SetDatapointThread(getDataAccessClientOPCUARef(),temString,m_nameSpace, data );
*/
	}
	//tabValue->push_back(return_value);

	// }
	// else cout << "Sensor is not online";

	return ret;
}

int CDM::set(std::string chaine, int commandStringAck, std::vector<boost::any> tabValue)
{
	int ret = 0;
	//printf("sonde jl SG::set() --> boost::any()\n");

	cout << "Set chaine is: " << chaine << endl;

	if (chaine.find("setExposure") != std::string::npos)
	{

		const string toErase = "setExposure ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}
		setExposure(std::stod(chaine));
		cout << "Trying to set exposure to: " << chaine << endl;
	}

	if (chaine.find("setFPS") != std::string::npos)
	{
		const string toErase = "setFPS ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}
		setFramerate(std::stod(chaine));
	}

	if (chaine.find("setPixelClock") != std::string::npos)
	{
		const string toErase = "setPixelClock ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}
		setPixelClock(std::stod(chaine));
	}

	if (chaine.find("setStarName") != std::string::npos)
	{
		const string toErase = "setStarName ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}
		setStarName(chaine);
	}

	if (chaine.find("setOffsetAzimuth") != std::string::npos)
	{
		const string toErase = "setOffsetAzimuth ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}

		//setPixelClock(std::stod(chaine));
		setOffsetAzimuth(std::stod(chaine));
	}

	if (chaine.find("setOffsetZenith") != std::string::npos)
	{
		const string toErase = "setOffsetZenith ";
		size_t pos = std::string::npos;
		while ((pos = chaine.find(toErase)) != std::string::npos)
		{ // If found then erase it from string
			chaine.erase(pos, toErase.length());
		}

		//setPixelClock(std::stod(chaine));
		setOffsetZenith(std::stod(chaine));
	}

	return ret;
}

/* int SG::getImage()
{
	pCameraID = "169.254.236.66"; //"10.0.1.140";
	pFileName = "SynchronousGrab.bmp";

	AVT::VmbAPI::Examples::ApiController apiController;
        
	std::cout << "Vimba Version V" << apiController.GetVersion() << "\n";

	VmbFrameStatusType status = VmbFrameStatusIncomplete;
	err = apiController.StartUp();
	if ( VmbErrorSuccess == err )
	{
		std::string strCameraID;
		if ( NULL == pCameraID )
		{
			AVT::VmbAPI::CameraPtrVector cameras = apiController.GetCameraList();
			if ( cameras.size() <= 0 )
			{
				err = VmbErrorNotFound;
			}
			else
			{
				err = cameras[0]->GetID( strCameraID );
			}
		}
		else
		{
			strCameraID = pCameraID;
		}
		
		if ( VmbErrorSuccess == err )
		{
			std::cout << "Camera ID:" << strCameraID.c_str() << "\n\n";

			AVT::VmbAPI::FramePtr pFrame;
			err = apiController.AcquireSingleImage( strCameraID, pFrame );
			if ( VmbErrorSuccess == err )
			{
				err = pFrame->GetReceiveStatus( status );
				if (    VmbErrorSuccess == err
						&& VmbFrameStatusComplete == status )
				{
					VmbPixelFormatType ePixelFormat = VmbPixelFormatMono8;
					err = pFrame->GetPixelFormat( ePixelFormat );
					if ( VmbErrorSuccess == err )
					{
						if (    ( VmbPixelFormatMono8 != ePixelFormat )
							&&  ( VmbPixelFormatRgb8 != ePixelFormat ))
						{
							err = VmbErrorInvalidValue;
						}
						else
						{
							VmbUint32_t nImageSize = 0; 
							err = pFrame->GetImageSize( nImageSize );
							if ( VmbErrorSuccess == err )
							{
								VmbUint32_t nWidth = 0;
								err = pFrame->GetWidth( nWidth );
								if ( VmbErrorSuccess == err )
								{
									VmbUint32_t nHeight = 0;
									err = pFrame->GetHeight( nHeight );
									if ( VmbErrorSuccess == err )
									{
										VmbUchar_t *pImage = NULL;
										err = pFrame->GetImage( pImage );
										if ( VmbErrorSuccess == err )
										{
											Mat src = cv::Mat(nHeight, nWidth, CV_8UC1, (uchar*) pImage);
											imwrite("OpenCV_image.png", src);

											AVTBitmap bitmap;

											if ( VmbPixelFormatRgb8 == ePixelFormat )
											{
												bitmap.colorCode = ColorCodeRGB24;
											}
											else
											{
												bitmap.colorCode = ColorCodeMono8;
											}

											bitmap.bufferSize = nImageSize;
											bitmap.width = nWidth;
											bitmap.height = nHeight;

											// Create the bitmap
											if ( 0 == AVTCreateBitmap( &bitmap, pImage ))
											{
												std::cout << "Could not create bitmap.\n";
												err = VmbErrorResources;
											}
											else
											{
												// Save the bitmap
												if ( 0 == AVTWriteBitmapToFile( &bitmap, pFileName ))
												{
													std::cout << "Could not write bitmap to file.\n";
													err = VmbErrorOther;
												}
												else
												{
													std::cout << "Bitmap successfully written to file \"" << pFileName << "\"\n" ;
													// Release the bitmap's buffer
													if ( 0 == AVTReleaseBitmap( &bitmap ))
													{
														std::cout << "Could not release the bitmap.\n";
														err = VmbErrorInternalFault;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		apiController.ShutDown();
	}

	if ( VmbErrorSuccess != err )
	{
		std::string strError = apiController.ErrorCodeToMessage( err );
		std::cout << "\nAn error occurred: " << strError.c_str() << "\n";
	}
	if( VmbFrameStatusIncomplete == status)
	{
		std::cout<<"received frame was not complete\n";
	}

	return err;
}
 */

int CDM::setStarName(string starname)
{
	cout << "Setting name of star to: " << starname << endl;
	StarName = starname;
	return 0;
}

int CDM::setPixelClock(uint setPixelClock)
{
	// Set this pixel clock
	int nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&setPixelClock, sizeof(setPixelClock));
	printf("Status Setting Pixel clock %d\n", nRet);
	// Get current pixel clock
	nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&pixelClock, sizeof(pixelClock));
	cout << "Pixel clock is: " << pixelClock << endl;
	return 0;
}

int CDM::setOffsetAzimuth(double offset_azimuth)
{
	cout << "Setting offset of Azimuth to: " << offset_azimuth << endl;
	CDM::offset_azimuth = offset_azimuth;
	return 0;
}

int CDM::setOffsetZenith(double offset_zenith)
{
	cout << "Setting offset of Zenith to: " << offset_zenith << endl;
	CDM::offset_zenith = offset_zenith;
	return 0;
}

int CDM::setFramerate(double setFramerate)
{
	// Set FPS
	int nRet = is_SetFrameRate(hCam, setFramerate, (double *)&framerate);
	printf("Status Setting FPS %d\n", nRet);
	//framerate = newFramerate;
	//Get current FPS
	//nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&pixelClock, sizeof(pixelClock));
	cout << "Framerate is: " << framerate << endl;
	return 0;
}

int CDM::setExposure(double setExposure)
{
	double maxExposure = 0;
	//setExposure = maxExposure;
	// Set exposure
	int nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void *)&setExposure, sizeof(setExposure));
	cout << "nRet when trying to set exposure is: " << nRet << endl;
	switch (nRet)
	{
	case IS_CANT_OPEN_DEVICE:
		cout << "IS_CANT_OPEN_DEVICE" << endl;
		break;
	case IS_INVALID_MODE:
		cout << "IS_INVALID_MODE" << endl;
		break;
	case IS_INVALID_PARAMETER:
		cout << "IS_INVALID_PARAMETER" << endl;
		break;
	case IS_IO_REQUEST_FAILED:
		cout << "IS_IO_REQUEST_FAILED" << endl;
		break;
	case IS_NO_SUCCESS:
		cout << "IS_NO_SUCCESS" << endl;
		break;
	case IS_NOT_SUPPORTED:
		cout << "IS_NOT_SUPPORTED" << endl;
		break;
	case IS_TIMED_OUT:
		cout << "IS_TIMED_OUT" << endl;
		break;
	}

	if (IS_INVALID_CAMERA_HANDLE == nRet)
		cout << "Invalid camera handle" << endl;
	if (IS_SUCCESS == nRet)
		cout << "Exposure set succesfully" << endl;
	else
		cout << "Exposure not set succesfully" << endl;

	nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&exposure, sizeof(exposure));
	cout << "nRet when trying to get exposure is: " << nRet << endl;
	switch (nRet)
		if (IS_SUCCESS == nRet)
			cout << "Exposure value get succesfully" << endl;
		else
			cout << "Exposure not get succesfully" << endl;
	cout << "Exposure is: " << exposure << endl;
	//exposure=setExposure;
	//printf("Status Setting FPS %d\n",nRet);
	//framerate = newFramerate;
	// Get current FPS
	//nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&pixelClock, sizeof(pixelClock));
	//cout << "Pixel clock is: " << pixelClock  << endl;

	double min_exposure;
	double *pmin_exposure = &min_exposure;
	is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN, pmin_exposure, 8);
	cout << "Min exposure is: " << min_exposure << endl;
	cout << "Min exposure is: " << *pmin_exposure << endl;
	return 0;
}

std::string CDM::getImage()
{

	//Set pixel clock
	//UINT nPixelClockDefault = 208;
	//nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelClock, sizeof(pixelClock));
	//printf("Status is_PixelClock %d\n",nRet);
	/*
	//Color mode
	INT colorMode = IS_CM_MONO8;
  //    INT colorMode = IS_CM_BGR8_PACKED;
	int nRet = is_SetColorMode(hCam,colorMode);
	printf("Status SetColorMode %d\n",nRet);

	//UINT formatID = 36;
	//For which resolution
 	nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, 4);
 	printf("Status ImageFormat %d\n",nRet);
  //
  //      //Allocating memory
	char* pMem = NULL;
 	int memID = 0;
	nRet = is_AllocImageMem(hCam, 4912, 3684, 8, &pMem, &memID);
	printf("Status AllocImage %d\n",nRet);
  
	//Activate memory
	nRet = is_SetImageMem(hCam, pMem, memID);
 	printf("Status SetImageMem %d\n",nRet);
  //
 	//Display mode
 	INT displayMode = IS_SET_DM_DIB;
 	nRet = is_SetDisplayMode (hCam, displayMode);
 	printf("Status displayMode %d\n",nRet);
  //
  */
	//Take image
	int nRet = is_FreezeVideo(hCam, IS_WAIT);
	printf("Status is_FreezeVideo %d\n", nRet);

	// ------------------------------------------
/* 	char* pMem2=NULL;
	int memID2 = 0;
	nRet = is_AllocImageMem(hCam, 4912, 3684, 8, &pMem2, &memID2);

	BUFFER_CONVERSION_PARAMS conversionParams;
	conversionParams.nDestPixelFormat               = IS_CM_MONO8; // BGRA8_PACKED;
	conversionParams.nDestPixelConverter            = IS_CONV_MODE_NONE; // IS_CONV_MODE_SOFTWARE_3X3;
	conversionParams.nDestColorCorrectionMode       = IS_CCOR_DISABLE;
	conversionParams.nDestGamma                     = 100;
	conversionParams.nDestSaturationU               = 100;
	conversionParams.nDestSaturationV               = 100;
	conversionParams.nDestEdgeEnhancement           = 0;
	conversionParams.pSourceBuffer                 = pMem;	//NULL; //pSourceBuffer;
	conversionParams.pDestBuffer                   = pMem2; //pDestBuffer;
	
	nRet = is_Convert(hCam, IS_CONVERT_CMD_APPLY_PARAMS_AND_CONVERT_BUFFER,
						(void*)&conversionParams, sizeof(conversionParams));

	cout << "Result of conversion: " << nRet << endl; */

	// ------------------------------------------

/* 	//Read the image from some memory and save it as a file
	IMAGE_FILE_PARAMS ImageFileParams;
	ImageFileParams.pwchFileName = L"/home/lstoperator/CDM/images/Bayer16.png";
	ImageFileParams.pnImageID = NULL;   //&memID;
	ImageFileParams.ppcImageMem = NULL; //&pMem;
	ImageFileParams.nQuality = 100;
	ImageFileParams.nFileType = IS_IMG_PNG;
	nRet = is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void *)&ImageFileParams, sizeof(ImageFileParams));
	printf("Status is_ImageFile %d\n", nRet); */

	/* 	std::ostringstream fileNameT1;
	fileNameT1 << std::setprecision(0);
	fileNameT1 << imagePath;
	fileNameT1 << unix_timestamp();
	fileNameT1 << "-EXP=";
	fileNameT1 << CDM::get_exposure();
	fileNameT1 << "-AZ=";
	fileNameT1 << CDM::get_Azimuth();
	fileNameT1 << "-AL=";
	fileNameT1 << CDM::get_Zenith();	
	fileNameT1 << ".tiff";
	std::string fileName = fileNameT1.str(); */

	std::ostringstream filePathT2;
	filePathT2 << std::setprecision(0);
	filePathT2 << imagePath;
	filePathT2 << "DisplayImage_resize.png";
	std::string filePath_resize = filePathT2.str();

	Mat src, dst;

	if (bitdepth == 8)
		src = cv::Mat(3684, 4912, CV_8UC1, (uchar *)pMem);

	else if (bitdepth == 16)
		src = cv::Mat(3684, 4912, CV_16UC1, (uint16_t *)pMem);

	else if (bitdepth == 12)
	{
		src = cv::Mat(3684, 4912, CV_16UC1, (uint16_t *)pMem);
		src = 16 * src;
	}

	else if (bitdepth == 10)
	{
		src = cv::Mat(3684, 4912, CV_16UC1, (uint16_t *)pMem);
		src = 64 * src;
	}

	else
	{
		cout << "Check bitdepth!" << endl;
		src = cv::Mat(3684, 4912, CV_16UC1, (uint16_t *)pMem);
	}


	//rotate(src, src, 0); //ROTATE_90_CLOCKWISE);
	transpose(src, src); //transpose+flip = 90 deg rotation
	flip(src, src, 1);
	//imwrite(filePath, src, azimuth, zenith);

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	resize(src, dst, cv::Size(0, 0), 0.15, 0.15, CV_INTER_AREA);
	imwrite(filePath_resize, dst, compression_params);

	std::string imageName = CDM::writeImage(src);

	std::string filePath = CDM::fitsPath + imageName;
	std::string remoteImagePath = CDM::remoteImagePathPrefix + imageName;

	char exec[300];
	sprintf(exec,"scp %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
	//sprintf(exec,"rsync -vh %s drivedev@10.1.8.1:/fefs/home/lapp/CDM_Images", filePath.c_str());
	cout << "Command is: " << exec << endl;
	system(exec);

	return remoteImagePath;

	//Exit camera
	//is_ExitCamera(hCam);
}

// you can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
// but becarefull, you have to call before doing  your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::init())
//
// This method is automaticaly call by the program "MOS" after "MOS" server is launched but the "MOS" server is not really ready.
// So don't use this method in ordr to communicate with the "MOS" Server.
// you can use the afertStart() method if needed.
int CDM::init(std::string chaine)
{
	int ret = 0;

	// Mandatory allways need
	cout << "In Init" << endl;
	PluginsBase::init(chaine);
	cout << "In Init" << endl;
	printf("sonde jl SG::SG::init\n***********************************\n");
	//
	//	afterStart();
	return ret;
}

// you can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
// but becarefull, you have to call before doing  your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::afterStart())
//
// This method is automaticaly call by the program "MOS" after "MOS" server is launched and ready.
int CDM::afterStart()
{
	int ret = 0;
	cout << "Now in afterStart" << endl;
	// Mandatory allways need
	ret = PluginsBase::afterStart();
	//
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

	cout << "Now in afterStart" << endl;
	//startSG();
	//getImage();

	//Camera init
	int nRet = is_InitCamera(&hCam, NULL);
	printf("Status Init camera %d\n", nRet);
	nRet = is_SetErrorReport(hCam, IS_ENABLE_ERR_REP);
	cout << "Error report code: " << nRet << endl;

	nRet = is_SetErrorReport(hCam, IS_GET_ERR_REP_MODE);
	cout << "Error report code: " << nRet << endl;

	nRet = is_SetColorMode(hCam, colorMode);
	printf("Status SetColorMode %d\n", nRet);

	//UINT formatID = 36;
	//For which resolution
	nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, &formatID, sizeof(formatID));
	printf("Status ImageFormat %d\n", nRet);

	//Allocating memory
	//char* pMem = NULL;
	//int memID = 0;

	//nRet = is_AllocImageMem(hCam, 4912, 3684, 8, &pMem, &memID);
	nRet = is_AllocImageMem(hCam, 4912, 3684, bitdepth, &pMem, &memID);

	printf("Status AllocImage %d\n", nRet);

	//Activate memory
	nRet = is_SetImageMem(hCam, pMem, memID);
	printf("Status SetImageMem %d\n", nRet);
	//
	//Display mode
	INT displayMode = IS_SET_DM_DIB;
	nRet = is_SetDisplayMode(hCam, displayMode);
	printf("Status displayMode %d\n", nRet);

	setPixelClock(pixelClock);
	setFramerate(framerate);
	setExposure(exposure);

	camera_ready = 1;

	// Trying to access other OPCUA server
	//connectOpcUa("opc.tcp://address:port"); // example opc.tcp://lappc-f578l:48080
	int connection_result = connectOpcUa("opc.tcp://10.1.8.3:48010");

	cout << "Drive status OPCUA: " << connection_result << endl;
	cout << "After start finished!" << endl;
	return ret;
}

int CDM::cmdAsynch(std::string command, int commandStringAck, std::string datapointName, int nameSpace, std::string *result)
{
	// not use in this example
	int ret = 0;
	*result = "";
	return ret;
}

int CDM::close()
{
	// here we do noting
	int ret = 0;

	//Exit camera
	is_ExitCamera(hCam);

	// Close connection
	m_clientOpcUaRef->disconnect();

	return ret;
}

int CDM::cmd(std::string chaine, int commandStringAck, std::string *result)
{
	int ret = 0;
	printf("plugin : cmd with the instruction :%s\n", chaine.c_str());
	chaine += " ";
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
			subChaine1.erase(pos);		  // find the pair name:value
			subChaine2.erase(0, pos + 1); // strore the rest of the string (example the arguments of the instrution)
			if (subChaine1.compare("startAllDevice") == 0)
			{
				// userMethodStartAll(subChaine2); // name of your method who manage this action
			}
			if (subChaine1.compare("stopAllDevice") == 0)
			{
				// userMethodStopAll(); // name of your method who manage this action
			}
		}
	}
	// example here do nothing but wait
	//sleep(3);

	return ret;
}

// int SG::userMethodStartAll(std::string argument) {
//         int ret=0;
// 	std::string resultCall;
// 	std::vector<boost::any> callRequest;

// 	boost::any temp=atoi(argument.c_str());
// 	callRequest.push_back(temp);

// 	getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48011");
//  	getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.start",2,callRequest,&resultCall);
// 	getDataAccessClientOPCUARef()->disconnect();

// 	getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48012");
//  	getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.start",2,callRequest,&resultCall);
// 	getDataAccessClientOPCUARef()->disconnect();

// 	getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48013");
//  	getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.start",2,callRequest,&resultCall);
// 	getDataAccessClientOPCUARef()->disconnect();
//         return ret;
// }

// int SG::userMethodStopAll() {
//         int ret=0;
//         std::string resultCall;
// 	std::vector<boost::any> callRequest;

//         boost::any temp;
// 	callRequest.push_back(temp);

//         getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48011");
//         getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.stop",2,callRequest,&resultCall);
//         getDataAccessClientOPCUARef()->disconnect();

//         getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48012");
//         getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.stop",2,callRequest,&resultCall);
//         getDataAccessClientOPCUARef()->disconnect();

//         getDataAccessClientOPCUARef()->connect("opc.tcp:\\<yourComputer>:48013");
//         getDataAccessClientOPCUARef()->callMethod("MOS_Server.Slave_device.stop",2,callRequest,&resultCall);
//         getDataAccessClientOPCUARef()->disconnect();

//         return ret;
// }

// be careful :  allways need : allow to connect this Plugin with MOS
extern "C"
{
	CDM *CDM_plugin()
	{
		return new CDM();
	}
}
