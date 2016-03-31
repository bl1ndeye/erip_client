


#include <sys/socket.h>

#include "abills_mysql_class.h"

using namespace std;




abills_mysql::abills_mysql()
{

    host="tcp://localhost:3306";

    login="root";
    password="iamroot";
}

abills_mysql::abills_mysql( sql::SQLString sethost,  sql::SQLString setlogin, sql::SQLString setpassword)
{
    host="tcp://"+sethost+":3306";
    login=setlogin;
    password=setpassword;
}
abills_mysql::~abills_mysql()
{
    // без инициализации res 
    // будет segmantation fail.
    if (!con->isClosed())
    {
        res = stmt->executeQuery("SELECT 1;");
        delete res;
        delete stmt;
        delete con;
    }
}


void abills_mysql::Set_Pref(string newpref)
{
    pref=newpref;
}

int abills_mysql::Connect()
{
    try
    {
        // получение драйвера
        driver = get_driver_instance();
        //коннект к базе с заданными параметрами
        con = driver->connect(host,login ,password );
        stmt = con->createStatement();
    }
    catch (sql::SQLException &e) 
    {
     
         Write_Log(e);
         return e.getErrorCode();
    }
    return 0;
  
}


int abills_mysql::Disconnect()
{
try
    {  
    // закрытие коннекта с базой
        con->close();
    }
    catch (sql::SQLException &e) 
    {
        Write_Log(e);
        return e.getErrorCode();
    }
}

void abills_mysql::Write_Log(sql::SQLException e)
{
    // запись ошибки в лог
    // по исключению создаваемому самой библиотекой mysql
   openlog("erip_mysql", 0 , LOG_LOCAL3);
   string err_text;
   err_text+="Error:";
   err_text+=e.what();
   err_text+="  MySQL error code:";
   err_text+=boost::lexical_cast<string>(e.getErrorCode());
   err_text+=" SQLState:";
   err_text+=e.getSQLState();
   syslog(LOG_INFO,err_text.c_str());
   closelog();
}

void abills_mysql::Write_Log_Text(string err_text)
{
    // запись строки в лог
    openlog("erip_mysql", 0 , LOG_LOCAL3);
    syslog(LOG_INFO,err_text.c_str());
    closelog();
}
string abills_mysql::Find_UID_From_Login(string login2find)
{
    // поиск uid абонента по его логину 
     string qtext ="SELECT * FROM abills.users WHERE id='"+login2find+"';";
     stmt = con->createStatement();
    try
    {
    res = stmt->executeQuery(qtext);
    res->last();
    }
    catch (sql::SQLException &e) 
    {
        Write_Log(e);
    }
    
    if (res->getRow()==0)
    {
         delete res;
         delete stmt;
        Write_Log_Text(qtext);
        qtext="Выбранный пользователь  ";
        qtext+=login2find;
        qtext+="   не найден.";
        Write_Log_Text(qtext);
        return "0";
    }
    else
    {
        string s=res->getString("uid");
        delete res;
        delete stmt;
     return s;
    };
}

string abills_mysql::Find_BILL_ID_From_Login(string login2find)
{
    
    // нахождение id счета по логину
    string qtext ="SELECT * FROM abills.users WHERE id='"+login2find+"';";
    stmt = con->createStatement();
    try
    {
    res = stmt->executeQuery(qtext);
    res->last();
    }
    catch (sql::SQLException &e) 
    {
        Write_Log(e);
    }

    if (res->getRow()==0)
    {
        delete res;
        delete stmt;
        Write_Log_Text(qtext);
        qtext="Выбранный пользователь  ";
        qtext+=login2find;
        qtext+="   не найден.";
        Write_Log_Text(qtext);
        return "0";
    }
    else
    {
        string s =res->getString("bill_id");
        delete res;
        delete stmt;
     return s;
    }
}

