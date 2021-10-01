#include "CDM.h"
#include "Camera.h"
#include "Helper.h"
#include "Logging.h"

#include <boost/any.hpp>

using namespace std;
using namespace cv;

// initialize object of Camera class
Camera camera;
Helper helper;

int CDM::init(const std::string &chaine)
{
    LOG_TRACE << "CDM::init()";

    // You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
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
{
    LOG_TRACE << "CDM::afterStart()";
    const int connection_failure = -1;

    // You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
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
    m_Thread = new AsynchronousThread(getDataAccessClientOPCUARef());
    m_ThreadMeteo = new AsynchronousThread(getDataAccessClientOPCUARef());
    m_ThreadLogRestart = new AsynchronousThread(getDataAccessClientOPCUARef());

    ret = m_Thread->startRun();
    int ret2 = m_ThreadMeteo->startRun();
    m_ThreadMeteo->cmdStartMeteo();
    int ret3 = m_ThreadLogRestart->startRun();
    m_ThreadLogRestart->cmdLogRestart();

    //helper.set_OPCUAref( getDataAccessClientOPCUARef() );

    // Trying to access other OPCUA server
    //connectOpcUa("opc.tcp://address:port"); // example opc.tcp://lappc-f578l:48080
    CDM::connection_result_DataBroker = helper.connectOpcUa_DataBroker("opc.tcp://10.1.12.1:48030", this); //DataBroker OPCUA

    LOG_DEBUG << "DataBroker status OPCUA: " << connection_result_DataBroker;

    // Creating datapoint monitor that will notify us when the subscribed datapoints change.
    if (connection_result_DataBroker != connection_failure)
    {
        LOG_DEBUG << "Connected to DataBroker.";
        // Subscription now happens in ControllerCB which calls the subscribe method.
        //subscribe_DataBroker();
    }
    else
    {
        LOG_ERROR << "Cannot connect to DataBroker!";
        throw std::exception();
    }

    getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 0);
    //SetDatapointThread *m_SetDatapointThread_state = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.state", 2, 0);


    LOG_DEBUG << "After start finished!";

    return ret;
}

int CDM::subscribe_DataBroker()
{
    if (CDM::connection_result_DataBroker != -1)
    {
        cout << "Subscribing to DataBroker datapoints." << endl;
        //delete CDM::dp_monitor_SG;
        //CDM::dp_monitor_SG = new DatapointMonitor(this);
        if (CDM::dp_monitor_DataBroker == NULL)
        {
            CDM::dp_monitor_DataBroker = new DatapointMonitor(this);

            helper.get_client_DataBroker()->subscribe(
                CDM::dp_monitor_DataBroker->getElements(),
                CDM::dp_monitor_DataBroker->getNameSpaces(),
                CDM::dp_monitor_DataBroker);
        }

        helper.get_client_DataBroker()->startSubscribe();
    }
}

