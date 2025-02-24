/******************************************************************************
 **
 **
 ** Copyright (C) LAPP. CNRS
 **
 ** Project: C++ OPCUA generique
 **
 ** Description:
 **
 ** Author : Panazol Jean Luc
 ******************************************************************************/

#include "AsynchronousThread.h"
#include "dataAccessClientOPCUA.h"
#include "Config.h"
#include "Logging.h"

extern Camera camera;
extern Helper helper;

#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"


/*std::string  AsynchronousThread::searchDatapoint (string element)
{
        ListElement *myElement = NULL;
        std::string rootElement = "";
        std::string nodeIdL1 = "";

        myElement = cdm_config->searchElement(element);
        rootElement = cdm_config->getRootName() + ".";
        if (myElement)
        {
                nodeIdL1 = rootElement + myElement->NodeId;
                return(nodeIdL1);
        }
        return("");
}
*/


AsynchronousThread::AsynchronousThread(DataAccessClientOPCUA *dataAccessClientOPCUA)
{
    m_pause = false;
    m_stop = false;
    m_command = 0;
    m_cmdGetMultipleImages = 0;
    m_cmdGetMultipleImagesStacked = 0;
    m_cmdConfigure = 0;
    m_cmdStartCDM = 0;
    m_cmdStartStream = 0;
    m_cmdMeteo = 0;
    m_dataAccessClientOPCUA = dataAccessClientOPCUA;
    cdm_config = new Config(CDM_CONFIGURATION_NAME,"");
}

AsynchronousThread::~AsynchronousThread()
{
    m_pause = true;
    m_stop = true;
    wait();
}

void AsynchronousThread::pause()
{
    m_pause = true;
}

void AsynchronousThread::resume()
{
    m_pause = false;
}

int AsynchronousThread::stop()
{
    int ret = 0;
    m_stop = true;
    return ret;
}

int AsynchronousThread::cmdStartMeteo()
{
    int ret = 0;
    m_cmdMeteo = true;
    return ret;
}

int AsynchronousThread::cmdLogRestart()
{
    int ret = 0;
    m_cmdLogRestart = true;
    return ret;
}

int AsynchronousThread::cmdGetMultipleImages(std::string datapointName, int nameSpace, int n_images)
{
    int ret = 0;
    m_cmdGetMultipleImages = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    AsynchronousThread::n_images = n_images;
    return ret;
}

int AsynchronousThread::cmdGetMultipleImagesStacked(std::string datapointName, int nameSpace, int n_images)
{
    int ret = 0;
    m_cmdGetMultipleImagesStacked = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    AsynchronousThread::n_images = n_images;
    return ret;
}

int AsynchronousThread::cmdStartCDM(std::string datapointName, int nameSpace)
{
    int ret = 0;
    m_cmdStartCDM = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    return ret;
}

int AsynchronousThread::cmdStartStream(std::string datapointName, int nameSpace)
{
    int ret = 0;
    m_cmdStartStream = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    return ret;
}

int AsynchronousThread::cmdConfigure(std::string datapointName, int nameSpace, int nPixelClock, double exposure, double fps, int gain, std::string pixel_format)
{
    int ret = 0;
    m_cmdConfigure = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    AsynchronousThread::nPixelClock = nPixelClock;
    AsynchronousThread::exposure = exposure;
    AsynchronousThread::fps = fps;
    AsynchronousThread::gain = gain;
    AsynchronousThread::pixel_format = pixel_format;
    return ret;
}

