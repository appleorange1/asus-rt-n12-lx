/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright ?1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <ctype.h>		//Added by Jerry

typedef unsigned int __u32;   // 1225 ham
#include <httpd.h>
//2008.08 magic{
#include <nvram/bcmnvram.h>	//2008.08 magic
#include <arpa/inet.h>	//2008.08 magic
#include <apmib.h>	//Added by Jerry
#include <sys/stat.h>

#define csprintf(fmt, args...) do{\
	FILE *cp = fopen("/dev/console", "w");\
	if(cp) {\
		fprintf(cp, fmt, ## args);\
		fclose(cp);\
	}\
}while(0)

#define eprintf2(fmt, args...) do{\
	FILE *ffp = fopen("/tmp/detect_wrong.log", "a+");\
	if (ffp) {\
		fprintf(ffp, fmt, ## args);\
		fclose(ffp);\
	}\
}while (0)

//2008.08 magic}

#ifdef vxworks
static void fcntl(int a, int b, int c) {}
#include <signal.h>
#include <ioLib.h>
#include <sockLib.h>
extern int snprintf(char *str, size_t count, const char *fmt, ...);
extern int strcasecmp(const char *s1, const char *s2); 
extern int strncasecmp(const char *s1, const char *s2, size_t n); 
extern char *strsep(char **stringp, char *delim);
#define socklen_t 		int
#define main			milli
#else
#include <error.h>
#include <sys/signal.h>
#endif

// Added by Joey for ethtool
#include <net/if.h>
#include <ethtool-util.h>
#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

#define ETCPHYRD	14
#define SIOCGETCPHYRD   0x89FE
#define SERVER_NAME "httpd"
#define SERVER_PORT 80
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

//Added by Jerry
char *WAN_IF;
char *BRIDGE_IF;
char *ELAN_IF;
char *ELAN2_IF;
char *ELAN3_IF;
char *ELAN4_IF;
char *PPPOE_IF;
char WLAN_IF[20];
int wlan_num;
int vwlan_num=0;
int mssid_idx=0;
//Added by Jerry
extern void notify_sysconf(const char *event_name, int do_wait);

int fwupgrade_flag = 0;	//2011.04.14 Jerry

/* A multi-family sockaddr. */
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    } usockaddr;

/* Globals. */
static FILE *conn_fp;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_realm[AUTH_MAX];
#ifdef TRANSLATE_ON_FLY
char Accept_Language[16];
#endif //TRANSLATE_ON_FLY

/* Forwards. */
static int initialize_listen_socket( usockaddr* usaP );
static int auth_check( char* dirname, char* authorization, char* url);
static void send_authenticate( char* realm );
static void send_error( int status, char* title, char* extra_header, char* text );
static void send_headers( int status, char* title, char* extra_header, char* mime_type );
static int b64_decode( const char* str, unsigned char* space, int size );
static int match( const char* pattern, const char* string );
static int match_one( const char* pattern, int patternlen, const char* string );
static void handle_request(void);

/* added by Joey */
//2008.08 magic{
int redirect = 0;	
int change_passwd = 0;	
int reget_passwd = 0;	
char url[128];
//2008.08 magic}
char wan_if[16];
int http_port=SERVER_PORT;

/* Added by Joey for handle one people at the same time */
unsigned int login_ip=0; // the logined ip
time_t login_timestamp=0; // the timestamp of the logined ip
unsigned int login_ip_tmp=0; // the ip of the current session.
unsigned int login_try=0;
unsigned int last_login_ip = 0;	// the last logined ip 2008.08 magic

// 2008.08 magic {
time_t request_timestamp = 0;
time_t turn_off_auth_timestamp = 0;
int temp_turn_off_auth = 0;	// for QISxxx.htm pages

void http_login(unsigned int ip, char *url);
void http_login_timeout(unsigned int ip);
void http_logout(unsigned int ip);

#include <sys/sysinfo.h>
long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

