#include "CDM.h"

using namespace std;


int CDM::Connect()
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
}

int CDM::init(const std::string& chaine)
{	// You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
	// but becarefull, you have to call before doing  your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::init())
	// This method is automaticaly call by the program "MOS" after "MOS" server is launched but the "MOS" server is not really ready.
	// So don't use this method in ordr to communicate with the "MOS" Server.
	// you can use the afertStart() method if needed.
	int ret = 0;

	// Mandatory allways need
	printf("\n***********************************\nIn CDM::init\n***********************************\n");
	PluginsBase::init(chaine);
	
	
	return ret;
}

int CDM::afterStart()
{	// You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
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

int CDM::cmdAsynch(const std::string& command, int commandStringAck, const std::string& datapointName, int nameSpace, std::string& result)
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

int CDM::cmd(const std::string& command, int commandStringAck, std::string& result)
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
			subChaine1.erase(pos);		  // find the pair name:value
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
				CDM::Connect();
				cout<<"DZ below Connect" << endl;
			}
		}
	}
	// example here do nothing but wait
	//sleep(3);

	return ret;
}

int CDM::get(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue)
{
	int ret = 0;
	return ret;
}

int CDM::set(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue)
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