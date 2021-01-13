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

#include "test_asynchroneThread.h"
#include "dataAccessClientOPCUA.h"

#include "Logging.h"

extern Camera camera;

TestAsynchroneThread::TestAsynchroneThread(DataAccessClientOPCUA *dataAccessClientOPCUA)
{
    m_pause = false;
    m_stop = false;
    m_command = 0;
    m_cmdGetMultipleImages = 0;
    m_cmdGetMultipleImagesStacked = 0;
    m_cmdConfigure = 0;
    m_cmdStartCDM = 0;
    m_cmdMeteo = 0;
    m_dataAccessClientOPCUA = dataAccessClientOPCUA;
}

TestAsynchroneThread::~TestAsynchroneThread()
{
    m_pause = true;
    m_stop = true;
    wait();
}

void TestAsynchroneThread::pause()
{
    m_pause = true;
}

void TestAsynchroneThread::resume()
{
    m_pause = false;
}

int TestAsynchroneThread::stop()
{
    int ret = 0;
    m_stop = true;
    return ret;
}

int TestAsynchroneThread::cmdStartMeteo()
{
    int ret = 0;
    m_cmdMeteo = true;
    return ret;
}

int TestAsynchroneThread::cmdGetMultipleImages(std::string datapointName, int nameSpace, int n_images)
{
    int ret = 0;
    m_cmdGetMultipleImages = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    TestAsynchroneThread::n_images = n_images;
    return ret;
}

int TestAsynchroneThread::cmdGetMultipleImagesStacked(std::string datapointName, int nameSpace, int n_images)
{
    int ret = 0;
    m_cmdGetMultipleImagesStacked = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    TestAsynchroneThread::n_images = n_images;
    return ret;
}

int TestAsynchroneThread::cmdStartCDM(std::string datapointName, int nameSpace)
{
    int ret = 0;
    m_cmdStartCDM = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    return ret;
}

int TestAsynchroneThread::cmdConfigure(std::string datapointName, int nameSpace, int nPixelClock, double exposure, double fps, int gain, std::string pixel_format)
{
    int ret = 0;
    m_cmdConfigure = true;
    m_datapointName = datapointName;
    m_nameSpace = nameSpace;
    TestAsynchroneThread::nPixelClock = nPixelClock;
    TestAsynchroneThread::exposure = exposure;
    TestAsynchroneThread::fps = fps;
    TestAsynchroneThread::gain = gain;
    TestAsynchroneThread::pixel_format = pixel_format;
    return ret;
}

