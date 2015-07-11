//
// Connection.cc
//
// Connection: This class forms a easy to use interface to the berkeley
//             tcp socket library. All the calls are basically the same, 
//             but the parameters do not have any stray _addr or _in
//             mixed in...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Connection.cpp,v 1.7 2006/09/11 07:09:47 root Exp $
//

#include "MyConnection.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _MSC_VER /* _WIN32 */
#include <windows.h>
#include <winsock.h>
#define EALREADY     WSAEALREADY
#define EISCONN      WSAEISCONN
#else
#include <sys/socket.h>
#include <arpa/inet.h>	// For inet_ntoa
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#endif

#ifndef _MSC_VER /* _WIN32 */
#include <sys/file.h>
#include <sys/time.h>
#else
#include <io.h>
#endif

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#include <strings.h>
#include <sys/select.h>

#include "util.h"
#include "ic_types.h"
#include"InfoCrawler.h"
typedef void (*SIGNAL_HANDLER) (...);

#ifndef _MSC_VER /* _WIN32 */
extern "C" {
    int rresvport(int *);
}
#endif

#undef MIN
#define	MIN(a,b)		((a)<(b)?(a):(b))

//List	all_connections;

//*************************************************************************
// Connection::Connection(int socket)
// - Default constructor
// PURPOSE:
//   Create a connection from just a socket.
// PARAMETERS:
//   int socket:  obvious!!!!
//
//*************************************************************************
MyConnection::MyConnection(int socket)
: pos(0), pos_max(0),
   sock(socket), connected(0), peer(""), server_name(""), server_ip_address(""),
   need_io_stop(0), timeout_value(0), retry_value(1),
   wait_time(5)// wait 5 seconds after a failed connection attempt
{
   buffer[0] = 0;
   Win32Socket_Init();

   if (socket > 0)
   {
      socklen_t length = sizeof(server);
      if (getpeername(socket, (struct sockaddr *)&server, &length) < 0)
      	 perror("getpeername");
   }
   
   //all_connections.Add(this);
}

// Copy constructor
MyConnection::MyConnection(const MyConnection& rhs)
: pos(rhs.pos), pos_max(rhs.pos_max),
   sock(rhs.sock), connected(rhs.connected),
   peer(rhs.peer), server_name(rhs.server_name), server_ip_address(rhs.server_ip_address),
   need_io_stop(rhs.need_io_stop), timeout_value(rhs.timeout_value),
   retry_value(rhs.retry_value),
   wait_time(rhs.wait_time) // wait 5 seconds after a failed connection attempt
{
   buffer[0] = 0;
    //all_connections.Add(this);
}


//*****************************************************************************
// Connection::~Connection()
//
MyConnection::~MyConnection()
{
    //all_connections.Remove(this);
    this->Close();
}


//*****************************************************************************
// int Connection::Win32Socket_init(void)
//
// This function is only used when Code is compiled as a Native Windows
// application
// 
// The native Windows socket system needs to be initialized.
//
int MyConnection::Win32Socket_Init(void)
{
#ifdef _MSC_VER /* _WIN32 */
    WORD    wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup(wVersionRequested, &wsaData))
        return(-1);

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 ) {
        WSACleanup();
        return(-1);
    }
#endif

    return(0);
}
//*****************************************************************************
// int Connection::Open(int priv)
//
int MyConnection::Open(int priv)
{
    if (priv)
    {
	int	aport = IPPORT_RESERVED - 1;

//  Native Windows (MSVC) has no rresvport
#ifndef _MSC_VER /* _WIN32 */
	sock = rresvport(&aport);
#else
	return NOTOK;
#endif
    }
    else
    {
	    sock = socket(AF_INET, SOCK_STREAM, 0);
	//cout << "socket()  sock=" << sock << endl;
    }

    if (sock == NOTOK)
	return NOTOK;

    int	on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    server.sin_family = AF_INET;

    return OK;
}


//*****************************************************************************
// int Connection::Ndelay()
//
int MyConnection::Ndelay()
{
#ifndef _MSC_VER /* _WIN32 */
    return fcntl(sock, F_SETFL, FNDELAY);
#else
    // Note:  This function is never called
    // TODO: Look into ioctsocket(..) of Win32 Socket API
    return(0);
#endif
}


