#include "Logging.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t)
{

    char *pPath;
    pPath = std:getenv("HOME");
    if (!pPath) {
        std::cerr << "HOME environment variable not set." << std::endl;
        return 1;
    }

    logger_t lg;

    logging::add_common_attributes(); // Adds "LineID", "TimeStamp", "ProcessID" and "ThreadID"

    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/target_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                //writes messages immediately to file. Should be used only for debug.
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "all");


    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active_image_taking.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/image_taking_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                //writes messages immediately to file. Should be used only for debug.
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "image");

    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active_debug.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/debug_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                //writes messages immediately to file. Should be used only for debug.
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "debug");


    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active_env.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/target_env_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                    //writes messages immediately to file. Should be used only for debug.
        //keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "env");

    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active_data.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/target_data_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                     //writes messages immediately to file. Should be used only for debug.
        //keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "data");

    logging::add_file_log(
        keywords::file_name = std::string(pPath) +"/log/active_settings.log",                                   //active filename
        keywords::target = std::string(pPath) +"/log/saved/target_settings_%Y-%m-%d_%H-%M-%S.%N.log", //filename after the program decides to save the log completely. Usually after the program closes or file size or time based settings set.
        keywords::auto_flush = true,                                                                    //writes messages immediately to file. Should be used only for debug.
        //keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0), // hour, minute, second
        keywords::open_mode = std::ios_base::out | std::ios_base::app,                 //Apends data to the log instead of overwriting.
        keywords::enable_final_rotation = false,                                       //If this is false the active file won't be moved to target_file on program closure. If true a new target_file will be created on program closure.
        keywords::filter = a_channel == "settings");
        
    logging::add_console_log(
        //std::cout,
        std::clog,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
        //, keywords::filter = a_channel == "all" // Doesn't work. Maybe try with the constructor
    );

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace);

    return lg;
}