int CDM::cmdAsynch(const std::string &command, int commandStringAck, const std::string &datapointName, int nameSpace, std::string &result)
{
    LOG_TRACE << "CDM::cdmAsynch()";

    int ret = 0;
    printf("In CMDAsync part: received command with the instruction: %s\n", command.c_str());
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

            if (subChaine1.compare("GetMultipleImages") == 0)
            {
                LOG_TRACE << "CDM::cdmAsynch() / GetMultipleImages";

                boost::trim_right(subChaine2);
                m_Thread->cmdGetMultipleImages(datapointName, nameSpace, atoi(subChaine2.c_str()));

                //SetDatapointThread *m_SetDatapointThread_nImagesGet = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, std::atoi(subChaine2.c_str()));
            }

            if (subChaine1.compare("GetMultipleImagesStacked") == 0)
            {
                LOG_TRACE << "CDM::cdmAsynch() / GetMultipleImagesStacked";

                boost::trim_right(subChaine2);
                m_Thread->cmdGetMultipleImagesStacked(datapointName, nameSpace, atoi(subChaine2.c_str()));

                //SetDatapointThread *m_SetDatapointThread_nImagesGet = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, std::atoi(subChaine2.c_str()));
            }

            if (subChaine1.compare("StartCDM") == 0)
            {
                LOG_TRACE << "CDM::cdmAsynch() / StartCDM";

                std::string datapointName = "Unit_CDM.AuxControl.FSM.Configure";
                int nameSpace = 2;
                m_Thread->cmdConfigure(datapointName,
                                       nameSpace,
                                       216,          // nPixelClock
                                       50,           // exposure
                                       10,           // fps
                                       0,            // gain
                                       "IS_CM_MONO8" // pixel_format
                );

                m_Thread->cmdStartCDM(datapointName, nameSpace);
            }

            if (subChaine1.compare("StartStream") == 0)
            {
                LOG_TRACE << "CDM::cdmAsynch() / StartStream";

                std::string datapointName = "Unit_CDM.AuxControl.FSM.Configure";
                int nameSpace = 2;

                /* 
                m_Thread->cmdConfigure(datapointName,
                                       nameSpace,
                                       216,          // nPixelClock
                                       50,           // exposure
                                       1,           // fps
                                       0,            // gain
                                       "IS_CM_MONO8" // pixel_format
                ); 
                */

                m_Thread->cmdStartStream(datapointName, nameSpace);
            }
        }
    }
    // example here do nothing but wait
    //sleep(3);

    return ret;
}

int CDM::cmd(const std::string &command, int commandStringAck, std::string &result)
{
    LOG_TRACE << "CDM::cdm()";

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
                LOG_TRACE << "CDM::cdm() / Connect";

                camera.Connect();

                std::string datapointName = "Unit_CDM.AuxControl.FSM.Configure";
                int nameSpace = 2;
                //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");
                //m_Thread->cmdConfigure(datapointName, nameSpace, 216, 50, 10, 0, "IS_CM_SENSOR_RAW16");
                m_Thread->cmdConfigure(datapointName,
                                       nameSpace,
                                       216,          // nPixelClock
                                       50,           // exposure
                                       10,           // fps
                                       0,            // gain
                                       "IS_CM_MONO8" // pixel_format
                );
                
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);


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
                LOG_TRACE << "CDM::cdm() / Disconnect";
                camera.Disconnect();
                
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }

            if (subChaine1.compare("Configure") == 0)
            {
                LOG_TRACE << "CDM::cdm() / Configure";

                //TODO: check what will happen if some of the parameters missing. The code below assumes that you received everything! Need to implement some safety guard.
                // Actually OPCUA should check that all the parameters are in the input, right?

                std::vector<std::string> results;
                boost::split(results, subChaine2, [](char c)
                             { return c == ' '; });

                // Check that the input string is some sensible value
                if (results[4] == "IS_CM_MONO8")
                {
                    camera.iBitsPerPixel = 8;
                }
                else
                {
                    results[4] = "IS_CM_SENSOR_RAW16";
                    camera.iBitsPerPixel = 16;
                }

                //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");
                std::string datapointName = "Unit_CDM.AuxControl.FSM.Configure";
                int nameSpace = 2;
                m_Thread->cmdConfigure(datapointName, nameSpace, stoi(results[0]), stod(results[1]), stod(results[2]), stoi(results[3]), results[4]);
            }

            if (subChaine1.compare("AddComment") == 0)
            {
                LOG_TRACE << "CDM::cdm() / AddComment";

                // TODO: Make some parsing/safety checks. best inside Comment function.
                boost::trim_right(subChaine2);
                CDM::AddComment(subChaine2);
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);

            }

            if (subChaine1.compare("GetImage") == 0)
            {
                LOG_TRACE << "CDM::cdm() / GetImage";
                camera.GetImage(getDataAccessClientOPCUARef()); // pushes the image to the datapoint inside the function
                LOG_DEBUG << "Finished GetImage";
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
                
                /* vector<unsigned char> displayImage = camera.GetImage();
                int m_nameSpace = 2;
                string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
                SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, displayImage); //pushes the image to the datapoint
                */
            }

            if (subChaine1.compare("GoToTpoint") == 0)
            {
                LOG_TRACE << "CDM::cdm() / GoToTpoint";                
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }

            if (subChaine1.compare("GoToReady") == 0)
            {
                LOG_TRACE << "CDM::cdm() / GoToReady";                
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }

            if (subChaine1.compare("StopGetMultipleImages") == 0)
            {
                LOG_TRACE << "CDM::cdm() / StopGetMultipleImages";
                camera.StopGetMultipleImages();
            }

            if (subChaine1.compare("StopCDM") == 0)
            {
                LOG_TRACE << "CDM::cdm() / StopCDM";
                camera.StopCDM();
            }

            if (subChaine1.compare("StopStream") == 0)
            {
                LOG_TRACE << "CDM::cdm() / StopStream";
                camera.StopStream();
            }

            if (subChaine1.compare("Error") == 0)
            {
                LOG_TRACE << "CDM::cdm() / Error";
                SetDatapointThread *m_SetDatapointThread_state = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.state", 2, 5);
                camera.StopCDM();
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }

            if (subChaine1.compare("Acknowledge") == 0)
            {
                LOG_TRACE << "CDM::cdm() / Acknowledge";
                camera.Disconnect();
                SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }
        }
    }
    // example here do nothing but wait
    //sleep(3);

    return ret;
}

