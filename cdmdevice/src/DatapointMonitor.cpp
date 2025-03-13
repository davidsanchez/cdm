#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#include "DatapointMonitor.h"
#include "CDM.h"
#include "Config.h"
#include "Logging.h"


#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"

DatapointMonitor::DatapointMonitor(CDM *caller)
{
    /*!
    Constructor of the class.

    @param caller
        Calling class instance, that will be used to issue commands on data change.
    */

    this->caller = caller;
}

//void DatapointMonitor::dataChange(std::vector<std::string> listElements, std::vector<std::string> listValues, std::vector<int> listQuality)
void DatapointMonitor::dataChange(std::vector<std::string> listElements, std::vector<std::string> listValues, std::vector<int> listQuality, std::vector<std::string> listTime)
{
    /*!
    This method is executed on OPC-UA data change. 
    When called it will check if this->target_name was changed
    and then run the methods of its caller instance.
    

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    CheckRaUpdate(listElements, listValues);
    CheckDecUpdate(listElements, listValues);
    CheckAzUpdate(listElements, listValues);
    CheckZdUpdate(listElements, listValues);
    CheckAzOffsetUpdate(listElements, listValues);
    CheckZdOffsetUpdate(listElements, listValues);
    CheckSourceUpdate(listElements, listValues);
    CheckOARLUpdate(listElements, listValues);
    CheckLEDsUpdate(listElements, listValues);
    CheckShutterUpdate(listElements, listValues);
    CheckSISUpdate(listElements, listValues);

    CheckDriveInMotionUpdate(listElements, listValues);
    CheckDriveInparkingPosUpdate(listElements, listValues);
    CheckDriveParkedUpdate(listElements, listValues);
    CheckDriveTrackingUpdate(listElements, listValues);
    CheckDriveRaTargetUpdate(listElements, listValues);
    CheckDriveDecTargetUpdate(listElements, listValues);

    CheckAuxDMEastBottomUpdate(listElements, listValues);
    CheckAuxDMEastTopUpdate(listElements, listValues);
    CheckAuxDMWestBottomUpdate(listElements, listValues);
    CheckAuxDMWestTopUpdate(listElements, listValues);

}

std::vector<std::string> DatapointMonitor::getElements()
{
    /*!
    Returns the monitored MOS data points list.
    */

    std::vector<std::string> elements;

    elements.push_back(this->ra_tel_var_name);
    elements.push_back(this->dec_tel_var_name);
    elements.push_back(this->az_var_name);
    elements.push_back(this->zd_var_name);
    elements.push_back(this->az_offset_var_name);
    elements.push_back(this->zd_offset_var_name);
    elements.push_back(this->source_var_name);
    elements.push_back(this->oarl_var_name);
    elements.push_back(this->leds_var_name);
    elements.push_back(this->shutter_var_name);
    

    elements.push_back(this->drive_inmotion_var_name);
    elements.push_back(this->drive_inparkinpos_var_name);
    elements.push_back(this->drive_parked_var_name);
    elements.push_back(this->drive_tracking_var_name);
    elements.push_back(this->drive_ra_target_var_name);
    elements.push_back(this->drive_dec_target_var_name);

    elements.push_back(this->aux_DMEastBottom_var_name);
    elements.push_back(this->aux_DMEastTop_var_name);
    elements.push_back(this->aux_DMWestBottom_var_name);
    elements.push_back(this->aux_DMWestTop_var_name);
    elements.push_back(this->sis_var_name);


    return elements;
}


std::vector<int> DatapointMonitor::getNameSpaces()
{
    /*!
    Returns the monitored MOS data points name spaces list.
    */

    std::vector<int> namespaces;

    auto elements = getElements();

    // TODO: No need for a for loop. You can initialize it with 2 with desired size.
    for (unsigned int i = 0; i < elements.size(); i++)
        namespaces.push_back(2);

    return namespaces;
}


int DatapointMonitor::GetTargetIndex(std::vector<std::string> listElements, std::vector<std::string> listValues, std::string var_name)
{
    int target_index = -1;

    auto iterator = std::find(listElements.begin(), listElements.end(), var_name);

    if (iterator != listElements.end())  target_index = iterator - listElements.begin();
    else LOG_FATAL<<"Element "<<var_name<<" not found" << std::endl;
    
    COND_LOG_DEBUG<<" Value of taget index "<<target_index<<std::endl;
    return target_index;
}


