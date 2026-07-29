/**
 * PluginCDM.cpp
 *
 * Couche plugin CDM — point d'entrée vers le serveur MOS.
 * Délègue la logique métier à CDMController.
 *
 * Pattern :
 *   cmd()   → parse la commande texte, positionne un flag booléen + valeurs, appelle start(NULL)
 *   run()   → consommateur de chaque flag → appelle la méthode相应 sur le Controller
 *
 * COMMANDES SYNCHRONES (exécutées directement dans cmd) :
 *   Connect, Disconnect, AddComment, Error, Acknowledge
 *
 * COMMANDES ASYNCHRONES (flag + thread) :
 *   GetMultipleImages, SingleAcquisition, ContinuousAcquisition,
 *   StartStream, StopStream, StartSG, StopSG, StartCDM, StopCDM,
 *   GoToTpoint, GoToReady, Configure
 *
 * CHOIX D'INTERPRÉTATION :
 * - Les paramètres de Configure sont stockés dans des variables membres (pas de struct)
 *   car le pattern de référence utilise des membres simples.
 * - Les commandes GoToTpoint / GoToReady étaient vides dans le code original
 *   (TODO) → flags ajoutés mais le Controller n'a pas de méthode implémentée pour l'instant.
 */

#include <string>
#include <sstream>
#include <iostream>

#include "Logging.h"
#include "PluginCDM.h"
#include "CDMController.h"
#include "Config.h"

//****************************************************
PluginCDM::PluginCDM() 
{
    m_cdmController = NULL;
    m_stop = false;

    // Initialisation de tous les flags à false
    fGetMultipleImagesFlag = false;
    fSingleAcquisitionFlag = false;
    fContinuousAcquisitionFlag = false;
    fContinuousAcquisitionMode = false;
    fStartStreamFlag = false;
    fStopStreamFlag = false;
    fStartSGFlag = false;
    fStopSGFlag = false;
    fStartCDMFlag = false;
    fStopCDMFlag = false;
    fGoToTpointFlag = false;
    fGoToReadyFlag = false;
    fConfigureFlag = false;
    fSubscribeDataBrokerFlag = false;

    fConfigurePixelClock = 216;
    fConfigureExposure = 2000.0;
    fConfigureFps = 10.0;
    fConfigureGain = 1.0;
    fConfigurePixelFormat = "IS_CM_MONO8";
}

//****************************************************
// init
/**
    Ouverture de la communication avec le device.
    Param : std::string parameters (format <Tag>:<Value> <Tag>:<Value>...)
    Return : 0 = ok, 1 = error
*/
int PluginCDM::init(const std::string& parameters) 
{
    LOG_TRACE<<"PluginCDM::init() : initialisation of the CDM\n";

    int ret = 0;
    ret = PluginsBase::init(parameters);
    return ret;
}

//****************************************************
// close
int PluginCDM::close() 
{
    int ret = 0;
    // Délégué au Controller via disconnect propre
    LOG_TRACE << "PluginCDMController::close called" << std::endl;
    if (m_cdmController != NULL) 
    {
    
        m_cdmController->close();
        delete m_cdmController;
        m_cdmController = NULL;
    
    return ret;
    }
}

