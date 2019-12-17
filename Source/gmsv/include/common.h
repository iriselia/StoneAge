#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#if PLATFORM_WINDOWS
#if defined(_WIN32) || defined(_WIN64)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
#else
#endif

/* MACROS */
#define RETURNFALSEIFFALSE(x) if(!x)return FALSE;
#define EXITWITHEXITCODEIFFALSE(x,code) if(!x)exit(code);
#ifdef __GNUC__
#define print(format,arg...) fprintf( stderr, format ,##arg)
#define fprint(format,arg...) fprintf( stderr, "%s:%d " format , __FILE__ , __LINE__ , ##arg)
#else
#define print(...) fprintf (stderr, __VA_ARGS__)
char __printbuf[512];
#define printBig5(...) \
snprintf(__printbuf, 512, __VA_ARGS__); \
big5ToUtf8(__printbuf, sizeof(__printbuf)); \
print("%s", __printbuf);

#define fprint(format, ...) fprintf( stderr, "%s:%d " format , __FILE__ , __LINE__ , __VA_ARGS__)
#endif
#define debug(x,y) fprintf( stderr, #x " = %" #y "\n" , x)
#define arraysizeof( x ) (sizeof(x)/sizeof(x[0]))

#define errorprint    {extern int errno;fprint( "%s\n" ,strerror(errno));}

#define BACKSLASH '\\'
#define NEWLINE '\n'
#define TAB '\t'
#define SPACE ' '

/*下の define より安全にする    */
#if PLATFORM_WINDOWS
#define min( x,y ) ((x)>(y)?(y):(x))
#define max( x,y ) ((x)>(y)?(x):(y))
#else
#define min( x,y ) ({typeof(x) __x=(x),__y=(y);(__x < __y) ? __x : __y; })
#define max( x,y ) ({typeof(x) __x=(x),__y=(y);(__x < __y) ? __y : __x; })
#endif
#define swap( x,y ) {typeof(x) __x=(y);(y)=(x);(x)=__x;}
#define swapT( T, x,y ) {T __x=(y);(y)=(x);(x)=__x;}
#define SUCCESSFUL_STRING "successful"
#define FAILED_STRING "failed"

#ifdef _BAD_PLAYER             // WON ADD 送壞玩家去關
#define BADPLAYER "badplayer"
#endif

#define LOCK 1
#define UNLOCK 0

#define STRINGBUFSIZ    4096

#define OPEN {FILE* f;f=fopen("a.txt","a");
#define CLOSE fclose(f);}

#define time_diff_us(new,old) ((unsigned)( (new.tv_sec - old.tv_sec)*1000000 + ( new.tv_usec - old.tv_usec  ) ))




extern int snprintf (char* , size_t, const char* , ...)
#ifdef __GNUC__
    __attribute__ ((format (printf,3,4)));
#else
    ;
#endif

#define ON  1
#define OFF 0
#if PLATFORM_WINDOWS
#ifndef _WINDEF_
typedef int BOOL;
#endif
#else
#define BOOL int
#endif
#define FALSE 0
#define TRUE  1
#if PLATFORM_WINDOWS
#define INLINE
#else
#define INLINE inline
#endif
#define USE_MTIO 0

#endif
