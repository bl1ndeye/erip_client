#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/ERIP_ftp_class.o \
	${OBJECTDIR}/abills_mysql_class.o \
	${OBJECTDIR}/erip_daemon.o \
	${OBJECTDIR}/ftplib.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/pugixml.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/home/winter/work/libs/boost_1_59_0/bin.v2/libs/filesystem -lboost_filesystem -lboost_system -lmysqlcppconn

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk erip_client

erip_client: ${OBJECTFILES}
	${LINK.cc} -o erip_client ${OBJECTFILES} ${LDLIBSOPTIONS} -DNOSSL

${OBJECTDIR}/ERIP_ftp_class.o: ERIP_ftp_class.c++ 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ERIP_ftp_class.o ERIP_ftp_class.c++

${OBJECTDIR}/abills_mysql_class.o: abills_mysql_class.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/abills_mysql_class.o abills_mysql_class.cpp

${OBJECTDIR}/erip_daemon.o: erip_daemon.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/erip_daemon.o erip_daemon.cpp

${OBJECTDIR}/ftplib.o: ftplib.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ftplib.o ftplib.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/pugixml.o: pugixml.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I/usr/include/cppconn -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pugixml.o pugixml.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} erip_client

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
