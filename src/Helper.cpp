#include <Helper.h>
#include <string>
#include <iostream>
#include <ctime>

using namespace std;


long int Helper::unix_timestamp()
{
	time_t t = std::time(0);
	long int now = static_cast<long int>(t);
	return now;
}

string Helper::UTC_time()
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

/* int Helper::publish_datapoint(std::string datapoint_name, int nameSpace, int data )
{
    //SetDatapointThread *m_SetDatapointThread_tmp = new SetDatapointThread(Helper::m_clientOpcUaRef_this, datapoint_name, nameSpace, data);

    //SetDatapointThread *m_SetDatapointThread_tmp = new SetDatapointThread(getDataAccessClientOPCUARef(), datapoint_name, nameSpace, data);

}
 */

double Helper::acquire_RA()
{
    std::string finalnode = "Drive.DriveControl.RA_Telescope.RA_Telescope_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    std::cout << "RA is: " << element << std::endl;
    RA = element;
}

double Helper::acquire_DEC()
{
    std::string finalnode = "Drive.DriveControl.Dec_Telescope.Dec_Telescope_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Dec is: " << element << endl;
    DEC = element;
}

double Helper::acquire_Azimuth()
{
    std::string finalnode = "Drive.DriveControl.CurrentPosition.azimuth_position.azimuth_position_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Azimuth is: " << element << endl;
    azimuth = element;
}

double Helper::acquire_Zenith()
{
    std::string finalnode = "Drive.DriveControl.CurrentPosition.zenithangle_position.zenithangle_position_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Zenith is: " << element << endl;
    zenith = element;
}

int Helper::acquire_LED_intensity()
{
    std::string finalnode = "ECC_LST.ECC.Monitoring.LEDPositions.Led_01.Led_01_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    int element = 0;
    if(m_clientOpcUaRef_ECC!=NULL)
        Helper::m_clientOpcUaRef_ECC->getDatapoint(finalnode, nameSpace, element);
    cout << "LED intensity is: " << element<< endl;
    LED_intensity = element;
}

bool Helper::acquire_OARL_state()
{
    std::string finalnode = "PLC 400 Server.CPU 414-3 PNDP.DB_IO_Module1_DishCenter.OARL_On";
    int nameSpace = 7;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    if(m_clientOpcUaRef_Relay!=NULL)
        Helper::m_clientOpcUaRef_Relay->getDatapoint(finalnode, nameSpace, element);
    cout << "OARL status is: " << element<< endl;
    OARL_state = element;
}

bool Helper::acquire_drive_status_in_motion()
{
    std::string finalnode = "Drive.DriveControl.Status.Status_In_Motion.Status_In_Motion_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    std::cout << "Drive status of In Motion is: " << element << std::endl;
    drive_status_in_motion = element;
}

bool Helper::acquire_drive_status_in_parking_position()
{
    std::string finalnode = "Drive.DriveControl.Status.Status_In_Parking_Position.Status_In_Parking_Position_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    std::cout << "Drive status of In Parking Position is: " << element << std::endl;
    drive_status_in_parking_position = element;
}

bool Helper::acquire_drive_status_parked()
{
    std::string finalnode = "Drive.DriveControl.Status.Status_Parked.Status_Parked_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    std::cout << "Drive status of Parked: " << element << std::endl;
    drive_status_parked = element;
}

bool Helper::acquire_drive_status_tracking_in_progress()
{
    std::string finalnode = "Drive.DriveControl.Status.Status_Tracking_In_Progress.Status_Tracking_In_Progress_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    
    if (m_clientOpcUaRef_Drive != NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    std::cout << "Drive status of Tracking In Progress: " << element << std::endl;
    drive_status_tracking_in_progress = element;
}

string Helper::acquire_StarName()
{
    std::string finalnode = "Drive.DriveControl.SourceName.SourceName_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    string element = "";
    if(m_clientOpcUaRef_Drive!=NULL)
        Helper::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "StarName is: " << element << endl;
    set_StarName(element);
	return element;
}

int Helper::connectOpcUa_Drive(std::string url)
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
	m_clientOpcUaRef_Drive = pluginsLoader->load();
	if (m_clientOpcUaRef_Drive == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_Drive->connect(url, NULL);
			//ret = m_clientOpcUaRef_Drive->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}


int Helper::connectOpcUa_Relay(std::string url)
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
	m_clientOpcUaRef_Relay = pluginsLoader->load();
	if (m_clientOpcUaRef_Relay == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_Relay->connect(url, NULL);
			//ret = m_clientOpcUaRef_Relay->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

int Helper::connectOpcUa_ECC(std::string url)
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
	m_clientOpcUaRef_ECC = pluginsLoader->load();
	if (m_clientOpcUaRef_ECC == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_ECC->connect(url, NULL);
			//ret = m_clientOpcUaRef_ECC->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

