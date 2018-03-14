#include "TCPSocket.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>


typedef struct SERVER
{
    int port;
    TCPSocket *srv;

    struct SERVER *next;
} SERVER;

static SERVER *servers=0;



struct sendq
{
    uint8_t *data;
    int len;
    struct sendq *next;
};


static void server_connect_cb(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    
    // Ищем сервер по порту
    SERVER *cur=servers;
    while (cur)
    {
        if (cur->port == conn->proto.tcp->local_port)
        {
            // Нашли
            espconn_set_opt(conn, ESPCONN_REUSEADDR | ESPCONN_NODELAY);
            TCPSocket *sock=cur->srv->clientConnected(conn);
            if (sock)
            {
        	sock->__delete_espconn=false;
        	return;
    	    }
            break;
        }
        cur=cur->next;
    }
    
    // Не нашли
    /*espconn_disconnect(conn);
    espconn_delete(conn);
    if (conn->proto.tcp) os_free(conn->proto.tcp);
    os_free(conn);*/
}


static void recv_cb(void *arg, char *data, unsigned short len)
{
    struct espconn *conn = (struct espconn *)arg;
    if (conn->reverse)
	((TCPSocket*)(conn->reverse))->dataReceived((const uint8_t*)data, len);
}


static void sent_cb(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    if (conn->reverse)
	((TCPSocket*)(conn->reverse))->dataSent_Q();
}


static void recon_cb(void *arg, sint8 err)
{
    struct espconn *conn = (struct espconn *)arg;
    if (conn->reverse)
    {
	TCPSocket *sock=(TCPSocket*)(conn->reverse);
	sock->error(err);
	conn->reverse=0;
	if (sock->__delete_espconn)
	{
	    espconn_delete(conn);
	    if (conn->proto.tcp) os_free(conn->proto.tcp);
	    os_free(conn);
	}
	
	delete sock;
    }
}


static void discon_cb(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    if (conn->reverse)
    {
	TCPSocket *sock=(TCPSocket*)(conn->reverse);
	sock->disconnected();
	conn->reverse=0;
	if (sock->__delete_espconn)
	{
	    espconn_delete(conn);
	    if (conn->proto.tcp) os_free(conn->proto.tcp);
	    os_free(conn);
	}
	delete sock;
    }
}


static void poll_cb(void *arg)
{
    if (arg)
	((TCPSocket*)arg)->poll();
}


static void client_connect_cb(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    if (conn->reverse)
    {
	((TCPSocket*)conn->reverse)->connected();
    } else
    {
	espconn_disconnect(conn);
	espconn_delete(conn);
	if (conn->proto.tcp) os_free(conn->proto.tcp);
	os_free(conn);
    }
}


static void dns_cb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    
    if ( (!conn) || (!conn->reverse) ) return;
    
    TCPSocket *sock=(TCPSocket*)conn->reverse;
    
    if (! ipaddr)
    {
	sock->error(-99);
	delete sock;
	conn->reverse=0;
	if (conn->proto.tcp) os_free(conn->proto.tcp);
	os_free(conn);
    } else
    {
        sock->__conn->type=ESPCONN_TCP;
        sock->__conn->state=ESPCONN_NONE;
        sock->__conn->proto.tcp=(esp_tcp *)os_zalloc(sizeof(esp_tcp));
        sock->__conn->proto.tcp->local_port=espconn_port();
        sock->__conn->proto.tcp->remote_port=sock->__port;
        os_memcpy(sock->__conn->proto.tcp->remote_ip, &ipaddr->addr, 4);
	
	// Регистрируем вызовы
        espconn_regist_connectcb(sock->__conn, client_connect_cb);
	espconn_regist_recvcb(sock->__conn, recv_cb);
	espconn_regist_sentcb(sock->__conn, sent_cb);
        espconn_regist_reconcb(sock->__conn, recon_cb);
        espconn_regist_disconcb(sock->__conn, discon_cb);
	
	// Подключаемся
	espconn_connect(sock->__conn);
    }
}


TCPSocket::TCPSocket()
{
    __parent=0;
    __is_server=false;
    __port=0;
    __delete_espconn=false;
    __sendq=0;
    __sendq_len=0;
    __sendq_used=0;
    __conn=0;
    
    // Настраиваем таймер
    os_timer_setfn(&__tmr, (os_timer_func_t*)poll_cb, (void*)this);
}


