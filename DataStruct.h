#pragma once

#include "DoubleList.h"
//////////////////////////////////////////////////////////////////////////

#define    BUFFER_SIZE           4096
#define    MAXTHREAD_COUNT       8

#define    PORT                  7000
#define    ADDR					 "127.0.0.1"

//////////////////////////////////////////////////////////////////////////

//
//��ɶ˿���ɵ��¼�
//
typedef enum
{
		IOCP_COMPLETE_READ,
		IOCP_COMPLETE_ACCEPT_READ,
		IOCP_COMPLETE_WRITE,
		IOCP_COMPLETE_ACCEPT
}IOCP_COMPLETE_STATE;

//
//�Զ���ö���������ͣ�������ʶ�׽���IO��������
//
typedef enum 
{
        IOCP_ACCEPT, //AcceptEx/accept
		IOCP_READ,   //WSARecv/recv/ReadFile
		IOCP_WRITE,   //WSASend/send/WriteFile
		IOCP_END
}IOCP_OPERATION, *IOCP_OPERATION_PTR;


//
//�Զ���ṹ��������ɼ���(���������)
//
typedef struct
{
	SOCKET                     socket;
}IOCP_KEY,*IOCP_KEY_PTR;

//
//��־SOCKET�ĵ�ǰ״̬
//
enum
{
	SOCKET_STATE_NOT_CONNECT,
	SOCKET_STATE_CONNECT,
	SOCKET_STATE_CONNECT_AND_READ,
	SOCKET_STATE_TIMEOUT
};


//
//��IO���ݣ���չ��WSAOVERLAPPED
//
typedef struct 
{
	WSAOVERLAPPED				ol;
	char						buf[BUFFER_SIZE];
	WSABUF						wsaBuf;
	SOCKET						socket;

	IOCP_OPERATION				operation;

	volatile int				state;
}IOCP_IO,*IOCP_IO_PTR;

typedef CDoubleList<IOCP_IO,		10>	IO_GROUP;
typedef CDoubleList<IOCP_KEY,		10>	KEY_GROUP;

typedef IO_GROUP::ITERATOR	IO_POS;
typedef KEY_GROUP::ITERATOR KEY_POS;