static int
initialize_listen_socket( usockaddr* usaP )
    {
    int listen_fd;
    int i;

    memset( usaP, 0, sizeof(usockaddr) );
    usaP->sa.sa_family = AF_INET;
    usaP->sa_in.sin_addr.s_addr = htonl( INADDR_ANY );
    usaP->sa_in.sin_port = htons( http_port );

    listen_fd = socket( usaP->sa.sa_family, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
	{
	perror( "socket" );
	return -1;
	}
    (void) fcntl( listen_fd, F_SETFD, 1 );
    i = 1;
    if ( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &i, sizeof(i) ) < 0 )
	{
	close(listen_fd);	// 1104 chk
	perror( "setsockopt" );
	return -1;
	}
    if ( bind( listen_fd, &usaP->sa, sizeof(struct sockaddr_in) ) < 0 )
	{
	close(listen_fd);	// 1104 chk
	perror( "bind" );
	return -1;
	}
    if ( listen( listen_fd, 1024 ) < 0 )
	{
	close(listen_fd);	// 1104 chk
	perror( "listen" );
	return -1;
	}
    return listen_fd;
    }


static int
auth_check( char* dirname, char* authorization ,char* url)
    {
    char authinfo[500];
    char* authpass;
    int l;
    /* Is this directory unprotected? */
    if ( !strlen(auth_passwd) )
	/* Yes, let the request go through. */
	return 1;

    /* Basic authorization info? */
    if ( !authorization || strncmp( authorization, "Basic ", 6 ) != 0) 
    {
	send_authenticate( dirname );
		if (login_ip != 0)	// 2008.08 magic
			http_logout(login_ip_tmp);
	
		last_login_ip = 0;	// 2008.08 magic
	return 0;
    }

    /* Decode it. */
    l = b64_decode( &(authorization[6]), authinfo, sizeof(authinfo) );
    authinfo[l] = '\0';
    /* Split into user and password. */
    authpass = strchr( authinfo, ':' );
    if ( authpass == (char*) 0 ) {
	/* No colon?  Bogus auth info. */
	send_authenticate( dirname );
	http_logout(login_ip_tmp);
	return 0;
    }
    *authpass++ = '\0';

    /* Is this the right user and password? */
    if ( strcmp( auth_userid, authinfo ) == 0 && strcmp( auth_passwd, authpass ) == 0 )
    {
    	/* Is this is the first login after logout */
    	if (login_ip==0 && last_login_ip==login_ip_tmp)
    	{
		send_authenticate(dirname);
		last_login_ip=0;
		return 0;
    	}
	return 1;
    }

    send_authenticate( dirname );
    http_logout(login_ip_tmp);
    return 0;
    }


static void
send_authenticate( char* realm )
    {
    char header[10000];

    (void) snprintf(
	header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", realm );
    send_error( 401, "Unauthorized", header, "Authorization required." );
    }


static void
send_error( int status, char* title, char* extra_header, char* text )
    {
    send_headers( status, title, extra_header, "text/html" );
    (void) fprintf( conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title );
    (void) fprintf( conn_fp, "%s\n", text );
    (void) fprintf( conn_fp, "</BODY></HTML>\n" );
    (void) fflush( conn_fp );
    }


static void
send_headers( int status, char* title, char* extra_header, char* mime_type )
    {
    time_t now;
    char timebuf[100];

    (void) fprintf( conn_fp, "%s %d %s\r\n", PROTOCOL, status, title );
    (void) fprintf( conn_fp, "Server: %s\r\n", SERVER_NAME );
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    (void) fprintf( conn_fp, "Date: %s\r\n", timebuf );	//Uncomment by Jerry
    if ( extra_header != (char*) 0 )
	(void) fprintf( conn_fp, "%s\r\n", extra_header );
    if ( mime_type != (char*) 0 )
	(void) fprintf( conn_fp, "Content-Type: %s\r\n", mime_type );
    (void) fprintf( conn_fp, "Connection: close\r\n" );
    (void) fprintf( conn_fp, "\r\n" );
    }


/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
    {
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int)*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
    }


