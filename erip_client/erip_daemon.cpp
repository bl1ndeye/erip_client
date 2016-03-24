/*
/* 
 * File:   erip_daemon.cpp
 * Author: winter
 *
 * Created on 23 ноября 2015 г., 20:11
 */



#include "erip_daemon.h"




void erip_daemon::setpidfile()
{
    // создание пид файла
    FILE* f;
    f = fopen("/var/run/erip_client", "w+");
    if (f)
    {
        fprintf(f, "%u", getpid());
        fclose(f);
    }
}
erip_daemon::erip_daemon()
{
    
    // конструктор по умолчанию
  erip_ftp ftp_lsv();
  abills_mysql abills_lsv();
 

}

bool erip_daemon::reconfigure() {

    // создание xml документа
   pugi::xml_node conf= config.append_child("erip_client_config");
   conf.append_attribute("ftp_ip");
   conf.append_attribute("ftp_login");
   conf.append_attribute("ftp_password");
  
   conf.append_attribute("abills_db_ip");
   conf.append_attribute("abills_db_login");
   conf.append_attribute("abills_db_password");
 
   
   // ввод настроек авторизации на фтп и на mysql сервере
   // где хранится база данных биллинга
   cout<<"введите ip ftp-сервера"<<endl; 
   string ftp_ip;
   cin>>ftp_ip;
   

   cout<<"введите логин ftp-сервера"<<endl; 
   string ftp_login;
   cin>>ftp_login;
   
   cout<<"введите пароль ftp-сервера"<<endl; 
   string ftp_password;
   cin>>ftp_password;
   
   cout<<"введите ip mysql-сервера"<<endl; 
   string abills_db_ip;
   cin>>abills_db_ip;
   
   cout<<"введите логин mysql-сервера"<<endl; 
   string abills_db_login;
   cin>>abills_db_login;

   cout<<"введите пароль mysql-сервера"<<endl; 
   string abills_db_password;
   cin>>abills_db_password;   

   
   
   
   
   conf.attribute("ftp_ip").set_value(ftp_ip.c_str());
   conf.attribute("ftp_login").set_value(ftp_login.c_str());
   // при вводе пароля на фтп , можно ввести 123 , чтобы 
   // сохранился пустой пароль.
   if (ftp_password!="123")
   {
    conf.attribute("ftp_password").set_value(ftp_password.c_str());
   }
   conf.attribute("abills_db_ip").set_value(abills_db_ip.c_str());
   conf.attribute("abills_db_login").set_value(abills_db_login.c_str());
   conf.attribute("abills_db_password").set_value(abills_db_password.c_str());
   // сохранение файла конфигурации
   return  config.save_file("erip_config.xml");
   
   
}


bool erip_daemon::read_config() {
  pugi::xml_parse_result res = config.load_file("erip_config.xml");
 
  if (res)
  {
      // чтение конфигурации
      pugi::xml_node conf= config.first_child();
      string pass_ftp=conf.attribute("ftp_password").as_string();
      
      string abills_db_password=conf.attribute("abills_db_password").as_string();
      
      ftp_lsv.set_ftp_config(conf.attribute("ftp_ip").as_string(),conf.attribute("ftp_login").as_string(),pass_ftp);
      abills_lsv.set_mysql_config(conf.attribute("abills_db_ip").as_string(),conf.attribute("abills_db_login").as_string(),abills_db_password);
      return true;
  } 
    else
        {
          //  счетчик для исключения бесконечной
         // рекурсии
        rec_count++;
        reconfigure();
        if (rec_count<=2)
        read_config();  
        else
        {
            cout<<"ошибка записи новой конфигурации, файл erip_config.xml"<<endl;
            cout<<"не может быть создан или прочитан, проверьте доступ и права рабочей директории"<<endl;
        }
            
        }
  
}



void erip_daemon::daemonize() {
    pid_t pid;
    pid_t sid;
    // форк приложения
    pid = fork();
    if (pid<0)
    {
        // при ошибке форка закрыть приложение
        exit(EXIT_FAILURE);
    } else
        if (pid>0)
        {
            // получен новый pid, следовательно приложение уже запущено
            // закрывается, что бы не было два экзэмпляра
            exit(EXIT_SUCCESS);
        }
            
            sid=setsid();
            if (sid < 0) { exit(EXIT_FAILURE); }
            // закрытие потока вывода
            close(1); 
            // изменение прав доступа к файлам
            umask(0);
            // открытие ведения лога
            openlog("erip_daemon", 0, LOG_LOCAL0);
            syslog(LOG_INFO,"демон запущен");
            setpidfile();
            closelog();
            abills_lsv.Set_Pref(pref);
            // запуск бесконечного цикла.
            // в цикле выполняются все функции автоматизации
            while (true)
                {
                openlog("erip_daemon", 0, LOG_LOCAL0);
                if (ftp_lsv.establish_connection())
                     syslog(LOG_INFO,"соединение с фтп прошло");
                else
                    syslog(LOG_INFO,"ошибка  соединения с фтп");
                
                if (abills_lsv.Connect()==0)
                     syslog(LOG_INFO,"соединение с mysql прошло");
                                else
                    syslog(LOG_INFO,"ошибка  соединения с mysql");
                time_t now;
                struct tm *now_tm;
                int hour;

                now = time(NULL);
                now_tm = localtime(&now);
                hour = now_tm->tm_hour;             
                int min =now_tm->tm_min;
                // проверка времени, для формирования 
                // платежного требования
                // сообщение 202 по стандарту ЕРИП
                // фомируется и сбрасывается в интервале от 01:00:00 и 01:14:59
                if (hour==1 && min<15 )
                {
                   if ( ftp_lsv.create_payment_request(abills_lsv.Get_USER_ID_DEPOSIT_List()))
                   {
                      syslog(LOG_INFO,"создано платежное требование");
                      if ( ftp_lsv.put_payment_request())
                      {
                        syslog(LOG_INFO," платежное требование сброшено на сервер");
                        
                      } else
                      {
                          ftp_lsv.pr_w84upload=true;
                          syslog(LOG_INFO," ошибка закачки платежного требования на сервер");
                      };
                   }  else
                   {
                       syslog(LOG_INFO,"ошибка создания платежного требования");
                   }
                    
                }
                // если по какой-то причине файл платежного требования был создан
                // но не был закачан в итерацию создания файла
                // то он будет загружен 
                if (ftp_lsv.pr_w84upload)
                {
                      if ( ftp_lsv.put_payment_request())
                      {
                          syslog(LOG_INFO," платежное требование сброшено на сервер");
                      } else
                      {
                          syslog(LOG_INFO," ошибка закачки платежного требования на сервер");
                      };
                }
                // получение сообщений о платежах, а также рузультатов обработки сообщений 202
                if ( ftp_lsv.get_payment_regs())
                   {
                       syslog(LOG_INFO,"были получены сообщения о платежах");
                       if (abills_lsv.Read_Execute_Payment_Registry("payment_regs"))
                          syslog(LOG_INFO,"Платежные документы обработы биллингом"); ;
                   }
                
                ftp_lsv.close_connection();
                abills_lsv.Disconnect();
                
                sleep(900);
                closelog();
                }
            

   
}


int erip_daemon::run(string pre)
{
    // инициализации объектов фтп и mysql
    // запуск демонизации 
    pref=pre;    
    ftp_lsv.Set_Pref(pref);
    daemonize();
    return 0;
}