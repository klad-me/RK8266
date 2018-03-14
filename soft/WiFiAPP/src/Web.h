#ifndef WEB_H
#define WEB_H


#include "HTTPd.h"


class Web : public HTTPHandler
{
public:
    uint32_t webPutStart(const char *path, int size);
    void webPutData(const char *path, const uint8_t *data, int size);
    const char* webPutEnd(const char *path);

    const char* webGet(const char *path, uint32_t *dataPtr, int *size);
    
    
    const char* getDir(uint32_t *dataPtr, int *size);
    const char* getFile(uint8_t type, const char *path, uint32_t *dataPtr, int *size);
    const char* delFile(const char *path, uint32_t *dataPtr, int *size);
};


#endif
