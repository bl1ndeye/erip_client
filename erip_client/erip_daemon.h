/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   erip_daemon.h
 * Author: winter
 *
 * Created on 23 ноября 2015 г., 20:10
 */



// config log
// # /etc/rsyslog.d/30-erip_client.conf

// ведение лога

//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/sinks/text_file_backend.hpp>
//#include <boost/log/utility/setup/file.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/sources/severity_logger.hpp>
//#include <boost/log/sources/record_ostream.hpp>
//
//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace sinks = boost::log::sinks;
//namespace keywords = boost::log::keywords;


#ifndef ERIP_DAEMON_H
#define ERIP_DAEMON_H

// всякие дефолтные либы
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <syslog.h>
#include <signal.h>

#include <vector>
#include <string>
#include <fstream>

#include <unistd.h>








// мои либы
#include "abills_mysql_class.h"
#include "ERIP_ftp_class.h"
// либа для работы с xml
#include "pugixml.hpp"




class erip_daemon {
    protected:
        
        
        // демонизация
        void daemonize();

       
        // записать pid в файл
        void setpidfile();
        // хрень какая то
        static void sighandler(int signum);
        // класс для транспорта фтп
        erip_ftp ftp_lsv;
        // работа с базой биллинга abills
        abills_mysql abills_lsv;
        // xml документ файл конфига
        pugi::xml_document config;  
  
    public:
        // запуск демона
        int run(string pre);
        // префикс рабочей директории
        string pref;
        // конструктор по дефалту
        erip_daemon();
        bool reconfigure();
        bool read_config();
        int rec_count; // счетчик для исключения рекурсии
 
        
};

#endif /* ERIP_DAEMON_H */