//****************************************************
// cmd — parse les commandes texte du serveur MOS
int PluginCDM::cmd(const std::string& parameters,
                   int commandStringAck,
                   std::string& result) 
{
    std::cout << std::endl;
    int ret = 0;

    if (parameters.length() == 0)
    {
        return 1;
    }

    // Décomposition de la ligne de commande
    std::vector<std::string> arglist;
    std::istringstream iss(parameters);
    std::string word;
    while (std::getline(iss, word, ' '))
    {
        arglist.push_back(word);
    }

    int argnum = arglist.size();

    // ================================================
    // COMMANDES SYNCHRONES (pas de thread, exécution directe)
    // ================================================

    // --- Connect ---
    if (parameters.find("Connect") == 0)
    {
        cout << "PluginCDM => Connect" << endl;
        if (argnum < 1)
        {
            cout << " Too few arguments" << endl;
            return 0;
        }
        if (m_cdmController != NULL)
        {
            m_cdmController->ConnectCamera();
            m_cdmController->ConfigureThreadCamera(
                fConfigurePixelClock,
                fConfigureExposure,
                fConfigureFps,
                fConfigureGain,
                fConfigurePixelFormat);
        }
    }

    // --- Disconnect ---
    if (parameters.find("Disconnect") == 0)
    {
        cout << "PluginCDM => Disconnect" << endl;
        if (m_cdmController != NULL)
        {
            m_cdmController->DisconnectCamera();
        }
    }

    // --- AddComment ---
    if (parameters.find("AddComment") == 0)
    {
        cout << "PluginCDM => AddComment" << endl;
        // Récupérer le reste de la chaîne après "AddComment "
        std::string comment = parameters.substr(10); // skip "AddComment"
        // Trim left spaces
        while (comment.length() > 0 && comment[0] == ' ')
            comment.erase(0, 1);
        if (m_cdmController != NULL)
        {
            m_cdmController->AddComment(comment);
        }
    }

    // --- Error ---
    if (parameters.find("Error") == 0)
    {
        cout << "PluginCDM => Error" << endl;
        if (m_cdmController != NULL)
        {
            m_cdmController->StopCDM();
        }
    }

    // --- Acknowledge ---
    if (parameters.find("Acknowledge") == 0)
    {
        cout << "PluginCDM => Acknowledge" << endl;
        if (m_cdmController != NULL)
        {
            m_cdmController->DisconnectCamera();
        }
    }

    // ================================================
    // COMMANDES ASYNCHRONES (flag + thread)
    // ================================================

    // --- GetMultipleImages ---
    if (parameters.find("GetMultipleImages") == 0)
    {
        cout << "PluginCDM => GetMultipleImages" << endl;
        fGetMultipleImagesCount = 1;
        fGetMultipleImagesArgs = "";
        if (argnum > 1)
        {
            fGetMultipleImagesCount = atoi(arglist[1].c_str());
        }
        fGetMultipleImagesFlag = true;
        start(NULL);
    }

    // --- SingleAcquisition ---
    if (parameters.find("SingleAcquisition") == 0)
    {
        cout << "PluginCDM => SingleAcquisition" << endl;
        fSingleAcquisitionFlag = true;
        start(NULL);
    }

    // --- ContinuousAcquisition ---
    if (parameters.find("ContinuousAcquisition") == 0)
    {
        cout << "PluginCDM => ContinuousAcquisition" << endl;
        fContinuousAcquisitionMode = true;
        if (argnum > 1)
        {
            if (arglist[1].compare("stop") == 0)
                fContinuousAcquisitionMode = false;
        }
        fContinuousAcquisitionFlag = true;
        start(NULL);
    }

    // --- Configure ---
    if (parameters.find("Configure") == 0)
    {
        cout << "PluginCDM => Configure" << endl;
        // Format attendu : Configure nPixelClock exposure fps gain pixel_format
        // ou : Configure nPixelClock exposure fps gain pixel_format id
        if (argnum >= 6)
        {
            fConfigurePixelClock = atoi(arglist[1].c_str());
            fConfigureExposure = atof(arglist[2].c_str());
            fConfigureFps = atof(arglist[3].c_str());
            fConfigureGain = atof(arglist[4].c_str());
            fConfigurePixelFormat = arglist[5];
            if (fConfigurePixelFormat == "IS_CM_MONO8")
            {
                // handled in Controller
            }
            else
            {
                fConfigurePixelFormat = "IS_CM_SENSOR_RAW16";
            }
        }
        fConfigureFlag = true;
        start(NULL);
    }

    // --- StartStream ---
    if (parameters.find("StartStream") == 0)
    {
        cout << "PluginCDM => StartStream" << endl;
        fStartStreamFlag = true;
        start(NULL);
    }

    // --- StopStream ---
    if (parameters.find("StopStream") == 0)
    {
        cout << "PluginCDM => StopStream" << endl;
        fStopStreamFlag = true;
        start(NULL);
    }

    // --- StartSG ---
    if (parameters.find("StartSG") == 0)
    {
        cout << "PluginCDM => StartSG" << endl;
        fStartSGFlag = true;
        start(NULL);
    }

    // --- StopSG ---
    if (parameters.find("StopSG") == 0)
    {
        cout << "PluginCDM => StopSG" << endl;
        fStopSGFlag = true;
        start(NULL);
    }

    // --- StartCDM ---
    if (parameters.find("StartCDM") == 0)
    {
        cout << "PluginCDM => StartCDM" << endl;
        fStartCDMFlag = true;
        start(NULL);
    }

    // --- StopCDM ---
    if (parameters.find("StopCDM") == 0)
    {
        cout << "PluginCDM => StopCDM" << endl;
        fStopCDMFlag = true;
        start(NULL);
    }

    // --- GoToTpoint ---
    if (parameters.find("GoToTpoint") == 0)
    {
        cout << "PluginCDM => GoToTpoint" << endl;
        // TODO: dans le code original cette commande était vide
        fGoToTpointFlag = true;
        start(NULL);
    }

    // --- GoToReady ---
    if (parameters.find("GoToReady") == 0)
    {
        cout << "PluginCDM => GoToReady" << endl;
        // TODO: dans le code original cette commande était vide
        fGoToReadyFlag = true;
        start(NULL);
    }

    // --- StopGetMultipleImages ---
    if (parameters.find("StopGetMultipleImages") == 0)
    {
        cout << "PluginCDM => StopGetMultipleImages" << endl;
        if (m_cdmController != NULL)
        {
            m_cdmController->StopGetMultipleImages();
        }
    }

    // --- Subscribe_DataBroker ---
    if (parameters.find("Subscribe_DataBroker") == 0)
    {
        cout << "PluginCDM => Subscribe_DataBroker" << endl;
        fSubscribeDataBrokerFlag = true;
        start(NULL);
    }

    return ret;
}

