/* Copyright (c) University of Dundee, 2001-2016 */


/*
 * Purpose:
 *   Platform-specific socket details.
 */

#ifndef SOCKET_STUFF_H_INCLUDED
#define SOCKET_STUFF_H_INCLUDED

/*----------------------------- _WIN32 ---------------------------------*/
#ifdef _WIN32
#include <winsock2.h>
#include <sys/types.h>

#define SOCKET_CLOSE(s)			closesocket(s)
#define SOCKET_SHUTDOWN_RO(s)		shutdown(s,SD_RECEIVE)
#define SOCKET_SHUTDOWN_WO(s)		shutdown(s,SD_SEND)
#define SOCKET_SHUTDOWN_RW(s)		shutdown(s,SD_BOTH)
#define SOCKET_RECV(s,p,n,f)		recv(s,(char *)p,n,f)
#define SOCKET_RECVFROM(s,p,n,f,t,l)	recvfrom(s,(char *)p,n,f,t,l)
#define SOCKET_SEND(s,p,n,f)		send(s,(char *)p,n,f)
#define SOCKET_SEND_TO(s,p,n,f,t,l)	sendto(s,(char *)p,n,f,t,l)

/*typedef socklen_t int; */

/*----------------------------- !_WIN32 ---------------------------------*/
#else
/* assume UNIX-like */
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

/* Needed because Winsock/MSVC++ does things slightly differently ... */
#define SOCKET				int
#define SOCKET_CLOSE(s)			close(s)
#define SOCKET_SHUTDOWN_RO(s)		shutdown(s,SHUT_RD)
#define SOCKET_SHUTDOWN_WO(s)		shutdown(s,SHUT_WR)
#define SOCKET_SHUTDOWN_RW(s)		shutdown(s,SHUT_RDWR)
#define SOCKET_RECV(s,p,n,f)		recv(s,p,n,f)
#define SOCKET_RECVFROM(s,p,n,f,t,l)	recvfrom(s,p,n,f,t,l)
#define SOCKET_SEND(s,p,n,f)		send(s,p,n,f)
#define SOCKET_SEND_TO(s,p,n,f,t,l)	sendto(s,p,n,f,t,l)
#endif


#endif
