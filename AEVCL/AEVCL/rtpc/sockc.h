#if !defined(__SOCKC_H__)
#define __SOCKC_H__

#include <stdio.h>
#include <malloc.h>

#if defined(__linux__)
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h> 
#include <sys/time.h>
#else
#include <Windows.h>
#endif

BOOL sock_create(HANDLE handle);
BOOL sock_inited(HANDLE handle);
BOOL sock_recovery(HANDLE handle);
BOOL sock_init(HANDLE* handle, BOOL tcp);
BOOL sock_deinit(HANDLE* handle);
int sock_connect(HANDLE handle, char* ip, int port);
int sock_read(HANDLE handle, char* buf, int size, int* len, BOOL maintain = TRUE);
BOOL sock_write(HANDLE handle, char* buf, int size);
BOOL sock_close(HANDLE handle);

#endif
