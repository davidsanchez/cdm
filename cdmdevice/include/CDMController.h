/**
 * CDMController.h
 *
 * Couche métier du plugin CDM — gère la logique fonctionnelle (caméra, drive, config).
 * Hérite de Controller (couche OPC-UA / datapoints).
 *
 * Pattern : Constructeurs avec/sans PluginsBase*, macros #define pour les configs XML,
 *           addDataProcessingCallback pour brancher les callbacks sur les fichiers de config.
 *
 * NOTE : Le code original CDM.cpp n'utilise pas de callbacks de traitement de données 
 *        (pas de addDataProcessingCallback trouvé). Les AsynchronousThread (m_Thread, 
 *        m_ThreadMeteo, m_ThreadLogRestart) sont conservés ici car ils correspondent à
 *        des threads de travail spécifiques (caméra, météo, restart log) et non à des
 *        callbacks OPC-UA standards.
 *
 * CHOIX D'INTERPRÉTATION :
 * - Les trois AsynchronousThread du code original (m_Thread pour la caméra, 
 *   m_ThreadMeteo et m_ThreadLogRestart) sont conservés en private du Controller.
 * - Les méthodes UpdateXXX deleguent au helper ( Helper.SetRaDrive, SetDecDrive, etc.)
 *   ce qui correspond à un objet Helper injecté. Dans le pattern Controller de référence,
 *   on utilise des Config* pour les datapoints OPC-UA. Ici on expose les UpdateXXX 
 *   comme méthodes publiques appelées depuis le Plugin.
 * - La config XML est "PLC_CDM.xml" (identifiable depuis le #define original).
 * - Les méthodes de drive (UpdateRaValue, UpdateDecValue, etc.) sont publiques 
 *   car appelées par le Plugin depuis le thread run().
 */

#ifndef CDMCONTROLLER_H_
#define CDMCONTROLLER_H_

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <boost/any.hpp>
#include "Controller.h"

#include "AsynchronousThread.h"
#include "DatapointMonitor.h"

// Configuration XML pour les datapoints OPC-UA du CDM
#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"
#define DATABROKER_CONFIGURATION_NAME "Mapping_Aux_DB.xml"

class Helper; // Forward declaration (Helper.h à inclure dans le .cpp)

class CDMController : public Controller
{
public:
    // Constructeur avec PluginsBase (pour le pattern standard)
    CDMController(int*, PluginsBase*);
    // Constructeur sans PluginsBase (appel parent avec un nom de config par défaut)
    CDMController(int*);
    ~CDMController();

    // =============================================
    // Méthodes métier publiques (appelées par PluginCDM)
    // =============================================

    // --- Caméra ---
    void ConnectCamera();
    void DisconnectCamera();
    void ConfigureThreadCamera(int nPixelClock, double exposure, double fps, 
                               double gain, const std::string& pixel_format);
    void GetImage();
    void GetMultipleImages(int count, DataAccessClientOPCUA *dataAccessClientOPCUA);
    void StopGetMultipleImages();
    void SingleAcquisition();
    void StartContinuousAcquisition();
    void StopContinuousAcquisition();
    void StartStream();
    void StopStream();
    void StartSG();
    void StopSG();
    void StartCDM();
    void StopCDM();

    void setCameraThread(DataAccessClientOPCUA *dataAccessClientOPCUA) {m_Thread = new AsynchronousThread(dataAccessClientOPCUA);}
    void setMeteoThread(DataAccessClientOPCUA *dataAccessClientOPCUA) {m_ThreadMeteo = new AsynchronousThread(dataAccessClientOPCUA);}
    void setLogThread(DataAccessClientOPCUA *dataAccessClientOPCUA) {m_ThreadLogRestart = new AsynchronousThread(dataAccessClientOPCUA);}

    void set_FSM(int state);
    void set_FSM_in_transition(bool transition);

    void setbitsPerPixel(int input){m_bitsPerPixel=input;}
    int getbitsPerPixel(){return m_bitsPerPixel;
    }
    void enableHeartbeat();
    void setDPQuality();

    void startThread();
    // --- Commentaire ---
    void AddComment(const std::string& comment);

    // --- DataBroker / subscription ---
    void SubscribeDataBroker();

    // --- Recherche datapoint ---
    std::string searchDatapoint(const std::string& element);

    std::map<std::string, std::string>  getCDMConfig(){ return m_config; }
    bool loadCDMConfiguration();
    void setupCameraFromConfig();

    private:
    // === Config OPC-UA ===
    Config *m_cdmConfig;   // "PLC_CDM.xml"
    Config *m_dbConfig;    // DataBroker config (si applicable)

    // === Map de configuration CDM ===
    std::map<std::string, std::string> m_config;

    // === Caméra ===
    Camera *m_camera;     // Pointeur vers l'instance camera (global dans le code original)
    Helper *m_helper;     // Pointeur vers l'instance helper
    int m_bitsPerPixel;   // 8 ou 16 selon le pixel_format

    // === Threads de travail ===
    // NOTE: Ces threads sont spécifiques au CDM et ne sont pas des callbacks 
    //       de traitement de données OPC-UA standards. Ils font le travail 
    //       caméra/météo/log.
    AsynchronousThread *m_Thread;        // Thread caméra principale
    AsynchronousThread *m_ThreadMeteo;   // Thread météorologie
    AsynchronousThread *m_ThreadLogRestart; // Thread log de restart

    // === DataBroker ===
    DatapointMonitor *m_dpMonitorDataBroker;
    int m_connectionResultDataBroker;



    // --- Commandes sur les threads caméra ---
    void startCameraThread();
    void startMeteoThread();
    void startLogRestartThread();


        // --- Réactions aux pertes de connexion (obligatoires, Controller pures virtuelles) ---
    void applyServerConnectionLossReaction() override;
    void applyClientConnectionLossReaction(Config*) override;
};

#endif // CDMCONTROLLER_H_