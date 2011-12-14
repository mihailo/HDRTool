

#ifndef OPEN_CLCOMPUTE_UNIT_H
#define OPEN_CLCOMPUTE_UNIT_H


class OpenClComputeUnit
{
public:
	virtual void allocateOpenCLMemory() = 0;
	virtual void setInputDataToOpenCLMemory() = 0;
	virtual void getDataFromOpenCLMemory() = 0;

	virtual size_t *getSzGlobalWorkSize() = 0;        // 1D var for Total # of work items
	virtual size_t *getSzLocalWorkSize() = 0;		    // 1D var for # of work items in the work group	

};

#endif