#ifndef HTTPD_H
#define HTTPD_H


#include "TCPSocket.h"


class HTTPHandler
{
public:
    virtual uint32_t webPutStart(const char *path, int size) = 0;
    virtual void webPutData(const char *path, const uint8_t *data, int size) = 0;
    virtual const char* webPutEnd(const char *path) = 0;
    
    virtual const char* webGet(const char *path, uint32_t *dataPtr, int *size) = 0;
};


class HTTPd : public TCPSocket
{
public:
    HTTPd(int port);
    virtual ~HTTPd();
    
    void setHandler(HTTPHandler *h);
    
    
    
protected:
    HTTPd(TCPSocket *parent, struct espconn *c);
    TCPSocket* clientConnected(struct espconn *c);
    
    void dataSent();
    void dataReceived(const uint8_t *data, int len);
    
    bool httpFS(const char *path);
    
    
    char buf[256], *bufend;
    int buflen;
    
    char http_method;
    
    HTTPHandler *handler;
    
    struct
    {
	const char *path;
	int size, contentLength;
	uint32_t addr;
	uint8_t *buffer;
	int bufferPos;
    } put;
    
    struct
    {
	const char *path;
	uint32_t addr;
	int size;
	uint32_t start;
	void *buf2free;
    } get;
};


#endif
