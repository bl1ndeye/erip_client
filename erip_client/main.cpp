/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: winter
 *
 * Created on 12 ноября 2015 г., 14:14
 */

#include <cstdlib>
#include <iostream>
#include <stdlib.h>

#include <unistd.h>


// написанные библиотеки
#include "abills_mysql_class.h"
#include "ERIP_ftp_class.h"
#include "erip_daemon.h"

using namespace std;
using namespace boost::filesystem;
int main(int argc, char** argv) {
    
     erip_daemon erip;
     
     
     
   
     if(argc>1)
     {
         if (string(argv[1])=="reconfig")
         {
             erip.reconfigure();
             return 0;
         }
        
         path pre (argv[1]);
        
         if ( exists(pre))
         {
          chdir(pre.string().c_str());
          erip.read_config(); 
          return erip.run(pre.string());
         }
     }
     
}

