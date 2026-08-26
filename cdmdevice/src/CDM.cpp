#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

#include "CDM.h"
#include "Camera.h"
#include "Helper.h"
#include "Logging.h"
#include "ConfigCDM.h"
#include "Config.h"
// #include <fstream>
#include <map>
#include <boost/any.hpp>

using namespace std;
using namespace cv;

// initialize object of Camera class
Camera camera;
Helper helper;


// a rajouter pour fichier mapping des Datapoints 
#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"
#define DATABROKER_CONFIGURATION_NAME "PLC_DATABROKER.xml"


int CDM::init(const std::string &chaine)
{
  
  // a rajouter pour recuperer les infos du fichier mapping des datapoints
  cdm_config = new Config(CDM_CONFIGURATION_NAME,"");
  DB_config = new Config(DATABROKER_CONFIGURATION_NAME,"");
    
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
  const int connection_failure = -1;
  int namespaceL2 = 2;
  string nodeIdL2 = "";
  
  // You can overwrite this method if you want but not mandatory because the class pluginsInterfaceImpl already implement it:)
  // but be careful, you have to call before doing your bussiness, call the father method (the father class) ( PluginsInterfaceImpl::afterStart())
  // This method is automatically called by the program "MOS" after "MOS" server is launched and ready.
  int ret = 0;
  COND_LOG_DEBUG << "CDM::afterStart(): SetDPQuality "<<helper.searchDatapoint("SetDPQuality",cdm_config);
  
  // map<std::string,std::string> config;
  
  bool conf = LoadCDMConfiguration(m_config);
  camera.SetConfig(m_config);
  // debug print the config
  map<std::string, std::string>::iterator it;
  for(it=m_config.begin(); it!=m_config.end(); ++it){
    COND_LOG_DEBUG << it->first << " => " << it->second << '\n';
  }
  
  
  // // set up config file of the camera object
  // camera.SetConfig(m_config);
  

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
        } 
    }

  std::string resultCall;
  m_Thread =           new AsynchronousThread(getDataAccessClientOPCUARef());
  m_ThreadMeteo =      new AsynchronousThread(getDataAccessClientOPCUARef());
  m_ThreadLogRestart = new AsynchronousThread(getDataAccessClientOPCUARef());

  ret = m_Thread->startRun();
  int ret2 = m_ThreadMeteo->startRun();
  m_ThreadMeteo->cmdStartMeteo();
  int ret3 = m_ThreadLogRestart->startRun();
  m_ThreadLogRestart->cmdLogRestart();

  //helper.set_OPCUAref( getDataAccessClientOPCUARef() );

  // Trying to access other OPCUA server
  std::cout<<"DB adress "<<DB_config->getOpcUaRef()<<std::endl;
  CDM::connection_result_DataBroker = helper.connectOpcUa_DataBroker(DB_config->getOpcUaRef(), this); //DataBroker OPCUA

  LOG_TRACE << "CDM::afterStart(): DataBroker status OPCUA: " << connection_result_DataBroker;

  // Creating datapoint monitor that will notify us when the subscribed datapoints change.
  if (connection_result_DataBroker != connection_failure)
    {
      LOG_TRACE << "CDM::afterStart(): Connected to DataBroker. subscribe now";
      // Subscription now happens in ControllerCB which calls the subscribe method.
      subscribe_DataBroker();
      LOG_TRACE << "CDM::afterStart(): subscribe to DataBroker";
    }
  else
    {
      LOG_ERROR << "CDM::afterStart(): Cannot connect to DataBroker!";
      throw std::exception();
    }
  // remplace le hardcodage des datapoints --> recuperation des infos provenant du fichier PLC_*****.xml
  //    getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), namespaceL2, 0);
    
  //Be sure that the state is 0 at the start (safe)
  getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), 2, 0);
  //enable Heartbeat
  getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.Diagnostics._Enable_Heart_Beat", 2, true);
  getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.Diagnostics._Error_Heart_Beat", 2, false);

  int quality = 0; // (0= Good, 1=Uncertain, 2 = Bad)
  std::string methodToCall = helper.searchDatapoint("SetDPQuality",cdm_config);
  std::string completeNodeName = "AuxControl";
            
  boost::any completeNodeNameAny = completeNodeName;
            
  std::vector<boost::any> callRequest;
  callRequest.push_back(completeNodeNameAny);
  callRequest.push_back(quality);
  int res = getDataAccessClientOPCUARef()->callMethod(methodToCall, 4, callRequest, resultCall);
  COND_LOG_DEBUG << methodToCall << "  method call result = " << res << endl;
  //getDataAccessClientOPCUARef()->setDPQuality("CDM", 1);



        double Az_deg=0;
    getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME)->getDatapointL1("azimuth_position", Az_deg);
    COND_LOG_DEBUG <<"CDMController::ConnectCamera: read DB AZ "<<Az_deg;

  return ret;
}