/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
** patterns separated by |.  Returns 1 or 0.
*/
int
match( const char* pattern, const char* string )
    {
    const char* or;

    for (;;)
	{
	or = strchr( pattern, '|' );
	if ( or == (char*) 0 )
	    return match_one( pattern, strlen( pattern ), string );
	if ( match_one( pattern, or - pattern, string ) )
	    return 1;
	pattern = or + 1;
	}
    }


static int
match_one( const char* pattern, int patternlen, const char* string )
    {
    const char* p;

    for ( p = pattern; p - pattern < patternlen; ++p, ++string )
	{
	if ( *p == '?' && *string != '\0' )
	    continue;
	if ( *p == '*' )
	    {
	    int i, pl;
	    ++p;
	    if ( *p == '*' )
		{
		/* Double-wildcard matches anything. */
		++p;
		i = strlen( string );
		}
	    else
		/* Single-wildcard matches anything but slash. */
		i = strcspn( string, "/" );
	    pl = patternlen - ( p - pattern );
	    for ( ; i >= 0; --i )
		if ( match_one( p, pl, &(string[i]) ) )
		    return 1;
	    return 0;
	    }
	if ( *p != *string )
	    return 0;
	}
    if ( *string == '\0' )
	return 1;
    return 0;
    }


void
do_file(char *path, FILE *stream)
{
	FILE *fp;
	int c;

	if (!(fp = fopen(path, "r")))
		return;
	while ((c = getc(fp)) != EOF)
		fputc(c, stream);
	fclose(fp);
}

time_t detect_timestamp, detect_timestamp_old, signal_timestamp;

int is_firsttime(void);

