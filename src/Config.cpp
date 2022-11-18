#include <fstream>
#include <map>
#include <vector>
#include <boost/any.hpp>
#include "Logging.h"

using namespace std;

#define M 12
// #define N 2
#define CONFIG_FILE_PATH  "/home/cdmmgr/cdm/config/CDM.config"
#define LEDLOC_FILE_PATH  "/home/cdmmgr/cdm/config/LedLoc.config"
#define OARLLOC_FILE_PATH  "/home/cdmmgr/cdm/config/ORALLoc.config"

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


 std::vector<std::vector<int>>  LoadLedLoc()
{
    LOG_DEBUG<<"Reading config file "<<LEDLOC_FILE_PATH;
    std::ifstream in(LEDLOC_FILE_PATH);
    if (!in.is_open()) 
    {
        LOG_ERROR <<"Config file not open "<<LEDLOC_FILE_PATH;
        LOG_ERROR <<strerror(errno) ;
        std::vector<std::vector<int>> matrix(M, std::vector<int>(2,0));
        return matrix;
    }

    std::vector<std::vector<int>> matrix;
    double px,py;

    while (!in.eof())
    {
        in>>px>>py;
        std::vector<int> v;
        v.push_back(px);
        v.push_back(py);
        // push back above one-dimensional vector
        matrix.push_back(v);
    }

    return matrix;
}


 std::vector<std::vector<int>>  LoadOARLLoc()
{
    LOG_DEBUG<<"Reading config file "<<OARLLOC_FILE_PATH;
    std::ifstream in(OARLLOC_FILE_PATH);
    if (!in.is_open()) 
    {
        LOG_ERROR <<"Config file not open "<<OARLLOC_FILE_PATH;
        LOG_ERROR <<strerror(errno) ;
        std::vector<std::vector<int>> matrix(M, std::vector<int>(2,0));
        return matrix;
    }

    std::vector<std::vector<int>> matrix;
    double px,py;

    while (!in.eof())
    {
        in>>px>>py;
        std::vector<int> v;
        v.push_back(px);
        v.push_back(py);
        // push back above one-dimensional vector
        matrix.push_back(v);
    }

    return matrix;
}