//*****************************************************************************
// int Connection::Nondelay()
//
int MyConnection::Nondelay()
{
#ifndef _MSC_VER /* _WIN32 */
    return fcntl(sock, F_SETFL, 0);
#else
    // Note:  This function is never called
    // TODO: Look into ioctsocket(..) of Win32 Socket API
    return(0);
#endif
}

//*****************************************************************************
// int Connection::Timeout(int value)
//
int MyConnection::Timeout(int value)
{
    int oval = timeout_value;
    timeout_value = value;
    return oval;
}

//*****************************************************************************
// int Connection::retries(int value)
//
int MyConnection::Retries(int value)
{
    int oval = retry_value;
    retry_value = value;
    return oval;
}

//*****************************************************************************
// int Connection::Close()
//
int MyConnection::Close()
{
    connected = 0;
    if (sock >= 0)
    {
	int ret = close(sock);
	sock = -1;
	return ret;
    }
    return NOTOK;
}


//*****************************************************************************
// int Connection::Assign_Port(int port)
//
int MyConnection::Assign_Port(int port)
{
    server.sin_port = htons(port);
    return OK;
}


//*****************************************************************************
// int Connection::Assign_Port(char *service)
//
int MyConnection::Assign_Port(char *service)
{
    struct servent		*sp;

    sp = getservbyname(service, "tcp");
    if (sp == 0)
    {
	return NOTOK;
    }
    server.sin_port = sp->s_port;
    return OK;
}

//*****************************************************************************
// int Connection::Assign_Server(unsigned int addr)
//
int MyConnection::Assign_Server(unsigned int addr)
{
    server.sin_addr.s_addr = addr;
    return OK;
}

#ifndef _MSC_VER /* _WIN32 */
//extern "C" unsigned int   inet_addr(char *);
#endif

//*****************************************************************************
//
int MyConnection::Assign_Server(char *name)
{
    struct hostent *hp;
    char **alias_list;
    unsigned int addr;

    //
    // inet_addr arg IS const char even though prototype says otherwise
    //
    addr = inet_addr(name);
    if (addr == (unsigned int)~0)
    {
        // Gets the host given a string

        int ret = 0; 
        int my_err = 1; 
        char buf[8192]; 
        struct hostent host_ent, *host_ent_result = NULL; 
        ret = gethostbyname_r(name, &host_ent, buf, 8192, &host_ent_result, &my_err); 
        hp = host_ent_result;

        if (ret != 0 || host_ent_result == NULL) {
           return NOTOK;
        }
        //hp = gethostbyname(name);

        /*if (hp == 0)
           return NOTOK;
           */

        alias_list = hp->h_aliases;
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    }
    else
    {
        memcpy((char *)&server.sin_addr, (char *)&addr, sizeof(addr));
    }

    server_name = name;
    server_ip_address = inet_ntoa(server.sin_addr);

    return OK;
}

//
// Do nothing, we are only interested in the EINTR return of the
// running system call.
//
static void handler_timeout(int) {
}

//*****************************************************************************
// int Connection::Connect()
//
int MyConnection::Connect()
{
    int	status;
    int retries = retry_value;

    while (retries--)
      {
#ifndef _MSC_VER /* _WIN32 */
	//
	// Set an alarm to make sure the connect() call times out
	// appropriately This ensures the call won't hang on a
	// dead server or bad DNS call.
	// Save the previous alarm signal handling policy, if any.
	//
	struct sigaction action;
	struct sigaction old_action;
	memset((char*)&action, '\0', sizeof(struct sigaction));
	memset((char*)&old_action, '\0', sizeof(struct sigaction));
	action.sa_handler = handler_timeout;
	sigaction(SIGALRM, &action, &old_action);
	alarm(timeout_value);
#endif

	status = connect(sock, (struct sockaddr *)&server, sizeof(server));

	//
	// Disable alarm and restore previous policy if any
	//
#ifndef _MSC_VER /* _WIN32 */
	alarm(0);
       	sigaction(SIGALRM, &old_action, 0);
#endif

	if (status == 0 || errno == EALREADY || errno == EISCONN)
	  {
	    connected = 1;
	    return OK;
	  }

	//
	// Only loop if timed out. Other errors are fatal.
	//
	if (status < 0 && (errno == EINTR || errno == EAGAIN))
	  break;
	
	// cout << " <"  << ::strerror(errno) << "> ";
	close(sock);
        Open();

        sleep(wait_time);

      }

#if 0
    if (status == ECONNREFUSED)
    {
	//
	// For the case where the connection attempt is refused, we need
	// to close the socket and create a new one in order to do any
	// more with it.
	//
	Close(sock);
	Open();
    }
#else
    close(sock);
    Open(0);
#endif

    connected = 0;
    return NOTOK;
}


