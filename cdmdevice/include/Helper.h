#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#ifndef Helper_H_
#define Helper_H_

#include "lappThread.h" // needed for MOS
#include "pluginsBase.h"
#include "Config.h"
#include "Logging.h" //RR
#include <stdlib.h>                     
//#include "DatapointMonitor.h"

class DataAccessClientOPCUA;
class PluginsBase;
class AsynchronousThread;

class CDM;

class SetDatapointThread : public LAPPThread
{
public:
    // TODO: refactor SetDatapointThread as a template instead of overloading?
    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<Byte> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vbyte = data;

        m_varType = varType::isVectorByte;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<std::string> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vstring = data;

        m_varType = varType::isVectorString;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<double> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vdouble = data;

        m_varType = varType::isVectorDouble;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::vector<float> data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_vfloat = data;

        m_varType = varType::isVectorFloat;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       std::string data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_str = data;

        m_varType = varType::isString;
        start(&m_varType);
    };

  
    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       float data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_float = data;

        m_varType = varType::isFloat;
        start(&m_varType);
    };
  
    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       double data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_double = data;

        m_varType = varType::isDouble;
        start(&m_varType);
    };

    SetDatapointThread(DataAccessClientOPCUA *dataAccessClientOPCUA, std::string datapointName, int nameSpace,
                       int data)
    {
        m_dataAccessClientOPCUA = dataAccessClientOPCUA;
        m_datapointName = datapointName;
        m_nameSpace = nameSpace;
        m_data_int = data;

        m_varType = varType::isInt;

        start(&m_varType);
    };

    ~SetDatapointThread(){};
  
  void *run(void *params)
  {
    //  std::string temString = m_datapointName + "._Done";
    //std::cout << "RR: run params: " << *(static_cast<varType*>(params)) << std::endl;
    //LOG_TRACE << "RR: SetDatapointThread::run"<<std::endl;
    //if (*(static_cast<varType *>(params)) == varType::isVectorByte) {
    if (m_varType == varType::isVectorByte) {
      //LOG_TRACE << "RR: SetDatapointThread::run VectorByte: "<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vbyte);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorString) {
    else if (m_varType == varType::isVectorString) {
      //LOG_TRACE << "RR: SetDatapointThread::run VectorString: "<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vstring);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorDouble) {
    else if (m_varType == varType::isVectorDouble) {
      //LOG_TRACE << "RR: SetDatapointThread::run VectorDouble: "<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vdouble);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorFloat) {
    else if (m_varType == varType::isVectorFloat) {
      //LOG_TRACE << "RR: SetDatapointThread::run VectorFloat: "<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vfloat);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isString) {
    else if (m_varType == varType::isString) {
      //LOG_TRACE << "RR: SetDatapointThread::run String: "<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_str);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isInt) {
    else if (m_varType == varType::isInt) {
      /* LOG_TRACE << "RR: SetDatapointThread::run Int: "<<m_datapointName
		<<" namespace "<<m_nameSpace
		<<" value "<<m_data_int
		<< " params "<< (*(static_cast<varType *>(params)))
		<< " vartype "<<m_varType
		<<std::endl;	       */
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_int);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isFloat) {
    else if (m_varType == varType::isFloat) {
      /*  LOG_TRACE << "RR: SetDatapointThread::run Float: "<<m_datapointName
		<< " namespace " << m_nameSpace
		<< " value "<<m_data_float
		<< " params "<< (*(static_cast<varType *>(params)))
		<< " vartype "<<m_varType
		<< " and for ref int val "<<m_data_int
		<<std::endl; */		
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_float);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isDouble) {
    else if (m_varType == varType::isDouble) {
      // LOG_TRACE << "RR: SetDatapointThread::run Double:"<<m_datapointName<<std::endl;
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_double);
      return nullptr;
    }
  };
  
private:
    int m_nameSpace;
    std::string m_datapointName;
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;

    int m_data_int;
    float m_data_float;
    double m_data_double;
    std::string m_data_str;
    std::vector<Byte> m_data_vbyte;
    std::vector<std::string> m_data_vstring;
    std::vector<double> m_data_vdouble;
    std::vector<float> m_data_vfloat;

    enum varType
    {
        isNULL = 0,
        isInt = 1,
        isFloat = 2,
        isDouble = 3,
        isString = 4,
        isVectorByte = 5,
        isVectorString = 6,
        isVectorDouble = 7,
        isVectorFloat = 8
    };
    varType m_varType = varType::isNULL;
};

class Helper
{
public:
    //Need a constructor first
    Helper(){
      // set up paths
      std::string basestr="/home/cdmmgr"; //defaults to home
      char *cdmpath;
      cdmpath = std::getenv("CDMPATH");
      if (cdmpath) {
	basestr=cdmpath;
      } else {          
        LOG_ERROR << "Helper init: CDMPATH not defined, paths default to /home/cdmmgr"
                  << std::endl;
      }        
      imagePath = basestr+"/output/images/";
      fitsPath = basestr+"/output/fits/";                           
    };

    //int publish_datapoint(std::string datapoint_name, int nameSpace, int data );

    //void set_OPCUAref(DataAccessClientOPCUA*);

    // declare a  new method

    double get_Ra_drive() { return Helper::Ra_drive; }
    double get_Dec_drive() { return Helper::Dec_drive; }
    double get_Ra_target() { return Helper::Ra_target; }
    double get_Dec_target() { return Helper::Dec_target; }
    double get_Azimuth() { return Helper::azimuth; }
    double get_Zenith() { return Helper::zenith; }
    //double get_exposure() { return Helper::exposure; }
    double get_OffsetAzimuth() { return Helper::offset_azimuth; }
    double get_OffsetZenith() { return Helper::offset_zenith; }
    int get_LEDs_state() { return Helper::LEDs_state; }
    bool get_OARL_state() { return Helper::OARL_state; }
    int get_Shutter_state() { return Helper::shutter_state; }
    int get_SIS_state() { return Helper::SIS_state; }
    bool get_Drive_status_in_motion() { return Helper::drive_status_in_motion; }
    bool get_Drive_status_in_parking_position() { return Helper::drive_status_in_parking_position; }
    bool get_Drive_status_parked() { return Helper::drive_status_parked; }
    bool get_Drive_status_tracking_in_progress() { return Helper::drive_status_tracking_in_progress; }
    bool get_Aux_status_DM_East_Bottom() { return Helper::aux_status_DM_East_Bottom; }
    bool get_Aux_status_DM_East_Top() { return Helper::aux_status_DM_East_Top; }
    bool get_Aux_status_DM_West_Bottom() { return Helper::aux_status_DM_West_Bottom; }
    bool get_Aux_status_DM_West_Top() { return Helper::aux_status_DM_West_Top; }

    int get_nImagesGet() { return Helper::nImagesGet; }
    void set_nImagesGet(int nImagesGet) { Helper::nImagesGet = nImagesGet; }
    std::string get_StarName() { return Helper::StarName; }
    std::string get_Comment() { return Helper::Comment; }

    std::string get_fitsPath() { return Helper::fitsPath; }
    std::string get_remoteImagePathPrefix() { return Helper::remoteImagePathPrefix; }

    void set_Comment(std::string Comment_tmp)
    {
        Helper::Comment = Comment_tmp;
        std::cout << "Comment is: " << get_Comment() << std::endl;
    }

    std::string searchDatapoint (std::string element, Config *config);
    int connectOpcUa_DataBroker(std::string url, CDM *cdm);

    DataAccessClientOPCUA *get_client_DataBroker() { return m_clientOpcUaRef_DataBroker; }

    int SetRaDrive(double newvalue)
    {
        Ra_drive = newvalue;
        return 0;
    }
    int SetDecDrive(double newvalue)
    {
        Dec_drive = newvalue;
        return 0;
    }
    int SetRaTarget(double newvalue)
    {
        Ra_target = newvalue;
        return 0;
    }
    int SetDecTarget(double newvalue)
    {
        Dec_target = newvalue;
        return 0;
    }
    int SetAz(double newvalue)
    {
        azimuth = newvalue;
        return 0;
    }
    int SetZd(double newvalue)
    {
        zenith = newvalue;
        return 0;
    }
    int SetAzOffset(double newvalue)
    {
        offset_azimuth = newvalue;
        return 0;
    }
    int SetZdOffset(double newvalue)
    {
        offset_zenith = newvalue;
        return 0;
    }
    int SetSource(std::string newvalue)
    {
        StarName = newvalue;
        return 0;
    }
    int SetOARL(bool newvalue)
    {
        OARL_state = newvalue;
        return 0;
    }
    int SetLEDs(bool newvalue)
    {
        LEDs_state = newvalue;
        return 0;
    }
    int SetShutter(int newvalue)
    {
        shutter_state = newvalue;
        return 0;
    }
    int SetSIS(int newvalue)
    {
        SIS_state = newvalue;
        return 0;
    }
    int SetDriveInMotion(bool newvalue)
    {
        drive_status_in_motion = newvalue;
        return 0;
    }
    int SetDriveInParkingPos(bool newvalue)
    {
        drive_status_in_parking_position = newvalue;
        return 0;
    }
    int SetDriveParked(bool newvalue)
    {
        drive_status_parked = newvalue;
        return 0;
    }    
    int SetDriveTracking(bool newvalue)
    {
        drive_status_tracking_in_progress = newvalue;
        return 0;
    }

    int SetAuxDMEastBottom(bool newvalue)
    {
        aux_status_DM_East_Bottom = newvalue;
        return 0;
    }
        int SetAuxDMEastTop(bool newvalue)
    {
        aux_status_DM_East_Top = newvalue;
        return 0;
    }
    int SetAuxDMWestBottom(bool newvalue)
    {
        aux_status_DM_West_Bottom = newvalue;
        return 0;
    }
        int SetAuxDMWestTop(bool newvalue)
    {
        aux_status_DM_West_Top = newvalue;
        return 0;
    }


    long int unix_timestamp();
    std::string UTC_time();

private:
    // Helper stuff

    // declare a new attribute
    DataAccessClientOPCUA *m_clientOpcUaRef_DataBroker = NULL;

    //DataAccessClientOPCUA* m_clientOpcUaRef_this=NULL;

    bool OARL_state = 0;
    bool LEDs_state = 0;
    int shutter_state = 0;
    int SIS_state = 0;
    double zenith = -999;
    double azimuth = -999;
    double offset_azimuth = 0;
    double offset_zenith = 0;
    double Ra_drive = -999;
    double Dec_drive = -999;
    double Ra_target = -999;
    double Dec_target = -999;
    bool drive_status_in_motion = 0;
    bool drive_status_in_parking_position = 0;
    bool drive_status_parked = 0;
    bool drive_status_tracking_in_progress = 0;
    bool aux_status_DM_East_Bottom = 0;
    bool aux_status_DM_East_Top = 0;
    bool aux_status_DM_West_Bottom = 0;
    bool aux_status_DM_West_Top = 0;
    int nImagesGet = 1;

    std::string StarName = "";
    std::string Comment = "";
    std::string imagePath;
    std::string fitsPath;
    std::string remoteImagePathPrefix = "/fefs/home/lapp/CDM_Images/";
};

#endif //  Helper_H_
