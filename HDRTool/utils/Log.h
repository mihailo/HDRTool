/*
* Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

#ifndef LOG_H
#define LOG_H

// *********************************************************************
// Generic utilities for NVIDIA GPU Computing SDK 
// *********************************************************************

// reminders for output window and build log
#ifdef _WIN32
    #pragma message ("Note: including windows.h")
    #pragma message ("Note: including math.h")
    #pragma message ("Note: including assert.h")
#endif

// OS dependent includes
#ifdef _WIN32
    // Headers needed for Windows
    #include <windows.h>
#else
    // Headers needed for Linux
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
#endif

// Other headers needed for both Windows and Linux
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Defines and enum for use with logging functions
// *********************************************************************
#define DEFAULTLOGFILE "SdkConsoleLog.txt"
#define MASTERLOGFILE "SdkMasterLog.csv"
enum LOGMODES 
{
    LOGCONSOLE = 1, // bit to signal "log to console" 
    LOGFILE    = 2, // bit to signal "log to file" 
    LOGBOTH    = 3, // convenience union of first 2 bits to signal "log to both"
    APPENDMODE = 4, // bit to set "file append" mode instead of "replace mode" on open
    MASTER     = 8, // bit to signal master .csv log output
    ERRORMSG   = 16, // bit to signal "pre-pend Error" 
    CLOSELOG   = 32  // bit to close log file, if open, after any requested file write
};
#define HDASHLINE "-----------------------------------------------------------\n"






// *********************************************************************
// Helper function to log standardized information to Console, to File or to both
//! Examples: shrLogEx(LOGBOTH, 0, "Function A\n"); 
//!         : shrLogEx(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
//! 
//! Automatically opens file and stores handle if needed and not done yet
//! Closes file and nulls handle on request
//! 
//! @param 0 iLogMode: LOGCONSOLE, LOGFILE, LOGBOTH, APPENDMODE, MASTER, ERRORMSG, CLOSELOG.  
//!          LOGFILE and LOGBOTH may be | 'd  with APPENDMODE to select file append mode instead of overwrite mode 
//!          LOGFILE and LOGBOTH may be | 'd  with CLOSELOG to "write and close" 
//!          First 3 options may be | 'd  with MASTER to enable independent write to master data log file
//!          First 3 options may be | 'd  with ERRORMSG to start line with standard error message
//! @param 2 dValue:    
//!          Positive val = double value for time in secs to be formatted to 6 decimals. 
//!          Negative val is an error code and this give error preformatting.
//! @param 3 cFormatString: String with formatting specifiers like printf or fprintf.  
//!          ALL printf flags, width, precision and type specifiers are supported with this exception: 
//!              Wide char type specifiers intended for wprintf (%S and %C) are NOT supported
//!              Single byte char type specifiers (%s and %c) ARE supported 
//! @param 4... variable args: like printf or fprintf.  Must match format specifer type above.  
//! @return 0 if OK, negative value on error or if error occurs or was passed in. 
// *********************************************************************
extern "C" int logEx(int iLogMode, int iErrNum, const char* cFormatString, ...);

// Short version of shrLogEx defaulting to shrLogEx(LOGBOTH, 0, 
// *********************************************************************
extern "C" int logFile(const char* cFormatString, ...);

// Optional LogFileNameOverride function
// *********************************************************************
extern "C" void setLogFileName (const char* cOverRideName);





#endif
