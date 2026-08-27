/**
 * CDMController.cpp
 *
 * Couche métier CDM — toute la logique fonctionnelle extraite de l'ancien CDM.cpp.
 *
 * Pattern : Héritage de Controller, macros #define pour configs XML,
 *           addDataProcessingCallback (non utilisé ici car CDM n'a pas de callbacks
 *           de traitement de données OPC-UA dans le code original).
 *
 * Les threads m_Thread, m_ThreadMeteo, m_ThreadLogRestart sont des threads de travail
 * (caméra, météo, log) et non des callbacks datapoint. Ils sont gérés directement
 * dans ce Controller via startRun() / cmd*().
 */

    #include <thread>
#include <chrono>



#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>

#include "CDMController.h"
#include "Camera.h"
#include "Helper.h"

#include "ConfigCDM.h"   // LoadCDMConfiguration()

#define FILENAME "config.ini" //TODO useless

using namespace std;

// ============================================================
// Variables globales camera et helper (conservées du code original)
// TODO: à remplacer par des membres du Controller si possible
// ============================================================
Camera camera;
Helper helper;

// ============================================================
// Constructeur sans PluginsBase
// ============================================================
// Constructeur simple (juste le statut de connexion, pas de plugin ni config nommée)
CDMController::CDMController(int *status_Client_Connection) 
    : Controller(status_Client_Connection)
{
    m_cdmConfig = NULL;
    m_dbConfig = NULL;
    m_camera = &camera;
    m_helper = &helper;
    m_bitsPerPixel = 8;
    m_Thread = NULL;
    m_ThreadMeteo = NULL;
    m_ThreadLogRestart = NULL;
    m_dpMonitorDataBroker = NULL;
    m_connectionResultDataBroker = -1;

    /*m_cdmConfig = new Config(CDM_CONFIGURATION_NAME, "");
    m_dbConfig = new Config(DATABROKER_CONFIGURATION_NAME, "");

    std::string configName = m_cdmConfig->getFileName();
    std::string configdgName = m_dbConfig->getFileName();*/
}

// Constructeur standard (pattern de référence, comme AuxiliaryController)
CDMController::CDMController(int *status_Client_Connection, PluginsBase *plugin)
    : Controller(status_Client_Connection, plugin, "Unit_CDM.AuxControl")

{
    m_cdmConfig = NULL;
    m_dbConfig = NULL;
    m_camera = &camera;
    m_helper = &helper;
    m_bitsPerPixel = 8;
    m_Thread = NULL;
    m_ThreadMeteo = NULL;
    m_ThreadLogRestart = NULL;
    m_dpMonitorDataBroker = NULL;
    m_connectionResultDataBroker = -1;

    // Récupère la config déjà chargée en interne par Controller::init()
    m_cdmConfig = getConfiguration(CDM_CONFIGURATION_NAME);
    m_dbConfig  = getConfiguration(DATABROKER_CONFIGURATION_NAME);


    if(m_cdmConfig != NULL)
    {
        std::string configName = m_cdmConfig->getFileName();
        // Créer et enregistrer le callback pour ce serveur si nécessaire
        // CB *m_CBcdm = new CB_cdm(this, m_cdmConfig);
        // addDataProcessingCallback(configName, m_CBcdm);
    }
    else
    {
        LOG_ERROR << "Configuration " << CDM_CONFIGURATION_NAME << " has not been found in the config.ini file" << endl;
    }

    if(m_dbConfig != NULL)
    {
        std::string dbConfigName = m_dbConfig->getFileName();
        // Idem pour le databroker
        //CB *m_CBdb = new CB_DataBroker(this, m_dbConfig);
        //addDataProcessingCallback(dbConfigName, m_CBdb);
    }
    else
    {
        LOG_ERROR << "Configuration " << DATABROKER_CONFIGURATION_NAME << " has not been found in the config.ini file" << endl;
    }
}

// ============================================================
// Destructeur
// ============================================================
CDMController::~CDMController()
{
    // Cleanup des threads si encore actifs
    if (m_Thread != NULL)
    {
        m_Thread->stop();
    }
    if (m_ThreadMeteo != NULL)
    {
        m_ThreadMeteo->stop();
    }
    if (m_ThreadLogRestart != NULL)
    {
        m_ThreadLogRestart->stop();
    }

    // Cleanup du datapoint monitor
    if (m_dpMonitorDataBroker != NULL)
    {
        delete m_dpMonitorDataBroker;
        m_dpMonitorDataBroker = NULL;
    }
}