//*****************************************************************************
// int Connection::Bind()
//
int MyConnection::Bind()
{
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == NOTOK)
    {
	return NOTOK;
    }
    return OK;
}


//*****************************************************************************
// int Connection::Get_Port()
//
int MyConnection::getPort() {
    return ntohs(server.sin_port);
}
int MyConnection::Get_Port()
{
    socklen_t length = sizeof(server);
    
    if (getsockname(sock, (struct sockaddr *)&server, &length) == NOTOK)
    {
	return NOTOK;
    }
    return ntohs(server.sin_port);
}


//*****************************************************************************
// int Connection::Listen(int n)
//
int MyConnection::Listen(int n)
{
    return listen(sock, n);
}


//*****************************************************************************
// Connection *Connection::Accept(int priv)
//
MyConnection *MyConnection::Accept(int priv)
{
    int	newsock;

    while (1)
    {
	newsock = accept(sock, (struct sockaddr *)0, (socklen_t *)0);
	if (newsock == NOTOK && (errno == EINTR || errno == EAGAIN))
	    continue;
	break;
    }
    if (newsock == NOTOK)
	return (MyConnection *)0;

    MyConnection	*newconnect = new MyConnection;
    newconnect->sock = newsock;

    socklen_t length = sizeof(newconnect->server);
    getpeername(newsock, (struct sockaddr *)&newconnect->server, &length);

    if (priv && newconnect->server.sin_port >= IPPORT_RESERVED)
    {
	delete newconnect;
	return (MyConnection *)0;
    }

    return newconnect;
}


//*************************************************************************
// Connection *Connection::Accept_Privileged()
// PURPOSE:
//   Accept  in  incoming  connection  but  only  if  it  is  from a
//   privileged port
//
MyConnection * MyConnection::Accept_Privileged()
{
    return Accept(1);
}

//*****************************************************************************
// int Connection::read_char()
//
int MyConnection::Read_Char()
{
    if (pos >= pos_max)
    {
	pos_max = Read_Partial(buffer, sizeof(buffer));
	pos = 0;
	if (pos_max <= 0)
	{
	    return -1;
	}
    }
    return buffer[pos++] & 0xff;
}


//*****************************************************************************
// String *Connection::Read_Line(String &s, char *terminator)
//
string *MyConnection::Read_Line(string &s, char *terminator)
{
    int		termseq = 0;

    for (;;)
    {
        int	ch = Read_Char();
        if (ch < 0)
        {
            //
            // End of file reached.  If we still have stuff in the input buffer
            // we need to return it first.  When we get called again we will
            // return 0 to let the caller know about the EOF condition.
            //
            if (s.length())
            break;
            else
            return &s;
        }
        else if (terminator[termseq] && ch == terminator[termseq])
        {
            //
            // Got one of the terminator characters.  We will not put
            // it in the string but keep track of the fact that we
            // have seen it.
            //
            termseq++;
            if (!terminator[termseq])
            break;
        }
        else
        {
            s.append(1, (char) ch);
        }
    }

    return &s;
}


//*****************************************************************************
// String *Connection::read_line(char *terminator)
//
string *MyConnection::Read_Line(char *terminator)
{
    string	*s;

    s = new string;
    return Read_Line(*s, terminator);
}


