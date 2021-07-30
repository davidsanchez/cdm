#include "ControllerCB_changeStatus.h"

#include "pluginsBase.h"

#include "string"
#include <stdio.h>

#include <fstream>
#include <unistd.h>

#include "Logging.h"

ControllerCB_changeStatus::ControllerCB_changeStatus(std::string server_name, CDM *cdm)
{
    m_controller = NULL;
    m_pluginRefServer = NULL;
    m_config = NULL;
    this->server_name = server_name;
    this->cdm = cdm;
}

void ControllerCB_changeStatus::message(int serverStatus)
{
    LOG_INFO << "-------------------------------------ControllerCB_changeStatus ------------------------\n";

    std::string message = "";
    std::string nameOfConfig = "";

    switch (serverStatus)
    {
    case Disconnected:
        message = server_name + " Connection status changed to Disconnected";
        cout << message << endl;
        subscriptionNeeded = true;
        if (m_pluginRefServer != NULL)
        {
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->setCommunicationLossStatus(true);
        break;
    case Connected:
        message = server_name + " Connection status changed to Connected";
        cout << message << endl;
        
        if(server_name == "DataBroker")
            {this->cdm->subscribe_DataBroker();}
        else if (server_name == "Drive")
            {this->cdm->subscribe_Drive();}

        if (m_pluginRefServer != NULL)
        {
            //m_controller->doSubscription(m_config);
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->setCommunicationLossStatus(false);
        //if (subscriptionNeeded)
            //m_controller->doSubscription(m_config);
        //break;
    case ConnectionWarningWatchdogTimeout:
        message = server_name + " Connection status changed to ConnectionWarningWatchdogTimeout";
        cout << message << endl;
        subscriptionNeeded = false;
        if (m_pluginRefServer != NULL)
        {
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->setCommunicationLossStatus(true);
        break;
    case ConnectionErrorApiReconnect:
        //printf("Connection status changed to ConnectionErrorApiReconnect\n");
        message = server_name + " Connection status changed to ConnectionErrorApiReconnect";
        cout << message << endl;
        subscriptionNeeded = true;
        if (m_pluginRefServer != NULL)
        {
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->setCommunicationLossStatus(true);
        break;
    case ServerShutdown:
        //printf("Connection status changed to ServerShutdown\n");
        message = server_name + " Connection status changed to ServerShutdown";
        cout << message << endl;
        subscriptionNeeded = true;
        if (m_pluginRefServer != NULL)
        {
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->setCommunicationLossStatus(true);
        break;
    case NewSessionCreated:
        //printf("Connection status changed to NewSessionCreated\n");
        message = server_name + " Connection status changed to NewSessionCreated";
        cout << message << endl;
        subscriptionNeeded = false;
        if (m_pluginRefServer != NULL)
        {
            //m_pluginRefServer->getDataAccessClientOPCUARef()->setDatapoint(DatapointError_, DatapointNamespace_, message);
        }
        //m_controller->doSubscription(m_config);
        //SubscribePLCDataPoints();
        //m_controller->setCommunicationLossStatus(false);
        break;
    }

    LOG_INFO << "-------------------------------------------------------------\n";
}

void ControllerCB_changeStatus::setRefPlugin(PluginsBase *plugin)
{
    m_pluginRefServer = plugin;
}

void ControllerCB_changeStatus::setRefController(Controller *controller)
{
    m_controller = controller;
}

void ControllerCB_changeStatus::setRefConfig(Config *config)
{
    m_config = config;
}

int ControllerCB_changeStatus::SubscribePLCDataPoints()
{
    // cout << "Re-subscribe to DPs after NewSessionCreated event received ..." << endl;
    // int ret = 1;
    // std::string url = m_config->getOpcUaRef();
    // std::vector<ListElement*>* tempListSubscribe = m_config->getListSubscribe();
    // CallbackL2 *callBack = new CallbackL2(m_controller, m_config);
    // if (m_controller)
    // {
    // 	ClientsManager *t = m_controller->getClientsManagerRef(m_config->getFileName());
    // 	if (t != NULL)
    // 	{
    // 		t->subscribe(tempListSubscribe, callBack);
    // 		t->startSubscribe();
    // 	}
    // }
    // return ret;
}
