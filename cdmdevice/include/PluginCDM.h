/**
 * PluginCDM.h
 *
 * Couche plugin (point d'entrée MOS) séparée de la logique métier CDMController.
 * Hérite de PluginsBase (interface MOS) et LAPPThread (thread asynchrone).
 *
 * Pattern : Parser les commandes texte dans cmd() → stocker flags+valeurs → 
 *           start(NULL) lancer le thread → dans run(), consommer chaque flag 
 *           en appelant la méthode correspondante sur CDMController.
 *
 * CHOIX D'INTERPRÉTATION :
 * - Les commandes asynchrones détectées dans CDM.cpp original (GetMultipleImages, 
 *   SingleAcquisition, ContinuousAcquisition, StartStream, StopStream, etc.) 
 *   sont transformées en flags dans ce plugin.
 * - Les commandes synchrones (Connect, Disconnect, AddComment, etc.) sont 
 *   exécutées directement dans cmd() sans passer par le thread.
 */

#ifndef PluginCDM_H_
#define PluginCDM_H_

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <map>

#include "pluginsBase.h"
#include "lappThread.h"

// Forward declaration
class CDMController;

#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"

class PluginCDM
{
public:
    PluginCDM();

    int init(const std::string& parameters);
    int close();

    int cmd(const std::string& parameters,
            int commandStringAck,
            std::string& result);

    int afterStart();

    int cmdAsynch(const std::string& command,
                  int commandStringAck,
                  const std::string& datapointName,
                  int nameSpace, std::string& result);

    int get(const std::string& chain,
            int commandStringAck,
            std::vector<boost::any>& tabValue);

    int set(const std::string& chain,
            int commandStringAck,
            std::vector<boost::any>& tabValue);

private:
    CDMController *m_cdmController;

    // === Flags de commandes asynchrones ===
    // Chaque flag correspond à une commande qui doit être traitée dans le thread run()

    // Acquisition
    bool fGetMultipleImagesFlag;
    int  fGetMultipleImagesCount;
    std::string fGetMultipleImagesArgs;

    bool fSingleAcquisitionFlag;

    bool fContinuousAcquisitionFlag;
    bool fContinuousAcquisitionMode; // true=start, false=stop

    bool fStartStreamFlag;
    bool fStopStreamFlag;

    bool fStartSGFlag;
    bool fStopSGFlag;

    bool fStartCDMFlag;
    bool fStopCDMFlag;

    bool fGoToTpointFlag;
    bool fGoToReadyFlag;

    // Camera configuration
    bool fConfigureFlag;
    std::string fConfigureArgs;       // contient tous les paramètres (nPixelClock, exposure, fps, gain, pixel_format)
    int  fConfigurePixelClock;
    double fConfigureExposure;
    double fConfigureFps;
    double fConfigureGain;
    std::string fConfigurePixelFormat;

    // Datapoint monitor
    bool fSubscribeDataBrokerFlag;

    // Méthode de thread (virtuelle de LAPPThread)
    void *run(void *params);

    int m_stop;
};

#endif // PluginCDM_H_