//*****************************************************************************
// char *Connection::read_line(char *buffer, int maxlength, char *terminator)
//
char *MyConnection::Read_Line(char *buffer, int maxlength, char *terminator)
{
    char	*start = buffer;
    int		termseq = 0;

    while (maxlength > 0)
    {
	int		ch = Read_Char();
	if (ch < 0)
	{
	    //
	    // End of file reached.  If we still have stuff in the input buffer
	    // we need to return it first.  When we get called again, we will
	    // return 0 to let the caller know about the EOF condition.
	    //
	    if (buffer > start)
		break;
	    else
		return (char *) 0;
	}
	else if (terminator[termseq] && ch == terminator[termseq])
	{
	    //
	    // Got one of the terminator characters.  We will not put
	    // it in the string but keep track of the fact that we
	    // have seen it.
	    //
	    termseq++;
	    if (!terminator[termseq])
		break;
	}
	else
	{
	    *buffer++ = ch;
	    maxlength--;
	}
    }
    *buffer = '\0';

    return start;
}


//*****************************************************************************
// int Connection::write_line(char *str, char *eol)
//
int MyConnection::Write_Line(char *str, char *eol)
{
    int		n, nn;

    if ((n = Write(str)) < 0)
	return -1;

    if ((nn = Write(eol)) < 0)
	return -1;

    return n + nn;
}


//*****************************************************************************
// int Connection::Write(char *buffer, int length)
//
int MyConnection::Write(char *buffer, int length)
{
    int nleft, nwritten;

    if (length == -1)
	length = strlen(buffer);

    nleft = length;
    while (nleft > 0)
    {
        nwritten = Write_Partial(buffer, nleft);
        if (nwritten < 0 && (errno == EINTR || errno == EAGAIN))
            continue;
        if (nwritten <= 0)
            return nwritten;
        nleft -= nwritten;
        buffer += nwritten;
    }
    return length - nleft;
}


//*****************************************************************************
// int Connection::Read(char *buffer, int length)
//
int MyConnection::Read(char *buffer, int length, int *ret)
{
    int nleft, nread;

    nleft = length;

    //
    // If there is data in our internal input buffer, use that first.
    //
    if (pos < pos_max)
    {
        int n = MIN(length, pos_max - pos);

        //memcpy(buffer, &this->buffer[pos], n);
        memmove(buffer, &this->buffer[pos], n);
        //VALGRIND_PRINTF("%d read ack from buffer \n", pthread_self());
        pos += n;
        buffer += n;
        nleft -= n;
    }
    

    while (nleft > 0)
    {
    //VALGRIND_PRINTF("%d read ack not from buffer left = %d\n", pthread_self(), nleft);
	nread = Read_Partial(buffer, nleft);
    //VALGRIND_PRINTF("%d read ack not from buffer nread = %d %d\n", pthread_self(), nread, errno);
	if (nread < 0 && (errno == EINTR || errno == EAGAIN))
	    continue;
	if (nread < 0) {
        if (ret)
            *ret = length - nleft;
	    return -1;
    }
	else if (nread == 0) {
	    break;
    }

	nleft -= nread;
	buffer += nread;
    }
    /*if((length - nleft) != length)
    {
       errorlog("ERROR:Read socket error %d , read length %d\n", errno, length );
    }*/
    return length - nleft;
}


void MyConnection::Flush()
{
   pos = pos_max = 0;
}

//*************************************************************************
// int Connection::Read_Partial(char *buffer, int maxlength)
// PURPOSE:
//   Read  at  most  <maxlength>  from  the  current TCP connection.
//   This  is  equivalent  to  the  workings  of the standard read()
//   system call
// PARAMETERS:
//   char *buffer:	Buffer to read the data into
//   int maxlength:	Maximum number of bytes to read into the buffer
// RETURN VALUE:
//   The actual number of bytes read in.
// ASSUMPTIONS:
//   The connection has been previously established.
// FUNCTIONS USED:
//   read()
//
int MyConnection::Read_Partial(char *buffer, int maxlength)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int		count;

    need_io_stop = 0;
    int trynum = 0;
    do
    {
      errno = 0;

      if (timeout_value > 0) {
          fd_set fds;
          FD_ZERO(&fds);
          FD_SET(sock, &fds);

          timeval tv;
          tv.tv_sec = timeout_value;
          tv.tv_usec = 0;

          int selected = select(sock+1, &fds, 0, 0, &tv);
          //VALGRIND_PRINTF("%d select number = %d\n", pthread_self(), selected);
          if (selected <= 0)
              need_io_stop++;
      }

      if (!need_io_stop) {
          count = recv(sock, buffer, maxlength, 0);
          //log_errno("recv \n");
      }
      else
          count = -1;         // Input timed out
    }
    while (count <= 0 && (errno == EINTR || errno == EAGAIN) && !need_io_stop && trynum++ < 20);
    //VALGRIND_PRINTF("%d errno = %d\n", errno, pthread_self());
    need_io_stop = 0;

    if(count< 0)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "Read socket error %d:%s , Read length %d - %s:%s:%d",errno, strerror(errno), count,INFO_LOG_SUFFIX);
    }
    return count;
}


