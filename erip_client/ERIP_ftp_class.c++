/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "ERIP_ftp_class.h"

erip_ftp::erip_ftp()
{
    
    ftp= new ftplib();
    host="anyhost";
    login="moria";
    password="sayfriendandlogin";
    pr_w84upload=false;
    
}

erip_ftp::erip_ftp(string con_host, string con_login, string con_password) 
{
    ftp= new ftplib();
    host=con_host;
    login=con_login;
    password=con_password;
    pr_w84upload=false;
}

erip_ftp::~erip_ftp() {
ftp->Quit();
delete ftp;
}

void erip_ftp::Set_Pref(string newpref)
{
    pref=newpref;
}


bool erip_ftp::delete_file(string file_name)
{
   file_name="/"+file_name;
   return ftp->Delete(file_name.c_str()); 
}



bool erip_ftp::establish_connection()
{
    bool rez;
        ftp= new ftplib();
	ftp->Connect(host.c_str());
	rez=ftp->Login(login.c_str(), password.c_str());
	//ftp->Dir(NULL, "/");
	return rez;
}

bool erip_ftp::close_connection()
{
    bool rez;
    rez=ftp->Quit();
    delete ftp;
    return rez;
}


bool erip_ftp::cdir(string cd)
{
    bool rez;
    cd="/"+cd;
    const char *cdr=cd.c_str();
    rez= ftp->Chdir(cdr);
    return rez;
     
}
bool erip_ftp::get_file(string file_path)
{
    bool rez;
    file_path="/"+file_path;
    path local_file(file_path);
    string file_name;
    file_name=pref+"/downloads/"+local_file.filename().generic_string();
    const char *cdr=file_path.c_str();
    rez= ftp->Get(file_name.c_str(),cdr,ftplib::ascii);
    return rez;
}
bool erip_ftp::get_out_files(string file_name)
{
    bool rez;
    path local_file(file_name);
    
    string file;
    
    file=pref+"out/"+local_file.filename().generic_string();
    const char *cdr=file_name.c_str();
    rez= ftp->Get(file.c_str(),cdr,ftplib::ascii);
        openlog("erip_ftp",0,LOG_LOCAL1);
    if (rez)
    {
        string text;
        text+="файл "; text+=+cdr;text+= " скачан с фтп в "; text+=file;
        syslog(LOG_INFO,text.c_str());
    }
    else
    {
        string text;
        text+="ошибка скачивания файла "; text+=cdr;
        syslog(LOG_INFO,text.c_str());
    }
    closelog();
    
    
    return rez; 
}



bool erip_ftp::put_file(string full_file_name,string file_path)
{
    bool rez;
    file_path="/"+file_path;
    const char *cdr=full_file_name.c_str();
    const char *cdr1=file_path.c_str();
    rez= ftp->Put(cdr,cdr1,ftplib::ascii);
    
    openlog("erip_ftp",0,LOG_LOCAL1);
    if (rez)
    {
        string text;
        text+="файл "; text+=+cdr;text+= " записан на фтп в "; text+=cdr1;
        syslog(LOG_INFO,text.c_str());
    }
    else
    {
        string text;
        text+="ошибка записи файла "; text+=cdr;
        syslog(LOG_INFO,text.c_str());
    }
    closelog();
    return rez;
}


bool erip_ftp::put_file_relate(string file_name, string file_path) {
    bool rez;
    file_name=pref+"/"+file_name;
    file_path="/"+file_path;
    cout<<file_name<<endl;
    const char *cdr=file_name.c_str();
    const char *cdr1=file_path.c_str();
    rez= ftp->Put(cdr,cdr1,ftplib::ascii);
    return rez;
}


bool erip_ftp::create_payment_request(sql::ResultSet *abons)
{
    bool rez;
    // создание заголовка сообщения
    string pr_caption; 
    int pr_message_version=4;
    string pr_num;
    string pr_file_name;
    string pr_sender_code="32201898";
    string pr_filename;
    string pr_datetime;
    string pr_rowcount;
    string pr_UNP;
    string pr_bank_code="782";
    string pr_checking_account="3012112780000";
    // ^ пустое поле
    string pr_currency="974";
    //  ^ пустое поле
    char * buf=new char[40];
    time_t sec=time(NULL);
    tm * timeinfo = localtime(& sec);
//    tmp="%Y%m%d";
    rez=true;
     // const char * format=tmp.c_str(); 
    const char * format="%Y%m%d"; 
    strftime(buf,40,format,timeinfo);
    pr_file_name="in/";
    pr_file_name+=buf;
    pr_filename+=buf;
    pr_file_name+=".202";  
    format="%Y%m%d%H%M%S";
    strftime(buf,40,format,timeinfo);
    pr_datetime=buf;
    pr_caption=boost::lexical_cast<std::string>(pr_message_version);
    pr_caption+="^"+pr_sender_code+"^"+pr_filename+"^"+pr_datetime;
    
    pr_rowcount=boost::lexical_cast<std::string>(abons->rowsCount());
    pr_UNP= "291351891";
    pr_caption+="^"+pr_rowcount+"^"+pr_UNP+"^"+pr_bank_code+"^"+pr_checking_account+"^1^"+pr_currency;
    
//    cout<<pr_caption<<endl;
    
    
    
    ofstream abon_file(pr_file_name.c_str());
    abon_file<<pr_caption<<endl;
    // создание списка абонентов
    string pr_abon;

   
    int n=0;
    while (abons->next())
    {
        n++;
        
        pr_abon=boost::lexical_cast<std::string>(n);
        pr_abon+="^";
        pr_abon+=abons->getString(1);
        pr_abon+="^";
        pr_abon+=abons->getString(2);
        pr_abon+="^^^";

        
        // инверсия +/-
       // pr_num=boost::lexical_cast<std::string> (-1*abons->getInt(3));
        pr_abon+= boost::lexical_cast<std::string> (-1*abons->getInt(3));
        pr_abon+="^^"+pr_datetime+"^^^^^^^";
     
        abon_file<<pr_abon<<endl;
      
    }
    abon_file.flush();
    abon_file.close();
    pr_file_name="iconv -f utf8 -t cp1251 "+pr_file_name+" -o "+pr_file_name;
    system(pr_file_name.c_str());
    delete abons;
    return rez;
    
}