void CDM::subscribe_DataBroker()
{
  if (CDM::connection_result_DataBroker != -1)
    {
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

  int ret = 0;
  LOG_INFO<<"In CMDAsync part: received command with the instruction: "<<command<<std::endl;
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

	  // do Asynch commands
	  	  
            
        }
    }
  // example here do nothing but wait
  //sleep(3);

  return ret;
}

int CDM::cmd(const std::string &command, int commandStringAck, std::string &result)
{

  int ret = 0;
  LOG_INFO<<"In CMD part: received command with the instruction: "<<command.c_str()<<std::endl;
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

	      camera.Connect();

	      std::string datapointName =  helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;
	      //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");
	      //m_Thread->cmdConfigure(datapointName, nameSpace, 216, 50, 10, 0, "IS_CM_SENSOR_RAW16");
	      //helper.searchDatapoint("ids_exposure_us",cdm_config));
	      //float fps=stof(helper.searchDatapoint("ids_fps",cdm_config));
	      //float gain=stof(helper.searchDatapoint("ids_gain",cdm_config));
	      m_Thread->cmdConfigure(datapointName,
				     nameSpace,
				     216,          // nPixelClock
				     95000.0,     // exposure
				     10.0,          // fps
				     5.0,         // gain
				     "IS_CM_SENSOR_RAW16" // pixel_format
				     );
                
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
		
	      //getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 0);
            }

	  if (subChaine1.compare("Disconnect") == 0)
            {
	      camera.Disconnect();
                
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
            }

	  if (subChaine1.compare("Configure") == 0)
            {

	      //TODO: check what will happen if some of the parameters missing. The code below assumes that you received everything! Need to implement some safety guard.
	      // Actually OPCUA should check that all the parameters are in the input, right?

	      std::vector<std::string> results;
	      boost::split(results, subChaine2, [](char c)
	      { return c == ' '; });

	      // Check that the input string is some sensible value
	      if (results[4] == "IS_CM_MONO8"){camera.iBitsPerPixel = 8;}
	      else
                {
		  results[4] = "IS_CM_SENSOR_RAW16";
		  camera.iBitsPerPixel = 16;
                }

	      //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");
	      std::string datapointName = helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;
	      m_Thread->cmdConfigure(datapointName, nameSpace, stoi(results[0]), stod(results[1]), stod(results[2]), stoi(results[3]), results[4]);
            }

	  if (subChaine1.compare("AddComment") == 0)
            {

	      // TODO: Make some parsing/safety checks. best inside Comment function.
	      boost::trim_right(subChaine2);
	      CDM::AddComment(subChaine2);
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);

            }

	  if (subChaine1.compare("GetImage") == 0)
            {
	      camera.GetImage(getDataAccessClientOPCUARef()); // pushes the image to the datapoint inside the function
	      COND_LOG_DEBUG << "Finished GetImage";
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
                
	      /* vector<unsigned char> displayImage = camera.GetImage();
		 int m_nameSpace = 2;
		 string temString = "Unit_CDM.AuxControl.CDM.image.image_v";
		 SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), temString, m_nameSpace, displayImage); //pushes the image to the datapoint
	      */
            }

	  if (subChaine1.compare("GoToTpoint") == 0)
            {
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
            }

	  if (subChaine1.compare("GoToReady") == 0)
            {
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
            }

            
	  if (subChaine1.compare("StopGetMultipleImages") == 0)
            {
	      camera.StopGetMultipleImages();
            }

	  if (subChaine1.compare("StartCDM") == 0)
            {
	  
	      //RR: should configure be async?
	      std::string datapointName = helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;
              // float
              // exposure=stof(helper.searchDatapoint("ids_exposure_us",cdm_config));
              // float fps=stof(helper.searchDatapoint("ids_fps",cdm_config));
              // float gain=stof(helper.searchDatapoint("ids_gain",cdm_config));
	      // RR why??              
              /*
              LOG_INFO<<"CDM::cmdAsynch: StartCDM: config camera"<<std::endl;
              m_Thread->cmdConfigure(datapointName,
                                     nameSpace,
                                     216,          // nPixelClock
                                     2000.0,     // exposure
                                     1.0,          // fps
                                     1.0,         // gain
                                     "IS_CM_MONO8" // pixel_format
                                     );
	      */
	      LOG_INFO<<"CDM::cmdAsynch: StartCDM: calling thread cmdStartCDM"<<std::endl;

	      m_Thread->cmdStartCDM(datapointName, nameSpace);
          }
            
	  if (subChaine1.compare("StopCDM") == 0)
            {
	      camera.StopCDM();
            }

	  if (subChaine1.compare("StartStream") == 0)
            {

	      std::string datapointName =  helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;


	      m_Thread->cmdStartStream(datapointName, nameSpace);
            }

            
	  if (subChaine1.compare("StopStream") == 0)
            {
	      camera.StopStream();
            }

	  if (subChaine1.compare("StartSG") == 0)
            {
	      std::string datapointName = helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;
              // float
              // exposure=stof(helper.searchDatapoint("ids_exposure_us",cdm_config));
              // float fps=stof(helper.searchDatapoint("ids_fps",cdm_config));
              // float gain=stof(helper.searchDatapoint("ids_gain",cdm_config));
              // RR why?
              /*
              LOG_INFO<<"CDM::cmdAsynch: StartSG: config camera"<<std::endl;
              m_Thread->cmdConfigure(datapointName,
                                     nameSpace,
                                     216,          // nPixelClock
                                     2000.0,     // exposure
                                     0.2,          // fps
                                     1.0,         // gain
                                     "IS_CM_MONO8" // pixel_format
                                     );
	      */
	      LOG_INFO<<"CDM::cmd: StartSG: calling thread cmdStartSG"<<std::endl;

	      m_Thread->cmdStartSG(datapointName, nameSpace);
            }          

	  if (subChaine1.compare("StopSG") == 0)
            {
	      camera.StopSG();
            }

	  if (subChaine1.compare("GetMultipleImagesStacked") == 0)
            {

         
	      std::string datapointName = helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;
            
	      boost::trim_right(subChaine2);
	      m_Thread->cmdGetMultipleImagesStacked(datapointName, nameSpace, atoi(subChaine2.c_str()));

	      //SetDatapointThread *m_SetDatapointThread_nImagesGet = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, std::atoi(subChaine2.c_str()));
            }

      if (subChaine1.compare("GetMultipleImages") == 0) {

	      LOG_INFO<<"CDM::cmd: GetMultipleImages"<<std::endl;

	      std::string datapointName = helper.searchDatapoint("Configure",cdm_config);
	      int nameSpace = 2;              

	      boost::trim_right(subChaine2);

        COND_LOG_DEBUG<<"CDM::cmd: subChaine2: "<<subChaine2<<std::endl;

	      m_Thread->cmdGetMultipleImages(datapointName, nameSpace, atoi(subChaine2.c_str()));
        LOG_INFO<<"CDM::cmd: GetMultipleImages: End"<<std::endl;
	      //SetDatapointThread *m_SetDatapointThread_nImagesGet = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.nImagesGet.nImagesGet_v", 2, std::atoi(subChaine2.c_str()));
            }
            
	  if (subChaine1.compare("Error") == 0)
            {
	      SetDatapointThread *m_SetDatapointThread_state = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.state", 2, 5);
	      camera.StopCDM();
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.FSM.transition", 2, 0);
            }

	  if (subChaine1.compare("Acknowledge") == 0)
            {
	      camera.Disconnect();
	      SetDatapointThread *m_SetDatapointThread_transition = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("transition",cdm_config), 2, 0);
            }
        }
    }
  // example here do nothing but wait
  //sleep(3);

  // DAVID
  // int FSM_state;
  // getDataAccessClientOPCUARef()->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
  // LOG_TRACE << "End of CDM::cdm(), State of the CDM : "<<FSM_state<<endl;
  return ret;
}

