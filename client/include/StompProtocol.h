#pragma once

#include <string>
#include <map>
#include "event.h"
#include <list>

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    enum class Command
    {
        SEND,
        SUBSCRIBE,
        UNSUBSCRIBE,
        CONNECTED,
        CONNECT,
        DISCONNECT,
        MESSAGE,
        ERROR,
        RECEIPT,
    };
    class Frame
    {
    public:
        Command command;
        std::map<std::string, std::string> headers;

        // std::list <std::map<std::string, std::string>> maps;
        std::array<std::map<std::string, std::string>, 3> maps;
        std::map<std::string, std::string> body;
        std::string end = "\0";


        Frame()=default;
    };
    std::string stompToStringForReports(Frame &);
    std::string stompToString(Frame &);

    Frame stringToStompForMessage(std::string &);
    Frame stringToStomp(std::string &);

    // build a frame
    Frame buildConnectFrame(std::string &acceptVersion, std::string &host, std::string &login, std::string &passcode);
    Frame buildDisconnectFrame(std::string &receipt);
    Frame buildSubscribeFrame(std::string &destination, std::string &id, std::string &receipt);
    Frame buildUnsubscribeFrame(std::string &id, std::string &receipt);
    Frame buildSendFrame(std::string &destination, Event &body, std::string &user);

private:
    static Command getCommand(std::string &str)
    {
        if (str == "SEND")
            return Command::SEND;
        else if (str == "SUBSCRIBE")
            return Command::SUBSCRIBE;
        else if (str == "UNSUBSCRIBE")
            return Command::UNSUBSCRIBE;
        else if (str == "CONNECTED")
            return Command::CONNECTED;
        else if (str == "CONNECT")
            return Command::CONNECT;
        else if (str == "DISCONNECT")
            return Command::DISCONNECT;
        else if (str == "MESSAGE")
            return Command::MESSAGE;
        else if (str == "ERROR")
            return Command::ERROR;
        else
            return Command::RECEIPT;
    }
    static std::string getCommandString(Command &command)
    {
        switch (command)
        {
        case Command::SEND:
            return "SEND";
        case Command::SUBSCRIBE:
            return "SUBSCRIBE";
        case Command::UNSUBSCRIBE:
            return "UNSUBSCRIBE";
        case Command::CONNECTED:
            return "CONNECTED";
        case Command::CONNECT:
            return "CONNECT";
        case Command::DISCONNECT:
            return "DISCONNECT";
        case Command::MESSAGE:
            return "MESSAGE";
        case Command::ERROR:
            return "ERROR";
        case Command::RECEIPT:
            return "RECEIPT";
        }
        return NULL;
    }
};