int abills_mysql::Get_DEPOSIT_From_Login(string login2find)
{
    double dep=0;
    // получение состояния депозита счета по логину
    string bill_id=Find_BILL_ID_From_Login(login2find);
    string qtext ="SELECT * FROM abills.bills WHERE id="+bill_id+";";
    stmt = con->createStatement();
      try
    {
        res = stmt->executeQuery(qtext);
        res->last();
    }
    catch (sql::SQLException &e) 
    {
       Write_Log(e);
    }
    if (res->getRow()==1)
    {
        dep=res->getDouble("deposit");
        delete res;
        delete stmt;
        return (int)dep;
    }
    else
    {
        delete res;
        delete stmt;
        Write_Log_Text(qtext);
        qtext="Выбранный пользователь  ";
        qtext+=login2find;
        qtext+="   не найден.";
        Write_Log_Text(qtext);
        return -1111111;  // значение если указанный логин не найден
    }
}


bool abills_mysql::Add_DEPOSIT_From_BILL_ID(string BILL_ID,int sum)
{
  // добавление суммы на депозит абонента по id счета
    char sumt[8];
    bool er=false;
    sprintf(sumt,"%d",sum);
    string st=sumt;
    string qtext ="UPDATE abills.bills SET deposit=deposit+"+st+" WHERE id="+BILL_ID+";";
    stmt = con->createStatement();
    try
    {
        res = stmt->executeQuery(qtext);
        res->last();
    }
    catch (sql::SQLException &e) 
    {
        Write_Log(e);   
    }
    delete stmt;
    return !er;
}




