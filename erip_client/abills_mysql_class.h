/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   abills_mysql_class.h
 * Author: winter
 *
 * Created on 12 ноября 2015 г., 14:15
 */

#ifndef ABILLS_MYSQL_CLASS_H
#define ABILLS_MYSQL_CLASS_H

// common libs
#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <alloca.h>

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
// work with file libs
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>

// для копирования файлов
#include <sys/sendfile.h>  // sendfile
#include <fcntl.h>         // open
#include <unistd.h>        // close
#include <sys/stat.h>      // fstat
#include <sys/types.h>     // fstat


// mysql libs
#include <driver.h>
#include <exception.h>
#include <resultset.h>
#include <statement.h>
#include <mysql_connection.h>
#include <prepared_statement.h>

// ведение лога
#include <syslog.h>



using namespace std;
class abills_mysql
{
public:
    // конструктор тестовый
    abills_mysql();
    // конструктор с параметрами
    abills_mysql( sql::SQLString sethost,  sql::SQLString setlogin, sql::SQLString setpassword);
    void set_mysql_config(sql::SQLString sethost,  sql::SQLString setlogin, sql::SQLString setpassword);;
    
    // деструктор по умолчанию
    ~abills_mysql();
    //  служебные функции
    
    // установка адреса рабочей папки
    void Set_Pref(string newpref);
    // коннект (thread init)
   int Connect();
   // дисконнект (thread init)
   int Disconnect();
   // при возникновении ошибок запись в лог
   void Write_Log(sql::SQLException e);
   void Write_Log_Text(string err_text);
   
   
   // функции работы с данными базы
   
   // поиск uid по логину
   // возвращает uid выбранного логина
   // возвращает 0 если не найден
   string  Find_UID_From_Login(string login2find);
   // поиск id счета по логину
   // возвращает id счета выбранного логина
   // возвращает 0 если не найден
   string  Find_BILL_ID_From_Login(string login2find);
   // получение суммы на счету выбранного логина
   // возвращает -1111111 если не найдена
   int     Get_DEPOSIT_From_Login (string login2find);
   // добавление суммы на счет по id счета
   // true если успех, false если ошибка
   bool    Add_DEPOSIT_From_BILL_ID(string BILL_ID,int sum);
   // создание документов по оформленному платежу
   // true если успех, false если ошибка
   bool    Insert_Payment_Invoice_From_Login(string login2find,int sum);
   // осуществление все процедуры платежа по одному абоненту
   // true если успех, false если ошибка
   bool    Execute_Payment(string login2pay,int sum);
   // получение списка абонентов (id абонента , фио, сумма на счету)    
   sql::ResultSet *Get_USER_ID_DEPOSIT_List();
   // чтение скачаных платежных сообщений и перемещение в bak папку
   bool Read_Execute_Payment_Registry(string file_list_name);
   // записи информации о проведенных 210 сообщениях
   bool Write_Payment_Doc_Result(string sender_code, string message_number, string message_date, string row_count,bool err);

  
protected:
         // хост , логин и пароль.
         sql::SQLString host,login,password;
         // драйвер mysql
         sql::Driver *driver;
         // соединение
         sql::Connection *con;
         // выражение 
         sql::Statement *stmt;
         // результирующий массив
         sql::ResultSet *res;
         // рабочая директория.
         string pref; //working directory;
};



#endif /* ABILLS_MYSQL_CLASS_H */