int CDM::UpdateRaValue(double newvalue)
{
    LOG_DEBUG << "UpdateRaValue: " << newvalue;
    helper.SetRaDrive(newvalue);
    return 0;
}

int CDM::UpdateDecValue(double newvalue)
{
    LOG_DEBUG << "UpdateDecValue: " << newvalue;
    helper.SetDecDrive(newvalue);
    return 0;
}

int CDM::UpdateAzValue(double newvalue)
{
    LOG_DEBUG << "UpdateAzValue: " << newvalue;
    helper.SetAz(newvalue);
    return 0;
}
int CDM::UpdateZdValue(double newvalue)
{
    LOG_DEBUG << "UpdateZdValue: " << newvalue;
    helper.SetZd(newvalue);
    return 0;
}

int CDM::UpdateAzOffsetValue(double newvalue)
{
    LOG_DEBUG << "UpdateAzOfssetValue: " << newvalue;
    helper.SetAzOffset(newvalue);
    return 0;
}

int CDM::UpdateZdOffsetValue(double newvalue)
{
    LOG_DEBUG << "UpdateZdOfssetValue: " << newvalue;
    helper.SetZdOffset(newvalue);
    return 0;
}

int CDM::UpdateSourceValue(string newvalue)
{
    LOG_DEBUG << "UpdateSourceValue: " << newvalue;
    helper.SetSource(newvalue);
    return 0;
}

int CDM::UpdateOARLValue(bool newvalue)
{
    LOG_DEBUG << "UpdateOARLValue: " << newvalue;
    helper.SetOARL(newvalue);
    return 0;
}

int CDM::UpdateLEDsValue(bool newvalue)
{
    LOG_DEBUG << "UpdateLEDsValue: " << newvalue;
    helper.SetLEDs(newvalue);
    return 0;
}

int CDM::UpdateLED01Value(int newvalue)
{
    LOG_DEBUG << "UpdateLED01Value: " << newvalue;
    helper.SetLED01(newvalue);
    return 0;
}

int CDM::UpdateShutterValue(int newvalue)
{
    LOG_DEBUG << "UpdateShutterValue: " << newvalue;
    helper.SetShutter(newvalue);
    return 0;
}

int CDM::UpdateSISValue(int newvalue)
{
    LOG_DEBUG << "UpdateSISValue: " << newvalue;
    helper.SetSIS(newvalue);
    return 0;
}

int CDM::UpdateDriveInMotionValue(bool newvalue)
{
    LOG_DEBUG << "UpdateDriveInMotionValue: " << newvalue;
    helper.SetDriveInMotion(newvalue);
    return 0;
}

