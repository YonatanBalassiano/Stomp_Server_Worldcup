#include "../include/StompProtocol.h"
typedef StompProtocol::Frame Frame;
#include <string>
#include <bits/stdc++.h>
#include <iterator> // for std::advance
#include <ranges>
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace boost;

std::string StompProtocol ::stompToStringForReports(Frame &frame)
{
    std::stringstream output;

    // Write the command
    output << getCommandString(frame.command) << "\n";

    // Write the headers
    for (const auto &a : frame.headers)
    {
        const auto [key, value] = a;
        if (key == "destination")
        {
            output << key << ":/" << value << "\n";
        }
        else
        {
            output << key << ":" << value << "\n";
        }
    }
    output << "\n";
    output << "user"
           << ":" << frame.body["user"] << "\n";
    output << "team a"
           << ":" << frame.body["team_a_name"] << "\n";
    output << "team b"
           << ":" << frame.body["team_b_name"] << "\n";
    output << "event name"
           << ":" << frame.body["event name"] << "\n";
    output << "time"
           << ":" << frame.body["time"] << "\n";

    if (frame.maps.size() > 0)
    {
        // test purpose

        output << "general game updates:"
               << "\n";
        if (frame.maps[0].size() > 0)
        {
            for (const auto &[keyT, valueT] : frame.maps[0])
            {
                output << "\t" << keyT << ":" << valueT << "\n";
            }
        }

        output << "team a updates:"
               << "\n";
        if (frame.maps[1].size() > 0)
        {
            for (const auto &[keyT, valueT] : frame.maps[1])
            {
                output << "\t" << keyT << ":" << valueT << "\n";
            }
        }

        output << "team b updates:"
               << "\n";
        if (frame.maps[2].size() > 0)
        {
            for (const auto &[keyT, valueT] : frame.maps[2])
            {
                output << "\t" << keyT << ":" << valueT << "\n";
            }
        }
    }

    output << "description"
           << ": "
           << "\n"
           << frame.body["description"] << "\n";
    output << frame.end;
    return output.str();
}

std::string StompProtocol ::stompToString(Frame &frame)
{
    std::stringstream output;

    // Write the command
    output << getCommandString(frame.command) << "\n";

    // Write the headers
    for (const auto &[key, value] : frame.headers)
    {
        output << key << ":" << value << "\n";
    }

    for (const auto &[key, value] : frame.maps[0])
    {
        output << key << ":" << value << "\n";
    }

    for (const auto &[key, value] : frame.maps[1])
    {
        output << key << ":" << value << "\n";
    }

    for (const auto &[key, value] : frame.maps[2])
    {
        output << key << ":" << value << "\n";
    }

    for (const auto &[key, value] : frame.body)
    {
        output << key << ":" << value << "\n";
    }

    output << frame.end;
    return output.str();
}

Frame StompProtocol ::stringToStomp(std::string &str)
{
    Frame frame = Frame();
    std::stringstream test(str);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(test, segment))
    {
        seglist.push_back(segment);
    }

    int i = 0;
    for (std::string a : seglist)
    {
        if (i == 0)
        {
            frame.command = getCommand(a);
            i++;
            continue;
        }
        std::size_t pos = a.find(':');
        if (pos != std::string::npos)
        {
            std::string key = a.substr(0, pos);
            std::string value = a.substr(pos + 1);
            frame.headers[key] = value;
        }
    }
    return frame;
}

Frame StompProtocol ::stringToStompForMessage(std::string &str)
{
    Frame frame = Frame();
    std::stringstream test(str);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(test, segment))
    {
        string temp = segment;
        trim_left(temp);
        trim_right(temp);
        seglist.push_back(temp);
    }
    frame.command = getCommand(seglist[0]);
    for (int i = 1; i < 10; i++)
    {
        if (seglist[i] != "'\n")
        {
            std::size_t pos = seglist[i].find(':');
            if (pos != std::string::npos)
            {
                std::string key = seglist[i].substr(0, pos);
                std::string value = seglist[i].substr(pos + 1);
                frame.headers[key] = value;
            }
        }
    }
    int index = 11;
    int tempSize = seglist.size();
    while (index < tempSize && seglist[index].compare("team a updates:") != 0)
    {
        if (seglist[index] != "\n")
        {
            std::size_t pos = seglist[index].find(':');
            if (pos != std::string::npos)
            {
                std::string key = seglist[index].substr(0, pos);
                std::string value = seglist[index].substr(pos + 1);
                frame.maps[0][key] = value;
            }
        }
        index++;
    }
    index++;
    while (index < tempSize && seglist[index].compare("team b updates:") != 0)
    {
        if (seglist[index] != "\n")
        {
            std::size_t pos = seglist[index].find(':');
            if (pos != std::string::npos)
            {
                std::string key = seglist[index].substr(0, pos);
                std::string value = seglist[index].substr(pos + 1);
                frame.maps[1][key] = value;
            }
        }
        index++;
    }
    index++;
    while (index < tempSize && seglist[index].compare("description:") != 0)
    {

        if (seglist[index] != "\n")
        {
            std::size_t pos = seglist[index].find(':');
            if (pos != std::string::npos)
            {
                std::string key = seglist[index].substr(0, pos);
                std::string value = seglist[index].substr(pos + 1);
                frame.maps[2][key] = value;
            }
        }
        index++;
    }

    index++;

    frame.headers["description"] = seglist[index];

    return frame;
}

Frame StompProtocol ::buildConnectFrame(std::string &acceptVersion, std::string &host, std::string &login, std::string &passcode)
{
    Frame frame;
    frame.command = Command::CONNECT;
    frame.headers["accept-version"] = acceptVersion;
    frame.headers["host"] = host;
    frame.headers["login"] = login;
    frame.headers["passcode"] = passcode;
    return frame;
}

Frame StompProtocol ::buildSubscribeFrame(std::string &destination, std::string &id, std::string &receipt)
{
    Frame frame;
    frame.command = Command::SUBSCRIBE;
    frame.headers["destination"] = destination;
    frame.headers["id"] = id;
    frame.headers["receipt"] = receipt;
    return frame;
}

Frame StompProtocol ::buildUnsubscribeFrame(std::string &id, std::string &receipt)
{
    Frame frame;
    frame.command = Command::UNSUBSCRIBE;
    frame.headers["id"] = id;
    frame.headers["receipt"] = receipt;

    return frame;
}

Frame StompProtocol ::buildDisconnectFrame(std::string &receipt)
{
    Frame frame;
    frame.command = Command::DISCONNECT;
    frame.headers["receipt"] = receipt;
    return frame;
}

Frame StompProtocol ::buildSendFrame(std::string &destination, Event &event, std::string &user)
{
    Frame frame;
    frame.command = Command::SEND;
    frame.headers["destination"] = destination;

    frame.body["user"] = user;
    frame.body["team_a_name"] = event.get_team_a_name();
    frame.body["team_b_name"] = event.get_team_b_name();
    frame.body["event name"] = event.get_name();
    frame.body["time"] = to_string(event.get_time());
    frame.body["description"] = event.get_discription();

    frame.maps[0] = event.get_game_updates();
    frame.maps[1] = event.get_team_a_updates();
    frame.maps[2] = event.get_team_b_updates();

    return frame;
}