static void
handle_request(void)
{
    char line[10000]={0}, *cur=NULL;
    char *method=NULL, *path=NULL, *protocol=NULL, *authorization=NULL, *boundary=NULL, *alang=NULL;
    char *cp=NULL;
    char *file=NULL;
    int len=0;
    struct mime_handler *handler;
    int cl = 0, flags=0;

    char detect_timestampstr[32]={0};

    unsigned char lang_mib[16]={0};	//2011.04.12 Jerry

    /* Initialize the request variables. */
    authorization = boundary = NULL;
    bzero( line, sizeof line );

    /* Parse the first line of the request. */
    if ( fgets( line, sizeof(line), conn_fp ) == (char*) 0 ) {
	send_error( 400, "Bad Request", (char*) 0, "No request found." );
	return;
    }

    method = path = line;
    strsep(&path, " ");
    while (path && *path == ' ') path++;	// oleg patch
    protocol = path;
    strsep(&protocol, " ");
    while (protocol && *protocol == ' ') protocol++;    // oleg pat
    cp = protocol;
    strsep(&cp, " ");
    if ( !method || !path || !protocol ) {
	send_error( 400, "Bad Request", (char*) 0, "Can't parse request." );
	return;
    }
    cur = protocol + strlen(protocol) + 1;

#ifdef TRANSLATE_ON_FLY
	memset(Accept_Language, 0, sizeof(Accept_Language));
#endif

    /* Parse the rest of the request headers. */
    while ( fgets( cur, line + sizeof(line) - cur, conn_fp ) != (char*) 0 )
	{
		
	if ( strcmp( cur, "\n" ) == 0 || strcmp( cur, "\r\n" ) == 0 ) {
	    break;
	}
#ifdef TRANSLATE_ON_FLY
	else if ( strncasecmp( cur, "Accept-Language:",16) ==0) {
		char *p=NULL;
		struct language_table *pLang;
		char lang_buf[256]={0};
		memset(lang_buf, 0, sizeof(lang_buf));
		alang = &cur[16];
		strcpy(lang_buf, alang);
		p = lang_buf;
		while (p != NULL)
		{
			p = strtok (p, "\r\n ,;");
			if (p == NULL)  break;
			//2008.11 magic{
			int i, len=strlen(p);
										
			for (i=0;i<len;++i)
				if (isupper(p[i])) {
					p[i]=tolower(p[i]);
				}

			//2008.11 magic}
			for (pLang = language_tables; pLang->Lang != NULL; ++pLang)
			{
				if (strcasecmp(p, pLang->Lang)==0)
				{
					snprintf(Accept_Language,sizeof(Accept_Language),"%s",pLang->Target_Lang);
					if (is_firsttime ())    {
						//2011.04.12 Jerry {
						sprintf(lang_mib, "%s", Accept_Language);
						apmib_set(MIB_PREFERRED_LANG, (void *)&lang_mib);
						//2011.04.12 Jerry }
					}
					break;
				}
			}

			if (Accept_Language[0] != 0)    {
				break;
			}
			p+=strlen(p)+1;
		}

		if (Accept_Language[0] == 0)    {
			// If all language setting of user's browser are not supported, use English.	
			strcpy (Accept_Language, "EN");
				if (is_firsttime()) {					
					//2011.04.12 Jerry {
					sprintf(lang_mib, "EN");
					apmib_set(MIB_PREFERRED_LANG, (void *)&lang_mib);
					//2011.04.12 Jerry }
				}
		}
	}
#endif
	else if ( strncasecmp( cur, "Authorization:", 14 ) == 0 )
	    {
	    cp = &cur[14];
	    cp += strspn( cp, " \t" );
	    authorization = cp;
	    cur = cp + strlen(cp) + 1;
	    }
	else if (strncasecmp( cur, "Content-Length:", 15 ) == 0) {
		cp = &cur[15];
		cp += strspn( cp, " \t" );
		cl = strtoul( cp, NULL, 0 );
	}
	else if ((cp = strstr( cur, "boundary=" ))) {
	    boundary = &cp[9];
	    for ( cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++ );
	    *cp = '\0';
	    cur = ++cp;
	}

	}

    if ( strcasecmp( method, "get" ) != 0 && strcasecmp(method, "post") != 0 ) {
	send_error( 501, "Not Implemented", (char*) 0, "That method is not implemented." );
	return;
    }
    if ( path[0] != '/' ) {
	send_error( 400, "Bad Request", (char*) 0, "Bad filename." );
	return;
    }
    file = &(path[1]);
    len = strlen( file );
    if ( file[0] == '/' || strcmp( file, ".." ) == 0 || strncmp( file, "../", 3 ) == 0 || strstr( file, "/../" ) != (char*) 0 || strcmp( &(file[len-3]), "/.." ) == 0 ) {
	send_error( 400, "Bad Request", (char*) 0, "Illegal filename." );
	return;
    }

//2008.08 magic{
	if (file[0] == '\0' || file[len-1] == '/')
		file = "index.asp";	
// 2007.11 James. {
	char *query;
	int file_len;
	
	memset(url, 0, 128);
	if ((query = index(file, '?')) != NULL) {
		file_len = strlen(file)-strlen(query);
		
		strncpy(url, file, file_len);
	}
	else
	{
		strcpy(url, file);
	}
// 2007.11 James. }

	http_login_timeout(login_ip_tmp);	// 2008.07 James.
	
	if (http_port == SERVER_PORT && http_login_check() == 3) {
		int x_setting = 0;	//2011.04.22 Jerry
		apmib_get(MIB_X_SETTING, (void *)&x_setting);	//2011.04.22 Jerry
		if ((strstr(url, ".htm") != NULL && !(!strcmp(url, "error_page.htm")						
                || (strstr(url, "QIS_") != NULL && x_setting == 0 && login_ip == 0) || !strcmp(url, "gotoHomePage.htm")))
		|| (strstr(url, ".asp") != NULL && login_ip != 0)) {
			file = "Nologin.asp";			
			memset(url, 0, 128);
			strcpy(url, file);
		}
	}
	
	for (handler = &mime_handlers[0]; handler->pattern; handler++) 
	{
		if (match(handler->pattern, url))
		{
			request_timestamp = uptime();			
			int login_state = http_login_check();
			if ((login_state == 1 || login_state == 2) && (strstr(url, "QIS_") != NULL   // to avoid the interference of the other logined browser. 2008.11 magic
							|| !strcmp(url, "result_of_get_changed_status.asp")
							|| !strcmp(url, "result_of_get_changed_status_QIS.asp")
							|| !strcmp(url, "detectWAN.asp")
							|| !strcmp(url, "WPS_info.asp")
							|| !strcmp(url, "WAN_info.asp")
							|| !strcmp(url, "result_of_detect_client.asp")
							|| !strcmp(url, "start_apply.htm")
							|| !strcmp(url, "start_apply2.htm")
// 2010.09 James. {
							|| !strcmp(url, "detectWAN2.asp")
							|| !strcmp(url, "automac.asp")
							|| !strcmp(url, "setting_lan.htm")
							|| !strcmp(url, "status.asp")
// 2010.09 James. }
							)
					) {
				turn_off_auth_timestamp = request_timestamp;
				temp_turn_off_auth = 1;
				redirect = 0;
			}
			else if(!strcmp(url, "error_page.htm")
					|| !strcmp(url, "jquery.js") // 2010.09 James.
					|| !strcmp(url, "Nologin.asp")
					|| !strcmp(url, "gotoHomePage.htm")
					) {
				;	// do nothing.
			}
			else if (login_state == 2
					&& !strcmp(url, "Logout.asp")) {
				turn_off_auth_timestamp = 0;
				temp_turn_off_auth = 0;
				redirect = 1;
			}
			else if (strstr(url, ".asp") != NULL
					|| strstr(url, ".cgi") != NULL
					|| strstr(url, ".htm") != NULL
					|| strstr(url, ".CFG") != NULL
					) {
				switch(login_state) {
					case 0:
						return;
					case 1:
					case 2:
						turn_off_auth_timestamp = 0;
						temp_turn_off_auth = 0;
						redirect = 0;
						break;
					case 3:
						printf("System error! the url: %s would be changed to Nologin.asp in this case!\n", url);
						break;
					default:
						printf("System error! the login_state is wrong!\n");
				}
			}
			else if (login_state == 2
					&& temp_turn_off_auth
					&& (unsigned long)(request_timestamp-turn_off_auth_timestamp) > 10
					) {
				http_logout(login_ip_tmp);
				turn_off_auth_timestamp = 0;
				temp_turn_off_auth = 0;
				redirect = 0;
			}
			
			if (handler->auth) {
				if (!temp_turn_off_auth) {
					handler->auth(auth_userid, auth_passwd, auth_realm);
					if (!auth_check(auth_realm, authorization, url))
						return;
				}
				if (!redirect)
				{
					if (strstr(url, "QIS_wizard.htm"))
						;
					else if (	strcasestr(url, ".asp") != NULL ||
                                        		strcasestr(url, ".cgi") != NULL ||
                                        		strcasestr(url, ".htm") != NULL ||
                                        		strcasestr(url, ".CFG") != NULL	)
						http_login(login_ip_tmp, url);
				}
			}
			
			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL, "That method is not implemented.");
				return;
			}
// 2007.11 James. for QISxxx.htm pages }
			
			if (handler->input) {
				handler->input(file, conn_fp, cl, boundary);
#if defined(linux)
				if ((flags = fcntl(fileno(conn_fp), F_GETFL)) != -1 &&
						fcntl(fileno(conn_fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);
					
					fcntl(fileno(conn_fp), F_SETFL, flags);
				}
#elif defined(vxworks)
				flags = 1;
				if (ioctl(fileno(conn_fp), FIONBIO, (int) &flags) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void)fgetc(conn_fp);
					flags = 0;
					ioctl(fileno(conn_fp), FIONBIO, (int) &flags);
				}
#endif
			}
			send_headers( 200, "Ok", handler->extra_header, handler->mime_type );

			if (handler->output) {
				handler->output(file, conn_fp);
			}
			
			break;
		}
	}
	
	if (!handler->pattern) 
		send_error( 404, "Not Found", (char*) 0, "File not found." );
	
	if (!strcmp(file, "Logout.asp")) {
		http_logout(login_ip_tmp);
	}
	
	if (!strcmp(file, "Reboot.asp")) {
		//2011.06.16 Jerry {
		DHCP_T dhcp;
		apmib_get( MIB_WAN_DHCP, (void *)&dhcp);
		if(dhcp == PPPOE)
			system("/bin/disconnect.sh option");
		//2011.06.16 Jerry }
		system("reboot");
	}
