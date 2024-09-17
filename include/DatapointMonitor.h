#ifndef DATAPOINTMONITOR_H_
#define DATAPOINTMONITOR_H_

#include <MOS_callbackInterface.h>

class CDM;

class DatapointMonitor : public MOS_CallbackInterface {
public:
    DatapointMonitor(CDM *caller);
    void dataChange(std::vector<std::string> listElements, std::vector<std::string> listValues, std::vector<int> listQuality);
    void infoDebug(){};
    void abort(){};

    std::vector<std::string> getElements();
    std::vector<int> getNameSpaces();

private:

    CDM *caller;

    //const std::string led01_var_name =   "DataBroker.SecurityBrokerControl.CameraControl.Led_01.Led_01_v"; 
    const std::string leds_var_name =    "DataBroker.SecurityBrokerControl.CameraControl.ECC.Monitoring.LEDPositions.GeneralStatus.GeneralStatus_v"; 
    const std::string shutter_var_name = "DataBroker.SecurityBrokerControl.CameraControl.ECC.Monitoring.Shutter.GeneralStatus.GeneralStatus_v"; 
    const std::string sis_var_name =     "DataBroker.SecurityBrokerControl.CameraControl.ModulesStatus.SIS.SIS_v"; 

    const std::string oarl_var_name =             "DataBroker.SecurityBrokerControl.AuxControl.RemotePowerCtrl.OARLRelay_Status.OARLRelay_Status_v"; 
    const std::string aux_DMEastBottom_var_name = "DataBroker.SecurityBrokerControl.AuxControl.DM.DM_East_Bottom.Status.Status_v"; 
    const std::string aux_DMEastTop_var_name =    "DataBroker.SecurityBrokerControl.AuxControl.DM.DM_East_Top.Status.Status_v";  
    const std::string aux_DMWestBottom_var_name = "DataBroker.SecurityBrokerControl.AuxControl.DM.DM_West_Bottom.Status.Status_v"; 
    const std::string aux_DMWestTop_var_name =    "DataBroker.SecurityBrokerControl.AuxControl.DM.DM_West_Top.Status.Status_v"; 

    const std::string drive_ra_target_var_name =   "DataBroker.SecurityBrokerControl.DriveControl.RA_Target.RA_Target_v";
    const std::string drive_dec_target_var_name =  "DataBroker.SecurityBrokerControl.DriveControl.Dec_Target.Dec_Target_v"; 

    const std::string az_offset_var_name = "DataBroker.SecurityBrokerControl.DriveControl.Azimuth_Offset.Azimuth_Offset_v"; 
    const std::string zd_offset_var_name = "DataBroker.SecurityBrokerControl.DriveControl.ZenithAngle_Offset.ZenithAngle_Offset_v"; 
    const std::string source_var_name =    "DataBroker.SecurityBrokerControl.DriveControl.SourceName.SourceName_v"; 
    const std::string ra_tel_var_name =    "DataBroker.SecurityBrokerControl.DriveControl.RA_Telescope.RA_Telescope_v"; 
    const std::string dec_tel_var_name =   "DataBroker.SecurityBrokerControl.DriveControl.Dec_Telescope.Dec_Telescope_v"; 
    const std::string az_var_name =        "DataBroker.SecurityBrokerControl.DriveControl.CurrentPosition.azimuth_position.azimuth_position_v"; 
    const std::string zd_var_name =        "DataBroker.SecurityBrokerControl.DriveControl.CurrentPosition.zenithangle_position.zenithangle_position_v"; 

    const std::string drive_inmotion_var_name =    "DataBroker.SecurityBrokerControl.DriveControl.Status.Status_In_Motion.Status_In_Motion_v"; 
    const std::string drive_inparkinpos_var_name = "DataBroker.SecurityBrokerControl.DriveControl.Status.Status_In_Parking_Position.Status_In_Parking_Position_v"; 
    const std::string drive_parked_var_name =      "DataBroker.SecurityBrokerControl.DriveControl.Status.Status_Parked.Status_Parked_v"; 
    const std::string drive_tracking_var_name =    "DataBroker.SecurityBrokerControl.DriveControl.Status.Status_Tracking_In_Progress.Status_Tracking_In_Progress_v"; 



    void CheckRaUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDecUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAzOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckZdOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSourceUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckOARLUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    // void CheckLED01Update(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckLEDsUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckShutterUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckSISUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

    void CheckDriveInMotionUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveInparkingPosUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveParkedUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveTrackingUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveRaTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveDecTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);


    void CheckAuxDMEastBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMEastTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);


    
};

#endif //DATAPOINTMONITOR_H_
