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

#ifndef __ASYNCHRONOUSCONTROLLER_H__
#define __ASYNCHRONOUSCONTROLLER_H__

#include "string"
#include "lappThread.h"
#include "Camera.h"
#include "Meteo.h"

typedef unsigned char Byte;

class DataAccessClientOPCUA;

class AsynchronousThread : public LAPPThread
{
public:
    AsynchronousThread(DataAccessClientOPCUA *m_dataAccessClientOPCUA);
    ~AsynchronousThread();
    void *run(void *params);

    int stop();
    void pause();
    void resume();
    int startRun();
    int cmdStartMeteo();
    int cmdGetMultipleImages(std::string datapointName, int nameSpace, int n_images);
    int cmdGetMultipleImagesStacked(std::string datapointName, int nameSpace, int n_images);
    int cmdStartCDM(std::string datapointName, int nameSpace);
    int cmdStartStream(std::string datapointName, int nameSpace);
    int cmdConfigure(std::string datapointName, int nameSpace, int nPixelClock, double exposure, double fps, int gain, std::string pixel_format);

private:
    int m_stop;
    int m_pause;
    int m_command;
    int m_cmdGetMultipleImages;
    int m_cmdGetMultipleImagesStacked;
    int m_cmdStartCDM;
    int m_cmdStartStream;
    int m_cmdConfigure;
	int m_cmdMeteo;
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

#endif //__ASYNCHRONOUSCONTROLLER_H__
