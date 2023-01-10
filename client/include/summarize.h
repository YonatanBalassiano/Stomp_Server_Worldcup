#pragma once

#include <string>
#include <map>
#include <list>
#include "StompProtocol.h"
#include <mutex>

typedef StompProtocol::Frame Frame;


class summarize{
    public:
    summarize() = default;
    void add (std::string&, std::string&, Frame&);
    void buildTextFile(std::string&, std::string&,std::string&);

    
    private:
    std::map<std::pair<std::string,std::string>, std::list<Frame>> summary;
    std::mutex mutex;

};
