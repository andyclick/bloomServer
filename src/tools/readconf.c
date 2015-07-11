#include <stdio.h> 
 #include <stdlib.h> 
 #include <ctype.h> 
 #include <string.h> 
 #include "readconf.h" 
 
 /* Set line length in configuration files */  
 #define CFG_LINE 2048 
 static int readline(char* line, FILE* stream) 
 { 
 int flag = 1; 
 char buf[CFG_LINE]; 
 int i, k = 0; 
 
 if (fgets(buf, CFG_LINE, stream) != NULL ) { 
 
 /* Delete the last '\r' or '\n' or ' ' or '\t' character */  
 for (i = strlen(buf) - 1; i >= 0; i--) { 
 if ( buf[i] == '\r' || buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\t' ) 
 buf[i] = '\0'; 
 else 
 break; /* dap loop */ 
 } 
 
 /* Delete the front '\r' or '\n' or ' ' or '\t' character */ for (i = 0; i <= 
 strlen(buf); i++) { if ( flag && (buf[i] == '\r' || buf[i] == '\n' || buf[i] == 
 ' ' || buf[i] == '\t') ) continue; else { flag = 0; line[k++] = buf[i]; } } 
  
 return 0; 
 } 
 
 return -1; 
 } 
 
 
 static int isremark(const char* line) 
 { 
 int i; 
 
 for (i = 0; i < strlen(line); i++) { 
 
 if ( isgraph(line[i]) ) { 
 if ( line[i] == '#' ) return 1; 
 else return 0; 
 } 
 } 
 
 return 1; 
 } 
 
 
 static int getsection(const char* line, char* section) 
 { 
 int start, end; 
 
 for (start = 0; start < strlen(line); start++) { 
 
 if ( line[start] != '[' ) return -1; /* fail */ 
 else break; 
 } 
 
 for (end = strlen(line) - 1; end > 1; end--) { 
 
 if ( line[end] != ']' ) return -1; /* fail */ 
 else break; 
 } 
 
 if ( end - start < 2 )  
 return -1; 
 
 memcpy(section, line + start + 1, end - start - 1); 
 section[end - start - 1] = '\0'; 
 return 0; 
 } 
 
 
 static int getkey(const char* line, char* keyname, char* keyvalue) 
 { 
 int i, start; 
 
 /* Find key name */ 
 for (start = 0; start < strlen(line); start++) { 
 /* Find '=' character */ 
 if ( line[start] == '=' ) break; 
 } 
 if ( start >= strlen(line) ) 
 return -1; /* not find '=', return */ 
 
 memcpy(keyname, line, start); 
 keyname[start] = '\0'; 
 
 /* Delete the last '\t' or ' ' character */ 
 for (i = strlen(keyname) - 1; i >= 0; i--) { 
 if ( keyname[i] == ' ' || keyname[i] == '\t' ) keyname[i] = '\0'; 
 else break; 
 } 
 
 /* Find key value */ 
 for (start = start + 1; start < strlen(line); start++) { 
 if ( line[start] != ' ' && line[start] != '\t' )  
 break;  
 } 
 
 strcpy(keyvalue, line + start); 
 
 /* Delete the last '\t' or ' ' character */ 
 for (i = strlen(keyvalue) - 1; i >= 0; i--) { 
 if ( keyvalue[i] == ' ' || keyvalue[i] == '\t' ) keyname[i] = '\0'; 
 else break; 
 } 
 
 return 0;  
 } 
 
 
 int getconfigstr(const char*  section, 
 const char*  keyname, 
 char*    keyvalue, 
 unsigned int len, 
 const char*  filename) 
 { 
 int step = 0; 
 FILE* stream; 
 char sec[CFG_LINE] = ""; 
 char ken[CFG_LINE] = ""; 
 char kev[CFG_LINE] = ""; 
 char line[CFG_LINE] = ""; 
 
 if( (stream = fopen(filename, "r") ) == NULL ) 
 return CFG_NOFILE; 
 
 while ( !feof(stream) ) { 
 
 if ( readline(line, stream) == -1 ) { 
 fclose(stream); 
 return CFG_NOFIND; 
 } 
 
 if ( !isremark(line) ) { 
 
 if ( step == 0 ) { /* first, find section */ 
 if ( getsection(line, sec) == 0 ) { 
 if ( strcmp(sec, section) == 0 ) step = 1; 
 } 
 }  
 else { /* second, find keyname, keyvalue */ 
 if ( getkey(line, ken, kev) == 0 ) { 
 if ( strcmp(ken, keyname) == 0 ) { 
 strncpy(keyvalue, kev, len); 
 fclose(stream); 
 return CFG_NOERROR; 
 }  
 } 
 } 
 
 } /* end isremark() */ 
 
 } /* end while */ 
 
 fclose(stream);  
 return CFG_NOFIND; 
 } 
 
 
 int getconfigint(const char*  section, 
 const char*  keyname, 
 int*     keyvalue, 
 const char*  filename) 
 { 
 int rs; 
 char kev[20]; 
 
 memset(kev, 0, sizeof(kev)); 
 rs = getconfigstr(section, keyname, kev, sizeof(kev), filename); 
 if ( rs == CFG_NOERROR )  
 *keyvalue = atoi(kev); 
 return rs; 
 } 
 