void *AsynchronousThread::run(void *params)
{
    // this method run all the time after calling the method startRun() -> in your squeletonPlugin_for_asynchroneMethodCall.cpp file )
    // you can stop with m_stop=true with calling the method stop() (when the program finish)
    // or make a pause with m_pause=true with calling the methods pause() and resume()
    // you implement here your methods who take a long time to execute
    // Here an example with to methods closeShutter and openShutter
    std::string temString;
    std::string tempValue;
    int t = 0;
    while (m_stop == false)
    {
        if (m_pause == false)
        {
            //LOG_TRACE << "In Async thread";
            usleep(1000000);

            if (m_cmdGetMultipleImages == 1)
            {
                int FSM_state;

                // Puts the FSM.state to 4
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 4);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 4);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // inform that the command is in progress
                temString = m_datapointName + "._InProgressBar";
                t = 1;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                //std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images);
                std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images, m_dataAccessClientOPCUA);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, v_image_paths);

                // We want to push these imagePaths as single string
                std::string imagePath_cat;
                for (const auto &piece : v_image_paths)
                    imagePath_cat += "\n" + piece;
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, imagePath_cat);

                //m_dataAccessClientOPCUA->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, FSM_state);
                // If not in error state then go back to standard state.
                if (FSM_state != 5)
                    m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 3);
                    //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 3);

                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // you can put the outputs arguments in this place to inform the server
                // temString = m_datapointName + "._OutputArguments._Val_Retour";
                // tempValue = "command Open : c'est bon c'est fini : JL ";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                // temString = m_datapointName + "._Done";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 0;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdGetMultipleImages = 0;
            }

            if (m_cmdGetMultipleImagesStacked == 1)
            {
                int FSM_state;

                // Puts the FSM.state to 4
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 4);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 4);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // inform that the command is in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 1;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                //std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images);
                std::vector<std::string> v_image_paths = camera.GetMultipleImagesStacked(n_images, m_dataAccessClientOPCUA);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, v_image_paths);

                // We want to push these imagePaths as single string
                std::string imagePath_cat;
                for (const auto &piece : v_image_paths)
                    imagePath_cat += "\n" + piece;
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, imagePath_cat);

                //m_dataAccessClientOPCUA->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, FSM_state);
                // If not in error state then go back to standard state.
                if (FSM_state != 5)
                	m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 3);
                   // m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 3);

                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // you can put the outputs arguments in this place to inform the server
                // temString = m_datapointName + "._OutputArguments._Val_Retour";
                // tempValue = "command Open : c'est bon c'est fini : JL ";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                // temString = m_datapointName + "._Done";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 0;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdGetMultipleImagesStacked = 0;
            }

            if (m_cmdStartCDM == 1)
            {
                int FSM_state;

                cout << "In AsynchronousThread: cmdStartCDM" << endl;
                // Puts the FSM.state to 2
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 2);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 2);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // inform that the command is in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 1;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************
                camera.StartCDM(m_dataAccessClientOPCUA);

                // you can put the outputs arguments in this place to inform the server
                // temString = m_datapointName + "._OutputArguments._Val_Retour";
                // tempValue = "command Open : c'est bon c'est fini : JL ";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                // temString = m_datapointName + "._Done";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 0;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdStartCDM = 0;

                //m_dataAccessClientOPCUA->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, FSM_state);
                // If not in error state then go back to standard state.
                if (FSM_state != 5)
                	m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 1);