//2008.08 magic}
}

//2008 magic{
void http_login_cache(usockaddr *u) {
	struct in_addr temp_ip_addr;
	char *temp_ip_str;
	
	login_ip_tmp = (unsigned int)(u->sa_in.sin_addr.s_addr);
	temp_ip_addr.s_addr = login_ip_tmp;
	temp_ip_str = inet_ntoa(temp_ip_addr);
}

//2011.04.26 Jerry {
void set_login_info(char *input, char *filename)
{
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) return;
	fprintf(fp, "%s\n", input);
	fclose(fp);
}

void http_login(unsigned int ip, char *url) {
	struct in_addr login_ip_addr;
	char *login_ip_str;
	
	if (http_port != SERVER_PORT || ip == 0x100007f)
		return;
	
	login_ip = ip;
	last_login_ip = 0;
	
	login_ip_addr.s_addr = login_ip;
	login_ip_str = inet_ntoa(login_ip_addr);
	
	if (strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp")) //2008.11 magic
		login_timestamp = uptime();
	
	char login_ipstr[32], login_timestampstr[32];
	
	memset(login_ipstr, 0, 32);
	sprintf(login_ipstr, "%u", login_ip);
	set_login_info(login_ipstr, LOGIN_IP_FILE);	//2011.04.22 Jerry
	
	if (strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp")) {//2008.11 magic
		memset(login_timestampstr, 0, 32);
		sprintf(login_timestampstr, "%lu", login_timestamp);
		set_login_info(login_timestampstr, LOGIN_TIMESTAMP_FILE);	//2011.04.22 Jerry
	}
}

// 0: can not login, 1: can login, 2: loginer, 3: not loginer.
int http_login_check(void) {
	if (http_port != SERVER_PORT || login_ip_tmp == 0x100007f)
		//return 1;
		return 0;	// 2008.01 James.
	if (login_ip == 0)
		return 1;
	else if (login_ip == login_ip_tmp)
		return 2;
	
	return 3;
}

void http_login_timeout(unsigned int ip)
{
	time_t now;
	
//	time(&now);
	now = uptime();
	
// 2007.10 James. for really logout. {
	if ((login_ip != 0 && login_ip != ip) && (unsigned long)(now-login_timestamp) > 60) //one minitues
// 2007.10 James }
	{
		http_logout(login_ip);
	}
}

void http_logout(unsigned int ip) {
	if (ip == login_ip) {
		last_login_ip = login_ip;
		login_ip = 0;
		login_timestamp = 0;		
		//2011.04.22 Jerry {
		unlink(LOGIN_IP_FILE);
		unlink(LOGIN_TIMESTAMP_FILE);
		//2011.04.22 Jerry }	
				
// 2008.03 James. {
		if (change_passwd == 1) {
			change_passwd = 0;
			reget_passwd = 1;
		}
// 2008.03 James. }
	}
}
//2008 magic}

//2011.04.25 Jerry {
void http_logout_change_mode()
{
	last_login_ip = login_ip;
	login_ip = 0;
	login_timestamp = 0;
	
	//2011.04.22 Jerry {
	unlink(LOGIN_IP_FILE);
	unlink(LOGIN_TIMESTAMP_FILE);
	//2011.04.22 Jerry }	

	if (change_passwd == 1) {
		change_passwd = 0;
		reget_passwd = 1;
	}
}
//2011.04.25 Jerry }


int is_auth(void)
{
	if (http_port==SERVER_PORT) return 1;
	else return 0;
}



#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
/* IOCTL system call */
static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	unsigned int args[4];
  	struct ifreq ifr;
  	int sockfd;

  	args[0] = arg0;
  	args[1] = arg1;
  	args[2] = arg2;
  	args[3] = arg3;

  	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    	{
      		perror("fatal error socket\n");
      		return -3;
    	}
  
  	strcpy((char*)&ifr.ifr_name, name);
  	((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;

  	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    	{
      		perror("device ioctl:");
      		close(sockfd);
      		return -1;
    	}
  	close(sockfd);
  	return 0;
} /* end re865xIoctl */

int is_phyconnected(void)       // ASUS add
{
	unsigned int    ret;
        unsigned int    args[0];
	re865xIoctl("eth1", RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;
	if(ret == 0)
		return 1;
        return 0;//return ret;
}

int is_fileexist(char *filename)
{
	FILE *fp;

	fp=fopen(filename, "r");

	if (fp==NULL) return 0;
	fclose(fp);
	return 1;
}


int is_firsttime(void)
{
	//2011.04.12 Jerry {
	int w_Setting = 0;
	apmib_get(MIB_W_SETTING, (void *)&w_Setting);
	if(w_Setting == 0)
		return 1;
	else
		return 0;
	//2011.04.12 Jerry }
}

#ifdef TRANSLATE_ON_FLY
int
load_dictionary (char *lang, pkw_t pkw)
{
	char dfn[16];
	char *p, *q;
	FILE *dfp = NULL;
	int dict_size = 0;
	const char *eng_dict = "EN.dict";
#ifndef RELOAD_DICT
	static char loaded_dict[12] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#endif  // RELOAD_DICT

	if (lang == NULL || (lang != NULL && strlen (lang) == 0))       {
		snprintf (dfn, sizeof (dfn), eng_dict);
	} else {
		snprintf (dfn, sizeof (dfn), "%s.dict", lang);
	}

#ifndef RELOAD_DICT
	if (strcmp (dfn, loaded_dict) == 0)     {
		return 1;
	}
	release_dictionary (pkw);
#endif  // RELOAD_DICT

	do      {
		dfp = fopen (dfn, "r");
		if (dfp != NULL)	{
#ifndef RELOAD_DICT
			snprintf (loaded_dict, sizeof (loaded_dict), "%s", dfn);
#endif  // RELOAD_DICT
			break;
		}

		if (dfp == NULL && strcmp (dfn, eng_dict) == 0) {
			return 0;
		} else {
			// If we can't open specified language file, use English as default
			snprintf (dfn, sizeof (dfn), eng_dict);
		}
	} while (1);

	memset (pkw, 0, sizeof (kw_t));
	fseek (dfp, 0L, SEEK_END);
	dict_size = ftell (dfp) + 128;
	REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));
	pkw->buf = q = malloc (dict_size);

	fseek (dfp, 0L, SEEK_SET);
	while (!feof (dfp))     {
		// if pkw->idx is not enough, add 32 item to pkw->idx
		REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));

		fscanf (dfp, "%[^\n]%*c", q);
		if ((p = strchr (q, '=')) != NULL)      {
			pkw->idx[pkw->len] = q;
			pkw->len++;
			q = p + strlen (p);
			*q = '\0';
			q++;
		}
	}

	fclose (dfp);
	return 1;
}


