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

    // TODO: Temporary until all aux datapoints added to DataBroker
    std::vector<std::string> getElements_aux();
    std::vector<int> getNameSpaces_aux();

private:

    CDM *caller;

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
    const std::string drive_ra_target_var_name = "Drive.DriveControl.RA_Target.RA_Target_v"; 
    const std::string drive_dec_target_var_name = "Drive.DriveControl.Dec_Target.Dec_Target_v"; 

    // These datapoints are in Aux only currently
    const std::string aux_DMEastBottom_var_name = "Auxiliary.AuxControl.DM_East_Bottom.Status.Status_v"; 
    const std::string aux_DMEastTop_var_name = "Auxiliary.AuxControl.DM_East_Top.Status.Status_v"; 
    const std::string aux_DMWestBottom_var_name = "Auxiliary.AuxControl.DM_West_Bottom.Status.Status_v"; 
    const std::string aux_DMWestTop_var_name = "Auxiliary.AuxControl.DM_West_Top.Status.Status_v"; 

    void CheckRaUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
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
    void CheckDriveRaTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckDriveDecTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);

    // These datapoints are in Aux only currently
    void CheckAuxDMEastBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMEastTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
    void CheckAuxDMWestTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues);
};

#endif //DATAPOINTMONITOR_H_
