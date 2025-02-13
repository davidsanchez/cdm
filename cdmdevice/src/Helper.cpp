#include <Helper.h>

#include <ctime>
#include <iostream>
#include <string>

#include "ControllerCB_changeStatus.h"
#include "Logging.h"

#include "CDM.h"

using namespace std;



std::string  Helper::searchDatapoint (string element)
{
        ListElement *myElement = NULL;
        std::string rootElement = "";
        std::string nodeIdL1 = "";

        myElement = cdm_config->searchElement(element);
        rootElement = cdm_config->getRootName() + ".";
        if (myElement)
        {
		nodeIdL1 = rootElement + myElement->NodeId;
        LOG_DEBUG<<"nodeIdL1 "<<nodeIdL1<< '\n';
                return(nodeIdL1);
        }
        return("");
}



long int Helper::unix_timestamp()
{
    time_t t = std::time(0);
    long int now = static_cast<long int>(t);
    return now;
}

int Helper::connectOpcUa_DataBroker(std::string url, CDM *cdm)
{

    const int timeout = 5; // in seconds

    ControllerCB_changeStatus *dataBroker_CB = new ControllerCB_changeStatus(cdm);

    LOG_TRACE << "Helper::connectOpcUa_DataBroker()";
    int ret = 0;

    std::string pluginClass = "ptr_Plugin";
    DynamicLoader *pluginsLoader;

    std::string pluginFile = API_LIB_PATH;
    char *pPath;
    pPath = getenv("MOS_PATH");
    if (pPath != NULL)
    {
        pluginFile = pPath;
        pluginFile += "/../lib/libDataAccessClientOPCUA.so";
    }

    pluginsLoader = new DynamicLoader(pluginFile, pluginClass);
    m_clientOpcUaRef_DataBroker = pluginsLoader->load();
    if (m_clientOpcUaRef_DataBroker == NULL)
    {
        ret = 1;
    }
    else
    {
        int cpt = 0;
        int flag = 0;
        do
        {

            int l_typeAuthentification=0;
            ret = m_clientOpcUaRef_DataBroker->setTypeAuthentification(l_typeAuthentification);
            ret = m_clientOpcUaRef_DataBroker->connect(url, dataBroker_CB, timeout);
            flag = ret;
            if (cpt == 3)
                flag = 0;
            cpt++;
        } while (flag == -1);
    }
    return ret;
}