int CDM::UpdateRaValue(double newvalue)
{
    
  helper.SetRaDrive(newvalue);
  return 0;
}

int CDM::UpdateDecValue(double newvalue)
{
    
  helper.SetDecDrive(newvalue);
  return 0;
}

int CDM::UpdateAzValue(double newvalue)
{
    
  helper.SetAz(newvalue);
  return 0;
}
int CDM::UpdateZdValue(double newvalue)
{
    
  helper.SetZd(newvalue);
  return 0;
}

int CDM::UpdateAzOffsetValue(double newvalue)
{
    
  helper.SetAzOffset(newvalue);
  return 0;
}

int CDM::UpdateZdOffsetValue(double newvalue)
{
  //LOG_DEBUG << "UpdateZdOfssetValue: " << newvalue;
  helper.SetZdOffset(newvalue);
  return 0;
}

int CDM::UpdateSourceValue(string newvalue)
{
  //LOG_DEBUG << "UpdateSourceValue: " << newvalue;
  helper.SetSource(newvalue);
  return 0;
}

int CDM::UpdateOARLValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateOARLValue: " << newvalue;
  helper.SetOARL(newvalue);
  return 0;
}

int CDM::UpdateLEDsValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateLEDsValue: " << newvalue;
  helper.SetLEDs(newvalue);
  return 0;
}

