#include "Logging.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

// Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t) {
    logger_t lg;

    // Retrieve the environment variable CDMPATH
    const char* pPath = std::getenv("CDMPATH");
    if (!pPath) {
        throw std::runtime_error("Environment variable CDMPATH is not set.");
    }

    logging::add_common_attributes(); // Adds "LineID", "TimeStamp", "ProcessID" and "ThreadID"

    // Configure the main log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active.log",
        keywords::target = std::string(pPath) + "/log/saved/target_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "all"
    );

    // Configure the image taking log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active_image_taking.log",
        keywords::target = std::string(pPath) + "/log/saved/image_taking_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "image"
    );

    // Configure the debug log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active_debug.log",
        keywords::target = std::string(pPath) + "/log/saved/debug_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = expr::stream
            << "[[%TimeStamp%]] "
            << "[" << expr::attr<std::string>("Channel") << "] "
            << expr::if_(expr::has_attr("Severity"))
                [expr::stream << "[" << expr::attr<boost::log::trivial::severity_level>("Severity") << "] "]
            << expr::smessage,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "debug"
    );

    // Configure the environment log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active_env.log",
        keywords::target = std::string(pPath) + "/log/saved/target_env_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "env"
    );

    // Configure the data log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active_data.log",
        keywords::target = std::string(pPath) + "/log/saved/target_data_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "data"
    );

    // Configure the settings log file
    logging::add_file_log(
        keywords::file_name = std::string(pPath) + "/log/active_settings.log",
        keywords::target = std::string(pPath) + "/log/saved/target_settings_%Y-%m-%d_%H-%M-%S.%N.log",
        keywords::auto_flush = true,
        keywords::format = "%TimeStamp% %Message%",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(7, 30, 0),
        keywords::open_mode = std::ios_base::out | std::ios_base::app,
        keywords::enable_final_rotation = false,
        keywords::filter = a_channel == "settings"
    );

    // Configure the console log
    logging::add_console_log(
        std::clog,
        keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );

    // Set the logging filter
    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace
    );

    return lg;
}
