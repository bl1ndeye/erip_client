# erip_client
Клиент для биллинга Abills.
Проект написан в NetBeans IDE.

Работает под линукс , используются платформозависимые функции, в частности iconv. <br>
Для функционирования нужны библиотеки boost, mysqlcppconn, а также общие библиотеки C++. <br>
Компиляция проводится стандартным GNU toolchain, в корне исходников выполнить <br> 
make -f Makefile CONF=Release
