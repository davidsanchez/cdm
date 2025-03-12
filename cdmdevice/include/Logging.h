#ifndef LOGGING_H_
#define LOGGING_H_

#pragma once

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/log/keywords/severity.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>


BOOST_LOG_ATTRIBUTE_KEYWORD(a_severity, "Severity", boost::log::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(a_channel, "Channel", std::string)

//Narrow-char thread-safe logger.
//typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;
typedef boost::log::sources::severity_channel_logger_mt< boost::log::trivial::severity_level, std::string > logger_t;

#define LOG_TRACE BOOST_LOG_CHANNEL_SEV(my_logger::get(), "all", boost::log::trivial::trace)
#define LOG_DEBUG BOOST_LOG_CHANNEL_SEV(my_logger::get(), "debug", boost::log::trivial::debug)
#define LOG_IMAGE BOOST_LOG_CHANNEL_SEV(my_logger::get(), "image", boost::log::trivial::info)
#define LOG_INFO BOOST_LOG_CHANNEL_SEV(my_logger::get(), "all", boost::log::trivial::info)
#define LOG_WARNING BOOST_LOG_CHANNEL_SEV(my_logger::get(), "all", boost::log::trivial::warning)
#define LOG_ERROR BOOST_LOG_CHANNEL_SEV(my_logger::get(), "all", boost::log::trivial::error)
#define LOG_FATAL BOOST_LOG_CHANNEL_SEV(my_logger::get(), "all", boost::log::trivial::fatal)


#define LOG_ENV BOOST_LOG_CHANNEL_SEV(my_logger::get(), "env", boost::log::trivial::fatal)
#define LOG_DATA BOOST_LOG_CHANNEL_SEV(my_logger::get(), "data", boost::log::trivial::fatal)
#define LOG_SETTINGS BOOST_LOG_CHANNEL_SEV(my_logger::get(), "settings", boost::log::trivial::fatal)


//declares a global logger with a custom initialization
BOOST_LOG_GLOBAL_LOGGER(my_logger, logger_t)

#endif //  LOGGING_H_