#ifndef Helper_H_
#define Helper_H_

#include "pluginsBase.h"
#include "lappThread.h" // needed for MOS
//#include "test_asynchroneThread.h"

class DataAccessClientOPCUA;
class PluginsBase;
class TestAsynchroneThread;
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
         //std::cout << "Params: " << *(static_cast<varType*>(params)) << std::endl;

        if (*(static_cast<varType *>(params)) == varType::isVectorByte)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vbyte);
        else if (*(static_cast<varType *>(params)) == varType::isVectorString)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_vstring);
        else if (*(static_cast<varType *>(params)) == varType::isString)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_str);
        else if (*(static_cast<varType *>(params)) == varType::isInt)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_int);
        else if (*(static_cast<varType *>(params)) == varType::isFloat)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_float);
        else if (*(static_cast<varType *>(params)) == varType::isDouble)
            m_dataAccessClientOPCUA->setDatapoint(m_datapointName, m_nameSpace, m_data_double);
    };

private:
    int m_nameSpace;
    std::string m_datapointName;
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;

    int m_data_int;
    float m_data_float;
    double m_data_double;
    std::vector<Byte> m_data_vbyte;
    std::vector<std::string> m_data_vstring;
    std::string m_data_str;  

    enum varType
    {
        isNULL = 0,
        isInt = 1, 
        isFloat = 2,
        isDouble = 3,
        isString = 4, 
        isVectorByte = 5, 
        isVectorString = 6
    };
    varType m_varType = varType::isNULL;
};


class Helper
{
public:
    //Need a constructor first 
    Helper()
    {
        
    };

    //int publish_datapoint(std::string datapoint_name, int nameSpace, int data );
    
    //void set_OPCUAref(DataAccessClientOPCUA*);

    // declare a  new method
    double acquire_RA();
    double acquire_DEC();
    double acquire_Azimuth();
    double acquire_Zenith();
    int acquire_LED_intensity();
    bool acquire_OARL_state();
    bool acquire_drive_status_in_motion();
    bool acquire_drive_status_in_parking_position();
    bool acquire_drive_status_parked();
    bool acquire_drive_status_tracking_in_progress();
    std::string acquire_StarName();

    double get_RA() { return Helper::RA; }
    double get_DEC() { return Helper::DEC; }
    double get_Azimuth() { return Helper::azimuth; }
    double get_Zenith() { return Helper::zenith; }
    //double get_exposure() { return Helper::exposure; }
    double get_OffsetAzimuth() { return Helper::offset_azimuth; }
    double get_OffsetZenith() { return Helper::offset_zenith; }
    int get_LED_intensity() {return Helper::LED_intensity;}
    bool get_OARL_state() {return Helper::OARL_state;}
    bool get_Drive_status_in_motion() {return Helper::drive_status_in_motion;}
    bool get_Drive_status_in_parking_position() {return Helper::drive_status_in_parking_position;}
    bool get_Drive_status_parked() {return Helper::drive_status_parked;}
    bool get_Drive_status_tracking_in_progress() {return Helper::drive_status_tracking_in_progress;}

    int get_nImagesGet() {return Helper::nImagesGet;}
    int set_nImagesGet(int nImagesGet) {Helper::nImagesGet = nImagesGet;}
    std::string get_StarName() { return Helper::StarName; }
    std::string get_Comment() { return Helper::Comment; }

    std::string get_fitsPath() {return Helper::fitsPath; }
    std::string get_remoteImagePathPrefix() {return Helper::remoteImagePathPrefix; }
    
    void set_StarName(std::string StarName_tmp) 
    {
        Helper::StarName=StarName_tmp; 
        std::cout<<"StarName is: " << get_StarName() <<std::endl;
    }

    void set_Comment(std::string Comment_tmp) 
    {
        Helper::Comment=Comment_tmp; 
        std::cout<<"Comment is: " << get_Comment() <<std::endl;
    }

    int connectOpcUa_Drive(std::string url);
    int connectOpcUa_Relay(std::string url);
    int connectOpcUa_ECC(std::string url);

    long int unix_timestamp();
    std::string UTC_time();
    
private:
    // Helper stuff

    // declare a new attribute
    DataAccessClientOPCUA* m_clientOpcUaRef_Drive=NULL;
    DataAccessClientOPCUA* m_clientOpcUaRef_Relay=NULL;
    DataAccessClientOPCUA* m_clientOpcUaRef_ECC=NULL;

    //DataAccessClientOPCUA* m_clientOpcUaRef_this=NULL;

    int LED_intensity=0;
    bool OARL_state=0;
    double zenith = 0;
    double azimuth = 0;
    double offset_azimuth = 0;
    double offset_zenith = 0;
    double RA = 0;
    double DEC = 0;
    bool drive_status_in_motion=0;
    bool drive_status_in_parking_position=0;
    bool drive_status_parked=0;
    bool drive_status_tracking_in_progress=0;
    int nImagesGet=1;

    std::string StarName = "";
    std::string Comment = "";
    std::string imagePath = "/home/lstoperator/CDM/images/";
    std::string fitsPath = "/home/lstoperator/CDM/fits/";
    std::string remoteImagePathPrefix = "/fefs/home/lapp/CDM_Images/";

};


#endif //  Helper_H_