int CDM::UpdateShutterValue(int newvalue)
{
  //LOG_DEBUG << "UpdateShutterValue: " << newvalue;
  helper.SetShutter(newvalue);
  return 0;
}

int CDM::UpdateSISValue(int newvalue)
{
  //LOG_DEBUG << "UpdateSISValue: " << newvalue;
  helper.SetSIS(newvalue);
  return 0;
}

int CDM::UpdateDriveInMotionValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateDriveInMotionValue: " << newvalue;
  helper.SetDriveInMotion(newvalue);
  return 0;
}

int CDM::UpdateDriveInParkingPosValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateDriveInParkingPosValue: " << newvalue;
  helper.SetDriveInParkingPos(newvalue);
  return 0;
}

int CDM::UpdateDriveParkedValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateDriveParkedValue: " << newvalue;
  helper.SetDriveParked(newvalue);
  return 0;
}

int CDM::UpdateDriveTrackingValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateDriveTrackingValue: " << newvalue;
  helper.SetDriveTracking(newvalue);
  return 0;
}

int CDM::UpdateDriveRaTargetValue(double newvalue)
{
  //LOG_DEBUG << "UpdateDriveRaTargetValue: " << newvalue;
  helper.SetRaTarget(newvalue);
  return 0;
}

int CDM::UpdateDriveDecTargetValue(double newvalue)
{
  //LOG_DEBUG << "UpdateDriveDecTargetValue: " << newvalue;
  helper.SetDecTarget(newvalue);
  return 0;
}

int CDM::UpdateAuxDMEastBottomValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateAuxDMEastBottomValue: " << newvalue;
  helper.SetAuxDMEastBottom(newvalue);
  return 0;
}
int CDM::UpdateAuxDMEastTopValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateAuxDMEastTopValue: " << newvalue;
  helper.SetAuxDMEastTop(newvalue);
  return 0;
}
int CDM::UpdateAuxDMWestBottomValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateAuxDMWestBottomValue: " << newvalue;
  helper.SetAuxDMWestBottom(newvalue);
  return 0;
}
int CDM::UpdateAuxDMWestTopValue(bool newvalue)
{
  //LOG_DEBUG << "UpdateAuxDMWestTopValue: " << newvalue;
  helper.SetAuxDMWestTop(newvalue);
  return 0;
}

