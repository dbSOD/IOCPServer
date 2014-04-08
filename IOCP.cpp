#include "IOCP.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////

CIOCP::CIOCP()
{
}
//���캯��

CIOCP::~CIOCP()
{
	Close();
}
//��������
/*-------------------------------------------------------------------------------------------
�������ܣ��رղ������Դ
����˵����
�������أ�
-------------------------------------------------------------------------------------------*/
void CIOCP::Close()
{
	int					i;
	IO_POS				pos;
	IOCP_IO_PTR	lp_io;
	bool bState;

	CloseHandle( m_h_iocp );

	 m_io_group.GetHeadPosition( pos, bState);

	while(bState)
	{
		lp_io = m_io_group.GetNext( pos , bState);

		closesocket( lp_io->socket );
	}

	for( i = 0; i < m_n_thread_count; i++ )
	{
		CloseHandle( m_h_thread[i] );

		m_h_thread[i] = NULL;
	}
}

/*-------------------------------------------------------------------------------------------
�������ܣ���ʼ��IO���
����˵����
�������أ�
-------------------------------------------------------------------------------------------*/
void CIOCP::InitIoContext(IOCP_IO_PTR lp_io)
{
	memset( &lp_io->ol,  0, sizeof( WSAOVERLAPPED ) );
	memset( &lp_io->buf, 0, BUFFER_SIZE );
	lp_io->wsaBuf.buf		= lp_io->buf;
	lp_io->wsaBuf.len		= BUFFER_SIZE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ���ʼ������SOCKET�˿ڣ�������ɶ˿�����������
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitSocket()
{
	m_listen_socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );

	if( INVALID_SOCKET == m_listen_socket )
	{
		return FALSE;
	}

	IOCP_KEY_PTR  lp_key = m_key_group.GetBlank();

	lp_key->socket = m_listen_socket;

	HANDLE hRet = CreateIoCompletionPort( (HANDLE)m_listen_socket, m_h_iocp, (DWORD)lp_key, 0 );

	if( hRet == NULL )
	{
		closesocket( m_listen_socket );

		m_key_group.RemoveAt( lp_key );

		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ��ر������߳�
����˵����
�������أ�
-------------------------------------------------------------------------------------------*/
void CIOCP::CloseThreadHandle(int count )
{
	if( count <= 0 )
	{
		return;
	}

	for( int i= 0; i < count; i++ )
	{
		CloseHandle( m_h_thread[i] );

		m_h_thread[i] = INVALID_HANDLE_VALUE;
	}
}

/*-------------------------------------------------------------------------------------------
�������ܣ��������˿ں��Լ���IP��PORT�󶨣�����ʼ����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::BindAndListenSocket()
{
	SOCKADDR_IN addr;

	memset( &addr, 0, sizeof(SOCKADDR_IN) );

	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= inet_addr( ADDR );
	addr.sin_port			= htons( PORT );

	int nRet;

	nRet = bind( m_listen_socket, (SOCKADDR*)&addr, sizeof( SOCKADDR ) );

	if( SOCKET_ERROR == nRet )
	{
		cout<<"bind fail!"<<endl;

		return FALSE;
	}

	nRet = listen( m_listen_socket, 20 );

	if( SOCKET_ERROR == nRet )
	{
		cout<<"listen fail!"<<endl;

		return FALSE;
	}

	return TRUE;
}


/*-------------------------------------------------------------------------------------------
�������ܣ�����CPU����Ŀ��������Ӧ���������ݴ����߳�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::StartThread()
{
	int i;

	SYSTEM_INFO	sys_info;

	GetSystemInfo( &sys_info );

	m_n_thread_count = sys_info.dwNumberOfProcessors > MAXTHREAD_COUNT ? MAXTHREAD_COUNT : sys_info.dwNumberOfProcessors;

	for( i = 0; i < m_n_thread_count; i++ )
	{
		m_h_thread[i] = CreateThread( NULL, 0, CompletionRoutine, (LPVOID)this, 0, NULL );

		if( NULL == m_h_thread[i] )
		{
			CloseThreadHandle( i );

			CloseHandle( m_h_iocp );

			return FALSE;
		}

		MSG("start a thread");
	}

	return TRUE;
}


/*-------------------------------------------------------------------------------------------
�������ܣ�����һ������������
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::PostAcceptEx()
{
	int		count = 10;
	DWORD	dwBytes;
	BOOL	bRet;

	for( int i = 0; i < count; i++ )
	{
		SOCKET socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );

		if( INVALID_SOCKET == socket )
		{
			cout<<"post accept ex fail!"<<WSAGetLastError()<<endl;

			continue;
		}

		IOCP_IO_PTR lp_io = m_io_group.GetBlank();

		InitIoContext( lp_io );

		lp_io->socket			= socket;
		lp_io->operation		= IOCP_ACCEPT;
		lp_io->state			= SOCKET_STATE_NOT_CONNECT;

		/////////////////////////////////////////////////

		bRet = lpAcceptEx(
			m_listen_socket,
			lp_io->socket,
			lp_io->buf,
//			lp_io->wsaBuf.len - 2 * ( sizeof(SOCKADDR_IN) + 16 ),
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			&dwBytes,&lp_io->ol);

		if( ( bRet == FALSE ) && ( WSA_IO_PENDING != WSAGetLastError() ) )
		{
			closesocket( socket );

			m_io_group.RemoveAt( lp_io );

			cout<<"post acceptex fail:"<<WSAGetLastError()<<endl;

			continue;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ��������ݺ���
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::HandleData(IOCP_IO_PTR lp_io, int nFlags)
{
	switch( nFlags )
	{
	case IOCP_COMPLETE_ACCEPT:
		{
			cout<<"Accept a link!*****************************"<<endl;

			InitIoContext( lp_io );

			lp_io->operation	= IOCP_READ;
		}
		break;
	case IOCP_COMPLETE_ACCEPT_READ:
		{
			cout<<"read a data!*******************************data length is:"<<endl;

			lp_io->operation	= IOCP_WRITE;

			GetAddrAndPort( lp_io->wsaBuf.buf, szAddress, uPort );

			MSG(lp_io->wsaBuf.len);

			memset( &lp_io->ol, 0, sizeof(lp_io->ol) );
		}
		break;
	case IOCP_COMPLETE_READ:
		{
			cout<<"read a data!*******************************"<<endl;

			lp_io->operation	= IOCP_WRITE;

			memset( &lp_io->ol, 0, sizeof(lp_io->ol) );
		}
		break;
	case IOCP_COMPLETE_WRITE:
		{
			cout<<"write a data!******************************"<<endl;

			InitIoContext( lp_io );

			//lp_io->operation	= IOCP_END;
			lp_io->operation	= IOCP_READ;
		}
		break;
	default:
		{
			cout<<"handleData do nothing!*********************"<<endl;

			return FALSE;
		}
	}

	return TRUE;
}


/*-------------------------------------------------------------------------------------------
�������ܣ�����һЩ�ص�����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::DataAction(IOCP_IO_PTR lp_io, IOCP_KEY_PTR lp_key )
{
	DWORD	dwBytes;
	int		nRet;
	DWORD	dwFlags;

	switch( lp_io->operation )
	{
	case IOCP_WRITE:
		{
			cout<<"post a write data!---------------------------------------"<<endl;
			strcpy(lp_io->buf, "hello");
			nRet = WSASend(
				lp_io->socket, 
				&lp_io->wsaBuf,
				1,
				&dwBytes,
				0,
				&lp_io->ol,NULL);

			if( ( nRet == SOCKET_ERROR ) && ( WSAGetLastError() != WSA_IO_PENDING ) )
			{
				cout<<"WSASend fail!----------------------------------------"<<WSAGetLastError()<<endl;

				closesocket( lp_io->socket );

				m_io_group.RemoveAt( lp_io );
				m_key_group.RemoveAt( lp_key );

				return FALSE;
			}
		}
		break;
	case IOCP_READ:
		{
			cout<<"post a read data!-----------------------------------------"<<endl;

			dwFlags = 0;

			nRet = WSARecv(
				lp_io->socket,
				&lp_io->wsaBuf,
				1,
				&dwBytes,
				&dwFlags,
				&lp_io->ol,NULL);

			if( ( nRet == SOCKET_ERROR ) && ( WSAGetLastError() != WSA_IO_PENDING ) )
			{
				cout<<"WSARecv fail!-------------------------------------------"<<WSAGetLastError()<<endl;

				closesocket( lp_io->socket );

				m_io_group.RemoveAt( lp_io );
				m_key_group.RemoveAt( lp_key );

				return FALSE;
			}
		}
		break;
	case IOCP_END:
		{
			cout<<"close a socket link!-------------------------------------------"<<endl;

			closesocket( lp_io->socket );
			
			m_io_group.RemoveAt( lp_io );
			m_key_group.RemoveAt( lp_key );
		}
		break;
	default:
		{
			cout<<"DataAction do nothing!------------------------------------------"<<endl;

			return FALSE;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ��õ�MS��װ��SOCKET����ָ�룬������������ٶ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetFunPointer()
{
	DWORD dwRet,nRet;

	nRet = WSAIoctl( 
		m_listen_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&g_GUIDAcceptEx, 
		sizeof(g_GUIDAcceptEx), 
		&lpAcceptEx,
		sizeof(lpAcceptEx),
		&dwRet,NULL,NULL);

	if( SOCKET_ERROR == nRet )
	{
		closesocket( m_listen_socket );

		cout<<"get acceptex fail!"<<WSAGetLastError()<<endl;

		return FALSE;
	}

	nRet = WSAIoctl( 
		m_listen_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&g_GUIDTransmitFile,
		sizeof(g_GUIDTransmitFile),
		&lpTransmitFile,
		sizeof(lpTransmitFile),
		&dwRet,NULL,NULL);

	if(nRet == SOCKET_ERROR )
	{
		closesocket( m_listen_socket );

		cout<<"get transmitfile fail!"<<WSAGetLastError()<<endl;

		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ������Ƿ��������ˣ����ܳ�ʱ��û�����ݵġ���Ч���ӡ����еĻ������ߵ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
void CIOCP::CheckForInvalidConnection()
{
	int			op,op_len,nRet;
	IOCP_IO_PTR	lp_io = NULL;
	IO_POS		pos;
	bool bState;
	m_io_group.GetHeadPosition( pos, bState);

	while(bState)
	{
		lp_io = m_io_group.GetNext( pos, bState);
		//�����ĸ���û�е�½�ģ��ٲ����û��½�೤ʱ����
		if( lp_io->state != SOCKET_STATE_CONNECT_AND_READ )
		{
			op_len = sizeof(op);

			nRet = getsockopt( lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len );

			if( SOCKET_ERROR == nRet )
			{
				MSG("SO_CONNECT_TIME failed:");
				MSG(WSAGetLastError());

				continue;
			}
			if( op != 0xffffffff && op > 20 )
			{
				closesocket( lp_io->socket );

				m_io_group.RemoveAt( lp_io );

				MSG("��һ�����ӣ���û�н��յ�����,�Ѿ����߳�ȥ��");

				MSG( lp_io );
			}
			else
			{
				cout<<op<<endl;
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------
�������ܣ�ע��FD_ACCEPTG�¼���m_h_accept_event�¼����Ա����з���ȥ�����Ӻĺľ�ʱ���õ�֪ͨ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::RegAcceptEvent()
{
	int		nRet;

	m_h_accept_event = CreateEvent( NULL, FALSE, FALSE, NULL );

	if( NULL == m_h_accept_event )
	{
		return FALSE;
	}

	nRet = WSAEventSelect( m_listen_socket, m_h_accept_event, FD_ACCEPT );

	if( nRet != 0 )
	{
		CloseHandle( m_h_accept_event );

		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ��õ����������Ŀͻ���IP��PORT
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetAddrAndPort(char*buf,char ip[],UINT &port)
{
	int		len	= BUFFER_SIZE - sizeof( SOCKADDR_IN ) - 16;
	char	*lp_buf = buf + len;	//ֱ�Ӷ�ȡԶ�˵�ַ

	SOCKADDR_IN	addr;

	memcpy( &addr, lp_buf, sizeof( SOCKADDR_IN ) );

	port	= ntohl( addr.sin_port );
	strcpy( ip, inet_ntoa( addr.sin_addr ) );

	MSG("�ͻ�IPΪ��");
	MSG(ip);
	MSG("�ͻ��˿�Ϊ��");
	MSG(port);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------------------------------
�������ܣ���ʼ����ɶ˿ڼ���ص����ж�����������ÿһ��10������.
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::Init()
{
	WSAData data;
	if( WSAStartup( MAKEWORD(2,2),&data) != 0 )
	{
		cout<<"WSAStartup fail!"<<WSAGetLastError() << endl;

		return FALSE;
	}

	m_h_iocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, 0 );

	if( NULL == m_h_iocp )
	{
		cout << "CreateIoCompletionPort() failed: " << GetLastError() << endl;

		return FALSE;
	}

	if( !StartThread() )
	{
		cout<<"start thread fail!"<<endl;

		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		CloseHandle( m_h_iocp );

		return FALSE;
	}

	if( !InitSocket() )
	{
		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		cout<<"Init sociket fail!"<<endl;

		CloseHandle( m_h_iocp );

		return FALSE;
	}

	if( !BindAndListenSocket() )
	{
		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		cout<<"Init sociket fail!"<<endl;

		CloseHandle( m_h_iocp );

		closesocket( m_listen_socket );

		return FALSE;
	}

	if( !GetFunPointer() )
	{
		cout<<"GetFunPointer fail!"<<endl;
		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		CloseHandle( m_h_iocp );

		closesocket( m_listen_socket );

		return FALSE;
	}

	if( !PostAcceptEx() )
	{
		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		cout<<"PostAcceptEx fail!"<<endl;

		CloseHandle( m_h_iocp );

		closesocket( m_listen_socket );

		return FALSE;
	}

	if( !RegAcceptEvent() )
	{
		PostQueuedCompletionStatus( m_h_iocp, 0, NULL, NULL );

		cout<<"RegAcceptEvent fail!"<<endl;

		CloseHandle( m_h_iocp );

		closesocket( m_listen_socket );

		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ���ѭ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::MainLoop()
{
	DWORD	dwRet;
	int		nCount = 0;

	cout << "Server is running.........." << nCount++ << " times" << endl;
	
	while( TRUE )
	{
		dwRet = WaitForSingleObject( m_h_accept_event, 10000 );

		switch( dwRet )
		{
		case WAIT_FAILED:
			{
				PostQueuedCompletionStatus( m_h_iocp, 0, 0, NULL );
				
				return FALSE;
			}
			break;
		case WAIT_TIMEOUT:
			{
				if( nCount > 100 )
				{
					nCount = 0;
				}

				cout << "Server is running.........." << nCount++ << " times" << endl;
				
				CheckForInvalidConnection();
			}
			break;
		case WAIT_OBJECT_0:	//���յ������з��������Ӷ��ù��˵���Ϣ���ٴη�������
			{
				if(  !PostAcceptEx() )
				{
					PostQueuedCompletionStatus( m_h_iocp, 0, 0, NULL );

					return FALSE;
				}
			}
			break;
		}
	}

	return TRUE;
}

/*-------------------------------------------------------------------------------------------
�������ܣ����ݴ����̺߳���
����˵����
�������أ�
-------------------------------------------------------------------------------------------*/
DWORD CIOCP::CompletionRoutine(LPVOID lp_param)
{
	CIOCP*			lp_this			= (CIOCP*)lp_param;

	int				nRet;
	BOOL			bRet;
	DWORD			dwBytes			= 0;
	HANDLE			hRet;

	IOCP_KEY_PTR	lp_key			= NULL;
	IOCP_IO_PTR		lp_io			= NULL;
	LPWSAOVERLAPPED	lp_ov			= NULL;

	IOCP_KEY_PTR	lp_new_key		= NULL;

	while( TRUE )
	{
		bRet = GetQueuedCompletionStatus( lp_this->m_h_iocp, &dwBytes, (LPDWORD)&lp_key, &lp_ov, INFINITE );

		cout<<"���յ�һ���¼�"<<endl;

		lp_io	= (IOCP_IO_PTR)lp_ov;

		if( FALSE == bRet )
		{
			cout << "GetQueuedCompletionStatus() failed: " << GetLastError() << endl;

			lp_this->m_io_group.RemoveAt( lp_io );

			lp_this->m_key_group.RemoveAt( lp_key );

			continue;
		}

		if( NULL == lp_key )
		{
			return 0;
		}

		if( lp_io == NULL )
		{
			cout<<"recv a null CIoContext!"<<endl;

			continue;
		}

		if( ( IOCP_ACCEPT != lp_io->operation ) && ( 0 == dwBytes ) )
		{
			closesocket( lp_io->socket );

			lp_this->m_io_group.RemoveAt( lp_io );

			lp_this->m_key_group.RemoveAt( lp_key );

			MSG("һ���û��˳���");

			continue;
		}

		switch( lp_io->operation )
		{
		case IOCP_ACCEPT:
			{
				cout<<"���յ�һ������"<<endl;

				lp_io->state = SOCKET_STATE_CONNECT;

				if( dwBytes > 0 )
				{
					lp_io->state = SOCKET_STATE_CONNECT_AND_READ;

					cout<<"��ȡ��һ������"<<endl;
				}

				nRet = setsockopt( 
					lp_io->socket, 
					SOL_SOCKET, 
					SO_UPDATE_ACCEPT_CONTEXT, 
					( char* )&lp_this->m_listen_socket,
					sizeof( lp_this->m_listen_socket ) );

				if( SOCKET_ERROR == nRet )
				{
					closesocket( lp_io->socket );

					lp_this->m_io_group.RemoveAt( lp_io );

					cout<<"update socket fail!"<<WSAGetLastError()<<endl;

					continue;
				}

				lp_new_key = lp_this->m_key_group.GetBlank();

				if( lp_new_key == NULL )
				{
					closesocket( lp_io->socket );

					lp_this->m_io_group.RemoveAt( lp_io );

					cout<<"get a handle fail!"<<GetLastError()<<endl;

					continue;
				}

				lp_new_key->socket = lp_io->socket;

				//���½�����SOCKETͬ��ɶ˿ڹ���������
				hRet = CreateIoCompletionPort( 
					( HANDLE )lp_io->socket,
					lp_this->m_h_iocp,
					(DWORD)lp_new_key,0 );

				if( NULL == hRet )
				{
					closesocket( lp_io->socket );

					lp_this->m_key_group.RemoveAt( lp_new_key );
					lp_this->m_io_group.RemoveAt( lp_io );

					cout<<"link to iocp fail!"<<WSAGetLastError()<<endl;

					continue;
				}

				//�����ȡ��������
				if( dwBytes > 0 )
				{
					lp_io->wsaBuf.len = dwBytes;

					lp_this->HandleData( lp_io, IOCP_COMPLETE_ACCEPT_READ );

					bRet = lp_this->DataAction( lp_io, lp_new_key );

					if( FALSE == bRet )
					{
						continue;
					}
				}
				else
				{
					lp_this->HandleData( lp_io, IOCP_COMPLETE_ACCEPT );

					bRet = lp_this->DataAction( lp_io, lp_new_key );

					if( FALSE == bRet )
					{
						continue;
					}
				}
			}
			break;
		case IOCP_READ:
			{
				if( SOCKET_STATE_CONNECT_AND_READ != lp_io->state )
				{
					lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
				}
				cout<<"shanlihou:"<<lp_io->buf<<endl;
				lp_io->wsaBuf.len = dwBytes;

				lp_this->HandleData( lp_io, IOCP_COMPLETE_READ );
				
				bRet = lp_this->DataAction( lp_io, lp_new_key );
				
				if( FALSE == bRet )
				{
					continue;
				}
			}
			break;
		case IOCP_WRITE:
			{
				lp_this->HandleData( lp_io, IOCP_COMPLETE_WRITE );
				
				bRet = lp_this->DataAction( lp_io, lp_new_key );
				
				if( FALSE == bRet )
				{
					continue;
				}
			}
			break;
		default:
			{
				continue;
			}
			break;
		}
	}

	return 0;
}
