//
// Connection.h
//
// Connection: This class forms a easy to use interface to the berkeley
//          tcp socket library. All the calls are basically the same, 
//          but the parameters do not have any stray _addr or _in
//          mixed in...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Connection.h,v 1.3 2006/07/24 11:23:52 root Exp $
//

#ifndef _Connection_h_
#define	_Connection_h_

#include <stdlib.h>
#include <sys/types.h>

#ifdef _MSC_VER /* _WIN32 */
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

/* According to POSIX 1003.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
using namespace std;

#define OK      0
#define NOTOK       (-1)

class MyConnection 
{
public:
   // Constructors & Destructors
   MyConnection(int socket = -1);  // Default constructor
   MyConnection(const MyConnection& rhs);  // Copy constructor
   ~MyConnection();

   // (De)initialization
   int Win32Socket_Init(void);
   int Open(int priv = 0);
   virtual int Close();
   int Ndelay();
   int Nondelay();
   int Timeout(int value);
   int Retries(int value);
   int WaitTime(unsigned int _wt);

   // Port stuff
   int Assign_Port(int port = 0);
   int Assign_Port(char *service);
   int Get_Port();
   inline int Is_Privileged();

   // Host stuff
   int Assign_Server(char *name);
   int Assign_Server(unsigned int addr = INADDR_ANY);
   string Get_Server() const { return server_name; }
   string Get_Server_IPAddress() const { return server_ip_address; }

   // Connection establishment
   virtual int Connect();
   MyConnection *Accept(int priv = 0);
   MyConnection *Accept_Privileged();

   // Registration things
   int Bind();
   int Listen(int n = 5);

   // IO
   string* Read_Line(string &, char *terminator = (char *)"\n");
   char* Read_Line(char *buffer, int maxlength, char *terminator=(char *)"\n");
   string* Read_Line(char *terminator = (char *)"\n");
   virtual int Read_Char();
   int Write_Line(char *buffer, char *eol = (char *)"\n");
   
   int Write(char *buffer, int maxlength = -1);
   int Read(char *buffer, int maxlength, int *ret = NULL);
   
   virtual int Read_Partial(char *buffer, int maxlength);
   virtual int Write_Partial(char *buffer, int maxlength);
   void Stop_IO() {need_io_stop = 1;}

   // Access to socket number
   char *Socket_as_String();
   int Get_Socket() { return sock; }
   int IsOpen() { return sock >= 0; }
   int IsConnected() { return connected; }

   // Access to info about remote socket
   const char* Get_PeerIP() const;
   const char* Get_Peername();

   // A method to re-initialize the buffer
   virtual void Flush();
    int getPort(); 

private:
   //
   // For buffered IO we will need a buffer
   //
   enum           {BUFFER_SIZE = 8192};
   char           buffer[BUFFER_SIZE];
   int            pos, pos_max;
   // Assignment operator declared private for preventing any use
   MyConnection& operator+ (const MyConnection& rhs) { return *this; }

protected:
   int            sock;
   struct         sockaddr_in server;
   int            connected;
   string         peer;
   string         server_name;
   string         server_ip_address;
   int            need_io_stop;
   int            timeout_value;
   int            retry_value;
   unsigned int   wait_time;  // time to wait after an
      	             	      // unsuccessful connection
};


//*************************************************************************
// inline int Connection::Is_Privileged()
// PURPOSE:
//   Return whether the port is priveleged or not.
//
inline int MyConnection::Is_Privileged()
{
   return server.sin_port < 1023;
}


//
// Get arround the lack of gethostip() library call...  There is a gethostname()
// call but we want the IP address, not the name!
// The call will put the ASCII string representing the IP address in the supplied
// buffer and it will also return the 4 byte unsigned long equivalent of it.
// The ip buffer can be null...
//
unsigned int gethostip(char *ip = 0, int length = 0);

#endif
