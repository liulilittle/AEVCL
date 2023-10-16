#include "sockc.h"
#include "..\mempool\mempool.h"

typedef struct
{
	int handle; // �׽��־��
	char* ip; // ������ַ
	int port; // �˿ں�
	fd_set rfs; // �ɶ�����
	BOOL tcp;
	struct sockaddr_in addr;
} SOCKC_WORK_OBJ_T;

BOOL sock_init(HANDLE* handle, BOOL tcp)
{
	if (handle == NULL)
		return FALSE;
	SOCKC_WORK_OBJ_T* socket = NULL;
	if (*handle != NULL) {
		socket = (SOCKC_WORK_OBJ_T*)(*handle);
		if (socket->handle != INVALID_SOCKET) {
			sock_deinit(handle);
			socket = NULL;
		}
	}
	if (socket == NULL) {
		socket = (SOCKC_WORK_OBJ_T*)mempool_alloc(sizeof(SOCKC_WORK_OBJ_T));
		*handle = socket;
		memset(socket, 0x00, sizeof(SOCKC_WORK_OBJ_T));
		socket->tcp = tcp;
	}
	return TRUE;
}

BOOL sock_deinit(HANDLE* handle)
{
	if (handle == NULL)
		return FALSE;
	SOCKC_WORK_OBJ_T* socket = NULL;
	if (*handle == NULL) 
		return FALSE;
	socket = (SOCKC_WORK_OBJ_T*)(*handle);
	if (socket->handle != INVALID_SOCKET) {
		mempool_free(socket);
		sock_close(handle);
		*handle = NULL;
	}
	return TRUE;
}

BOOL sock_close(HANDLE handle)
{
	SOCKC_WORK_OBJ_T* socket = (SOCKC_WORK_OBJ_T*)handle;
	if (socket != NULL || socket->handle != INVALID_SOCKET) {
		closesocket(socket->handle);
		socket->handle = INVALID_SOCKET;
		FD_ZERO(&socket->rfs);
		memset(&socket->rfs, 0x00, sizeof(fd_set));
	}
	return TRUE;
}

int sock_connect(HANDLE handle, char* ip, int port)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (s == NULL)
		return FALSE;
	if (s->handle != INVALID_SOCKET)
		sock_close(handle);
	if (s->handle == INVALID_SOCKET) {
		sock_create(handle);
		s->ip = ip;
		s->port = port;
		struct sockaddr_in addr_in;
		memset(&addr_in, 0x00, sizeof(struct sockaddr_in));
		addr_in.sin_port = htons(s->port);
		addr_in.sin_family = AF_INET;
		//
		long sin_addr = inet_addr(s->ip); //  INADDR_NONE
		memcpy(&addr_in.sin_addr, &sin_addr, sizeof(long));
		//
		s->addr = addr_in;
	}
	return connect(s->handle, (struct sockaddr*)&s->addr, sizeof(s->addr)) == 0; // sizeof is 16
}

int sock_read(HANDLE handle, char* buf, int size, int* len, BOOL maintain)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (s == NULL || buf == NULL || size <= 0)
		return FALSE;
	if (!FD_ISSET(s->handle, &s->rfs))
		FD_SET(s->handle, &s->rfs);
	struct timeval tv; // ��s
	tv.tv_sec = 0;
	tv.tv_usec = 25;
	int error = select(s->handle + 1, &s->rfs, NULL, NULL, &tv);
	if (error > 0) { // û���׽��ִ������ݵ���ϵͳ������
		int ofs = 0;
		if (s->tcp)
			ofs = recv(s->handle, buf, size, 0);
		else
		{
			int al = sizeof(s->addr);
			ofs = recvfrom(s->handle, buf, *len, 0, (struct sockaddr*)&s->addr, &al);
		}
		if (maintain && ofs <= 0) // �ղ�������ʱ����������ʱ�������ѻָ�
			sock_recovery(handle);
		if (!maintain && ofs <= 0)
			return SOCKET_ERROR;
		if (len != NULL)
			*len = ofs;
		return ofs;
	}
	if (maintain && error < 0) // ��������׽��ִ�����ָ�����
		sock_recovery(handle);
	return error; // ���ش����ţ�����������������������·�Ͽ�
}

BOOL sock_write(HANDLE handle, char* buf, int size)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (s == NULL || buf == NULL || size <= 0)
		return FALSE;
	if (s->tcp)
		return send(s->handle, buf, size, 0) > SOCKET_ERROR;
	return sendto(s->handle, buf, size, 0, (struct sockaddr*)&s->addr, sizeof(s->addr)) > SOCKET_ERROR;
}

BOOL sock_create(HANDLE handle)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (s == NULL)
		return FALSE;
	if (s->tcp)
		s->handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else {
		struct ip_mreq mq;
		mq.imr_multiaddr.s_addr = htonl(s->port);
		mq.imr_interface.s_addr = htonl(INADDR_ANY);
		s->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		setsockopt(s->handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mq, sizeof(mq));
	}
	return TRUE;
}

BOOL sock_inited(HANDLE handle)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (s == NULL || s->handle == NULL || s->handle == INVALID_SOCKET)
		return FALSE;
	return TRUE;
}

BOOL sock_recovery(HANDLE handle)
{
	SOCKC_WORK_OBJ_T* s = (SOCKC_WORK_OBJ_T*)handle;
	if (sock_inited(handle)) {
		sock_close(handle); // �ȹر����Ӳ�����һЩ������Դ
		sock_create(handle); // ���¹������Ӳ��ؽ�������Դ
		return connect(s->handle, (struct sockaddr*)&s->addr, sizeof(s->addr)) == 0;
	}
	return FALSE;
}