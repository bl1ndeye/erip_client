/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ERIP_ftp.h
 * Author: winter
 *
 * Created on 14 ноября 2015 г., 22:51
 */



 

#ifndef ERIP_FTP_H
#define ERIP_FTP_H
//#define BOOST_NO_CXX11_SCOPED_ENUMS


#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <alloca.h>
#include <string.h>
#include <fstream>

// для копирования файлов
#include <sys/sendfile.h>  // sendfile
#include <fcntl.h>         // open
#include <unistd.h>        // close
#include <sys/stat.h>      // fstat
#include <sys/types.h>     // fstat


// sql resultset
#include <resultset.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
//#include <boost/assign/std/vector.hpp>
// может куда нибудь захерачить вектор.



// ведение лога
#include <syslog.h>


// need ftp lib 
// https://github.com/mkulke/ftplibpp
// ftplib needs ssl.h
// in debian /usr/include/openssl/ssl.h


#include "ftplib.h"

using namespace std;
using namespace boost::filesystem;
        

class erip_ftp
{
public:
    
    // конструктор дефолтный
    
    erip_ftp();
    
    // конструктор для установки заданных параметров подключения
    erip_ftp(string con_host, string con_login, string con_password);
    
    void  set_ftp_config(string con_host, string con_login, string con_password)
    {
        host=con_host;
        login=con_login;
        password=con_password;
        pr_w84upload=false;
    }
   
    
    // служебные функции
    bool establish_connection();
    bool close_connection();
    
    //установление префикса (адрес рабочей директории)
    void Set_Pref(string newpref);
    
    
    // работа с фтп
    
    // изменение текущей директории
    bool cdir(string cd);
    // получение файла с ФТП
    bool get_file(string file_path);
    // получение файлов папки out
    bool get_out_files(string file_name);
    
    
    // запись файла на ФТП
    bool put_file(string full_file_name,string file_path);
    // запись файла (имя файла относительное рабочей директории))
    bool put_file_relate(string file_name,string file_path);
    // удаление файла с фтп
    bool delete_file(string file_name);
    
    
    // работа с сообщениями ЕРИП
    
    // создание сообщения 202 "Платежное требование"
    bool create_payment_request(sql::ResultSet *abons);
    
     
    // запись на ftp платежных требований
    bool put_payment_request();
    
    bool get_payment_regs();
    
    
    // функция  Nlst 
    // деструктор закрывает соединение
    ~erip_ftp();
    
    
    bool pr_w84upload;
protected:
    
    string host;
    string login;
    string password;
    // адрес рабочей директории
    string pref;
    ftplib *ftp;
};




#endif /* ERIP_FTP_H */

