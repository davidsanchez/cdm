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

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>

#include "CDMController.h"
#include "Camera.h"
#include "Helper.h"
#include "Logging.h"
#include "ConfigCDM.h"   // LoadCDMConfiguration()
#include "Config.h"
#include "pluginsBase.h"

using namespace std;

// ============================================================
// Variables globales camera et helper (conservées du code original)
// TODO: à remplacer par des membres du Controller si possible
// ============================================================
extern Camera camera;
extern Helper helper;

// ============================================================
// Constructeur sans PluginsBase
// ============================================================
CDMController::CDMController(int *status_Client_Connection) 
    : Controller(CDM_CONFIGURATION_NAME)  // appelle Controller(const string&)
{
    // Initialisation des membres
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

    // Chargement de la config CDM depuis PLC_CDM.xml
    m_cdmConfig = new Config(CDM_CONFIGURATION_NAME, "");
    std::string configName = m_cdmConfig->getFileName();
    std::string configPath = m_cdmConfig->getPath();

    // NOTE: Pas de addDataProcessingCallback car le code original CDM
    //       n'utilise pas de callback de traitement de données OPC-UA.
    //       Les threads (m_Thread, m_ThreadMeteo, m_ThreadLogRestart) 
    //       sont des threads de travail, pas des callbacks datapoint.

    // NOTE: Les AsynchronousThread (m_Thread, m_ThreadMeteo, m_ThreadLogRestart)
    //       sont créés dans loadCDMConfiguration() ou dans afterStart() du Plugin.
    //       Ici on initialise juste les pointeurs à NULL.
    //       Le vrai démarrage des threads se fait dans startCameraThread() etc.
}

// ============================================================
// Constructeur avec PluginsBase (pattern standard)
//
// CHOIX D'INTERPRÉTATION : PluginsBase* fourni pour compatibilité avec
// le pattern de référence, mais non utilisé directement dans ce Controller
// (la config OPC-UA est gérée via le Config*).
// ============================================================
CDMController::CDMController(int *status_Client_Connection, PluginsBase *config)
    : Controller(CDM_CONFIGURATION_NAME)
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

    if (config != NULL)
    {
        m_cdmConfig = new Config(CDM_CONFIGURATION_NAME, "");
    }

    // NOTE: Pas de addDataProcessingCallback dans le code original CDM.
}

