#include <map>
#include <string>
#include <list>
#include "summarize.h"
#include <iostream>
#include <fstream>
#include "StompProtocol.h"
using namespace std;
typedef StompProtocol::Frame Frame;

// summarize :: summarize(){};

void summarize ::add(std::string &destination, std::string &user, Frame &frame)
{
    destination = destination.substr(1, destination.length());
    std::lock_guard<std::mutex> lock(mutex);
    summary[std::make_pair(destination, user)].push_back(frame);
}

void summarize ::buildTextFile(std::string &destination, std::string &user, std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex);
    std ::list<Frame> list = summary[std::make_pair(destination, user)];

    // all json arguments
    std::string gameName = destination.replace(destination.find("_"), 1, " VS "); // change _ to " VS "
    std::map<std::string, std::string> genreralStats;
    std::map<std::string, std::string> teamAStats;
    std::map<std::string, std::string> teamBStats;
    std::list<std::string> gameEvents;

    // general stats
    for (Frame frame : list)
    {
        if (frame.headers["user"] != user)
        {
            continue;
        }
        for (auto const &[key, val] : frame.maps[0])
        {
            genreralStats[key] = val;
        }

        for (auto const &[key, val] : frame.maps[1])
        {
            teamAStats[key] = val;
        }

        for (auto const &[key, val] : frame.maps[2])
        {
            teamBStats[key] = val;
        }

        std::string a = frame.headers["time"] + " - " + frame.headers["event name"] + ":\n\n" + frame.headers["description"] + "\n\n";
        gameEvents.push_back(a);
    }

    ofstream fw(path, std::ofstream::out);

    if (fw.is_open())
    {
        // write the file
        fw << gameName << "\n";
        fw << "Game stats:"
           << "\n";
        fw << "General stats:"
           << "\n";
        for (auto const &[key, val] : genreralStats)
        {
            fw << key << ": " << val << "\n";
        }

        fw << destination.substr(0, destination.find(" VS ")) << " stats:"
           << "\n";
        for (auto const &[key, val] : teamAStats)
        {
            fw << key << ": " << val << "\n";
        }

        fw << destination.substr(destination.find(" VS ") + 4) << " stats:"
           << "\n";

        for (auto const &[key, val] : teamBStats)
        {
            fw << key << ": " << val << "\n";
        }

        fw << "Game event reports:"
           << "\n";
        for (std::string s : gameEvents)
        {
            fw << s << "\n";
        }

        fw.close();
    }
    else
        cout << "Problem with opening file";
}