bool erip_ftp::put_payment_request()
{
    path in_dir(pref+"/in/");
    path bak_file;
    directory_iterator end_iter;
    bool rez=false;
    
    //структура данных о файле 
//    typedef multimap<time_t, path> result_set_t;
//    result_set_t result_set;
   
    
    
    if ( exists(in_dir) && is_directory(in_dir))
    {
        //cout<<"YEP MAZAFAKA we made it"<<endl;
        for( directory_iterator dir_iter(in_dir) ; dir_iter != end_iter ; ++dir_iter)
            {
                if (is_regular_file(dir_iter->status()) )
                {
                    if (dir_iter->path().extension().string()==".202")
                       
                    {
                       if (put_file(dir_iter->path().string(),"in/"+dir_iter->path().filename().string()))
                       {
                          pr_w84upload=false;
                          bak_file= dir_iter->path().parent_path();
                          bak_file +="/bak/";
                          bak_file+=dir_iter->path().filename();
                          rez=true;
                          
                          
                          int source = open(dir_iter->path().string().c_str(), O_RDONLY, 0);
                          int dest = open(bak_file.string().c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
                          struct stat stat_source;
                          fstat(source, &stat_source);
                          sendfile(dest, source, 0, stat_source.st_size);

                          close(source);
                          close(dest);
                          
                          // проверка размера файла (или произошло копирование))
                          if (exists(bak_file))
                          {
                              remove(dir_iter->path());
                          }
                          else
                          {
                               openlog("erip_ftp",0,LOG_LOCAL1);
                               string text;
                               text+="Файл ";text+=bak_file.string();
                               text+=" не был скопирован.";
                               syslog(LOG_INFO,text.c_str());
                               closelog();
                              // добавить обработку ошибки
                          }             
                       }
                       else
                       {
                           // обработка ошибки в функции put_file
                       }
                       
                       
                       
                    }                                       
                }
            }
    }
    else
    {
        openlog("erip_ftp",0,LOG_LOCAL1);
        string text;
        text+="Файл папки in не найден, или "; 
        text+=" данный файл не является папкой";
        text+=" создайте папку in с вложенными папка bak и err в рабочей директории.";        
        syslog(LOG_INFO,text.c_str());
        closelog();
    };
    return rez;
   
  }


bool erip_ftp::get_payment_regs() 
{
    bool rez=false;
    string tmp;
    tmp+=pref+"payment_regs";
    path pr(tmp);
    if (exists(pr))
        remove(pr);
    

    ftp->Nlst("payment_regs","/out");
    ifstream file;
    file.open("payment_regs");
    string file_payment_reg_name; 
    
    if (file.is_open())
    {
        openlog("erip_ftp",0,LOG_LOCAL1);
        string text;
        text+="Список файлов сервера получен";     
        syslog(LOG_INFO,text.c_str());
        closelog();
    }
    
    int local_size;
    while(!file.eof())
    {
        file>>file_payment_reg_name;
     
        if (file_payment_reg_name.length()>3)
        {
            
            if (file_payment_reg_name.find("/out/",0)==-1)
            {
                tmp="/out/";
                tmp+=file_payment_reg_name;
            
            }
            else
            {
               tmp=file_payment_reg_name;
            }
            path t (tmp);
            if ((t.extension().string()==".206") || (t.extension().string()==".210")|| (t.extension().string()==".204"))
            {
                rez= get_out_files(tmp.c_str());
                if (file_payment_reg_name.find("/out/",0)==-1)
                    {
                        tmp=pref;
                        tmp+="out/";
                        tmp+=file_payment_reg_name;

                    }
                    else
                    {
                       tmp=pref;
                       tmp+=file_payment_reg_name;
                    }
                path local(tmp);
                local_size=file_size(local);
                if (rez)
                {
                    delete_file(t.string());
                }
               
            }
            file_payment_reg_name="";
        }
    }
    return rez;
    
}


