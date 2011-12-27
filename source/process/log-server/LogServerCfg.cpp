#include "LogServerCfg.h"

#include <util/kv2string.h>
#include <util/string/StringUtils.h>

#include <iostream>
#include <fstream>

namespace sf1r
{

typedef izenelib::util::kv2string properties;

static const unsigned int DEFAULT_THREAD_NUM = 30;
static const std::size_t DEFAULT_DRUM_NUM_BUCKETS = 64;
static const std::size_t DEFAULT_DRUM_BUCKET_BUFF_ELEM_SIZE = 8192;
static const std::size_t DEFAULT_DRUM_BUCKET_BYTE_SIZE = 1048576;

LogServerCfg::LogServerCfg()
    : rpcPort_(0)
    , driverPort_(0)
{
}

LogServerCfg::~LogServerCfg()
{
}

bool LogServerCfg::parse(const std::string& cfgFile)
{
    cfgFile_ = cfgFile;
    return parseCfgFile_(cfgFile);
}

bool LogServerCfg::parseCfgFile_(const std::string& cfgFile)
{
    try
    {
        // load configuration file
        std::ifstream cfgInput(cfgFile.c_str());
        std::string cfgString;
        std::string line;

        if (!cfgInput.is_open())
        {
            std::cerr << "Could not open file: " << cfgFile << std::endl;
            return false;
        }

        while (getline(cfgInput, line))
        {
            izenelib::util::Trim(line);
            if (line.empty() || line[0] == '#')
            {
                // ignore empty line and comment line
                continue;
            }

            if (!cfgString.empty())
            {
                cfgString.push_back('\n');
            }
            cfgString.append(line);
        }

        // check configuration properties
        properties props('\n', '=');
        props.loadKvString(cfgString);

        if (!props.getValue("logServer.host", host_))
        {
            host_ = "localhost";
        }
        if (!props.getValue("logServer.rpcPort", rpcPort_))
        {
            throw std::runtime_error("Log Server Configuration missing proptery: logServer.rpcPort");
        }
        if (!props.getValue("logServer.driverPort", driverPort_))
        {
            throw std::runtime_error("Log Server Configuration missing proptery: logServer.driverPort");
        }
        if (!props.getValue("logServer.threadNum", threadNum_))
        {
            threadNum_ = DEFAULT_THREAD_NUM;
        }
        if (!props.getValue("drum.name", drum_name_))
        {
            throw std::runtime_error("Log Server Configuration missing proptery: drum.name");
        }
        if (!props.getValue("drum.num_buckets", drum_num_buckets_))
        {
            drum_num_buckets_ = DEFAULT_DRUM_NUM_BUCKETS;
        }
        if (!props.getValue("drum.bucket_buff_elem_size", drum_bucket_buff_elem_size_))
        {
            drum_bucket_buff_elem_size_ = DEFAULT_DRUM_BUCKET_BUFF_ELEM_SIZE;
        }
        if (!props.getValue("drum.bucket_byte_size", drum_bucket_byte_size_))
        {
            drum_bucket_byte_size_ = DEFAULT_DRUM_BUCKET_BYTE_SIZE;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

}