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

#ifndef __TESTASYNCHRONECONTROLLER_H__
#define __TESTASYNCHRONECONTROLLER_H__

#include "string"
#include "lappThread.h"
#include "Camera.h"

//#include "Helper.h" //remove?

typedef unsigned char Byte;

class DataAccessClientOPCUA;

class TestAsynchroneThread: public LAPPThread {
public:
	TestAsynchroneThread(DataAccessClientOPCUA* m_dataAccessClientOPCUA);
	~TestAsynchroneThread();
	void* run(void *params);

	int  stop();
	void pause();
	void resume();
	int  startRun();
	int cmdOpenShutter( std::string datapointName, int nameSpace);
	int cmdCloseShutter( std::string datapointName, int nameSpace);
	int cmdGetMultipleImages( std::string datapointName, int nameSpace, int n_images);
	int cmdStart( std::string datapointName, int nameSpace);
private:
	int m_stop;
	int m_pause;
	int m_command;
	int m_cmdCloseShutter;
	int m_cmdOpenShutter;
	int m_cmdGetMultipleImages;
	int m_cmdStart;
	int m_nameSpace;
	std::string m_datapointName;
	DataAccessClientOPCUA* m_dataAccessClientOPCUA;

	std::vector<int> imagePaths;
	int n_images;
	double RA;
	double DEC;
	double azimuth;
	double zenith;
	int LED_intensity;
	bool OARL_state;
};

#endif //__TESTASYNCHRONECONTROLLER_H__ 
