#ifndef CDM_H_
#define CDM_H_

#include <cstring>
#include <fstream>
#include <sstream>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctime>
#include <iomanip> // Used for setprecision
#include <cmath>
#include <unistd.h> // TODO: why is this used? For usleep for example.

#include "pluginsBase.h"	//TODO: move this to .h file? Was in .cpp before
#include "lappThread.h" // needed for MOS
#include "test_asynchroneThread.h"

//#include <ueye.h> // IDS camera

// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/video/video.hpp>
// #include <opencv2/opencv.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/bimap.hpp>
//#include <boost/assign.hpp>


//namespace logging = boost::log;
//namespace keywords = boost::log::keywords;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;


class CDM : public PluginsBase
{
public:
    int init(const std::string& chaine);
    int close();
    int cmd(const std::string& chaine, int commandStringAck, std::string& result);

    // new virtual methods appears with the version 3.0 of MOS
    int afterStart();
    int cmdAsynch(const std::string& command, int commandStringAck, const std::string& datapointName, int nameSpace,
                  std::string& result);

    // new virtual methods who replace the setAnay getAny methods  with the version 4.0 of MOS
    int get(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);
    int set(const std::string& chaine, int commandStringAck, std::vector<boost::any>& tabValue);

    int AddComment(std::string comment);

private:

    std::string element_opcua_cdm_image = "MOS_Server.CDM.Image.Image_v";
    TestAsynchroneThread* m_testThread;


};
#endif //  CDM_H_