//                    m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 1);

                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                cout << "End of cdmStartCDM inside AsynchronousThread.cpp" << endl;
            }

            if (m_cmdStartStream == 1)
            {
                int FSM_state;

                cout << "In AsynchronousThread: cmdStartStream" << endl;
                // Puts the FSM.state to 6
//                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 6);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 6);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                // inform that the command is in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 1;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************
                camera.StartStream(m_dataAccessClientOPCUA);

                // you can put the outputs arguments in this place to inform the server
                // temString = m_datapointName + "._OutputArguments._Val_Retour";
                // tempValue = "command Open : c'est bon c'est fini : JL ";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                // temString = m_datapointName + "._Done";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 0;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdStartStream = 0;

 //               m_dataAccessClientOPCUA->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, FSM_state);
                // If not in error state then go back to standard state.
                if (FSM_state != 5)
                	m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_state",cdm_config), 2, 3);
//                    m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 1);
                
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(helper.searchDatapoint("CDM_FSM_transition",cdm_config), 2, 0);

                cout << "End of cdmStartStream inside AsynchronousThread.cpp" << endl;
            }

            if (m_cmdMeteo == 1)
            {
                vector<float> meteo_val = meteo.Update_sensor();
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.temperature.temperature_v", 2, meteo_val[0]);
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.humidity_relative.humidity_relative_v", 2, meteo_val[1]);
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.humidity_absolute.humidity_absolute_v", 2, meteo_val[2]);
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.pressure.pressure_v", 2, meteo_val[3]);


                // The logger will write first the date and time automatically before the these datapoints.
                // We don't make a loop to store this because in each loop the logger would automatically put endl.
                // Although You could bypass that by redefining the logger format.

                LOG_ENV << meteo_val[0] << " " // sensor - temperature
                        << meteo_val[1] << " " // sensor - humidity relative
                        << meteo_val[2] << " " // sensor - humidity absolute in g/m^3
                        << meteo_val[3]  // sensor - pressure in hPa
                    ;

                usleep(9000000);
            }

            if (m_cmdLogRestart == 1)
            {
                // Get current UTC time
                boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
                int h = time.time_of_day().hours();
                int m = time.time_of_day().minutes();
                int s = time.time_of_day().seconds();

                // Forces the log rotation in the morning. 
                // The log are already setup to rotate at 7:30 but there needs to be something written to log for it to rotate.
                if ((h == 7) && (m == 30) && (s == 2))
                {
                    cout << "Log rotation time." << endl;
                    LOG_ENV << " ";
                    LOG_TRACE << " ";
                    LOG_DATA << " ";
                    LOG_SETTINGS << " ";
                    
                }
            }

            if (m_cmdConfigure == 1)
            {
                // Checks the current FSM state so we can return to that state after Configure is done.
                int FSM_state;
//                m_dataAccessClientOPCUA->getDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
                m_dataAccessClientOPCUA->setDatapoint(searchDatapoint("CDM_FSM_state",cdm_config), 2, FSM_state);

                // Puts the FSM.transition to 1
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 1);
                m_dataAccessClientOPCUA->setDatapoint(searchDatapoint("CDM_FSM_transition",cdm_config), 2, 1);

                // inform that the command is in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 1;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");

                std::vector<boost::any> configure_settings = camera.Configure(nPixelClock, exposure, fps, gain, pixel_format);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.pixelClock.pixelClock_v", 2, boost::any_cast<int>(configure_settings[0]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.FPS.FPS_v", 2, boost::any_cast<double>(configure_settings[1]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.exposure.exposure_v", 2, boost::any_cast<double>(configure_settings[2]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.gain.gain_v", 2, boost::any_cast<int>((configure_settings[3])));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.Camera.pixelFormat.pixelFormat_v", 2, boost::any_cast<string>(configure_settings[4]));

                // you can put the outputs arguments in this place to inform the server
                // temString = m_datapointName + "._OutputArguments._Val_Retour";
                // tempValue = "command Open : c'est bon c'est fini : JL ";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                // temString = m_datapointName + "._Done";
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                // temString = m_datapointName + "._InProgressBar";
                // t = 0;
                // m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdConfigure = 0;

                // Puts the FSM.transition back to 0
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);
                m_dataAccessClientOPCUA->setDatapoint(searchDatapoint("CDM_FSM_transition"), 2, 0);

                // Puts the FSM.state back to the initial one if the state was Ready(1) or Tpoint(3)
                if ((FSM_state == 1) || (FSM_state == 3))
                	m_dataAccessClientOPCUA->setDatapoint(searchDatapoint("CDM_FSM_state"), 2, FSM_state);
                    //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, FSM_state);
            }
        }
    }
    return NULL;
}

int AsynchronousThread::startRun()
{
    int ret = 0;
    start(NULL);
    return ret;
}
