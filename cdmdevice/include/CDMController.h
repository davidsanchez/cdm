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
#include "Config.h"
#include "AsynchronousThread.h"
#include "DatapointMonitor.h"

// Configuration XML pour les datapoints OPC-UA du CDM
#define CDM_CONFIGURATION_NAME "PLC_CDM.xml"

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
    void ConfigureCamera(int nPixelClock, double exposure, double fps, 
                         double gain, const std::string& pixel_format);
    void ConfigureThreadCamera(int nPixelClock, double exposure, double fps, 
                               double gain, const std::string& pixel_format);
    void GetImage();
    void GetMultipleImages(int count, const std::string& args);
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

    // --- Commentaire ---
    void AddComment(const std::string& comment);

    // --- Drive (RA, Dec, Az, Zd, offsets, source, etc.) ---
    void UpdateRaValue(double newvalue);
    void UpdateDecValue(double newvalue);
    void UpdateAzValue(double newvalue);
    void UpdateZdValue(double newvalue);
    void UpdateAzOffsetValue(double newvalue);
    void UpdateZdOffsetValue(double newvalue);
    void UpdateSourceValue(const std::string& newvalue);
    void UpdateOARLValue(bool newvalue);
    void UpdateLEDsValue(bool newvalue);
    void UpdateShutterValue(int newvalue);
    void UpdateSISValue(int newvalue);

    void UpdateDriveInMotionValue(bool newvalue);
    void UpdateDriveInParkingPosValue(bool newvalue);
    void UpdateDriveParkedValue(bool newvalue);
    void UpdateDriveTrackingValue(bool newvalue);
    void UpdateDriveRaTargetValue(double newvalue);
    void UpdateDriveDecTargetValue(double newvalue);

    void UpdateAuxDMEastBottomValue(bool newvalue);
    void UpdateAuxDMEastTopValue(bool newvalue);
    void UpdateAuxDMWestBottomValue(bool newvalue);
    void UpdateAuxDMWestTopValue(bool newvalue);

    // --- DataBroker / subscription ---
    void SubscribeDataBroker();

    // --- Recherche datapoint ---
    std::string searchDatapoint(const std::string& element);

    // =============================================
    // Config / configuration
    // =============================================
    Config* getCDMConfig() const { return m_cdmConfig; }

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

    // === Méthodes privées ===
    int loadCDMConfiguration(std::map<std::string, std::string>& configOut);
    void setupCameraFromConfig();

    // --- Commandes sur les threads caméra ---
    void startCameraThread();
    void startMeteoThread();
    void startLogRestartThread();
};

#endif // CDMCONTROLLER_H_