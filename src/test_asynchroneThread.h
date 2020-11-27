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

#include "Camera.h"
#include "lappThread.h"
#include "string"

#include "Meteo.cpp"

typedef unsigned char Byte;

class DataAccessClientOPCUA;

class TestAsynchroneThread : public LAPPThread
{
public:
    TestAsynchroneThread(DataAccessClientOPCUA *m_dataAccessClientOPCUA);
    ~TestAsynchroneThread();
    void *run(void *params);

    int stop();
    void pause();
    void resume();
    int startRun();
    int cmdGetMultipleImages(std::string datapointName, int nameSpace, int n_images);
    int cmdGetMultipleImagesStacked(std::string datapointName, int nameSpace, int n_images);
    int cmdStartCDM(std::string datapointName, int nameSpace);
    int cmdConfigure(std::string datapointName, int nameSpace, int nPixelClock, double exposure, double fps, int gain, std::string pixel_format);

private:
    int m_stop;
    int m_pause;
    int m_command;
    int m_cmdGetMultipleImages;
    int m_cmdGetMultipleImagesStacked;
    int m_cmdStartCDM;
    int m_cmdConfigure;
	int m_cmdMeteo=1;
    int m_nameSpace;
    std::string m_datapointName;
    DataAccessClientOPCUA *m_dataAccessClientOPCUA;
	Meteo meteo;

    std::vector<int> imagePaths;
    int n_images;
    double RA;
    double DEC;
    double azimuth;
    double zenith;
    int LED_intensity;
    bool OARL_state;
    int nPixelClock;
    double exposure;
    double fps;
    int gain;
    std::string pixel_format;
};

#endif //__TESTASYNCHRONECONTROLLER_H__