int CDM::AddComment(std::string comment)
{
  cout << "Comment is: " << comment << endl;

  helper.set_Comment(comment);
  SetDatapointThread *m_SetDatapointThread_comment = new SetDatapointThread(getDataAccessClientOPCUARef(), helper.searchDatapoint("comment",cdm_config), 2, comment);
}

int CDM::close()
{

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
  COND_LOG_DEBUG << "CDM::get(): Start";
  COND_LOG_DEBUG << "CDM::get(): Chain: " << chain;

  // Usually you would push_back to the tabValue vector and they would be automatically updated in the OPCUA server.
  // But there is some bug that crashes the program when resizing that vector and the chain is empty.
  // So as a workaround using setDatapoint.

  int ret = 0;
  double return_value_double = 0;
  string return_value_string = "";

    
  if (chain.find("get_HeartBeatError") != std::string::npos)
    {
      bool HeartBeatError = false;
      bool HeartBeatEnable = false;
      bool HeartBeat = false;
      getDataAccessClientOPCUARef()->getDatapoint("Unit_CDM.Diagnostics._Heart_Beat", 2, HeartBeat);
      //std::cout << "CDM::get(): HeartBeat " << HeartBeat<< endl;
      getDataAccessClientOPCUARef()->getDatapoint("Unit_CDM.Diagnostics._Enable_Heart_Beat", 2, HeartBeatEnable);
      //std::cout << "CDM::get(): HeartBeatEnable " << HeartBeatEnable<< endl;
      getDataAccessClientOPCUARef()->getDatapoint("Unit_CDM.Diagnostics._Error_Heart_Beat", 2, HeartBeatError);
      //std::cout << "CDM::get(): HeartBeatError " << HeartBeatError<< endl;

      //        if (HeartBeatError ) {ret = 1;}
      //        else {ret = 0;}


      /*int FSM_state;
        getDataAccessClientOPCUARef()->getDatapoint(helper.searchDatapoint("state",cdm_config), 2, FSM_state);
        std::cout << "CDM::get(): state is " << FSM_state<< endl;
        if (HeartBeatError and HeartBeatEnable) {
	switch(FSM_state){
	case 1: //ready to safe
	camera.Disconnect();
	getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), 2, 0);
	break;
	case 3: //TPoint to safe
	getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), 2, 1);
	break;
	case 2: //Observing to ready
	camera.StopCDM();
	getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), 2, 1);
	break;
	case 6: //Stream to ready
	camera.StopStream();
	getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("state",cdm_config), 2, 1);
	break;
	}
        }*/
    } 
  else if (chain.find("get_temperatureValue") != std::string::npos)
    {
      return_value_double = camera.get_temperature_value();
      COND_LOG_DEBUG << "CDM::get(): Camera temperature value is: " << return_value_double << endl;
      //TODO hard coded
      //getDataAccessClientOPCUARef()->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.temperatureValue.temperatureValue_v", 2, return_value_double);
      getDataAccessClientOPCUARef()->setDatapoint(helper.searchDatapoint("CameraTemp",cdm_config), 2, return_value_double);
    }
  else if (chain.find("get_temperatureStatus") != std::string::npos)
    {
      return_value_string = camera.get_temperature_status();
      COND_LOG_DEBUG << "CDM::get(): Camera temperature status is: " << return_value_string << endl;
      tabValue.resize(0);
      tabValue.push_back(return_value_string);
    }
    /* RR: old style SG image as periodic measurement
      else if (chain.find("get_ImagePoint") != std::string::npos) {
      camera.GetImage(getDataAccessClientOPCUARef());
      return_value_string = "ImagePoint"; //TODO
      tabValue.resize(0);
      tabValue.push_back(return_value_string);
    }
    */  
  
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
