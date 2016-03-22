/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "erip_daemon.h"

void erip_daemon::sighandler(int signum) {
    waitpid(0, 0, WNOHANG);
}


void erip_daemon::setpidfile()
{
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
  erip_ftp ftp_lsv();
  abills_mysql abills_lsv();
 

}

bool erip_daemon::reconfigure() {

    
   pugi::xml_node conf= config.append_child("erip_client_config");
   conf.append_attribute("ftp_ip");
   conf.append_attribute("ftp_login");
   conf.append_attribute("ftp_password");
  
   conf.append_attribute("abills_db_ip");
   conf.append_attribute("abills_db_login");
   conf.append_attribute("abills_db_password");
 
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

   
   unsigned  char *key_lv=(unsigned char *)"123123";
   
   // дохера всякой шифровки
   conf.attribute("ftp_ip").set_value(ftp_ip.c_str());
   conf.attribute("ftp_login").set_value(ftp_login.c_str());
   if (ftp_password!="123")
   {
    conf.attribute("ftp_password").set_value(ftp_password.c_str());
   }
   conf.attribute("abills_db_ip").set_value(abills_db_ip.c_str());
   conf.attribute("abills_db_login").set_value(abills_db_login.c_str());
   conf.attribute("abills_db_password").set_value(abills_db_password.c_str());

   return  config.save_file("erip_config.xml");
   
   
}


bool erip_daemon::read_config() {
  pugi::xml_parse_result res = config.load_file("erip_config.xml");
 
  if (res)
  {
      //  дохера всякой расшифровки
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
    pid = fork();

    if (pid<0)
    {
        exit(EXIT_FAILURE);
    } else
        if (pid>0)
        {
            exit(EXIT_SUCCESS);
        }
            
            sid=setsid();
            if (sid < 0) { exit(EXIT_FAILURE); }
            close(1); 
            umask(0);
            openlog("erip_daemon", 0, LOG_LOCAL0);
            syslog(LOG_INFO,"демон запущен");
            setpidfile();
            abills_lsv.Set_Pref(pref);
            while (true)
                {
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
        
                if ( ftp_lsv.get_payment_regs())
                   {
                       syslog(LOG_INFO,"были получены сообщения о платежах");
                       if (abills_lsv.Read_Execute_Payment_Registry("payment_regs"))
                          syslog(LOG_INFO,"Платежные документы обработы биллингом"); ;
                   }
                
                ftp_lsv.close_connection();
                abills_lsv.Disconnect();
                
                sleep(900);
                
                }
                
                
            
            
            
            //mainloop();
            closelog();


        //};
   
}


int erip_daemon::run(string pre)
{
    pref=pre;    
    ftp_lsv.Set_Pref(pref);
    daemonize();
    return 0;
}