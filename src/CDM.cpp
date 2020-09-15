#include "CDM.h"
#include "Camera.h"

#include <boost/any.hpp>

using namespace std;
using namespace cv;

// initialize object of Camera class
Camera camera;

double CDM::acquire_RA()
{
    std::string finalnode = "Drive.DriveControl.RA.RA_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        CDM::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "RA is: " << element << endl;
    RA = element;
}

double CDM::acquire_DEC()
{
    std::string finalnode = "Drive.DriveControl.Dec.Dec_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        CDM::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Dec is: " << element << endl;
    DEC = element;
}

double CDM::acquire_Azimuth()
{
    std::string finalnode = "Drive.DriveControl.CurrentPosition.azimuth_position.azimuth_position_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        CDM::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Azimuth is: " << element << endl;
    azimuth = element;
}

double CDM::acquire_Zenith()
{
    std::string finalnode = "Drive.DriveControl.CurrentPosition.elevation_position.elevation_position_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    float element = 0;
    if (m_clientOpcUaRef_Drive != NULL)
        CDM::m_clientOpcUaRef_Drive->getDatapoint(finalnode, nameSpace, element);
    cout << "Zenith is: " << element << endl;
    zenith = element;
}

int CDM::acquire_LED_intensity()
{
    std::string finalnode = "ECC_LST.ECC.Monitoring.LEDPositions.Led_01.Led_01_v";
    int nameSpace = 2;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    int element = 0;
    if(m_clientOpcUaRef_ECC!=NULL)
        CDM::m_clientOpcUaRef_ECC->getDatapoint(finalnode, nameSpace, element);
    cout << "LED intensity is: " << element<< endl;
    LED_intensity = element;
}

bool CDM::acquire_OARL_state()
{
    std::string finalnode = "PLC 400 Server.CPU 414-3 PNDP.DB_IO_Module1_DishCenter.OARL_On";
    int nameSpace = 7;
    //short int element; //(change with the good type of the datapoint float/int/string/.....)
    bool element = 0;
    if(m_clientOpcUaRef_Relay!=NULL)
        CDM::m_clientOpcUaRef_Relay->getDatapoint(finalnode, nameSpace, element);
    cout << "OARL status is: " << element<< endl;
    OARL_state = element;
}



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

    // Trying to access other OPCUA server
	//connectOpcUa("opc.tcp://address:port"); // example opc.tcp://lappc-f578l:48080
	int connection_result_Drive = connectOpcUa_Drive("opc.tcp://10.200.100.105:48010"); //This is Drive OPCUA. Old = opc.tcp://10.1.8.3:48010
	int connection_result_Relay = connectOpcUa_Relay("opc.tcp://10.1.10.5:4845"); //This is Central Dish Cabinet Relay. Used for toggling SG camera power. Old = opc.tcp://10.1.8.3:48010
	int connection_result_ECC = connectOpcUa_ECC("opc.tcp://10.1.4.66:4841"); //This is ECC OPCUA.

	cout << "Drive status OPCUA: "<< connection_result_Drive << endl;
	cout << "Central dish cabinet relay status OPCUA: "<< connection_result_Relay << endl;
	cout << "ECC status OPCUA: "<< connection_result_ECC << endl;
	cout << "After start finished!" << endl;

    return ret;
}

int CDM::cmdAsynch(const std::string &command, int commandStringAck, const std::string &datapointName, int nameSpace, std::string &result)
{
    printf("CDM::cmdAsynch\n");
    cout << "Async command is: " << command << endl;
    cout << "Datapoint name: " << datapointName << endl;
    // not use in this example
    int ret = 0;
    result = "";
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

        acquire_Azimuth();
        acquire_Zenith();
        acquire_RA();
        acquire_DEC();
        acquire_LED_intensity();
        acquire_OARL_state();

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
                std::vector<boost::any> configure_settings = camera.Configure(); // Sets default parameters

                //Todo: add this part as a function
                SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, boost::any_cast<int> (configure_settings[0]));
                SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, boost::any_cast<double>(configure_settings[1]));
                SetDatapointThread *m_SetDatapointThread_exposure = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, boost::any_cast<double> ((configure_settings[2])));
                SetDatapointThread *m_SetDatapointThread_gain = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.gain.gain_v", 2, boost::any_cast<int> ((configure_settings[3])));
                SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, boost::any_cast<string> (configure_settings[4]));


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
                std::vector<boost::any> configure_settings = camera.Configure(stoi(results[0]), stod(results[1]), stod(results[2]), stoi(results[3]), results[4]);

                //Todo: add this part as a function
                SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, boost::any_cast<int> (configure_settings[0]));
                SetDatapointThread *m_SetDatapointThread_fps = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, boost::any_cast<double>(configure_settings[1]));
                SetDatapointThread *m_SetDatapointThread_exposure = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, boost::any_cast<double> ((configure_settings[2])));
                SetDatapointThread *m_SetDatapointThread_gain = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.gain.gain_v", 2, boost::any_cast<int> ((configure_settings[3])));
                SetDatapointThread *m_SetDatapointThread_pixel_format = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, boost::any_cast<string> (configure_settings[4]));
    

                // Put here the rest Datapoint Threads or refactor
            }

            if (subChaine1.compare("Comment") == 0)
            {
                // TODO: Make some parsing/safety checks. best inside Comment function.
                CDM::Comment(subChaine2);
            }

            if (subChaine1.compare("GetImage") == 0)
            {

                image_header_info.azimuth = acquire_Azimuth();
                image_header_info.zenith = acquire_Zenith();
                image_header_info.RA = acquire_RA();
                image_header_info.DEC = acquire_DEC();
                image_header_info.LED_intensity = acquire_LED_intensity();
                image_header_info.OARL_state = acquire_OARL_state();

                vector<unsigned char> displayImage = camera.GetImage();

                int m_nameSpace = 2;
                string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
                //getDataAccessClientOPCUARef()->setDatapoint(temString,m_nameSpace, data);
                SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, displayImage); //pushes the image to the datapoint


                
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

int CDM::Comment(string comment)
{
    this->comment = comment;
    SetDatapointThread *m_SetDatapointThread_pixel_clock = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.comment.comment_v", 2, this->comment);
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


int CDM::connectOpcUa_Drive(std::string url)
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

int CDM::connectOpcUa_Relay(std::string url)
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

int CDM::connectOpcUa_ECC(std::string url)
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



// Be careful, always need to allow to connect this Plugin with MOS
extern "C"
{
    CDM *CDM_plugin()
    {
        return new CDM();
    }
}