void DatapointMonitor::CheckRaUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the RA data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */


    int index = GetTargetIndex(listElements, listValues, this->ra_tel_var_name);
    if (index != -1 ) {
        double ra_value = std::stod(listValues[index]);
        caller->UpdateRaValue(ra_value);
    }
    /*
    int target_index;
    auto ra_iterator = std::find(listElements.begin(), listElements.end(), this->ra_tel_var_name);

    if (ra_iterator != listElements.end())
    {
        // RA data point updated, acting
        target_index = ra_iterator - listElements.begin();

        double ra_value = std::stod(listValues[target_index]);
        caller->UpdateRaValue(ra_value);
    }*/
}

void DatapointMonitor::CheckDecUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the Dec data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->dec_tel_var_name);
    if (index != -1 ) {
        double dec_value = std::stod(listValues[index]);
        caller->UpdateDecValue(dec_value);
    }
    /*
    int target_index;

    auto dec_iterator = std::find(listElements.begin(), listElements.end(), this->dec_tel_var_name);

    if (dec_iterator != listElements.end())
    {
        // Dec data point updated, acting
        target_index = dec_iterator - listElements.begin();

        double dec_value = std::stod(listValues[target_index]);
        caller->UpdateDecValue(dec_value);
    }*/
}

void DatapointMonitor::CheckAzUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    
    /*
    Checks if the Az data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */
    int index = GetTargetIndex(listElements, listValues, this->az_var_name);
    if (index != -1 ) {
        double az_value = std::stod(listValues[index]);
        caller->UpdateAzValue(az_value);
    }
    /*
    int target_index;

    auto az_iterator = std::find(listElements.begin(), listElements.end(), this->az_var_name);

    if (az_iterator != listElements.end())
    {
        // Az data point updated, acting
        target_index = az_iterator - listElements.begin();

        double az_value = std::stod(listValues[target_index]);
        caller->UpdateAzValue(az_value);
    }*/
}

void DatapointMonitor::CheckZdUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the Zd data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->zd_var_name);
    if (index != -1 ) {
        double zd_value = std::stod(listValues[index]);
        caller->UpdateZdValue(zd_value);
    }
    /*
    int target_index;
    auto zd_iterator = std::find(listElements.begin(), listElements.end(), this->zd_var_name);
    
    if (zd_iterator != listElements.end())
    {
        // Zd data point updated, acting
        target_index = zd_iterator - listElements.begin();
        
        double zd_value = std::stod(listValues[target_index]);
        caller->UpdateZdValue(zd_value);
    }*/
}

void DatapointMonitor::CheckAzOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the AzOffset data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->az_offset_var_name);
    if (index != -1 ) {
        double az_offset_value = std::stod(listValues[index]);
        caller->UpdateAzOffsetValue(az_offset_value);
    }

    /*
    int target_index;

    auto az_offset_iterator = std::find(listElements.begin(), listElements.end(), this->az_offset_var_name);

    if (az_offset_iterator != listElements.end())
    {
        // Az_offset data point updated, acting
        target_index = az_offset_iterator - listElements.begin();

        double az_offset_value = std::stod(listValues[target_index]);
        caller->UpdateAzOffsetValue(az_offset_value);
    }*/
}

void DatapointMonitor::CheckZdOffsetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the Zd_offset data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->zd_offset_var_name);
    if (index != -1 ) {
        double zd_offset_value = std::stod(listValues[index]);
        caller->UpdateZdOffsetValue(zd_offset_value);
    }
    /*
    int target_index;

    auto zd_offset_iterator = std::find(listElements.begin(), listElements.end(), this->zd_offset_var_name);

    if (zd_offset_iterator != listElements.end())
    {
        // Zd_offset data point updated, acting
        target_index = zd_offset_iterator - listElements.begin();

        double zd_offset_value = std::stod(listValues[target_index]);
        caller->UpdateZdOffsetValue(zd_offset_value);
    }*/
}

