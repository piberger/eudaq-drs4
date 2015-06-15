/*
 * TestLogger.cxx
 *
 *  Created on: Jun 15, 2015
 *      Author: bachmair
 */
#include "eudaq/RunControl.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include <iostream>
#include "eudaq/LogSender.hh"

bool finish= false;
std::string m_type = "Logger";
std::string m_name = "Test";
std::string m_addr = "tcp://44002";


int analyseCommand(char cmd, std::string line){
    switch (cmd) {
        case '\0': // ignore
            break;
        case 'l':
            try{
                EUDAQ_USER(line);
            }
            catch(...){
                EUDAQ_LOG_RECONNECT();//m_type, m_name,m_addr);
            }
            break;
        case 'q':
            finish=true;
            break;
        case '?':
            return true;
            break;
        case 'h':
            return true;
        default:
            std::cout << "Unrecognised command, type ? for help" << std::endl;
    }
    return false;
}
int main(int /*argc*/, const char ** argv) {
    eudaq::OptionParser op("EUDAQ logger", "1.0", "A command-line Logger for EUDAQ");
    eudaq::Option<std::string> addr (op, "a", "send-address", "tcp://44002", "address",
            "The address on which to listen for connections");
    eudaq::Option<std::string> level(op, "l", "log-level",      "NONE", "level",
            "The minimum level for displaying log messages locally");
    eudaq::Option<std::string> name (op, "n", "name", "Test", "string",
        "The name of this Logger");
    try {
      op.Parse(argv);
      m_name = name.Value();
      m_addr =  addr.Value();
      bool help = true;
      EUDAQ_LOG_CONNECT(m_type,m_name ,m_addr);
      EUDAQ_LOG_LEVEL(level.Value());
      EUDAQ_USER("TEST");

      do {
           if (help) {
             help = false;
             std::cout << "--- Commands ---\n"
               << "s data Send StringEvent with 'data' as payload\n"
               << "r size Send RawDataEvent with size bytes of random data (default=1k)\n"
               << "l msg  Send log message\n"
               << "o msg  Set status=OK\n"
               << "w msg  Set status=WARN\n"
               << "e msg  Set status=ERROR\n"
               << "q      Quit\n"
               << "?      \n"
               << "----------------" << std::endl;
           }
           std::string line;
           std::getline(std::cin, line);
           //std::cout << "Line=\'" << line << "\'" << std::endl;
           char cmd = '\0';
           if (line.length() > 0) {
             cmd = std::tolower(line[0]);
             line = eudaq::trim(std::string(line, 1));
           } else {
             line = "";
           }
           help = analyseCommand(cmd,line);
         } while (!finish);
         std::cout << "Quitting" << std::endl;

    } catch (...) {
        return op.HandleMainException();
    }
    return 0;
}