// ============================================================
// loadCDMConfiguration — charge la config depuis le fichier
// CHOIX: extraite de afterStart() original
// ============================================================
bool CDMController::loadCDMConfiguration()
{
    bool conf = LoadCDMConfiguration(m_config);
    COND_LOG_DEBUG<<"CDMController::loadCDMConfiguration:"<<conf;
    return conf;
}

// ============================================================
// setupCameraFromConfig — configure la caméra avec la config
// CHOIX: extraite de afterStart() original
// ============================================================
void CDMController::setupCameraFromConfig()
{
    m_camera->SetConfig(m_config);
}

// ============================================================
// searchDatapoint — cherche un datapoint par nom
// ============================================================
std::string CDMController::searchDatapoint(const std::string& element)
{
    // Délégué au helper OPC-UA pour chercher un datapoint
    // NOTE: dans le code original, c'était une méthode de la classe CDM
    //       Ici on utilise le helper comme dans les autres méthodes UpdateXXX
    return "";  // À implémenter avec le client OPC-UA réel
}

// ============================================================
// Subscribe_DataBroker — subscription aux datapoints du DataBroker
// ============================================================
void CDMController::SubscribeDataBroker()
{
    if (m_connectionResultDataBroker != -1)
    {
        if (m_dpMonitorDataBroker != NULL && m_dpMonitorDataBroker != nullptr)
        {
            // Accès au client DataBroker via helper
            // helper.get_client_DataBroker()->subscribe(...);
        }
    }
}

// ============================================================
// COMMANDES CAMÉRA (correspondent aux anciens cmd() synchrones)
// ============================================================

void CDMController::ConnectCamera()
{
    m_camera->Connect();
    double Az_deg=0;
    getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME)->getDatapointL1("azimuth_position", Az_deg);
    COND_LOG_DEBUG <<"CDMController::ConnectCamera: read DB AZ "<<Az_deg;
    COND_LOG_DEBUG<<"getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME): "<<getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME);
    
}

void CDMController::DisconnectCamera()
{
    m_camera->Disconnect();
}



void CDMController::ConfigureThreadCamera(int nPixelClock, double exposure, double fps,
                                          double gain, const std::string& pixel_format)
{
    if (m_Thread != NULL)
    {
        if (pixel_format == "IS_CM_MONO8"){m_bitsPerPixel = 8;}
        else{m_bitsPerPixel = 16;}
        m_camera->iBitsPerPixel = m_bitsPerPixel;


        std::string datapointName = ""; // à récupérer depuis la config
        int nameSpace = 2;
        //getDataPointFinderRef(CDM_CONFIGURATION_NAME)->searchDatapointL2("Configure", datapointName, nameSpace);
        
        m_Thread->cmdConfigure(nameSpace, nPixelClock, exposure, fps, gain, pixel_format);
                
        
        COND_LOG_DEBUG<<"CDMController::ConfigureThreadCamera: End "<<std::endl;

    }
}

void CDMController::GetImage()
{
    // std::string datapointName = "";
    // m_camera->GetImage(getDataAccessClientOPCUARef());
    // (CODE ORIGINAL COMMENTÉ)
}

void CDMController::GetMultipleImages(int count, DataAccessClientOPCUA *dataAccessClientOPCUA)
{
    // Appel au thread caméra pour acquisition multiple
    // NOTE: dans le code original, GetMultipleImages lançait un thread asynchrone

    m_camera->GetMultipleImages(count, dataAccessClientOPCUA, getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME));
}

void CDMController::StopGetMultipleImages()
{
    m_camera->StopGetMultipleImages();
}

void CDMController::SingleAcquisition()
{
    // Acquisition unique via le thread caméra
    // NOTE: dans le code original, cette commande était vide (TODO)
    // m_Thread->cmdSingleAcquisition();
}

void CDMController::StartContinuousAcquisition()
{
    // m_Thread->cmdStartContinuous();
}

