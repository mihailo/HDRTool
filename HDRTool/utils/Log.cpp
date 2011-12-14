#include "Log.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;





// Optional LogFileName Override function
// *********************************************************************
char* cLogFilePathAndName = NULL;
void setLogFileName (const char* cOverRideName)
{
    if( cLogFilePathAndName != NULL ) {
        free(cLogFilePathAndName);
    }
    cLogFilePathAndName = (char*) malloc(strlen(cOverRideName) + 1);
    #ifdef WIN32
        strcpy_s(cLogFilePathAndName, strlen(cOverRideName) + 1, cOverRideName);
    #else
        strcpy(cLogFilePathAndName, cOverRideName);
    #endif
    return;
}

// Function to log standardized information to console, file or both
// *********************************************************************
static int logV(int iLogMode, int iErrNum, const char* cFormatString, va_list vaArgList)
{
    static FILE* pFileStream0 = NULL;
    static FILE* pFileStream1 = NULL;
    size_t szNumWritten = 0;
    char cFileMode [3];

    // if the sample log file is closed and the call includes a "write-to-file", open file for writing
    if ((pFileStream0 == NULL) && (iLogMode & LOGFILE))
    {
        // if the default filename has not been overriden, set to default
        if (cLogFilePathAndName == NULL)
        {
            setLogFileName(DEFAULTLOGFILE); 
        }

        #ifdef _WIN32   // Windows version
            // set the file mode
            if (iLogMode & APPENDMODE)  // append to prexisting file contents
            {
                sprintf_s (cFileMode, 3, "a+");  
            }
            else                        // replace prexisting file contents
            {
                sprintf_s (cFileMode, 3, "w"); 
            }

            // open the individual sample log file in the requested mode
            errno_t err = fopen_s(&pFileStream0, cLogFilePathAndName, cFileMode);
            
            // if error on attempt to open, be sure the file is null or close it, then return negative error code            
            if (err != 0)
            {
                if (pFileStream0)
                {
                    fclose (pFileStream0);
                }
				iLogMode = LOGCONSOLE; // if we can't open a file, we will still output to the console window
            }
        #else           // Linux & Mac version
            // set the file mode
            if (iLogMode & APPENDMODE)  // append to prexisting file contents
            {
                sprintf (cFileMode, "a+");  
            }
            else                        // replace prexisting file contents
            {
                sprintf (cFileMode, "w"); 
            }

            // open the file in the requested mode
            if ((pFileStream0 = fopen(cLogFilePathAndName, cFileMode)) == 0)
            {
                // if error on attempt to open, be sure the file is null or close it, then return negative error code
                if (pFileStream0)
                {
                    fclose (pFileStream0);
                }
				iLogMode = LOGCONSOLE; // if we can't open a file, we will still output to the console window
            }
        #endif
    }
    
    // if the master log file is closed and the call incudes a "write-to-file" and MASTER, open master logfile file for writing
    if ((pFileStream1 == NULL) && (iLogMode & LOGFILE) && (iLogMode & MASTER))
    {
        #ifdef _WIN32   // Windows version
            // open the master log file in append mode
            errno_t err = fopen_s(&pFileStream1, MASTERLOGFILE, "a+");

            // if error on attempt to open, be sure the file is null or close it, then return negative error code
            if (err != 0)
            {
                if (pFileStream1)
                {
                    fclose (pFileStream1);
					pFileStream1 = NULL;
                }
				iLogMode = LOGCONSOLE;  // Force to LOGCONSOLE only since the file stream is invalid
//				return -err;
            }
        #else           // Linux & Mac version

            // open the file in the requested mode
            if ((pFileStream1 = fopen(MASTERLOGFILE, "a+")) == 0)
            {
                // if error on attempt to open, be sure the file is null or close it, then return negative error code
                if (pFileStream1)
                {
                    fclose (pFileStream1);
					pFileStream1 = NULL;
                }
				iLogMode = LOGCONSOLE;  // Force to LOGCONSOLE only since the file stream is invalid
//              return -1;
            }
        #endif
        
        // If master log file length has become excessive, empty/reopen
		if (iLogMode != LOGCONSOLE)
		{
			fseek(pFileStream1, 0L, SEEK_END);            
			if (ftell(pFileStream1) > 50000L)
			{
				fclose (pFileStream1);
			#ifdef _WIN32   // Windows version
				fopen_s(&pFileStream1, MASTERLOGFILE, "w");
			#else
				pFileStream1 = fopen(MASTERLOGFILE, "w");
			#endif
			}
		}
    }

    // Handle special Error Message code
    if (iLogMode & ERRORMSG)  
    {   
        // print string to console if flagged
        if (iLogMode & LOGCONSOLE) 
        {
            szNumWritten = printf ("\n !!! Error # %i at ", iErrNum);                           // console 
        }
        // print string to file if flagged
        if (iLogMode & LOGFILE) 
        {
            szNumWritten = fprintf (pFileStream0, "\n !!! Error # %i at ", iErrNum);            // sample log file
        }
    }

    // Vars used for variable argument processing
    const char*     pStr; 
    const char*     cArg;
    int             iArg;
    double          dArg;
    unsigned int    uiArg;
    std::string sFormatSpec;
    const std::string sFormatChars = " -+#0123456789.dioufnpcsXxEeGgAa";
    const std::string sTypeChars = "dioufnpcsXxEeGgAa";
    char cType = 'c';

    // Start at the head of the string and scan to the null at the end
    for (pStr = cFormatString; *pStr; ++pStr)
    {
        // Check if the current character is not a formatting specifier ('%') 
        if (*pStr != '%')
        {
            // character is not '%', so print it verbatim to console and/or files as flagged
            if (iLogMode & LOGCONSOLE) 
            {
                szNumWritten = putc(*pStr, stdout);                                             // console 
            }
            if (iLogMode & LOGFILE)    
            {
                szNumWritten  = putc(*pStr, pFileStream0);                                      // sample log file
                if (iLogMode & MASTER)                          
                {
                    szNumWritten = putc(*pStr, pFileStream1);                                   // master log file
                }
            }
        } 
        else 
        {
            // character is '%', so skip over it and read the full format specifier for the argument
            ++pStr;
            sFormatSpec = '%';

            // special handling for string of %%%%
            bool bRepeater = (*pStr == '%');
            if (bRepeater)
            {
                cType = '%';
            }

            // chars after the '%' are part of format if on list of constants... scan until that isn't true or NULL is found
            while (pStr && ((sFormatChars.find(*pStr) != string::npos) || bRepeater))    
            {
                sFormatSpec += *pStr;

                // If the char is a type specifier, trap it and stop scanning
                // (a type specifier char is always the last in the format except for string of %%%)
                if (sTypeChars.find(*pStr) != string::npos)    
                {
                    cType = *pStr;
                    break;                                      
                }

                // Special handling for string of %%%
                // If a string of %%% was started and then it ends, break (There won't be a typical type specifier)
                if (bRepeater && (*pStr != '%'))
                {
                    break;
                }

                pStr++;
            }

            // Now handle the arg according to type 
            switch (cType)
            {
                case '%':   // special handling for string of %%%%
                {
                    if (iLogMode & LOGCONSOLE) 
                    {
                        szNumWritten = printf(sFormatSpec.c_str());                             // console 
                    }
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str());             // sample log file
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str());         // master log file
                        }
                    }
                    continue;
                }
                case 'c':   // single byte char
                case 's':   // string of single byte chars
                {
                    // Set cArg as the next value in list and print to console and/or files if flagged
                    cArg = va_arg(vaArgList, char*);
                    if (iLogMode & LOGCONSOLE) 
                    {
                        szNumWritten = printf(sFormatSpec.c_str(), cArg);                       // console 
                    }
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), cArg);       // sample log file
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), cArg);   // master log file
                        }
                    }
                    continue;
                }
                case 'd':   // signed decimal integer 
                case 'i':   // signed decimal integer 
                {
                    // set iArg as the next value in list and print to console and/or files if flagged
                    iArg = va_arg(vaArgList, int);
                    if (iLogMode & LOGCONSOLE) 
                    {
                        szNumWritten = printf(sFormatSpec.c_str(), iArg);                       // console 
                    }
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), iArg);       // sample log file  
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), iArg);   // master log file
                        }
                    }
                    continue;
                }
                case 'u':   // unsigned decimal integer 
                case 'o':   // unsigned octal integer 
                case 'x':   // unsigned hexadecimal integer using "abcdef"
                case 'X':   // unsigned hexadecimal integer using "ABCDEF"
                {
                    // set uiArg as the next value in list and print to console and/or files if flagged
                    uiArg = va_arg(vaArgList, unsigned int);
                    if (iLogMode & LOGCONSOLE)                                                  
                    {
                        szNumWritten = printf(sFormatSpec.c_str(), uiArg);                      // console 
                    }
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), uiArg);      // sample log file
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), uiArg);  // master log file
                        }
                    }
                    continue;
                }
                case 'f':   // float/double
                case 'e':   // scientific double/float
                case 'E':   // scientific double/float
                case 'g':   // scientific double/float
                case 'G':   // scientific double/float
                case 'a':   // signed hexadecimal double precision float
                case 'A':   // signed hexadecimal double precision float
                {
                    // set dArg as the next value in list and print to console and/or files if flagged
                    dArg = va_arg(vaArgList, double);
                    if (iLogMode & LOGCONSOLE) 
                    {
                        szNumWritten = printf(sFormatSpec.c_str(), dArg);                       // console 
                    }
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), dArg);       // sample log file
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), dArg);   // master log file
                        }
                    }
                    continue;
                }
                default: 
                {
                    // print arg of unknown/unsupported type to console and/or file if flagged
                    if (iLogMode & LOGCONSOLE)                          // console 
                    {
                        szNumWritten = putc(*pStr, stdout);
                    }
                    if (iLogMode & LOGFILE)    
                    {
                        szNumWritten  = putc(*pStr, pFileStream0);      // sample log file
                        if (iLogMode & MASTER)                          
                        {
                            szNumWritten  = putc(*pStr, pFileStream1);  // master log file
                        }
                    }
                }
            }
        }
    }

    // end the sample log with a horizontal line if closing
    if (iLogMode & CLOSELOG) 
    {
        if (iLogMode & LOGCONSOLE) 
        {
            printf(HDASHLINE);
        }
        if (iLogMode & LOGFILE)
        {
            fprintf(pFileStream0, HDASHLINE);
        }
    }

    // flush console and/or file buffers if updated
    if (iLogMode & LOGCONSOLE) 
    {
        fflush(stdout);
    }
    if (iLogMode & LOGFILE)
    {
        fflush (pFileStream0);

        // if the master log file has been updated, flush it too
        if (iLogMode & MASTER)
        {
            fflush (pFileStream1);
        }
    }

    // If the log file is open and the caller requests "close file", then close and NULL file handle
    if ((pFileStream0) && (iLogMode & CLOSELOG))
    {
        fclose (pFileStream0);
        pFileStream0 = NULL;
    }
    if ((pFileStream1) && (iLogMode & CLOSELOG))
    {
        fclose (pFileStream1);
        pFileStream1 = NULL;
    }

    // return error code or OK 
    if (iLogMode & ERRORMSG)
    {
        return iErrNum;
    }
    else 
    {
        return 0;
    }
}

// Function to log standardized information to console, file or both
// *********************************************************************
int logEx(int iLogMode = LOGCONSOLE, int iErrNum = 0, const char* cFormatString = "", ...)
{
    va_list vaArgList;

    // Prepare variable agument list 
    va_start(vaArgList, cFormatString);
    int ret = logV(iLogMode, iErrNum, cFormatString, vaArgList);

    // end variable argument handler
    va_end(vaArgList);

    return ret;
}

// Function to log standardized information to console, file or both
// *********************************************************************
int logFile(const char* cFormatString = "", ...)
{
    va_list vaArgList;

    // Prepare variable agument list 
    va_start(vaArgList, cFormatString);
    int ret = logV(LOGBOTH, 0, cFormatString, vaArgList);

    // end variable argument handler
    va_end(vaArgList);

    return ret;
}