void *TestAsynchroneThread::run(void *params)
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
            usleep(1000000);

            if (m_cmdGetMultipleImages == 1)
            {
                // Puts the FSM.state to 4
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 4);

                // inform that the command is in progress
                temString = m_datapointName + "._InProgressBar";
                t = 1;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                // Put the transition state to 1. But then you can't call StopGetMultipleImages.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 1);

                //std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images);
                std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images, m_dataAccessClientOPCUA);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, v_image_paths);

                // We want to push these imagePaths as single string
                std::string imagePath_cat;
                for (const auto &piece : v_image_paths)
                    imagePath_cat += "\n" + piece;
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, imagePath_cat);

                // Puts the FSM.state back to 3
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 3);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);

                // you can put the outputs arguments in this place to inform the server
                temString = m_datapointName + "._OutputArguments._Val_Retour";
                tempValue = "command Open : c'est bon c'est fini : JL ";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                temString = m_datapointName + "._Done";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                temString = m_datapointName + "._InProgressBar";
                t = 0;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdGetMultipleImages = 0;
            }

            if (m_cmdGetMultipleImagesStacked == 1)
            {

                // Puts the FSM.state to 4
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 4);

                // inform that the command is in progress
                temString = m_datapointName + "._InProgressBar";
                t = 1;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                // Put the transition state to 1. But then you can't call StopGetMultipleImages.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 1);

                //std::vector<std::string> v_image_paths = camera.GetMultipleImages(n_images);
                std::vector<std::string> v_image_paths = camera.GetMultipleImagesStacked(n_images, m_dataAccessClientOPCUA);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, v_image_paths);

                // We want to push these imagePaths as single string
                std::string imagePath_cat;
                for (const auto &piece : v_image_paths)
                    imagePath_cat += "\n" + piece;
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, imagePath_cat);

                // Puts the FSM.state back to 3
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 3);
                // Put the transition state to 0.
                //m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);

                // you can put the outputs arguments in this place to inform the server
                temString = m_datapointName + "._OutputArguments._Val_Retour";
                tempValue = "command Open : c'est bon c'est fini : JL ";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                temString = m_datapointName + "._Done";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                temString = m_datapointName + "._InProgressBar";
                t = 0;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdGetMultipleImagesStacked = 0;
            }

            if (m_cmdStartCDM == 1)
            {
                cout << "In asynchroneThread: cmdStartCDM" << endl;
                // Puts the FSM.state to 2
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 2);

                // inform that the command is in progress
                temString = m_datapointName + "._InProgressBar";
                t = 1;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************
                camera.StartCDM(m_dataAccessClientOPCUA);

                // you can put the outputs arguments in this place to inform the server
                temString = m_datapointName + "._OutputArguments._Val_Retour";
                tempValue = "command Open : c'est bon c'est fini : JL ";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                temString = m_datapointName + "._Done";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                temString = m_datapointName + "._InProgressBar";
                t = 0;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdStartCDM = 0;

                // Puts the FSM.state back to 1
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.state", 2, 1);

                cout << "End of cdmStartCDM inside AsynchroneThread.cpp" << endl;
            }

            if (m_cmdMeteo == 1)
            {
                vector<float> meteo_val = meteo.Update();
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.temperature.temperature_v", 2, meteo_val[0]);
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.humidity.humidity_v", 2, meteo_val[1]);
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.pressure.pressure_v", 2, meteo_val[2]);

                vector<string> ws_data = meteo.Update_WS();

				// The logger will write first the date and time automatically
				// Sensor: temperature, humidity, pressure
				// Weather station: time, date, temperature, pressure, wind direction, humidity, wind speed, wind gusts
                LOG_ENV << meteo_val[0] << " " << meteo_val[1] << " " << meteo_val[2] << " " << ws_data[0] << " " << ws_data[1] << " " << ws_data[2] << " " << ws_data[3] << " " << ws_data[4] << " " << ws_data[5] << " " << ws_data[6] << " "<< ws_data[7];

                usleep(5000000);
            }

            if (m_cmdConfigure == 1)
            {
                // Puts the FSM.transition to 1
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 1);

                // inform that the command is in progress
                temString = m_datapointName + "._InProgressBar";
                t = 1;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // ****************** here put the code for Start who take a long time to execute ***************

                //Configure(int nPixelClock=216, double exposure=50, double fps=10, int gain=0, std::string pixel_format="IS_CM_MONO8");

                std::vector<boost::any> configure_settings = camera.Configure(nPixelClock, exposure, fps, gain, pixel_format);

                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.pixelClock.pixelClock_v", 2, boost::any_cast<int>(configure_settings[0]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.FPS.FPS_v", 2, boost::any_cast<double>(configure_settings[1]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.exposure.exposure_v", 2, boost::any_cast<double>(configure_settings[2]));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.gain.gain_v", 2, boost::any_cast<int>((configure_settings[3])));
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.pixelFormat.pixelFormat_v", 2, boost::any_cast<string>(configure_settings[4]));

                // Puts the FSM.transition back to 0
                m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.FSM.transition", 2, 0);

                // you can put the outputs arguments in this place to inform the server
                temString = m_datapointName + "._OutputArguments._Val_Retour";
                tempValue = "command Open : c'est bon c'est fini : JL ";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, tempValue);

                // inform that the command is done
                temString = m_datapointName + "._Done";
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, true);

                // inform that the command is not in progress
                temString = m_datapointName + "._InProgressBar";
                t = 0;
                m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

                // reset the command
                m_cmdConfigure = 0;
            }
        }
    }
    return NULL;
}

int TestAsynchroneThread::startRun()
{
    int ret = 0;
    start(NULL);
    return ret;
}
