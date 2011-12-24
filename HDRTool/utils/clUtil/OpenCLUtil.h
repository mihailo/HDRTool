

#ifndef SHR_QATEST_H
#define SHR_QATEST_H



extern "C" char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength);
extern "C" size_t roundUp(int group_size, int global_size);

#endif