void DatapointMonitor::CheckSourceUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the Source data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */
    int index = GetTargetIndex(listElements, listValues, this->zd_offset_var_name);
    if (index != -1 ) {
        double zd_offset_value = std::stod(listValues[index]);
        caller->UpdateZdOffsetValue(zd_offset_value);
    }
    /*

    int target_index;

    auto source_iterator = std::find(listElements.begin(), listElements.end(), this->source_var_name);

    if (source_iterator != listElements.end())
    {
        // Source data point updated, acting
        target_index = source_iterator - listElements.begin();

        string source_value = listValues[target_index];
        caller->UpdateSourceValue(source_value);
    }*/
}

void DatapointMonitor::CheckOARLUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the OARL data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    /*
    int target_index;

    auto oarl_iterator = std::find(listElements.begin(), listElements.end(), this->oarl_var_name);
    COND_LOG_DEBUG<<" OLD way  this->oarl_var_name "<< this->oarl_var_name<<std::endl;
    if (oarl_iterator != listElements.end())
    {
        // OARL data point updated, acting
        target_index = oarl_iterator - listElements.begin();
        COND_LOG_DEBUG<<" FOUND OLD way target_index "<<target_index<<std::endl;
        // Converts the string to bool value.
        bool oarl_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> oarl_value;
        COND_LOG_DEBUG<<" FOUND OLD way oarl_value "<<oarl_value<<std::endl;
        caller->UpdateOARLValue(oarl_value);
    }*/

    int index = GetTargetIndex(listElements, listValues, this->oarl_var_name);
    COND_LOG_DEBUG<<" NEW way index "<<index<<std::endl;
    if (index != -1 ) {
        bool oarl_value;
        istringstream(listValues[index]) >> std::boolalpha >> oarl_value;
        COND_LOG_DEBUG<<" FOUND NEW way oarl_value "<<oarl_value<<std::endl;
        caller->UpdateOARLValue(oarl_value);
    }
    else {COND_LOG_DEBUG<<" No UPDATE of OARL "<<std::endl;}

}

void DatapointMonitor::CheckLEDsUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the LEDs data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->leds_var_name);
    if (index != -1 ) {
        // Converts the string to bool value.
        bool leds_value;
        istringstream(listValues[index]) >> std::boolalpha >> leds_value;
        caller->UpdateLEDsValue(leds_value);
    }
    /*
    int target_index;

    auto leds_iterator = std::find(listElements.begin(), listElements.end(), this->leds_var_name);

    if (leds_iterator != listElements.end())
    {
        // LEDs data point updated, acting
        target_index = leds_iterator - listElements.begin();
        
        // Converts the string to bool value.
        bool leds_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> leds_value;
        caller->UpdateLEDsValue(leds_value);
    }*/
}

void DatapointMonitor::CheckShutterUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    
/*
    std::cout << "Elements de listElements :" << std::endl;
    for (const auto& element : listElements) {
        std::cout << element << std::endl;
    }

    int target_index;

    auto shutter_iterator = std::find(listElements.begin(), listElements.end(), this->shutter_var_name);

    if (shutter_iterator != listElements.end())
    {
        
        // Shutter data point updated, acting
        std::cout<<"shutter_iterator "<<*shutter_iterator<<std::endl;
        target_index = shutter_iterator - listElements.begin();
        std::cout<<"target_index "<<target_index<<std::endl;
        int shutter_value = std::stoi(listValues[target_index]);
        std::cout<<"shutter_value "<<shutter_value<<std::endl;
        caller->UpdateShutterValue(shutter_value);
    }*/
}

void DatapointMonitor::CheckSISUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{

    /*


   
 
        */
    }



void DatapointMonitor::CheckDriveInMotionUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the DriveInMotion data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int index = GetTargetIndex(listElements, listValues, this->drive_inmotion_var_name);
    if (index != -1 ) {
        // Converts the string to bool value.
        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveInMotionValue(drive_value);
    }
    /*
    int target_index;

    auto drive_iterator = std::find(listElements.begin(), listElements.end(), this->drive_inmotion_var_name);

    if (drive_iterator != listElements.end())
    {
        // drive data point updated, acting
        target_index = drive_iterator - listElements.begin();

        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveInMotionValue(drive_value);
    }*/
}

void DatapointMonitor::CheckDriveInparkingPosUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*
    Checks if the DriveInParkinPos data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */


    int index = GetTargetIndex(listElements, listValues, this->drive_inparkinpos_var_name);
    if (index != -1 ) {
        // Converts the string to bool value.
        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveInParkingPosValue(drive_value);
    }
    /*
    int target_index;

    auto drive_iterator = std::find(listElements.begin(), listElements.end(), this->drive_inparkinpos_var_name);

    if (drive_iterator != listElements.end())
    {
        // drive data point updated, acting
        target_index = drive_iterator - listElements.begin();

        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveInParkingPosValue(drive_value);
    }*/
}

