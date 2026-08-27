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
    //if (*(static_cast<varType *>(params)) == varType::isVectorByte) {
    if (m_varType == varType::isVectorByte) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vbyte);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorString) {
    else if (m_varType == varType::isVectorString) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vstring);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorDouble) {
    else if (m_varType == varType::isVectorDouble) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vdouble);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isVectorFloat) {
    else if (m_varType == varType::isVectorFloat) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vfloat);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isString) {
    else if (m_varType == varType::isString) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_str);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isInt) {
    else if (m_varType == varType::isInt) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_int);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isFloat) {
    else if (m_varType == varType::isFloat) {
      m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_float);
      return nullptr;
    }
    //else if (*(static_cast<varType *>(params)) == varType::isDouble) {
    else if (m_varType == varType::isDouble) {
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


    long int unix_timestamp();
    std::string UTC_time();

private:
    // Helper stuff

    // declare a new attribute
    DataAccessClientOPCUA *m_clientOpcUaRef_DataBroker = NULL;

    //DataAccessClientOPCUA* m_clientOpcUaRef_this=NULL;

    int nImagesGet = 1;

    std::string StarName = "";
    std::string Comment = "";
    std::string imagePath;
    std::string fitsPath;
    std::string remoteImagePathPrefix = "/fefs/home/lapp/CDM_Images/";
};

#endif //  Helper_H_