// ============================================================
// Destructeur
// ============================================================
CDMController::~CDMController()
{
    // Cleanup des threads si encore actifs
    if (m_Thread != NULL)
    {
        // m_Thread->stop();
    }
    if (m_ThreadMeteo != NULL)
    {
        // m_ThreadMeteo->stop();
    }
    if (m_ThreadLogRestart != NULL)
    {
        // m_ThreadLogRestart->stop();
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
int CDMController::loadCDMConfiguration(std::map<std::string, std::string>& configOut)
{
    bool conf = LoadCDMConfiguration(configOut);
    return conf ? 0 : -1;
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
}

void CDMController::DisconnectCamera()
{
    m_camera->Disconnect();
}

void CDMController::ConfigureCamera(int nPixelClock, double exposure, double fps,
                                    double gain, const std::string& pixel_format)
{
    if (pixel_format == "IS_CM_MONO8")
    {
        m_bitsPerPixel = 8;
    }
    else
    {
        m_bitsPerPixel = 16;
    }
    m_camera->iBitsPerPixel = m_bitsPerPixel;

    // Apply configuration via helper/getDataAccessClientOPCUARef()
    // L'ancien code appelait m_Thread->cmdConfigure() pour cela
}

void CDMController::ConfigureThreadCamera(int nPixelClock, double exposure, double fps,
                                          double gain, const std::string& pixel_format)
{
    if (m_Thread != NULL)
    {
        // std::string datapointName = ""; // à récupérer depuis la config
        // int nameSpace = 2;
        // m_Thread->cmdConfigure(datapointName, nameSpace, nPixelClock, exposure, fps, gain, pixel_format);
    }
}

void CDMController::GetImage()
{
    // std::string datapointName = "";
    // m_camera->GetImage(getDataAccessClientOPCUARef());
    // (CODE ORIGINAL COMMENTÉ)
}

void CDMController::GetMultipleImages(int count, const std::string& args)
{
    // Appel au thread caméra pour acquisition multiple
    // NOTE: dans le code original, GetMultipleImages lançait un thread asynchrone
    // m_Thread->cmdGetMultipleImages(count, args);
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
    m_camera->StartStream();
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
    // m_Thread->cmdStartCDM(datapointName, nameSpace);
}

void CDMController::StopCDM()
{
    m_camera->StopCDM();
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
        m_ThreadLogRestart->startRun();
        m_ThreadLogRestart->cmdLogRestart();
    }
}

// ============================================================
// DRIVE — Méthodes de mise à jour des valeurs de drive
// Délèguent à l'objet Helper (comme dans le code original CDM.cpp)
// ============================================================

int CDMController::UpdateRaValue(double newvalue)
{
    m_helper->SetRaDrive(newvalue);
    return 0;
}

int CDMController::UpdateDecValue(double newvalue)
{
    m_helper->SetDecDrive(newvalue);
    return 0;
}

int CDMController::UpdateAzValue(double newvalue)
{
    m_helper->SetAz(newvalue);
    return 0;
}

int CDMController::UpdateZdValue(double newvalue)
{
    m_helper->SetZd(newvalue);
    return 0;
}

int CDMController::UpdateAzOffsetValue(double newvalue)
{
    m_helper->SetAzOffset(newvalue);
    return 0;
}

int CDMController::UpdateZdOffsetValue(double newvalue)
{
    m_helper->SetZdOffset(newvalue);
    return 0;
}

int CDMController::UpdateSourceValue(const std::string& newvalue)
{
    m_helper->SetSource(newvalue);
    return 0;
}

int CDMController::UpdateOARLValue(bool newvalue)
{
    m_helper->SetOARL(newvalue);
    return 0;
}

int CDMController::UpdateLEDsValue(bool newvalue)
{
    m_helper->SetLEDs(newvalue);
    return 0;
}

int CDMController::UpdateShutterValue(int newvalue)
{
    m_helper->SetShutter(newvalue);
    return 0;
}

int CDMController::UpdateSISValue(int newvalue)
{
    m_helper->SetSIS(newvalue);
    return 0;
}

int CDMController::UpdateDriveInMotionValue(bool newvalue)
{
    m_helper->SetDriveInMotion(newvalue);
    return 0;
}

int CDMController::UpdateDriveInParkingPosValue(bool newvalue)
{
    m_helper->SetDriveInParkingPos(newvalue);
    return 0;
}

int CDMController::UpdateDriveParkedValue(bool newvalue)
{
    m_helper->SetDriveParked(newvalue);
    return 0;
}

int CDMController::UpdateDriveTrackingValue(bool newvalue)
{
    m_helper->SetDriveTracking(newvalue);
    return 0;
}

int CDMController::UpdateDriveRaTargetValue(double newvalue)
{
    m_helper->SetRaTarget(newvalue);
    return 0;
}

int CDMController::UpdateDriveDecTargetValue(double newvalue)
{
    m_helper->SetDecTarget(newvalue);
    return 0;
}

int CDMController::UpdateAuxDMEastBottomValue(bool newvalue)
{
    m_helper->SetAuxDMEastBottom(newvalue);
    return 0;
}

int CDMController::UpdateAuxDMEastTopValue(bool newvalue)
{
    m_helper->SetAuxDMEastTop(newvalue);
    return 0;
}

int CDMController::UpdateAuxDMWestBottomValue(bool newvalue)
{
    m_helper->SetAuxDMWestBottom(newvalue);
    return 0;
}

int CDMController::UpdateAuxDMWestTopValue(bool newvalue)
{
    m_helper->SetAuxDMWestTop(newvalue);
    return 0;
}