bool abills_mysql::Insert_Payment_Invoice_From_Login(string login2find, int sum)
{
    // method 2 = прочие
    // method 1 = банк
    // method 0 = наличные
    string date,st,ip,last_deposit,uid,aid,method,bill_id,amount,currency,reg_date;
    int pid,iid;
    pid=0;
    iid=0;
    char * buf=new char[40];
    time_t sec=time(NULL);
    tm * timeinfo = localtime(& sec);
    date="%Y-%m-%d %H:%M:%S";
    const char * format=date.c_str(); 
    strftime(buf,40,format,timeinfo);
    date=buf;
    char sumt[8];
    sprintf(sumt,"%d",sum);
    st=sumt;
    ip="0";
    sprintf(sumt,"%d",Get_DEPOSIT_From_Login(login2find)-sum);
    last_deposit=sumt;
    uid=Find_UID_From_Login( login2find);
    if (uid=="0")
        return false;
    // указывается admin id - это будет отображаться в оплатах
    // в поле администратор
    aid="3";
    // метод оплаты
    // отображается в поле Вид оплаты
    method="2";
    bill_id=Find_BILL_ID_From_Login(login2find);
    // сумма полаты
    amount=st;
    // код валюты
    currency="974";
    // регистрация оплаты
    reg_date=date;
    string qtext;
    // вставка записи о платеже
    string val="'"+date+"','"+st + "','" +ip + "','" +last_deposit + "','" +uid + "','" +aid + "','" +method + "','" +bill_id + "','" +amount + "','" +currency + "','" +reg_date+"'";
    qtext= "INSERT INTO abills.payments (`date`,`sum`,`ip`,`last_deposit`,`uid`,`aid`,`method`,`bill_id`,`amount`,`currency`,`reg_date`) VALUES ("+val+");";
    stmt = con->createStatement();
    try
    {
    res = stmt->executeQuery(qtext);
    }
    catch (sql::SQLException &e) 
    {
        if (e.getErrorCode()!=0)
        {
            Write_Log(e);   
            Write_Log_Text(qtext);
            qtext="Выбранный платеж данному абоненту ";
            qtext+=login2find;
            qtext+="   не был создан.";
            Write_Log_Text(qtext);  
        };
    }
    res=stmt->executeQuery("SELECT LAST_INSERT_ID();");
    res->last();
    pid=res->getInt(1);
    sprintf(sumt,"%d",pid);
    delete res;
    string payment_id=sumt;
    // вставка квитанций, вещь опциональная
    // по факту для отображения оплаты не нужна.
    val="'"+date+"','"+aid +  "','" +uid + "','" + date + "','" +"0.00" + "','" +payment_id + "','" +amount +  "','" +currency + "'";
    qtext= "INSERT INTO abills.docs_receipts (`date`,`aid`,`uid`,`created`,`vat`,`payment_id`,`deposit`,`currency`) VALUES ("+val+");";
    try {
          res = stmt->executeQuery(qtext);
        }    
    catch (sql::SQLException &e) 
      {
         if (e.getErrorCode()!=0)
         {
           Write_Log(e); 
           Write_Log_Text(qtext);
           qtext="Выбранная квитанция данному абоненту ";
           qtext+=login2find;
           qtext+="   не была создана.";
           Write_Log_Text(qtext);  
         }

      }   
    // insert docs_invoice
    val="'"+date+"','" + date +"','"+aid +  "','" +uid +  "','" +"0.00" + "','" +payment_id + "','" +amount +  "','" +currency + "'";
    qtext= "INSERT INTO abills.docs_invoices (`date`,`created`,`aid`,`uid`,`vat`,`payment_id`,`deposit`,`currency`) VALUES ("+val+");";
    try 
    {
        res = stmt->executeQuery(qtext);
    }   
    catch (sql::SQLException &e) 
    {
         if (e.getErrorCode()!=0)
         {
           Write_Log(e); 
           Write_Log_Text(qtext);
           qtext="Выбранная счет фактура данному абоненту ";
           qtext+=login2find;
           qtext+="   не была создана.";
           Write_Log_Text(qtext);  
         }
    }          
    res=stmt->executeQuery("SELECT LAST_INSERT_ID();");
    res->last();
    iid=res->getInt(1);
    sprintf(sumt,"%d",iid);
    string invoice_id=sumt;
    // insert docs_invoice2payments
    // вставка строки для связи между инвойсом и платежом
    delete res;
    val="'"+payment_id+"','" + invoice_id +"','" + amount +"'";
    qtext= "INSERT INTO abills.docs_invoice2payments (`invoice_id`,`payment_id`,`sum`) VALUES ("+val+");";
    try 
    {
       res = stmt->executeQuery(qtext);
    }    
    catch (sql::SQLException &e) 
    {
        if (e.getErrorCode()!=0)
        {
            Write_Log(e);
            Write_Log_Text(qtext);
            qtext="Связь между счет фактурой и платежкой данному абоненту  ";
            qtext+=login2find;
            qtext+="   не был создан.";
            Write_Log_Text(qtext);  
        }
    }   
    delete stmt;
    if ((pid>=1)&&(iid>=1))
        return true;
    else
     return false;
}

bool abills_mysql::Execute_Payment(string login2pay,int sum)
{
    bool er,er1,er2;
    // проверка если логина нет в системе то платеж не начинает
    // обрабатываться
    if (Find_BILL_ID_From_Login(login2pay)=="0")
    {
        return false;
    } 
    else
    er1=Add_DEPOSIT_From_BILL_ID(Find_BILL_ID_From_Login(login2pay),sum);
    er2=Insert_Payment_Invoice_From_Login(login2pay,sum);
    // проверка обоих этапов проведения платежа
    // пополнение непосредственно счета абонента
    // и запись о платеже в таблицах платежей, квитанций, инвойсов.
    er=er1&&er2;
    return er;
}

sql::ResultSet *abills_mysql::Get_USER_ID_DEPOSIT_List()
{
    // функция формирования списка абонентов и состояний их счетов.
    // потом по этому списку формируется 202 сообщение.
    sql::ResultSet *user_list;
    string qtext ="SELECT abills.users.id,abills.users_pi.fio,abills.bills.deposit FROM abills.users INNER JOIN abills.bills ON abills.users.bill_id=abills.bills.id INNER JOIN abills.users_pi ON abills.users.uid=abills.users_pi.uid WHERE abills.users.deleted=0;";
    stmt = con->createStatement();
    try
    {
        user_list = stmt->executeQuery(qtext);
    }
    catch (sql::SQLException &e) 
    {
        Write_Log(e);
        Write_Log_Text(qtext);
        qtext="Список пользователей не был сформирован. ";
        Write_Log_Text(qtext);  
    }
    delete stmt;
    return user_list;
}