int CDM::UpdateDriveInParkingPosValue(bool newvalue)
{
    LOG_DEBUG << "UpdateDriveInParkingPosValue: " << newvalue;
    helper.SetDriveInParkingPos(newvalue);
    return 0;
}

int CDM::UpdateDriveParkedValue(bool newvalue)
{
    LOG_DEBUG << "UpdateDriveParkedValue: " << newvalue;
    helper.SetDriveParked(newvalue);
    return 0;
}

int CDM::UpdateDriveTrackingValue(bool newvalue)
{
    LOG_DEBUG << "UpdateDriveTrackingValue: " << newvalue;
    helper.SetDriveTracking(newvalue);
    return 0;
}

int CDM::UpdateDriveRaTargetValue(double newvalue)
{
    LOG_DEBUG << "UpdateDriveRaTargetValue: " << newvalue;
    helper.SetRaTarget(newvalue);
    return 0;
}

int CDM::UpdateDriveDecTargetValue(double newvalue)
{
    LOG_DEBUG << "UpdateDriveDecTargetValue: " << newvalue;
    helper.SetDecTarget(newvalue);
    return 0;
}

int CDM::UpdateAuxDMEastBottomValue(bool newvalue)
{
    LOG_DEBUG << "UpdateAuxDMEastBottomValue: " << newvalue;
    helper.SetAuxDMEastBottom(newvalue);
    return 0;
}
int CDM::UpdateAuxDMEastTopValue(bool newvalue)
{
    LOG_DEBUG << "UpdateAuxDMEastTopValue: " << newvalue;
    helper.SetAuxDMEastTop(newvalue);
    return 0;
}
int CDM::UpdateAuxDMWestBottomValue(bool newvalue)
{
    LOG_DEBUG << "UpdateAuxDMWestBottomValue: " << newvalue;
    helper.SetAuxDMWestBottom(newvalue);
    return 0;
}
int CDM::UpdateAuxDMWestTopValue(bool newvalue)
{
    LOG_DEBUG << "UpdateAuxDMWestTopValue: " << newvalue;
    helper.SetAuxDMWestTop(newvalue);
    return 0;
}

int CDM::AddComment(std::string comment)
{
    cout << "Comment is: " << comment << endl;

    helper.set_Comment(comment);
    SetDatapointThread *m_SetDatapointThread_comment = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.comment.comment_v", 2, comment);
}

int CDM::close()
{
    LOG_TRACE << "CDM::close()";

    // here we do nothing
    int ret = 0;

    //Exit camera for example
    // ...
    camera.Disconnect();

    // Close connection
    /* m_clientOpcUaRef_Drive->disconnect();
    */
    return ret;
}

int CDM::get(const std::string &chain, int commandStringAck, std::vector<boost::any> &tabValue)
{
    //LOG_TRACE << "CDM::get()";
    //LOG_TRACE << "Chain: " << chain;

    // Usually you would push_back to the tabValue vector and they would be automatically updated in the OPCUA server.
    // But there is some bug that crashes the program when resizing that vector and the chain is empty.
    // So as a workaround using setDatapoint.

    int ret = 0;
    double return_value_double = 0;
    string return_value_string = "";

    if (chain.find("get_temperatureValue") != std::string::npos)
    {
        return_value_double = camera.get_temperature_value();
        LOG_DEBUG << "Camera temperature value is: " << return_value_double << endl;
        getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.temperatureValue.temperatureValue_v", 2, return_value_double);
    }
    else if (chain.find("get_temperatureStatus") != std::string::npos)
    {
        return_value_string = camera.get_temperature_status();
        LOG_DEBUG << "Camera temperature status is: " << return_value_string << endl;
        tabValue.resize(0);
        tabValue.push_back(return_value_string);
    }

    return ret;
}

int CDM::set(const std::string &chaine, int commandStringAck, std::vector<boost::any> &tabValue)
{
    //LOG_TRACE << "CDM::set()";
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