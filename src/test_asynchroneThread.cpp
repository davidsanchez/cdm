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

extern Camera camera;

TestAsynchroneThread::TestAsynchroneThread(DataAccessClientOPCUA *dataAccessClientOPCUA)
{
	m_pause = false;
	m_stop = false;
	m_command = 0;
	m_cmdCloseShutter = 0;
	m_cmdOpenShutter = 0;
	m_cmdStart = 0;
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

int TestAsynchroneThread::cmdCloseShutter(std::string datapointName, int nameSpace)
{
	int ret = 0;
	m_cmdCloseShutter = true;
	m_datapointName = datapointName;
	m_nameSpace = nameSpace;
	return ret;
}

int TestAsynchroneThread::cmdOpenShutter(std::string datapointName, int nameSpace)
{
	int ret = 0;
	m_cmdOpenShutter = true;
	m_datapointName = datapointName;
	m_nameSpace = nameSpace;
	return ret;
}

int TestAsynchroneThread::cmdGetMultipleImages( std::string datapointName, int nameSpace, int n_images)
{
	int ret = 0;
	m_cmdGetMultipleImages = true;
	m_datapointName = datapointName;
	m_nameSpace = nameSpace;
	TestAsynchroneThread::n_images = n_images;
	return ret;
}

int TestAsynchroneThread::cmdStart(std::string datapointName, int nameSpace)
{
	int ret = 0;
	m_cmdStart = true;
	m_datapointName = datapointName;
	m_nameSpace = nameSpace;
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
		//std::cout << "DZ async not stopped" << std::endl;
		if (m_pause == false)
		{
			usleep(1000000);
			if (m_cmdCloseShutter == 1)
			{
				// inform that the command is in progress
				temString = m_datapointName + "._InProgressBar";
				t = 1;
				m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

				// ****************** here put the code for closeShutter who take a long time to execute ***************

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
				m_cmdCloseShutter = 0;
			}

			if (m_cmdOpenShutter == 1)
			{
				
				// inform that the command is in progress
				temString = m_datapointName + "._InProgressBar";
				t = 1;
				m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

				// ****************** here put the code for closeShutter who take a long time to execute ***************

				std::cout << "In async OpenShutter" << std::endl;

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
				m_cmdOpenShutter = 0;
			}

			if (m_cmdGetMultipleImages == 1)
			{
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
				for (const auto &piece : v_image_paths) imagePath_cat += "\n" + piece;
				m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.CDM.imagePath_cat.imagePath_cat_v", 2, imagePath_cat);


				//SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, v_image_paths);

				//SetDatapointThread *m_SetDatapointThread = new SetDatapointThread(getDataAccessClientOPCUARef(), "Unit_CDM.AuxControl.CDM.imagePath.imagePath_v", 2, 2);


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

			if (m_cmdStart == 1)
			{
				// inform that the command is in progress
				temString = m_datapointName + "._InProgressBar";
				t = 1;
				m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

				// ****************** here put the code for Start who take a long time to execute ***************

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
				m_cmdStart = 0;
			}

			if (m_cmdMeteo == 1)
			{
				vector<float> meteo_val = meteo.Update();
				m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.temperature.temperature_v", 2, meteo_val[0]);			
				m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.humidity.humidity_v", 2, meteo_val[1]);
				m_dataAccessClientOPCUA->setDatapoint("Unit_CDM.AuxControl.Sensor.pressure.pressure_v", 2, meteo_val[2]);
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