void
release_dictionary (pkw_t pkw)
{
	if (pkw == NULL)	{
		return;
	}

	pkw->len = pkw->tlen = 0;

	if (pkw->idx != NULL)   {
		free (pkw->idx);
		pkw->idx = NULL;
	}

	if (pkw->buf != NULL)   {
		free (pkw->buf);
		pkw->buf = NULL;
	}
}

char*
search_desc (pkw_t pkw, char *name)
{
	int i;
	char *p, *ret = NULL;

	if (pkw == NULL || (pkw != NULL && pkw->len <= 0))      {
		return NULL;
	}
	for (i = 0; i < pkw->len; ++i)  {
		p = pkw->idx[i];
		if (strncmp (name, p, strlen (name)) == 0)      {
			ret = p + strlen (name);
			break;
		}
	}

	return ret;
}
#endif //TRANSLATE_ON_FLY

void reapchild()	// 0527 add
{
	signal(SIGCHLD, reapchild);
	wait(NULL);
}

void sigHandler_wsc(int signo)
{
	#define REINIT_WEB_FILE		T("/tmp/reinit_web")
	struct stat status;
	int reinit=1;

	if (stat(REINIT_WEB_FILE, &status) == 0) { // file existed
        	unlink(REINIT_WEB_FILE);
		reinit = 0;
		apmib_reinit();
	}

	if (reinit) { // re-init system
		int x_setting = 1;
		int wanduckproc = 0;
		apmib_reinit();
		apmib_set(MIB_X_SETTING, (void *)&x_setting);
		apmib_update(CURRENT_SETTING);
		wanduckproc = find_pid_by_name("wanduck");
		if(wanduckproc > 0)
			kill(wanduckproc, SIGUSR1);
		notify_sysconf("restart_wlan", 0);
	}
}

