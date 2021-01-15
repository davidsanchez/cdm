#ifndef DATAPOINTMONITOR_H_
#define DATAPOINTMONITOR_H_

#include <MOS_callbackInterface.h>

class CDM;

class DatapointMonitor : public MOS_CallbackInterface {
public:
    DatapointMonitor(CDM *caller);
    void dataChange(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void infoDebug(){};
    void abort(){};

    std::vector<std::string> getElements();
    std::vector<int> getNameSpaces();

    // TODO: Temporary until all drive datapoints added to DataBroker
    std::vector<std::string> getElements_drive();
    std::vector<int> getNameSpaces_drive();

    

private:

    CDM *caller;

    const std::string test_var_name = "Unit_SB.SecurityBrokerControl.Power.CameraMain.Frame_I3.Frame_I3_v"; 

    const std::string ra_tel_var_name = "Unit_SB.SecurityBrokerControl.Drive.RA_Telescope.RA_Telescope_v"; 
    const std::string dec_tel_var_name = "Unit_SB.SecurityBrokerControl.Drive.Dec_Telescope.Dec_Telescope_v"; 
    const std::string az_var_name = "Unit_SB.SecurityBrokerControl.Drive.Az.Az_v"; 
    const std::string zd_var_name = "Unit_SB.SecurityBrokerControl.Drive.Zd.Zd_v"; 
    const std::string az_offset_var_name = "Unit_SB.SecurityBrokerControl.Drive.Azimuth_Offset.Azimuth_Offset_v"; 
    const std::string zd_offset_var_name = "Unit_SB.SecurityBrokerControl.Drive.ZenithAngle_Offset.ZenithAngle_Offset_v"; 
    const std::string source_var_name = "Unit_SB.SecurityBrokerControl.Drive.SourceName.SourceName_v"; 
    const std::string oarl_var_name = "Unit_SB.SecurityBrokerControl.Auxiliary.OARL_Status.OARL_Status_v"; 
    const std::string led01_var_name = "Unit_SB.SecurityBrokerControl.Camera.Led_01.Led_01_v"; 
    const std::string leds_var_name = "Unit_SB.SecurityBrokerControl.Camera.Led_Status.Led_Status_v"; 
    const std::string shutter_var_name = "Unit_SB.SecurityBrokerControl.Camera.Shutter_Status.Shutter_Status_v"; 
    const std::string sis_var_name = "Unit_SB.SecurityBrokerControl.Camera.SIS_Status.SIS_Status_v"; 

    // These datapoints are in Drive only currently
    const std::string drive_inmotion_var_name = "Drive.DriveControl.Status.Status_In_Motion.Status_In_Motion_v"; 
    const std::string drive_inparkinpos_var_name = "Drive.DriveControl.Status.Status_In_Parking_Position.Status_In_Parking_Position_v"; 
    const std::string drive_parked_var_name = "Drive.DriveControl.Status.Status_Parked.Status_Parked_v"; 
    const std::string drive_tracking_var_name = "Drive.DriveControl.Status.Status_Tracking_In_Progress.Status_Tracking_In_Progress_v"; 


    void CheckTestUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    
    void CheckRAUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDecUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSourceUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckOARLUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckLED01Update(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckLEDsUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckShutterUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSISUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

    // These datapoints are in Drive only currently
    void CheckDriveInMotionUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveInparkingPosUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveParkedUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveTrackingUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

};

#endif //DATAPOINTMONITOR_H_