void DatapointMonitor::CheckDriveParkedUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the DriveParked data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto drive_iterator = std::find(listElements.begin(), listElements.end(), this->drive_parked_var_name);

    if (drive_iterator != listElements.end())
    {
        // drive data point updated, acting
        target_index = drive_iterator - listElements.begin();

        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveParkedValue(drive_value);
    }
}

void DatapointMonitor::CheckDriveTrackingUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the DriveTracking data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto drive_iterator = std::find(listElements.begin(), listElements.end(), this->drive_tracking_var_name);

    if (drive_iterator != listElements.end())
    {
        // drive data point updated, acting
        target_index = drive_iterator - listElements.begin();

        // Converts the string to bool value.
        bool drive_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> drive_value;
        caller->UpdateDriveTrackingValue(drive_value);
    }
}


void DatapointMonitor::CheckDriveRaTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the RA target data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto ra_iterator = std::find(listElements.begin(), listElements.end(), this->drive_ra_target_var_name);

    if (ra_iterator != listElements.end())
    {
        // RA data point updated, acting
        target_index = ra_iterator - listElements.begin();

        double drive_ra_value = std::stod(listValues[target_index]);
        caller->UpdateDriveRaTargetValue(drive_ra_value);
    }
}

void DatapointMonitor::CheckDriveDecTargetUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the Dec target data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto dec_iterator = std::find(listElements.begin(), listElements.end(), this->drive_dec_target_var_name);

    if (dec_iterator != listElements.end())
    {
        // Dec data point updated, acting
        target_index = dec_iterator - listElements.begin();

        double drive_dec_value = std::stod(listValues[target_index]);
        caller->UpdateDriveDecTargetValue(drive_dec_value);
    }
}


void DatapointMonitor::CheckAuxDMEastBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the Aux DM East Bottom data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto aux_iterator = std::find(listElements.begin(), listElements.end(), this->aux_DMEastBottom_var_name);

    if (aux_iterator != listElements.end())
    {
        // aux data point updated, acting
        target_index = aux_iterator - listElements.begin();

        // Converts the string to bool value.
        bool aux_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> aux_value;
        caller->UpdateAuxDMEastBottomValue(aux_value);
    }
}


void DatapointMonitor::CheckAuxDMEastTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the Aux DM East Top data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto aux_iterator = std::find(listElements.begin(), listElements.end(), this->aux_DMEastTop_var_name);

    if (aux_iterator != listElements.end())
    {
        // aux data point updated, acting
        target_index = aux_iterator - listElements.begin();

        // Converts the string to bool value.
        bool aux_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> aux_value;
        caller->UpdateAuxDMEastTopValue(aux_value);
    }
}


void DatapointMonitor::CheckAuxDMWestBottomUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the Aux DM West Bottom data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto aux_iterator = std::find(listElements.begin(), listElements.end(), this->aux_DMWestBottom_var_name);

    if (aux_iterator != listElements.end())
    {
        // aux data point updated, acting
        target_index = aux_iterator - listElements.begin();

        // Converts the string to bool value.
        bool aux_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> aux_value;
        caller->UpdateAuxDMWestBottomValue(aux_value);
    }
}


void DatapointMonitor::CheckAuxDMWestTopUpdate(std::vector<std::string> listElements, std::vector<std::string> listValues)
{
    /*!
    Checks if the Aux DM West Top data point was changed 
    and reports the update to the "caller" object in such a case.

    @param listElements
        List of changed data points
    @param listValues
        List of values of the changed data points
    */

    int target_index;

    auto aux_iterator = std::find(listElements.begin(), listElements.end(), this->aux_DMWestTop_var_name);

    if (aux_iterator != listElements.end())
    {
        // aux data point updated, acting
        target_index = aux_iterator - listElements.begin();

        // Converts the string to bool value.
        bool aux_value;
        istringstream(listValues[target_index]) >> std::boolalpha >> aux_value;
        caller->UpdateAuxDMWestTopValue(aux_value);
    }
}
