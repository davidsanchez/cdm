#include <Helper.h>
#include <string>
#include <iostream>
#include <ctime>

#include "Logging.h"

using namespace std;


long int Helper::unix_timestamp()
{
	time_t t = std::time(0);
	long int now = static_cast<long int>(t);
	return now;
}

int Helper::connectOpcUa_Drive(std::string url)
{
    LOG_TRACE << "Helper::connectOpcUa_Drive()";
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
	m_clientOpcUaRef_Drive = pluginsLoader->load();
	if (m_clientOpcUaRef_Drive == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_Drive->connect(url, NULL);
			//ret = m_clientOpcUaRef_Drive->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}


int Helper::connectOpcUa_DataBroker(std::string url)
{
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
			ret = m_clientOpcUaRef_DataBroker->connect(url, NULL);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

int Helper::connectOpcUa_Aux(std::string url)
{
    LOG_TRACE << "Helper::connectOpcUa_Aux()";
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
	m_clientOpcUaRef_Aux = pluginsLoader->load();
	if (m_clientOpcUaRef_Aux == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_Aux->connect(url, NULL);
			//ret = m_clientOpcUaRef_Aux->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

/* 
int Helper::connectOpcUa_Relay(std::string url)
{
    LOG_TRACE << "Helper::connectOpcUa_Relay()";
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
	m_clientOpcUaRef_Relay = pluginsLoader->load();
	if (m_clientOpcUaRef_Relay == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_Relay->connect(url, NULL);
			//ret = m_clientOpcUaRef_Relay->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}

int Helper::connectOpcUa_ECC(std::string url)
{
    LOG_TRACE << "Helper::connectOpcUa_ECC()";
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
	m_clientOpcUaRef_ECC = pluginsLoader->load();
	if (m_clientOpcUaRef_ECC == NULL)
	{
		ret = 1;
	}
	else
	{
		int cpt = 0;
		int flag = 0;
		do
		{
			ret = m_clientOpcUaRef_ECC->connect(url, NULL);
			//ret = m_clientOpcUaRef_ECC->connect(url);
			flag = ret;
			if (cpt == 3)
				flag = 0;
			cpt++;
		} while (flag == -1);
	}
	return ret;
}
 */