bool abills_mysql::Read_Execute_Payment_Registry(string file_list_name)
{
    // обработка сообщений о платежах
   bool rez=false;
   ifstream file,payment_registry;
   boost::filesystem::path bak_file;
   file.open(file_list_name.c_str());
   while(!file.eof())
   {
        string file_payment_reg_name;     
        string prefi=pref;
        file>>file_payment_reg_name; 
        if (file_payment_reg_name.size()>0)
        {   
           // проверка на наличие префикса.
           // с одних фтп читает без префикса,
           // с других с префиксом
            if (file_payment_reg_name.find("/out/",0)==-1)
            {
                prefi+="out/";
                file_payment_reg_name=prefi+file_payment_reg_name;
            }
            else
            {
                file_payment_reg_name=prefi+file_payment_reg_name;
            }      
            string str;                
            boost::filesystem::path t(file_payment_reg_name);
      
            // обработка для 206 сообщений
            if (t.extension().string()==".206")
            {
            payment_registry.open(file_payment_reg_name.c_str());
            if (payment_registry.is_open())
            {
                 // проверка заголовка файла
                 getline(payment_registry,str);
                 const string delimeters1 = " ";
                     for (string::iterator iter = str.begin(); iter != str.end(); ++iter)
                     {
                        if (delimeters1.find(*iter) != std::string::npos)
                        {
                           *iter = '\0';
                        }
                     }
                 const string delimeters = "^";
                     for (string::iterator iter = str.begin(); iter != str.end(); ++iter)
                     {
                        if (delimeters.find(*iter) != std::string::npos)
                        {
                           *iter = '\n';
                        }
                     }
                        istringstream istr(str);
                        string sender_code;
                        string message_number;
                        string message_date;
                        string row_count;
                        bool err=false;
                        string tmp1;
                        getline(istr,tmp1);
                        getline(istr,sender_code);
                        getline(istr,message_number);
                        getline(istr,message_date);
                        getline(istr,row_count);
                        // обработка записей сообщения
                while (!payment_registry.eof())
                {
                    getline(payment_registry,str);

                    const string delimeters1 = " ";
                     for (string::iterator iter = str.begin(); iter != str.end(); ++iter)
                     {
                        if (delimeters1.find(*iter) != std::string::npos)
                        {
                           *iter = '\0';
                        }
                     }
                     const string delimeters = "^";
                     for (string::iterator iter = str.begin(); iter != str.end(); ++iter)
                     {
                        if (delimeters.find(*iter) != std::string::npos)
                        {
                           *iter = '\n';
                        }
                     }
                     if (str.size()>0)
                     {
                        istringstream ist(str);
                        string login;
                        string sum;
                        string num;
                        string tmp1;
                        // пропуск ненужной информации
                        getline(ist,num);
                        getline(ist,tmp1);
                        getline(ist,login);
                        getline(ist,tmp1);
                        getline(ist,tmp1);
                        getline(ist,tmp1);
                        getline(ist,sum);
                        double sumi;
                        sumi=boost::lexical_cast<double>(sum);
                        if (! Execute_Payment(login,round(sumi)))
                           err=true;
                     }     
                     rez=true;
                  }
                 // запись в базу о состоянии проведенной обработки платежного сообщения
                 Write_Payment_Doc_Result(sender_code,message_number,message_date,row_count,err);  
            }   
            payment_registry.close();
            bak_file= t.parent_path();
            // копирование в bak после обработки сообщений.
            bak_file +="/bak/";
            bak_file+=t.filename();      
            int source = open(t.string().c_str(), O_RDONLY, 0);
            int dest = open(bak_file.string().c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
            struct stat stat_source;
            fstat(source, &stat_source);
            sendfile(dest, source, 0, stat_source.st_size);
            close(source);
            close(dest);
            // проверка существования файла (или произошло копирование))
            if (boost::filesystem::exists(bak_file))
               {
                boost::filesystem::remove(t);
               }
            }
            else 
            {
                if (t.extension().string()==".210")
                {
                    // запись что было получено 210 сообщение.
                    string qtext;
                    qtext="Получено сообщение о переводе средств, имя: ";
                    qtext+=t.filename().string();
                    Write_Log_Text(qtext);
                    bak_file= t.parent_path();
                    bak_file +="/bak/";
                    bak_file+=t.filename();      
                    int source = open(t.string().c_str(), O_RDONLY, 0);
                    int dest = open(bak_file.string().c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
                    struct stat stat_source;
                    fstat(source, &stat_source);
                    sendfile(dest, source, 0, stat_source.st_size);
                    close(source);
                    close(dest);
                    // проверка существования файла (или произошло копирование))
                    if (boost::filesystem::exists(bak_file))
                       {
                        qtext=bak_file.string();
                        boost::filesystem::remove(t);
                        // конвертация в utf8 чтобы можно было просматривать под linux
                        qtext="iconv -f cp1251 -t utf8 "+qtext+" -o "+qtext;
                        system(qtext.c_str());
                       }
                 }
                else
                    if (t.extension().string()==".204")
                    {
                    string qtext;
                    qtext="Получено сообщение о результатах обработки сообщений, имя: ";
                    qtext+=t.filename().string();
                    Write_Log_Text(qtext);
                    bak_file= t.parent_path();
                    bak_file +="/bak/";
                    bak_file+=t.filename();      
                    int source = open(t.string().c_str(), O_RDONLY, 0);
                    int dest = open(bak_file.string().c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);
                    struct stat stat_source;
                    fstat(source, &stat_source);
                    sendfile(dest, source, 0, stat_source.st_size);
                    close(source);
                    close(dest);
                    // проверка существования файла (или произошло копирование))
                    if (boost::filesystem::exists(bak_file))
                       {
                        qtext=bak_file.string();
                        boost::filesystem::remove(t);
                        qtext="iconv -f cp1251 -t utf8 "+qtext+" -o "+qtext;
                        system(qtext.c_str());
                       }
                    }
                   
            }
        }
   }   
   return rez;
}

bool abills_mysql::Write_Payment_Doc_Result(string sender_code, string message_number, string message_date, string row_count,bool err) 
{
 // вставка разделителей в дату, там она формируется без них
 message_date.insert(4,"-");
 message_date.insert(7,"-");
 message_date.insert(10," ");
 message_date.insert(13,":");
 message_date.insert(16,":");
 string qtext;    
 string serr;
  // запись в базу корректно 
 if (err) serr="1";
 else
     serr="0";
 string val="'"+sender_code+"','"+message_number +  "','"+message_date + "','" + row_count + "','" +serr + "'";
 qtext= "INSERT INTO abills.erip_206_messages (`sender_code`,`message_number`,`message_date`,`row_count`,`errors`) VALUES ("+val+");";
 stmt = con->createStatement();
    try
    {
    res = stmt->executeQuery(qtext);
    }
    catch (sql::SQLException &e) 
    {
        if (e.getErrorCode()!=0)
        {
            Write_Log(e);   
            Write_Log_Text(qtext);
            qtext="Выбранная обработка сообщения №  ";
            qtext+=message_number;
            qtext+="   не была проведена.";
            Write_Log_Text(qtext);  
            return false;
        };

    }
 return true;
}


void abills_mysql::set_mysql_config(sql::SQLString sethost, sql::SQLString setlogin, sql::SQLString setpassword) {
    // задание параметров mysql для авторизации
    host = "tcp://" + sethost + ":3306";
    login = setlogin;
    password = setpassword;
}
     
