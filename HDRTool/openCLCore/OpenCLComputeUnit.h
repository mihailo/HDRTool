

#ifndef OPEN_CLCOMPUTE_UNIT_H
#define OPEN_CLCOMPUTE_UNIT_H


class OpenClComputeUnit
{
public:
	virtual void allocateOpenCLMemory() = 0;
	virtual void setInputDataToOpenCLMemory() = 0;
	virtual void getDataFromOpenCLMemory() = 0;
	virtual void clearDeviceMemory() = 0;

	virtual size_t *getSzGlobalWorkSize() = 0;        
	virtual size_t *getSzLocalWorkSize() = 0;

};

#endif