//****************************************************
int PluginCDM::afterStart() 
{
    LOG_TRACE << "PluginCDM::afterStart(): start of the function";
    int ret = 0;
    ret = PluginsBase::afterStart();
    m_cdmController = new CDMController(&ret, this);
    COND_LOG_DEBUG << "PluginCDM::afterStart(): SetDPQuality ";
    std::cout<<getDataAccessClientOPCUARef()<<std::endl;
    return ret;
}

//****************************************************
int PluginCDM::cmdAsynch(const std::string& command,
                         int commandStringAck,
                         const std::string& datapointName,
                         int nameSpace, std::string& result) 
{
    return 0;
}

//****************************************************
int PluginCDM::get(const std::string& chain,
                   int commandStringAck,
                   std::vector<boost::any>& tabValue) 
{
    int ret = 0;
    // Délégué au Controller si besoin de logique get
    if (m_cdmController != NULL)
    {
        // m_cdmController->get(...)
    }
    return ret;
}

//****************************************************
int PluginCDM::set(const std::string& chain,
                   int commandStringAck,
                   std::vector<boost::any>& tabValue) 
{
    int ret = 0;
    return ret;
}

//****************************************************
// run — consommateur de flags dans le thread
void *PluginCDM::run(void *params) 
{
    // === Acquisition ===
    if (fGetMultipleImagesFlag == true)
    {
        fGetMultipleImagesFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->GetMultipleImages(fGetMultipleImagesCount, fGetMultipleImagesArgs);
        }
    }

    if (fSingleAcquisitionFlag == true)
    {
        fSingleAcquisitionFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->SingleAcquisition();
        }
    }

    if (fContinuousAcquisitionFlag == true)
    {
        fContinuousAcquisitionFlag = false;
        if (m_cdmController != NULL)
        {
            if (fContinuousAcquisitionMode)
                m_cdmController->StartContinuousAcquisition();
            else
                m_cdmController->StopContinuousAcquisition();
        }
    }

    if (fStartStreamFlag == true)
    {
        fStartStreamFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StartStream();
        }
    }

    if (fStopStreamFlag == true)
    {
        fStopStreamFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StopStream();
        }
    }

    if (fStartSGFlag == true)
    {
        fStartSGFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StartSG();
        }
    }

    if (fStopSGFlag == true)
    {
        fStopSGFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StopSG();
        }
    }

    if (fStartCDMFlag == true)
    {
        fStartCDMFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StartCDM();
        }
    }

    if (fStopCDMFlag == true)
    {
        fStopCDMFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->StopCDM();
        }
    }

    if (fGoToTpointFlag == true)
    {
        fGoToTpointFlag = false;
        if (m_cdmController != NULL)
        {
            // TODO: à implémenter dans CDMController
            // m_cdmController->GoToTpoint();
        }
    }

    if (fGoToReadyFlag == true)
    {
        fGoToReadyFlag = false;
        if (m_cdmController != NULL)
        {
            // TODO: à implémenter dans CDMController
            // m_cdmController->GoToReady();
        }
    }

    if (fConfigureFlag == true)
    {
        fConfigureFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->ConfigureCamera(
                fConfigurePixelClock,
                fConfigureExposure,
                fConfigureFps,
                fConfigureGain,
                fConfigurePixelFormat);
        }
    }

    if (fSubscribeDataBrokerFlag == true)
    {
        fSubscribeDataBrokerFlag = false;
        if (m_cdmController != NULL)
        {
            m_cdmController->SubscribeDataBroker();
        }
    }
}

//****************************************************
// Export C pour le chargement dynamique du plugin
extern "C" {
    PluginCDM *ptr_Plugin()
    {
        return new PluginCDM();
    }
}