//*************************************************************************
// int Connection::Write_Partial(char *buffer, int maxlength)
//
int MyConnection::Write_Partial(char *buffer, int maxlength)
{
    LogGlobalCtrl *m_pLogGlobalCtrl = InfoCrawler::getInstance()->getLogGlobalCtrl();
    int		count;
    int need_io_stop = 0;

    do
    {
 	    //count = send(sock, buffer, maxlength, MSG_NOSIGNAL | MSG_DONTWAIT);
 	    count = send(sock, buffer, maxlength, 0);
    }
    while (count < 0 && (errno == EINTR || errno == EAGAIN) && !need_io_stop);
    need_io_stop = 0;

    if(count< 0)
    {
        mylog_error(m_pLogGlobalCtrl->errorlog, "write socket error %d:%s , write length %d - %s:%s:%d",errno, strerror(errno), count,INFO_LOG_SUFFIX);
    }
    return count;
}


//*************************************************************************
// char * Connection::Socket_as_String()
// PURPOSE:
//   Return  the  numeric  ASCII  equivalent  of  the socket number.
//   This is needed to pass the socket to another program
//
char * MyConnection::Socket_as_String()
{
    char	*buffer = new char[20];

    sprintf(buffer, "%d", sock);
    return buffer;
}

#ifndef _MSC_VER /* _WIN32 */
extern "C" char *inet_ntoa(struct in_addr);
#endif

//*************************************************************************
// char *Connection::Get_Peername()
//
const char* MyConnection::Get_Peername()
{
    if (peer.empty())
    {
	struct sockaddr_in	p;
	socklen_t length = sizeof(p);
	struct hostent		*hp;
	
	if (getpeername(sock, (struct sockaddr *) &p, &length) < 0)
	{
	    return 0;
	}
	
	length = sizeof(p.sin_addr);
	hp = gethostbyaddr((const char *) &p.sin_addr, length, AF_INET);
	if (hp)
	    peer = (char *) hp->h_name;
	else
	    peer = (char *) inet_ntoa(p.sin_addr);
    }
    return (const char*) peer.c_str();
}


//*************************************************************************
// char *Connection::Get_PeerIP()
//
const char* MyConnection::Get_PeerIP() const
{
    struct sockaddr_in	p;
    socklen_t length = sizeof(p);
    
    if (getpeername(sock, (struct sockaddr *) &p, &length) < 0)
    {
	return 0;
    }
    return (const char*) inet_ntoa(p.sin_addr);
}

#ifdef NEED_PROTO_GETHOSTNAME
extern "C" int gethostname(char *name, int namelen);
#endif

//*************************************************************************
// unsigned int GetHostIP(char *ip, int length)
//
unsigned int GetHostIP(char *ip, int length)
{
    char	hostname[100];
    if (gethostname(hostname, sizeof(hostname)) == NOTOK)
	return 0;

    struct hostent	*ent = NULL;
    //gethostbyname(hostname);

    int ret = 0; 
    int my_err = 1; 
    char buf[4096]; 
    struct hostent host_ent, *host_ent_result = NULL; 
    ret = gethostbyname_r(hostname, &host_ent, buf, 4096, &host_ent_result, &my_err); 
    ent = host_ent_result;

    if (ret != 0 || host_ent_result == NULL) {
       return 0;
    }

    /*if (!ent)
	return 0;
    */

    struct in_addr	addr;
    memcpy((char *) &addr.s_addr, ent->h_addr, sizeof(addr));
    if (ip)
	strncpy(ip, inet_ntoa(addr), length);
    return addr.s_addr;
}



//*************************************************************************
// int Connection::WaitTime(unsigned int _wt)
//
int MyConnection::WaitTime(unsigned int _wt)
{
   wait_time = _wt;
   return OK;
}