void CDMController::StopContinuousAcquisition()
{
    // m_Thread->cmdStopContinuous();
}

void CDMController::StartStream()
{
//TODO
 //   m_camera->StartStream();
}

void CDMController::StopStream()
{
    m_camera->StopStream();
}

void CDMController::StartSG()
{
    // std::string datapointName = "";
    // int nameSpace = 2;
    // m_Thread->cmdStartSG(datapointName, nameSpace);
}

void CDMController::StopSG()
{
    m_camera->StopSG();
}

void CDMController::StartCDM()
{
    // std::string datapointName = "";
    // int nameSpace = 2;
    m_Thread->cmdStartCDM(getDataPointFinderRef(DATABROKER_CONFIGURATION_NAME));
}

void CDMController::StopCDM()
{
    m_camera->StopCDM();
}

void CDMController::enableHeartbeat()
{
    COND_LOG_DEBUG<<"enable Heartbeat"<<std::endl;
    // TODO fix hard coded values
    getDataPointFinderRef(CDM_CONFIGURATION_NAME)->setDatapointL2("_Enable_Heart_Beat", true);
    getDataPointFinderRef(CDM_CONFIGURATION_NAME)->setDatapointL2("_Error_Heart_Beat", false);
}




/**
 * @brief Sets the FSM state to in transition.
 * @param transition The transition state.
 */
void CDMController::set_FSM_in_transition (bool transition)
{
	LOG_TRACE << "[CDM] Set FSM to in transition " << transition << std::endl;
	int32_t transition_state = (transition) ? 1 : 0;
	getDataPointFinderRef(CDM_CONFIGURATION_NAME)->setDatapointL2("FSM_transition", 0);
    
	return;
}


/**
 * @brief Sets the FSM state.
 * @param state The FSM state.
 */
void CDMController::set_FSM (int state)
{
	LOG_TRACE << "[CDM] Set FSM to " << state << std::endl;
	getDataPointFinderRef(CDM_CONFIGURATION_NAME)->setDatapointL2("FSM_state", state);
	return;

}





void CDMController::setDPQuality()
{

}

// ============================================================
// AddComment — ajoute un commentaire via le helper
// ============================================================
void CDMController::AddComment(const std::string& comment)
{
    cout << "CDMController::AddComment: " << comment << endl;
    m_helper->set_Comment(comment);
}

// ============================================================
// startCameraThread — démarrage du thread caméra
// ============================================================
void CDMController::startCameraThread()
{
    if (m_Thread != NULL)
    {
        COND_LOG_DEBUG<<"CDMController::startCameraThread()"<<std::endl;
        m_Thread->startRun();
    }
}

// ============================================================
// startMeteoThread — démarrage du thread météo
// ============================================================
void CDMController::startMeteoThread()
{
    if (m_ThreadMeteo != NULL)
    {
        COND_LOG_DEBUG<<"CDMController::startMeteoThread()"<<std::endl;
        m_ThreadMeteo->startRun();
        m_ThreadMeteo->cmdStartMeteo();
    }
}

// ============================================================
// startLogRestartThread — démarrage du thread log restart
// ============================================================
void CDMController::startLogRestartThread()
{
    if (m_ThreadLogRestart != NULL)
    {
        COND_LOG_DEBUG<<"CDMController::startLogRestartThread()"<<std::endl;
        m_ThreadLogRestart->startRun();
        m_ThreadLogRestart->cmdLogRestart();
    }
}




void CDMController::applyServerConnectionLossReaction()
{
    *getDeviceLogger()->getLogger() << MSG_WARNING 
        << "[CDM] Apply Safety Action triggered by HeartBeat with client => Nothing specific\n";
        //TODO
}

void CDMController::applyClientConnectionLossReaction(Config* config)
{
    *getDeviceLogger()->getLogger() << MSG_WARNING 
        << "[CDM] Apply Safety Action triggered by loss of communication with the server associated to " 
        << config->getFileName().c_str() << "\n";
        //TODO
}


void CDMController::startThread()
{
    if (m_Thread != NULL)
    {
        startCameraThread();
    }
    if (m_ThreadMeteo != NULL)
    {
         startMeteoThread();
    }
    if (m_ThreadLogRestart != NULL)
    {
        startLogRestartThread();
    }
}