int conn_fd=0;
int main(int argc, char **argv)
{
	usockaddr usa;
	int listen_fd=0;
	socklen_t sz = sizeof(usa);
	char pidfile[32]={0};
	char *strLoginIP = NULL;	//2011.04.22 Jerry
	char *strLoginTimestamp = NULL;	//2011.04.22 Jerry

	//Added by Jerry
	wlan_num = 1;	// set 1 as default
	vwlan_num = 4;
	WAN_IF = T("eth1");
	BRIDGE_IF = T("br0");
	ELAN_IF = T("eth0");
	ELAN2_IF = T("eth2");
	ELAN3_IF = T("eth3");
	ELAN4_IF = T("eth4");
	PPPOE_IF = T("ppp0"); //Add by Emily 2011.05.11
	strcpy(WLAN_IF,"wlan0");
	//Added by Jerry

	// Added by Joey for handling WAN Interface 
	// usage: httpd [wan interface] [port]
	if (argc>2) http_port=atoi(argv[2]);
	if (argc>1) strcpy(wan_if, argv[1]);
	else strcpy(wan_if, "");

	if ( apmib_init() == 0 )
		printf("Initialize AP MIB failed! Can't start httpd!\n");
	else
		printf("Start httpd!\n");

	unlink(LOGIN_IP_FILE);
	unlink(LOGIN_TIMESTAMP_FILE);	

	detect_timestamp_old = 0;
	detect_timestamp = 0;
	signal_timestamp = 0;

	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, reapchild);	// 0527 add
	signal(SIGUSR1, sigHandler_wsc);	//2011.05.11 Jerry

	/* Initialize listen socket */
	if ((listen_fd = initialize_listen_socket(&usa)) < 2) {
		fprintf(stderr, "can't bind to any address\n" );
		exit(errno);
	}

#if !defined(DEBUG) && !defined(vxworks)
	{
	FILE *pid_fp;

	if (http_port==SERVER_PORT)
		strcpy(pidfile, "/var/run/httpd.pid");
	else sprintf(pidfile, "/var/run/httpd-%d.pid", http_port);

	if (!(pid_fp = fopen(pidfile, "w"))) {
		perror(pidfile);
		return errno;
	}
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);
	}
#endif

	/* Loop forever handling requests */
	for (;;) {
		if ((conn_fd = accept(listen_fd, &usa.sa, &sz)) < 0) {
			perror("accept");
			return errno;
		}
		if (!(conn_fp = fdopen(conn_fd, "r+"))) {
			perror("fdopen");
			return errno;
		}
		http_login_cache(&usa);
		handle_request();
		fflush(conn_fp);
		fclose(conn_fp);
		close(conn_fd);
		//2011.04.14 Jerry {
		if(fwupgrade_flag == 1) {
			printf("[httpd] exit!!!\n");
			notify_sysconf("fw_upgrade", 0);
			break;
		}
			//firmware_upgrade();
		//2011.04.14 Jerry }
	}

	shutdown(listen_fd, 2);
	close(listen_fd);

	return 0;
}
