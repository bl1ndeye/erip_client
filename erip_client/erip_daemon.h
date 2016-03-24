/* 
 * File:   erip_daemon.h
 * Author: winter
 *
 * Created on 23 ноября 2015 г., 20:10
 */




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
// одули приложения
#include "abills_mysql_class.h"
#include "ERIP_ftp_class.h"
// модуль для работы с xml
#include "pugixml.hpp"




class erip_daemon {
    protected:
              
        // демонизация
        void daemonize();
     
        // записать pid в файл
        void setpidfile();

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
        // задание параметров авторизаций, а также 
        // создание в рабочей директории файла настроек.
        bool reconfigure();
        // при запуске читает файл настроек авторизации из рабочей директории
        bool read_config();
        // счетчик для исключения бесонечной рекурсии при 
        // чтении файла настроек
        int rec_count; 
 
        
};

#endif /* ERIP_DAEMON_H */

