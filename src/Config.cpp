#include <fstream>
#include <map>
#include <boost/any.hpp>
#include "Logging.h"

using namespace std;

#define CONFIG_FILE_PATH  "/home/cdmmgr/cdm/config/CDM.config"

bool LoadCDMConfiguration( map<std::string,std::string> &config )
{
	LOG_DEBUG<<"Reading config file "<<CONFIG_FILE_PATH;
    std::ifstream in(CONFIG_FILE_PATH);
    if (!in.is_open()) 
    {
        LOG_ERROR <<"Config file not open "<<CONFIG_FILE_PATH;
        LOG_ERROR <<strerror(errno) ;
        return false;
    }

    std::string param,value;

    while (!in.eof())
    {
        in>>param>>value;
        config.insert(make_pair(param, value));
    }

    return true;
}