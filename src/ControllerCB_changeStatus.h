#ifndef DRIVE_CB_changeStatus_H
#define DRIVE_CB_changeStatus_H

#include "CB_changeStatus.h"

enum ServerStatus
{
    Disconnected, /*!< The connection to the server is deactivated by the user of the client API. */
    Connected, /*!< The connection to the server is established and is working in normal mode. */
    ConnectionWarningWatchdogTimeout, /*!< The monitoring of the connection to the server indicated a potential connection problem. */
    ConnectionErrorApiReconnect, /*!< The monitoring of the connection to the server detected an error and is trying to reconnect to the server. */
    ServerShutdown, /*!< The server sent a shut-down event and the client API tries a reconnect. */
    NewSessionCreated /*!< The client was not able to reuse the old session and created a new session during reconnect. This requires to redo register nodes for the new session or to read the namespace array. */
};

class PluginsBase;
class Controller;
class Config;

class ControllerCB_changeStatus : public CB_changeStatus
{
  public:
    ControllerCB_changeStatus(std::string server_name);

    ~ControllerCB_changeStatus()
    {
    };

    void message(int serverStatus);
    void setRefPlugin(PluginsBase *plugin);
    void setRefController(Controller* controller);
    void setRefConfig(Config* config);
  private:
    PluginsBase *m_pluginRefServer;
    int SubscribePLCDataPoints();
    Config *m_config;
    Controller *m_controller;
    bool subscriptionNeeded;
    std::string server_name;


};

#endif
