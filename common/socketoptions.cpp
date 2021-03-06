
#include "socketoptions.hpp"

using namespace std;


set<string> true_names = { "1", "yes", "on", "true" };
set<string> false_names = { "0", "no", "off", "false" };


SocketOption::Mode SrtConfigurePre(SRTSOCKET socket, string host, map<string, string> options, vector<string>* failures)
{
    vector<string> dummy;
    vector<string>& fails = failures ? *failures : dummy;

    if ( options.count("passphrase") )
    {
        // Insert default
        if ( options.count("pbkeylen") == 0 )
        {
            options["pbkeylen"] = "16"; // m_output_direction ? "16" : "0";
        }
    }

    SocketOption::Mode mode;
    string modestr = "default";

    if ( options.count("mode") )
    {
        modestr = options["mode"];
    }

    if ( modestr == "client" || modestr == "caller" )
    {
        mode = SocketOption::CALLER;
    }
    else if ( modestr == "server" || modestr == "listener" )
    {
        mode = SocketOption::LISTENER;
    }
    else if ( modestr == "default" )
    {
        // Use the following convention:
        // 1. Server for source, Client for target
        // 2. If host is empty, then always server.
        if ( host == "" )
            mode = SocketOption::LISTENER;
        //else if ( !dir_output )
        //mode = "server";
        else
        {
            // Host is given, so check also "adapter"
            if ( options.count("adapter") )
                mode = SocketOption::RENDEZVOUS;
            else
                mode = SocketOption::CALLER;
        }
    }
    else
    {
        mode = SocketOption::FAILURE;
        fails.push_back("mode");
    }

    bool all_clear = true;
    for (auto o: srt_options)
    {
        if ( o.binding == SocketOption::PRE && options.count(o.name) )
        {
            string value = options.at(o.name);
            bool ok = o.apply<SocketOption::SRT>(socket, value);
            if ( !ok )
            {
                fails.push_back(o.name);
                all_clear = false;
            }
        }
    }

    return all_clear ? mode : SocketOption::FAILURE;
}

void SrtConfigurePost(SRTSOCKET socket, map<string, string> options, vector<string>* failures)
{
    vector<string> dummy;
    vector<string>& fails = failures ? *failures : dummy;

    for (auto o: srt_options)
    {
        if ( o.binding == SocketOption::POST && options.count(o.name) )
        {
            string value = options.at(o.name);
            bool ok = o.apply<SocketOption::SRT>(socket, value);
            if ( !ok )
                fails.push_back(o.name);
        }
    }
}