TCPSocket::TCPSocket(int port)
{
    // Настраиваем таймер
    os_timer_setfn(&__tmr, (os_timer_func_t*)poll_cb, (void*)this);
    
    listen(port);
}


TCPSocket::TCPSocket(const char *host, int port)
{
    // Настраиваем таймер
    os_timer_setfn(&__tmr, (os_timer_func_t*)poll_cb, (void*)this);
    
    connectToHost(host, port);
}


TCPSocket::TCPSocket(TCPSocket *parent, struct espconn *c)
{
    // Сохраняем ссылку на класс в сокете
    __parent=parent;
    __is_server=false;
    __sendq=0;
    __sendq_len=0;
    __sendq_used=0;
    __delete_espconn=true;
    __conn=c;
    __conn->reverse=(void*)this;
    espconn_set_opt(__conn, ESPCONN_REUSEADDR | ESPCONN_NODELAY);
    
    // Регистрируем вызовы
    espconn_regist_recvcb(__conn, recv_cb);
    espconn_regist_reconcb(__conn, recon_cb);
    espconn_regist_sentcb(__conn, sent_cb);
    espconn_regist_disconcb(__conn, discon_cb);
    
    // Настраиваем таймер
    os_timer_setfn(&__tmr, (os_timer_func_t*)poll_cb, (void*)this);
    
    // Дергаем обработчик того, что сокет подключился
    connected();
}


TCPSocket::~TCPSocket()
{
    os_timer_disarm(&__tmr);
    
    if (__sendq)
    {
	struct sendq *p=__sendq;
	while (p)
	{
	    os_free((void*)p->data);
	    struct sendq *n=p->next;
	    os_free((void*)p);
	    p=n;
	}
    }
    
    if (__is_server)
    {
	// Удаляем себя из пула серверов
	SERVER *prev=0, *cur=servers;
	
	while (cur)
	{
    	    if (cur->srv == this)
    	    {
        	if (prev) prev->next=cur->next; else servers=cur->next;
        	delete cur;
        	break;
    	    }
    	    prev=cur;
    	    cur=cur->next;
	}
	
	if (__conn)
	{
	    espconn_disconnect(__conn);
	    espconn_delete(__conn);
	    os_free(__conn);
	}
    }
}


void TCPSocket::connectToHost(const char *host, int port)
{
    // Создаем сокет
    __parent=0;
    __is_server=false;
    __port=port;
    __delete_espconn=true;
    __sendq=0;
    __sendq_len=0;
    __sendq_used=0;
    __conn=(struct espconn *)os_zalloc(sizeof(struct espconn));
    ets_memset(__conn, 0, sizeof(struct espconn));
    __conn->reverse=(void*)this;
    espconn_set_opt(__conn, ESPCONN_REUSEADDR | ESPCONN_NODELAY);
    
    switch (espconn_gethostbyname(__conn, host, &__ip, dns_cb))
    {
        case ESPCONN_OK:
            dns_cb(host, &__ip, __conn);
            break;
	
        case ESPCONN_INPROGRESS:
            break;
	
        default:
            dns_cb(host, 0, __conn);
            break;
    }
}


void TCPSocket::listen(int port)
{
    // Создаем сокет
    __parent=0;
    __is_server=true;
    __port=port;
    __delete_espconn=true;
    __sendq=0;
    __sendq_len=0;
    __sendq_used=0;
    __conn=(struct espconn *)os_zalloc(sizeof(struct espconn));
    ets_memset(__conn, 0, sizeof(struct espconn));
    
    // Создаем сервер
    espconn_create(__conn);
    __conn->type=ESPCONN_TCP;
    __conn->state=ESPCONN_NONE;
    __conn->proto.tcp=(esp_tcp *)os_zalloc(sizeof(esp_tcp));
    __conn->proto.tcp->local_port=port;
    
    // Добавляем в пул серверов
    SERVER *s=new SERVER;
    s->port=__port;
    s->srv=this;
    s->next=servers;
    servers=s;
    
    // Функция при подключении
    espconn_regist_connectcb(__conn, server_connect_cb);
    
    // Принимаем подключения
    espconn_accept(__conn);
}


