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
#include "CDM.h" //TODO: implement this some other way, split file

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
	std::cout << "DZ test async" << std::endl;
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
				usleep(9000000);
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

			if (m_cmdStart == 1)
			{
				// inform that the command is in progress
				temString = m_datapointName + "._InProgressBar";
				t = 1;
				m_dataAccessClientOPCUA->setDatapoint(temString, m_nameSpace, t);

				// ****************** here put the code for Start who take a long time to execute ***************
				//CDM::Start();

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
