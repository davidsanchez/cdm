#include "ImageFITS.h"

using namespace CCfits;

std::string ImageFITS::writeFITSImage(Mat image) //, double exposure_value) //,  double azimuth, double zenith)
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
	streamObj << "-LED=";
	streamObj << CDM::get_LED_int();
	streamObj << "-OARL=";
	streamObj << CDM::get_OARL();
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
	pFits->pHDU().addKey("LED", CDM::get_LED_int(), "LED01 intensity");
	pFits->pHDU().addKey("OARL", CDM::get_OARL(), "OARL status");

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