TCPSocket* TCPSocket::clientConnected(struct espconn *c)
{
    return 0;
}


void TCPSocket::connected()
{
}


void TCPSocket::dataReceived(const uint8_t *data, int len)
{
}


void TCPSocket::dataSent()
{
}


void TCPSocket::dataSent_Q()
{
    if (__sendq)
    {
	// Есть что отправить из очереди
	espconn_send(__conn, __sendq->data, __sendq->len);
	
	__sendq_used-=__sendq->len;
	
	struct sendq *n=__sendq->next;
	os_free((void*)__sendq->data);
	os_free((void*)__sendq);
	__sendq=n;
    }
    
    // Если нет очереди отправки (или она пуста) - дергаем вызов dataSent
    if (! __sendq) dataSent();
}


void TCPSocket::poll()
{
}


void TCPSocket::error(int err)
{
}


void TCPSocket::disconnected()
{
}


bool TCPSocket::send(const uint8_t *data, int len)
{
    int rv=-1;
    
    if (! __sendq)
    {
	// Нет очереди - отправляем так
	rv=espconn_send(__conn, (uint8_t*)data, len);
    }
    
    if ( (rv != 0) && (__sendq_len > 0) && (__sendq_used < __sendq_len) )
    {
	// Можно поместить в очередь
	struct sendq *v=(struct sendq*)os_malloc(sizeof(struct sendq));
	if (v) v->data=(uint8_t*)os_malloc(len);
	if ( (v) && (v->data) )
	{
	    // Создали буфер
	    os_memcpy(v->data, data, len);
	    v->len=len;
	    v->next=0;
	    
	    // Ищем конец очереди
	    struct sendq **q=&__sendq;
	    while (*q) q=&((*q)->next);
	    
	    // Удачно
	    __sendq_len+=len;
	    (*q)=v;
	    rv=0;
	} else
	{
	    // Ошибка выделения памяти
	    if (v->data) os_free((void*)v->data);
	    if (v) os_free((void*)v);
	}
    }
    
    return rv==0;
}


bool TCPSocket::sendAndFree(const uint8_t *data, int len)
{
    int rv=-1;
    
    if (! __sendq)
    {
	// Нет очереди - отправляем так
	rv=espconn_send(__conn, (uint8_t*)data, len);
	
	if (rv==0)
	{
	    // Сразу отправилось - можно убить буфер
	    os_free((void*)data);
	    return true;
	}
    }
    
    if ( (rv != 0) && (__sendq_len > 0) && (__sendq_used < __sendq_len) )
    {
	// Можно поместить в очередь
	struct sendq *v=(struct sendq*)os_malloc(sizeof(struct sendq));
	if (v)
	{
	    // Создали буфер
	    v->data=(uint8_t*)data;
	    v->len=len;
	    v->next=0;
	    
	    // Ищем конец очереди
	    struct sendq **q=&__sendq;
	    while (*q) q=&((*q)->next);
	    
	    // Удачно
	    __sendq_len+=len;
	    (*q)=v;
	    rv=0;
	} else
	{
	    // Ошибка выделения памяти
	    rv=-1;
	}
    }
    
    if (rv!=0) os_free((void*)data);
    
    return rv==0;
}


void TCPSocket::enableSendQueue(int len)
{
    __sendq_len=len;
}


int TCPSocket::sendQueueFree()
{
    return __sendq_len - __sendq_used;
}


void TCPSocket::hold()
{
    espconn_recv_hold(__conn);
}


void TCPSocket::unhold()
{
    espconn_recv_unhold(__conn);
}


void TCPSocket::setPoll(int ms)
{
    os_timer_disarm(&__tmr);
    if (ms > 0) os_timer_arm(&__tmr, ms, 1);
}


void TCPSocket::close()
{
    espconn_disconnect(__conn);
}


void TCPSocket::setTimeout(int secs)
{
    espconn_regist_time(__conn, secs, 0);
}


const uint8_t* TCPSocket::localIP()
{
    return __conn->proto.tcp->local_ip;
}


const uint8_t* TCPSocket::remoteIP()
{
    return __conn->proto.tcp->remote_ip;
}
