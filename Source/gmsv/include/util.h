#ifndef __UTIL_H__
#define __UTIL_H__

#if PLATFORM_WINDOWS
#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if PLATFORM_WINDOWS
#define random() rand()
#endif

#include "common.h"

typedef struct tagSTRING32
{
    char string[32];
}STRING32;
typedef struct tagSTRING64
{
    char string[64];
}STRING64;
typedef struct tagSTRING128
{
    char string[128];
}STRING128;

typedef struct tagRECT_SA
{
    int     x;
    int     y;
    int     width;
    int     height;
}RECT_SA;
typedef struct tagPOINT_SA
{
    int     x;
    int     y;
}POINT_SA;
/* strtol のラッパー用に使う */
typedef enum
{
    CHAR_SA,SHORT_SA,INT_SA,DOUBLE_SA
}CTYPE;

void chop( char* src );
void dchop( char* src , char* del);
#define chomp(src)  dchop(src,"\n");
void chompex( char *buf );
void pohcd( char* src , char* del);
void strcpysafe( char* dest ,size_t n,const char* src );
void strncpysafe( char* dest ,const size_t n ,
                 const char* src ,const int length );
char* strcatsafe( char* src, int size, char* ap );
BOOL strtolchecknum( char* arg , void* number,int base ,CTYPE type);
int strcmptail( char *s1 , char *s2 );
int texttoiTail( char *hoge);
int textToInt( char *str );
void deleteCharFromString( char* src , char* dels );
void deleteCharFromStringNoEscape( char* src , char* dels );
#define deleteWhiteSpace( src ) deleteCharFromStringNoEscape( src," \t" )
char*   replaceString( char* src, char oldc ,char newc );

BOOL	utf8ToBig5(char* str, int size);
BOOL	big5ToUtf8(char* str, int size);
char*   escapeStrStr( char* src, char* needle );
BOOL getStringFromIndexWithDelim_body( char* src ,char* delim ,int index, char* buf , int buflen, char *file, int line );
#define getStringFromIndexWithDelim( src, delim, index, buf, buflen ) getStringFromIndexWithDelim_body( src, delim, index, buf, buflen, __FILE__, __LINE__ )

void getFourIntsFromString(char* src,int* int1,int* int2,int* int3,
                           int* int4);
void deleteSequentChar( char* src , char* dels );
int hashpjw ( char* s );
double gettimemillis( void );
double gettimemicros( void );

int bindlocalhost( int port );
int connectHost( char* hostname , unsigned short port );
int existsNewLineCharacter( char* line );
int hashpjw ( char* s );
int charInclude( char* src, char* include );
char* nindex( char* string, int c , int number );
int rgetFileName( char* dirname , STRING64* string, int size); 
#define RAND(x,y)   ((x-1)+1+ \
                     (int)( (double)(y-(x-1))*rand()/(RAND_MAX+1.0)) )
/*#define RAND(x,y)   (x+(int)( (double)(y)*rand() / (RAND_MAX+1.0)) )*/
void makeRandomString( char *cand , char *out , int len );
BOOL checkStringsUnique( char** strings, int num ,int verbose);

char*   makeEscapeString( char* src , char* dest, int sizeofdest);
char*   makeStringFromEscaped( char* src );
INLINE double time_diff(struct timeval subtrahend,struct timeval subtractor);
BOOL PointInRect( RECT_SA* rect, POINT_SA* p );
BOOL CoordinateInRect( RECT_SA* rect, int x, int y);
int clipRect( RECT_SA *rect1, RECT_SA *rect2, RECT_SA *ret );
BOOL isstring1or0( char*  string );
void easyGetTokenFromString( char *src , int count , char*output , int len );

float  linearDiv( float val1 , float val2 , float d );
BOOL isExistFile( char* filename );
char *cnv10to62( int a, char *out, int outlen );
BOOL checkRedundancy( int *src, int srclen);
void shuffle_ints( int num, int *a, int t );
unsigned short CheckCRC( unsigned char *p , int size );
// Shan Add
INLINE double time_diff(struct timeval subtrahend,struct timeval subtractor);
 

/*  マクロなり  */
#define ABS(x)      ((x)>=0 ? (x) : -(x))

/*  方向を 0 - 7 に絶対にする   */
#if PLATFORM_WINDOWS
#define VALIDATEDIR(x)  {(x)%=8;(x)=((x)+8)%8;}
#else
#define VALIDATEDIR(x)  ({(x)%=8;(x)=((x)+8)%8;})
#endif

/*  4バイトデータを2バイトデータとして使う  */
#define GETLOWVALUE(x)  (  (x)&0x0000ffff)
#define GETHIGHVALUE(x) ((((x)&0xffff0000)>>16)&0x0000ffff)
#define SETLOWVALUE(x,y)  ((x)=((x)&0xffff0000)|( (y)     &0x0000ffff))
#define SETHIGHVALUE(x,y) ((x)=((x)&0x0000ffff)|(((y)<<16)&0xffff0000))
#define MAKE2VALUE(x,y)  ((((x)<<16)&0xffff0000)|((y)&0x0000ffff))

/*  4バイトデータを1バイト*4としてあつかうマクロ    */
#define GETFIRSTVALUE(x)  (  (x)&0x000000ff )
#define GETSECONDVALUE(x) ((((x)&0x0000ff00)>>8 )&0x000000ff)
#define GETTHIRDVALUE(x)  ((((x)&0x00ff0000)>>16)&0x000000ff)
#define GETFOURTHVALUE(x) ((((x)&0xff000000)>>24)&0x000000ff)

#define SETFIRSTVALUE(x,y)  ((x)=(((x)&0xffffff00)\
                                  |(  y      &0x000000ff)))
#define SETSECONDVALUE(x,y) ((x)=(((x)&0xffff00ff)\
                                  |(((y)<<8 )&0x0000ff00)))
#define SETTHIRDVALUE(x,y)  ((x)=(((x)&0xff00ffff)\
                                  |(((y)<<16)&0x00ff0000)))
#define SETFOURTHVALUE(x,y) ((x)=(((x)&0x00ffffff)\
                                  |(((y)<<24)&0xff000000)))

#define MAKE4VALUE(a,b,c,d) ( (((a)<<24)&0xff000000)\
                             |(((b)<<16)&0x00ff0000)\
                             |(((c)<< 8)&0x0000ff00)\
                             |( (d)     &0x000000ff))

#endif
 /* __UTIL_H__ */
