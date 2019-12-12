#include "version.h"
#include <sys/types.h>
#include <sys/stat.h>

#if PLATFORM_WINDOWS
#include <WinSock2.h>
#include "3rdParty/dirent.h"
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "util.h"
#include "configfile.h"

#define IS_2BYTEWORD( _a_ ) ( (char)(0x80) <= (_a_) && (_a_) <= (char)(0xFF) )

/*-----------------------------------------
  けつの改行をころす
  引数： もとの文字列。直接変更されるぞ

  windows , macintosh , unixのどの行も対応できる。

  windows : \r\n
  macintosh : \n\r
  unix : \n

  だったかな
  macintosh : \r
  です。
  それとこのプログラムだとけつの文字じゃなくても
  改行消すけどいいんですか
  -----------------------------------------*/
void chompex( char *buf )
{
    while( *buf ){
        if( *buf == '\r' || *buf == '\n' ){
            *buf='\0';
        }
        buf++;
    }
}



/*----------------------------------------
 * けつの1文字を取り除く
 * 引数
 *  src         変更する文字列
 ----------------------------------------*/
void chop( char* src )
{
    /*
     * ただたんに最後の文字を取り除く
     */
    int length = strlen( src );
    if( length == 0 ) return;
    src[length-1] = '\0';
}


/*----------------------------------------
 * けつの文字を取る。その文字を複数指定できる。
 * 引数
 *  src     元の文字列
 *  delim   削除する文字の配列。最後はNULL文字で終っている事
 ----------------------------------------*/
void dchop( char* src , char* del)
{
    int dellen, srclen;
    int i;
    BOOL delete=FALSE;

    /*
     * srcの後ろから調べていく。1文字につき del すべてをループして
     * 一致すれば消す。そして次の文字にいく。
     */

    srclen = strlen( src );
    dellen = strlen( del );
    if( srclen == 0 || dellen == 0 ) return;

    for( i = 0 ; i < dellen ; i ++ ){
        if( src[srclen-1] == del[i] ){
            delete = TRUE;
            break;
        }
    }

    /*  切り取り    */
    if( delete )src[srclen - 1] = '\0';
}


/*----------------------------------------
 * dchopの逆。最初の文字を取る。その文字を複数指定できる。
 * 引数
 *  src     元の文字列
 *  delim   削除する文字の配列。最後はNULL文字で終っている事
 ----------------------------------------*/
void pohcd( char* src , char* del)
{
    int dellen, srclen;
    int i;
    BOOL delete=FALSE;

    /*
     * srcの前から調べていく。1文字につき del すべてをループして
     * 一致すれば消す。そして次の文字にいく。
     */

    srclen = strlen( src );
    dellen = strlen( del );
    if( srclen == 0 || dellen == 0 ) return;

    for( i = 0 ; i < dellen ; i ++ ){
        if( src[0] == del[i] ){
            delete = TRUE;
            break;
        }
    }

    /*  削除    */
    if( delete )
        /*  このコピーで NULL文字もコピーされる */
        for( i = 0 ; i < srclen ; i ++ )
            src[i] = src[i+1];
}


/*----------------------------------------
 * 文字列に文字が含まれているかどうか調べる。
 * 引数
 *  src     char*   調べる文字列
 *  include char*   含まれているかどうかの文字列。
 * 返り値
 *  成功    最初に含まれていたsrcの場所
 *  失敗    -1
 ----------------------------------------*/
int charInclude( char* src, char* include )
{
    int     i,j;
    int     srclen,includelen;

    srclen = strlen( src );
    includelen = strlen( include );

    for( i = 0 ; i < srclen ; i ++ ){
        for( j = 0 ; j < includelen ; j ++ )
            if( src[i]  == include[j] )
                return i;
    }
    return -1;
}


/*------------------------------------------------------------
 * EUCの2バイト目だけをコピーしないstrncpy
 * freeBSD のライブラリから strncpy を取ってきて、
 * それをもとに改造。*dest++ とか使いたいけど、結合順序に自身が
 * ないので、使っていない。かわりに配列だけど、ポインタも使ってて
 * かなり変なプログラム
 * 引数
 *  dest        char*   コピー先
 *  src         char*   ソース
 *  n           size_t  長さ
 * 返り値
 *  dest
 ------------------------------------------------------------*/
char* strncpy2( char* dest, const char* src, size_t n )
{
    if( n > 0 ){
        char*   d = dest;
        const char*   s = src;
        int i;
        for( i=0; i<n ; i++ ){
            if( *(s+i) == 0 ){
                /*  コピーしおわったら NULL 文字を入れる   */
                *(d+i) = '\0';
                return dest;
            }
            if( *(s+i) & 0x80 ){
                *(d+i)  = *(s+i);
                i++;
                if( i>=n ){
                    *(d+i-1)='\0';
                    break;
                }
                *(d+i)  = *(s+i);
            }else
                *(d+i) = *(s+i);
        }
    }
    return dest;
}

/*----------------------------------------
 * src から dest にコピーする。 コピー先のサイズを調べる。
 * 引数
 *      dest        コピー先
 *      n           サイズ
 *      src         元の文字列
 ----------------------------------------*/
void strcpysafe( char* dest ,size_t n ,const char* src )
{
    /*
     * src から dest へコピーする.
     * strcpy, strncpy では dest より コピーする量が
     * 大きい時に呼ぶと,メモリ外アクセスが起る.
     * これを防ぐ為に, strncpy があるが strlen( src ) が n より
     * 大きい時には, dest の最後が NULL 文字とはならない.
     *
     * したがって dest の大きさより src のほうが長い時には
     * n-1 で strncpy をする. 小さい時はそのままコピーする
     *
     * n が負の時はおかしくなるので負の時は 何もしない。
     *
     */
    // Nuke +1 (08/25): Danger if src=0
    if (!src) {
        *dest = '\0';
        return;
    }
    if( n <= 0 )        /* 何もしない   */
        return;

    /*  この時点で、 n >= 1 以上が決定  */
    /*  NULL文字を考慮して比較する  */
    else if( n < strlen( src ) + 1 ){
        /*
         * バッファが足りないので n - 1(NULL文字)
         * で strncpy を呼ぶ
         */
        strncpy2( dest , src , n-1 );
        dest[n-1]='\0';
    }else
        strcpy( dest , src );

}

/*----------------------------------------
 * src から length 映す。   コピー先の大きさを調べる。
 * 引数
 *          dest        コピー先
 *          n           コピー先のサイズ
 *          src         コピー元
 *          length      何文字コピーするか
 ----------------------------------------*/
void strncpysafe( char* dest , const size_t n ,
                  const char* src ,const int length )
{
    /*
     * src から dest に length コピーする
     * strcpy, strncpy では dest より コピーする量が
     * 大きい時に呼ぶと,メモリ外アクセスが起る.
     * この関数では、strlen( src ) と length の小さいほう
     * (実際にコピーする量) と dest のサイズを比べて、
     * strcpysafe と同じ事をする。
     */

    int Short;
    Short = min( strlen( src ) , length );

    /* NULL文字 を考慮した比較 */
    if( n < Short + 1 ){
        /*
         * バッファが足りないので n - 1(NULL文字)
         * で strncpy を呼ぶ
         */
        strncpy2( dest , src , n-1 );
        dest[n-1]='\0';

    }else if( n <= 0 ){
        return;
    }else{
        /*
         * バッファは十分にあるので Short でstrncpyを呼ぶ
         * なお src には Short の長さ中に NULL がないので、
         * dest には 追加しておく。
         */

        strncpy2( dest , src , Short );
        dest[Short]= '\0';

    }
}

/*------------------------------------------------------------
 * strcatを安全に実現する。遅いので、特別な場合を除いて使わない事。
 * 引数
 *  src     char*       元になる文字列
 *  size    int         src のサイズ( 後ろにどれだけ残っているかでは
 *                          なくてバッファ全体 )
 *  ap      char*       付け加える物
 * 返り値
 *  src     (入れ子用)
 ------------------------------------------------------------*/
char* strcatsafe( char* src, int size, char* ap )
{
    strcpysafe( src + strlen( src ) , size - strlen(src), ap );
    return src;
}

/*----------------------------------------
 * 文字列の末尾を比較する。同じかどうかしか判定しない by ringo
 * char *s1 , *s2 : 比較する文字列
 * 返り値
 * 0 ： おなじ
 * 1 : ことなる
 * これはNPCのテキスト解析にやくだつ。
 ----------------------------------------*/
int strcmptail( char *s1 , char *s2 )
{
    int i;
    int len1 = strlen( s1 );
    int len2 = strlen( s2 );

    for(i=0;;i++){
        int ind1 = len1 - 1 - i;
        int ind2 = len2 - 1 - i;
        if( ind1 < 0 || ind2 < 0 ) return 0;
        if( s1[ind1] != s2[ind2] ) return 1;
    }
}



/*----------------------------------------
 * 文字列の末尾に書かれている整数をゲットする。
 * NPCのテキスト解析にやくだつ。
 * ただし末尾にいらん文字がある場合無視する。
 * 信岡が落ちないアルゴリズムに変更。
 * 引数
 *  hoge    char*   解析する文字列
 * 返り値
 *  解析した整数。
 *
 * 例：
 * "ringo ( 吧噗吧噗星人 ) ： 100"
 *      の場合は100がかえる。
 * "ringo ( 吧噗吧噗星人 ) ： 是100"
 *      も100になる。ほんまにNPC用なりね。
 ----------------------------------------*/
#define DECIMALCHARNUM 10
#define FIGURECHARNUM 3
#define SPECIALCHARNUM 2
char *UTIL_decimalchar[DECIMALCHARNUM] = {"0","1","2",
                                     "3",  "4","5",
                                     "6","7","8","9"};
char *UTIL_figurechar[FIGURECHARNUM] = {"","",""};
char *UTIL_specialchar[SPECIALCHARNUM] = {"",""};

/*
 * 文字列から、数値を表すと思われる部分を抜き出す。先頭のほうから
 * 探して抜き出す。euc用。
 *
 * 例 : in "這本書七萬冊300美金嗎？"
 *      out "7萬"
 * 引数
 * char *in:入力文字列
 * char *out:出力文字列
 * int outsiz:出力文字列のバッファサイズ
 * 返り値
 * 数値を表す部分があったらその文字列の長さ、なかったら０
 */
static int findNumberString( char *in, char *out, int outsiz )
{
    int len,i,j,k, findflag, numstrflag;
    char tmp[3];

    len = strlen(in);
    if( len == 0 ) return 0;

    numstrflag=0;
    findflag = 0;
    for( i=0,j=0;in[i]!='\0'&& j<outsiz-2;i++ ){
        findflag = 0;
        if( in[i] & 0x80 ){
            /* EUCの場合 */
            tmp[0] = in[i];
            tmp[1] = in[i+1];
            tmp[2] = '\0';
        }else{
            /* ASCIIの場合 */
            tmp[0] = in[i];
            tmp[1] = '\0';
        }
        for( k=0;k<DECIMALCHARNUM;k++ ){
            if( strstr( UTIL_decimalchar[k], tmp ) ){
                numstrflag = 1;
                findflag = 1;
            }
        }
        for( k=0;k<FIGURECHARNUM;k++ ){
            if( strstr( UTIL_figurechar[k], tmp ) ){
                numstrflag = 1;
                findflag = 1;
            }
        }
        for( k=0;k<SPECIALCHARNUM;k++ ){
            if( strstr( UTIL_specialchar[k], tmp ) ){
                numstrflag = 1;
                findflag = 1;
            }
        }
        if( findflag ){
            if( in[i] & 0x80 ){
                out[j] = tmp[0];
                out[j+1] = tmp[1];
                j+=2;
                i++;
            }else{
                out[j] = tmp[0];
                j+=1;
            }
            continue;
        }

        if( numstrflag ){
            out[j] = '\0';
            return j;
        }

    }

    if( findflag ){
        out[j] = '\0';
        return j;
    }else{
        return 0;
    }

}


/*
 * 文字列をintに変換。
 *
 */
int numstrIsKanji( char *str );
int decimalstrToInt( char *str );
int kanjistrToInt( char *str );

int textToInt( char *str )
{
    char numstr[256];
    if( !findNumberString(str,numstr,sizeof(numstr)) ) return 0;

    if( numstrIsKanji(numstr) ) return kanjistrToInt(numstr);
    else return decimalstrToInt(numstr);

}

/*
 * 数値を表す文字列が、日本語式なのか世界式なのかを判別
 * 引数 判別する文字列。数字を表す文字列以外は含んではいけない。
 * 返り値 日本語式なら１、世界式なら０
 * 例 str が"一億四千萬。"のとき　返り値1
 
 *    str が"123"のとき　返り値０
 *
 */
int numstrIsKanji( char *str )
{
    int i,j;
    char tmp[3];

    for( i=0;str[i]!='\0';i++ ){
        if( str[i] & 0x80 ){
            tmp[0] = str[i];
            tmp[1] = str[i+1];
            tmp[2] = '\0';
            i++;
        }else{
            tmp[0] = str[i];
            tmp[1] = '\0';
        }
        for( j=0;j<FIGURECHARNUM;j++ ){
            if( strstr(UTIL_figurechar[j],tmp) ){
                return 1;
            }
        }
        for( j=0;j<SPECIALCHARNUM;j++ ){
            if( strstr(UTIL_specialchar[j],tmp) ){
                return 1;
            }
        }
    }
    return 0;

}

/*
 * 十進法で数値を表した文字列をintに変換
 * char *str 文字列
 * 返り値 文字列が表す数値
 */
int decimalstrToInt( char *str )
{

    double val;
    char tmp[3];
    int i,j;

    val = 0;
    for( i=0;str[i]!='\0';i++ ){
        if( str[i] & 0x80 ){
            tmp[0] = str[i];
            tmp[1] = str[i+1];
            tmp[2] = '\0';
            i++;
        }else{
            tmp[0] = str[i];
            tmp[1] = '\0';
        }
        for( j=0;j<DECIMALCHARNUM;j++ ){
            if( strstr(UTIL_decimalchar[j],tmp) ){
                val = val*10+j;
            }
        }
    }

    if( val > 0x7fffffff ) return 0x7fffffff;
    else     return (int)val;

}


/*
 * 日本式で表した数値を整数に変換
 * char *str 数値を表す文字列
 * 返り値 数値
 * 例
 * strが"一億4千万？"のとき返り値 140000000
 */
int kanjistrToInt( char *str )
{
    double val,tmpval;
    char tmp[3];
    int num,i,j,numflag;

    numflag = 0;
    num = 1;
    tmpval = 0;
    val = 0;
    for( i=0;str[i]!='\0';i++ ){
        if( str[i] & 0x80 ){
            tmp[0] = str[i];
            tmp[1] = str[i+1];
            tmp[2] = '\0';
            i++;
        }else{
            tmp[0] = str[i];
            tmp[1] = '\0';
        }
        for( j=0;j<SPECIALCHARNUM;j++ ){
            if( strstr(UTIL_specialchar[j],tmp ) ){
                if( numflag == 1 ) tmpval += num;
                if( j == 0 ) val += tmpval*10000;
                else if( j == 1 ) val += tmpval*100000000;
                num = 1;
                numflag = 0;
                tmpval = 0;
                goto nextchar;
            }
        }
        for( j=0;j<FIGURECHARNUM;j++ ){
            if( strstr(UTIL_figurechar[j],tmp) ){
                if( j == 0 ) tmpval += num*10;
                else if( j == 1 ) tmpval += num*100;
                else if( j == 2 ) tmpval += num*1000;
                num = 1;
                numflag = 0;
                goto nextchar;
            }
        }
        for( j=0;j<DECIMALCHARNUM;j++ ){
            if( strstr(UTIL_decimalchar[j],tmp) ){
                num = j;
                numflag = 1;
                goto nextchar;
            }
        }
nextchar:
		continue;
    }
    if( numflag == 1 ) tmpval += num;
    val += tmpval;

    if( val > 0x7fffffff ) return 0x7fffffff;
    else     return (int)val;

}



/*----------------------------------------
 * 文字列の末尾に書かれている整数をゲットする。
 * NPCのテキスト解析にやくだつ。
 * ただし末尾にいらん文字がある場合無視する。
 * 信岡が落ちないアルゴリズムに変更。
 * 引数
 *  hoge    char*   解析する文字列
 * 返り値
 *  解析した整数。
 *
 * 例：
 * "ringo ( 吧噗吧噗星人 ) ： 100"
 *      の場合は100がかえる。
 * "ringo ( 吧噗吧噗星人 ) ： 是100"
 *      も100になる。ほんまにNPC用なりね。
 ----------------------------------------*/
int texttoiTail( char *hoge)
{
    return 0; /* ばぐってたのでこうした。そのうち関数ごと消す。 */


#if 0
    int len = strlen(hoge);

    if( len == 0 )return atoi(hoge);
    /*  後から探して最初に数字を見つける    */
    while(--len && len >= 0){
        if( isdigit(hoge[len]) )
            break;
    }
    while( --len && len >= 0 ){
        if( !isdigit(hoge[len]) )
            break;
        if( hoge[len]=='+' || hoge[len]=='-')
            break;
    }
    return atoi(hoge+len+1);
#endif
#if 0
    int f = 0;
    for(i=len-1;i>=0;i--){
        if( isdigit( hoge[i] ) ) f = 1;
        if( (f == 1)
            && !( isdigit(hoge[i]) || hoge[i]=='+' || hoge[i]=='-') ){
            return atoi( hoge + i + 1 );
        }
    }
    return atoi(hoge);
#endif
}

/*----------------------------------------
 * 文字列を数字かどうかチェックして、数字だったら
 *  引数に入れて返す
 * 引数
 *      arg             変換元
 *      number          変換後を代入する所
 *      base            基数
 *      type            引数の型( CTYPE は common.h に宣言されている )
 * 返り値
 *      TRUE(1)    成功
 *      FALSE(0)   strtolが完璧には成功しなかった
 *                  この時のnumberはstrtolの返り値である
 ----------------------------------------*/
BOOL strtolchecknum( char* arg , void* number,int base ,CTYPE type)
{
    char* buf;
    int     num;

    num = strtol( arg, &buf, base);

    switch( type ){
    case CHAR_SA:
        *(char*)number = (char)num;
        break;
    case SHORT_SA:
        *(short*)number = (short)num;
        break;
    case INT_SA:
        *(int*)number = num;
        break;
    case DOUBLE_SA:
        *(double*)number = (double)num;
        break;
    default:
        break;
    }


    if( strlen( buf ) >= 1 )
        /*
         * 残り文字があるという事は変換しきれてないという事なのだ。
         */
        return FALSE;

    return TRUE;

}




/*----------------------------------------
 * src から dels で指定した文字を取り除く
 * バックスラッシュはエスケープである事に注意。
 * つまりバックスラッシュは削除できない。
 * 引数
 *  src    変更される文字列
 *  char*  削除する文字(文字列で指定可)
 * 返り値
 *  なし
 ----------------------------------------*/
void deleteCharFromString( char* src , char* dels )
{
    int index=0;    /* できあがりの文字列での index */
    int delength;   /* dels の長さを設定する(少しは速度アップの為 */
    int i=0,j;/* i は src をループする変数 j は dels をループする変数 */

    delength= strlen( dels );

    while( src[i] != '\0' ){
        if( src[i] == BACKSLASH  )
            /*
             * バックスラッシュなのでエスケープする
             * つまり次の文字は無条件に移動する
             */
            src[index++] = src[++i];         /* substitute next char */

        else{
            for( j = 0 ; j < delength ; j ++ ){
                if( src[i] == dels[j] )
                    /*
                     * 削除する文字だ。よって i に 1 をたす
                     */
                    goto incrementi;
            }
            /*
             * 消す物に指定されていないのでコピーする。
             */
            src[index++] =  src[i];
        }
    incrementi:
        i++;
    }

    src[index] = '\0';
}



/*----------------------------------------
 * src から dels で指定した文字を取り除く
 * エスケープはない
 * 引数
 *  src    変更される文字列
 *  char*  削除する文字(文字列で指定可)
 * 返り値
 *  なし
 ----------------------------------------*/
void deleteCharFromStringNoEscape( char* src , char* dels )
{
    int index=0;    /* できあがりの文字列での index */
    int delength;   /* dels の長さを設定する(少しは速度アップの為 */
    int i=0,j;/* i は src をループする変数 j は dels をループする変数 */

    delength= strlen( dels );

    while( src[i] != '\0' ){
        for( j = 0 ; j < delength ; j ++ ){
            if( src[i] == dels[j] )
                /*
                 * 削除する文字だ。よって i に 1 をたす
                 */
                goto incrementi;
        }
        /*
         * 消す物に指定されていないのでコピーする。
         */
        src[index++] =  src[i];
    incrementi:
        i++;
    }

    src[index] = '\0';
}

/*------------------------------------------------------------
 * 指定された文字を指定された文字で置きかえる
 * 引数
 *  src     char*       変更される文字列
 *  oldc    char        変更される文字
 *  newc    char        変更する文字
 * 返り値
 *  src
 ------------------------------------------------------------*/
char*   replaceString( char* src, char oldc ,char newc )
{
    char*   cp=src;

    do{
        if( *cp == oldc ) *cp=newc;
    }while( *cp++ );
    return src;
}

typedef struct tagEscapeChar
{
    char     escapechar;
    char     escapedchar;
}EscapeChar;
static EscapeChar   escapeChar[]=
{
    { '\n',   'n' },
    { ',',    'c' },
    { '|',    'z' },
    { '\\',   'y' },
};


char makeCharFromEscaped( char c )//add this function,because the second had it
{
        int i;
        
        for( i = 0; i < sizeof( escapeChar )/sizeof( escapeChar[0] ); i++ )
        {
            if( escapeChar[i].escapedchar == c )
            {
               c = escapeChar[i].escapechar;
               break;
            }
        }
        return c;
}
/*----------------------------------------
 * makeEscapeStringで作成された文字列を元に戻す
 * 引数
 *  src             char*       元になる文字列。これが書き替る。
 * 返り値
 *  src    を返す。(関数を入れこにしやすいように)
 ----------------------------------------*/
char   *makeStringFromEscaped( char* src )
{//ttom this function all change,beside copy from the second

    int     i;
    // CoolFish: Fix bug 2001/10/13
    // int     srclen = strlen( src );
    int	    srclen = 0;
    int     searchindex=0;
    
    // CoolFish: Fix bug 2001/10/13
    if (!src)	return	NULL;
    srclen = strlen(src);
    
    for( i = 0; i < srclen; i ++ )
    {
    // for 2Byte Word
    if( IS_2BYTEWORD( src[i] ) ){
        src[searchindex++] = src[i++];
        src[searchindex++] = src[i];
    }else
            if( src[i] == '\\' )
            {	
                // 次の文字に行く
                i++;
                src[searchindex++] = makeCharFromEscaped( src[i] );
            }
            else
            {
                src[searchindex++] = src[i];
            }
    }        
            src[searchindex] = '\0';
        
            return src;
}

char*  makeEscapeString( char* src , char* dest, int sizeofdest)
{ //ttom this function all change, copy from the second
    int i;
    int	    srclen = 0;
    int     destindex=0;
    
    // CoolFish: Fix bug 2001/10/13
    if (!src)	return	NULL;
    srclen = strlen(src);
  
    for( i = 0 ; i < srclen ; i ++ ){
         BOOL dirty=FALSE;
         int  j;
         char escapechar='\0';
		if( destindex + 1 >= sizeofdest )break;
         if( IS_2BYTEWORD( src[i] ) ){
               if( destindex + 2 >= sizeofdest )break;
               
            dest[destindex] = src[i];
            dest[destindex+1] = src[i+1];
                 destindex += 2;
                 i ++;
                 continue;
          }
          for( j = 0; j<sizeof(escapeChar)/sizeof(escapeChar[0]); j++){
               if( src[i] == escapeChar[j].escapechar ){
                   dirty=TRUE;
                   escapechar= escapeChar[j].escapedchar;
                   break;
               }
          }
          if( dirty == TRUE ){
              if( destindex + 2 < sizeofdest ){
                  dest[destindex] = '\\';
                  dest[destindex+1] = escapechar;
                  destindex+=2;
                  dirty=TRUE;
                  continue;
              }else{
                  dest[destindex] = '\0';
                  return dest;
              }
          }else{
                dest[destindex] = src[i];
                destindex++;
          }
     }
     dest[destindex] = '\0';
     return dest;
}

//this function copy all from the second
char * ScanOneByte( char *src, char delim ){
	// Nuke
	if (!src) return NULL;

        // 文字列がなくなるまで検索
        for( ;src[0] != '\0'; src ++ ){
             // 全角の１バイト目かどうかをチェック
          if( IS_2BYTEWORD( src[0] ) ){
              // 全角だ。その場合は１バイト余分に進ませる。
              // ただし１バイトしかない場合はそうしない
              if( src[1] != 0 ){
                  src ++;
              }
              continue;
          }
          // 半角だった。ここで対象の文字と比較
          if( src[0] == delim ){
              return src;
          }
        }
        // ループ抜けたら見つからなかった。
        return NULL;
}
                                                                                                                                                                                                                                                                                                                


/*----------------------------------------
 * delim で指定された文字列を区切り文字として
 * index 番目の物を得る。indexは1ではじまる。
 * 文字列仕様。
 * 引数
 *  src     元の文字列
 *  delim   デリミタとなる文字列。最後はNULL文字で終っている事
 *  index   何番目の物を取るか
 *  buf     結果文字列を格納する場所へのポインター
 *  buflen  結果文字列を格納する場所のサイズ
 * 返り値
 *  あったら TRUE(1);
 *  なかったら FALSE(0);
 *  ex
 *      getStringFromIndexWithDelim( "aho=hoge","=",1,buf,sizeof(buf) );
 *      buf ... aho
 *
 *      getStringFromIndexWithDelim( "aho=hoge","=",2,buf,sizeof(buf) );
 *      buf ... hoge
 *      これも認められる。
 *
 *      getStringFromIndexWithDelim( "aho=hoge","=",3,buf,sizeof(buf) );
 *      返り値 FALSE
 ----------------------------------------*/
BOOL getStringFromIndexWithDelim_body( char* src ,char* delim ,int index,
                                 char* buf , int buflen ,
                                       char *file, int line )
{//ttom this function all change,copy from the second
    int i;          /* ループ変数 */
    int length =0;  /* 取り出した文字列の長さ */
    int addlen=0;   /* 足される長さ */
    int oneByteMode = 0; /* １バイトモードか？ */

    if( strlen( delim ) == 1 ){ // 検索が１バイトなら１バイトモードにする
        oneByteMode = 1;// その際２バイト文字はチェックしない
    }
    for( i =  0 ; i < index ; i ++ ){
         char* last;
         src += addlen;/* みつかった長さを足す */
      
         if( oneByteMode ){
             // １バイトモードだったらこちらで検索
             last = ScanOneByte( src, delim[0] );
         }else{
                 last  = strstr( src , delim );  /* 見付ける */
         }
         if( last == NULL ){
            /*
             * みつからなかったのですべてコピーして return。
            */
            strcpysafe( buf , buflen, src );

            if( i == index - 1 )
                /*ちょうどみつかった*/
                return TRUE;
                                                                                                           
                /*みつからなかった*/
             return FALSE;
          }
          
          /*
           * みつかった所と最初の位置の差を求める
           * つまり区切られている文字列の長さ
          */
          length = last - src;
                                           
          /*
           * 次のループの為にみつかった長さと delim の長さを足しておく
          */
          addlen= length + strlen( delim );
       }
       strncpysafe( buf, buflen , src,length );

       return TRUE;
}


/*------------------------------------------------------------
 * 文字列 "a,b,c,d" を引数に代入する。デフォルトは 0 にする。
 * かなりの特殊関数。現在はちょっと遅い。
 * 引数
 *  src         char*   元の文字列
 *  int1        int*    intのポインタ。(aを代入する)
 *  int2        int*    intのポインタ。(bを代入する)
 *  int3        int*    intのポインタ。(cを代入する)
 *  int4        int*    intのポインタ。(dを代入する)
 * 返り値
 *  なし
 ------------------------------------------------------------*/
void getFourIntsFromString(char* src,int* int1,int* int2,int* int3,
                           int* int4)
{
    int     ret;
    char    string[128];

    ret = getStringFromIndexWithDelim( src,"," ,1,string,sizeof(string));
    if( ret == FALSE )*int1=0;
    else    *int1 = atoi(string);

    ret = getStringFromIndexWithDelim( src, ",",2,string,sizeof(string) );
    if( ret == FALSE )*int2=0;
    else    *int2 = atoi(string);

    ret = getStringFromIndexWithDelim( src, ",",3,string,sizeof(string) );
    if( ret == FALSE )*int3=0;
    else    *int3 = atoi(string);

    ret = getStringFromIndexWithDelim( src, ",",4,string,sizeof(string) );
    if( ret == FALSE )*int4=0;
    else    *int4 = atoi(string);

}


/*----------------------------------------------
 * src の中でdelsで指定した文字が連続していたら
 * 1つにまとめる。
 * バックスラッシュはエスケープシーケンスとなる。
 * したがって連続したバックスラッシュを消す事はできない。
 * 引数
 *  src    変化する文字列
 *  dels   消す文字(文字列で指定可)
 * 返り値
 *  なし
 ---------------------------------------------*/
void deleteSequentChar( char* src , char* dels )
{
    int length;         /* src の長さ */
    int delength;       /* dels の長さ */
    int i,j;            /* ループ変数 */
    int index=0;        /* 確定文字列の index */
    char backchar='\0';   /* 削除する文字 */

    length = strlen( src );
    delength = strlen( dels );

    /*
     * ループするごとにコピーする
     */
    for( i = 0 ; i < length ; i ++ ){
        if( src[i] == BACKSLASH ){
            /*
             * バックスラッシュだったら
             * 強制的に二つ( BACKSLASH自身と、次の文字 )
             * 進める
             */
            src[index++]=src[i++];
            if( i >= length ){
                print( "*\n");
                break;
            }
            src[index++]=src[i];
            /* 連続が止った事にする */
            backchar = '\0';
            continue;
        }
        if( src[i] == backchar )
            /*
             * 削除するもじだったら何もしない。つまり削除になる。
             */
            continue;

        /*
         * 連続の削除する文字がなくなったので backchar は NULL 文字にする
         * これがかんじん
         */
        backchar = '\0';
        for( j = 0 ; j < delength ; j ++ ){
            if( src[i] == dels[j] ){
                /*
                 * 削除する文字列だ。 ここで continue しない事により
                 * 一文字は削除しない事が保証できる。
                 */
                backchar=src[i];
                break;
            }
        }
        src[index++]=src[i];
    }
    src[index++]='\0';
}

/*----------------------------------------
 * hash値を返す。文字列にのみ使うこと
 * これは本そのままなのでアルゴリズムはようわからんが
 * いいアルゴリズムらしい。
 * 引数
 *  s   文字列
 * 返り値
 *  int ハッシュ値
 *----------------------------------------*/
#define PRIME 211
int hashpjw ( char* s )
{
    char *p;
    unsigned int h= 0 ,g;
    for( p = s ; *p ; p ++ ){
        h = ( h<< 4 ) + (*p);
        if( (g = h & 0xf0000000) != 0){
            h = h ^ (g>>24);
            h = h ^ g;
        }
    }
    return h % PRIME;
}

/*----------------------------------------
 * ローカルにバインドしたソケットを返す( TCP )
 * 引数
 *          port        ローカルポート
 * 返り値
 *          -1      エラー
 *                      1. socketシステムコールのエラー
 *                      2. bindシステムコールのエラー
 *                      3. listenシステムコールのエラー
 ----------------------------------------*/
int bindlocalhost( int port )
{
    struct sockaddr_in sin;         /*ローカルアドレス用*/
    int sfd;                        /*ソケットディスクリプタ*/
    int rc;                         /*ローカルリターンコード*/

    /*AF_INETプロトコル の SOCK_STREAM */
    sfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( sfd == -1 ){
        print( "%s\n" , strerror(errno ) );
        return -1;
    }
    
    if( getReuseaddr() ) {
		int sendbuff;
	    /* ローカルアドレスの再利用 */
		setsockopt( sfd, SOL_SOCKET, SO_REUSEADDR, 
						(char *)&sendbuff, sizeof( sendbuff));
	}
	
    /*0クリアして値を代入*/
    memset( &sin ,0, sizeof( struct sockaddr_in ) );
    sin.sin_family=AF_INET;
    sin.sin_port = htons( port );
    sin.sin_addr.s_addr = INADDR_ANY;

    rc = bind( sfd , (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
    if( rc == -1 ){
        print( "%s\n" , strerror(errno ) );
        return -1;
    }
	
	
    rc = listen( sfd , 5 );
    if( rc == -1 ){
        print( "%s\n" , strerror(errno ) );
        return -1;
    }
    return sfd;
}



/*----------------------------------------
 * コネクトする。( TCP )
 * 引数
 *      hostname    接続するホスト名
 *                      (xx.xxx.xxx.xxxでもいいしxxx.co.jpとかでもいい)
 *      port        接続するホストのポート名
 * 返り値
 *      ディスクリプタ( -1 の時はエラー )
 ----------------------------------------*/
int connectHost( char* hostname , unsigned short port )
{
    struct sockaddr_in sock;        /*connectの時に使う*/
    struct hostent* hoste;          /*hostnameのhostent*/
    int     fd;                     /*コネクトしたソケットディスクリプタ*/
    int     lr;                     /*ローカルリターンコード*/

    memset( &sock ,  0 , sizeof( struct sockaddr_in ) );
    sock.sin_family = AF_INET;
    sock.sin_port = htons( port );

    /* dot notation かどうかを調べる */
    sock.sin_addr.s_addr = inet_addr( hostname );
    if( sock.sin_addr.s_addr == -1 ){
        /*
         *エラー dot notation ではない。したがって普通の名前解決が必要だ。
         * dnsをひかなければならない時はひきにいくのもここでやってくれる。
         */
        hoste = gethostbyname( hostname );
        if( hoste == NULL ){
            print( "gethostbyname error hostname: %s\n", hostname);
            return -1;
        }

        memcpy((void*)&sock.sin_addr.s_addr ,
               hoste->h_addr , sizeof(struct in_addr) );
    }

    /*ソケットを作る*/
    fd = socket( AF_INET, SOCK_STREAM , 0 );
    if( fd == -1 ){
        print("Cannot Create Socket(%s errno:%d)\n"
              ,strerror( errno ) ,errno );
        return -1;
    }
    /*コネクトする*/
    lr = connect(fd,(struct sockaddr*)&sock,sizeof(struct sockaddr_in));
    if( lr != 0 ){
        print("Cannot connect. (%s errno:%d)\n"
              ,strerror( errno ) ,errno );
        return -1;
    }

    return fd;
}


/*----------------------------------------
 * バッファの中に改行記号があるかどうか。
 * 少し早いアルゴリズムに変更。
 * 引数
 *  char*   調べる文字列
 * 返り値
 *  最初に見つかた改行記号の位置 配列アクセスした時にいい値。
 *  どういう事かというと、
 *      existsNewLinwCharacter( "aho\nhoge\n" )
 *  だと、返り値は 3 になる。これに注意する事。
 *  -1      なかった
 *
 ----------------------------------------*/
int existsNewLineCharacter( char* line )
{
#if 1
    char*   old = line;
    do{
        if( *line == NEWLINE)return line-old;
    }while( *(line++) );
#else
    int i;                          /*ループ変数*/
    int length = strlen( line );    /*調べる文字列の長さ*/
    for( i = 0 ; i < length ; i ++ ){
        if( line[i] == NEWLINE )
            return i;
    }
#endif

    return -1;
}



/*----------------------------------------
 *  文字列中に文字が表われる場所をかえす。何番目かを決めれる。
 *  nindex( string , c , 1 ) と index( string , c ) は equivalent である。
 *  numberが負または0の時は返り値は、stringと同じである。
 *
 *  引数
 *      string  char*   調べる文字列
 *      c       int     調べる文字
 *      number  int     表わる番号
 *  返り値
 *      一致した文字のポインター
 *      NULL    みつからなかった。
 ----------------------------------------*/
char* nindex( char* string, int c , int number )
{
    int i;                          /*ループ変数*/
    int num=0;
    int length = strlen( string );  /*調べる文字列の長さ*/
    if( number <= 0 )return string;
    for( i = 0 ; i < length ; i ++ ){
        if( string[i] == c )num++;
        if( number == num ) return &string[i];
    }
    return NULL;
}

BOOL rrd( char* dirname , STRING64* buf ,int bufsize, int* index)
{
    DIR*    d;
    char    dirn[1024];

    d = opendir( dirname );
    if( d == NULL )return FALSE;

    while( 1 ){
        struct dirent* dent;
        struct  stat    st;
        dent = readdir( d );
        if( dent == NULL ){
            if( errno == EBADF ){
                errorprint;
                closedir(d);
                return FALSE;
            }else
                /*  successful */
                break;
        }

        /*  . で始まるファイルは読まない    */
        if(  dent->d_name[0] == '.' )continue;

        snprintf(dirn, sizeof(dirn), "%s/%s" , dirname,dent->d_name );
        if( stat( dirn , &st ) == -1 )continue;
        if( S_ISDIR( st.st_mode ) ){
            if( rrd( dirn , buf ,bufsize, index) == FALSE ){
                closedir(d);
                return FALSE;
            }
        }else{
            if( *index >= bufsize )
                break;

            strcpysafe( buf[*index].string , sizeof( buf[*index].string ), dirn );
            (*index)++;
        }
    }
    closedir(d);
    return TRUE;
}

int rgetFileName( char* dirname , STRING64* string, int size)
{
    int     index=0;
    if( rrd( dirname , string ,size ,&index ) == FALSE )return -1;
    else return index;
}

BOOL checkStringsUnique( char** strings, int num ,int verbose)
{
    int     i,j;
    for( i = 0 ; i < num - 1; i ++ ){
        for( j = i+1 ; j < num ; j ++){
            if( !strcmp( strings[i] ,strings[j] )){
                if( verbose )
                    print( "Overlapp string is %s\n", strings[i] );
                return FALSE;
            }
		}
	}
    return TRUE;
}

BOOL PointInRect( RECT_SA* rect, POINT_SA* p )
{
    if( rect->x         <= p->x && p->x <= rect->x + rect->width &&
        rect->y         <= p->y && p->y <= rect->y + rect->height )
        return TRUE;
    return FALSE;
}

BOOL CoordinateInRect( RECT_SA* rect, int x, int y)
{
    POINT_SA   p={x,y};
    return PointInRect(rect,&p);
}

int clipRect( RECT_SA *rect1, RECT_SA *rect2, RECT_SA *ret )
{
    if(   rect1->x > rect2->x + rect2->width -1
       || rect2->x > rect1->x + rect1->width -1
       || rect1->y > rect2->y + rect2->height -1
       || rect2->y > rect1->y + rect1->height -1 )
	   return 0;
    ret->x = max( rect1->x, rect2->x );
    ret->y = max( rect1->y, rect2->y );
    ret->width = min( rect1->x+rect1->width, rect2->x+rect2->width ) - ret->x;
    ret->height = min( rect1->y+rect1->height, rect2->y+rect2->height ) - ret->y;
    return 1;
}

BOOL isstring1or0( char*  string )
{
    if( strcasecmp(string,"TRUE" ) == 0 )return TRUE;
    if( strcasecmp(string,"FALSE") == 0 )return FALSE;
    if( strcasecmp(string,"1"    ) == 0 )return TRUE;
    if( strcasecmp(string,"0"    ) == 0 )return FALSE;
    if( strcasecmp(string,"ON"   ) == 0 )return TRUE;
    if( strcasecmp(string,"OFF"  ) == 0 )return FALSE;
    return FALSE;
}

void easyGetTokenFromString( char *src,int count,char*output,int len )
{
    int i;
    int counter = 0;

    if( len <= 0 )return;


#define ISSPACETAB( c )   ( (c) == ' ' || (c) == '\t' )

    for(i=0;;i++){
        if( src[i]=='\0'){
            output[0] = '\0';
            return;
        }
        if( i > 0 && ! ISSPACETAB( src[i-1] ) &&
            ! ISSPACETAB(  src[i] ) ){
            continue;
        }

        if( ! ISSPACETAB( src[i]) ){
            counter++;
            if( counter == count){
                /* copy it */
                int j;
                for(j=0;j<len-1;j++){
                    if( src[i+j] == '\0' ||
                        ISSPACETAB( src[i+j] ) ){
                        break;
                    }
                    output[j]=src[i+j];
                }
                output[j]='\0';
                return;

            }
        }

    }
}

/*------------------------------------------------------------
 * 線型で内分点をもとめる。 by ringo
 * 船の移動用
 *
 * double val1 , val2 : この値のあいだをとる
 * double d : 内分率
 *
 *
 *  -d<0----- val1 ---0<d<1------- val2 ------d>1----
 *
 *
 ------------------------------------------------------------*/
float  linearDiv( float val1 , float val2 , float d )
{
    return val1 + ( val2 - val1 ) * ( d );
}


/*------------------------------------------------------------
 * 文字列のなかから任意な文字をlen文字とりだしてランダムな
 * 文字列をつくる。NPCのランダムパスワード生成につかってる
 *
 * char *cand : ここから選ぶ。候補の文字。
 * char *out : 出力バッファ
 * int len : 出力の長さ
 ------------------------------------------------------------*/
void makeRandomString( char *cand , char *out , int len )
{
    int i;
    int l = strlen( cand );

    for(i=0;i<len;i++){
        out[i] = cand[ rand() % l];
    }
    out[i] = '\0';
}


/*------------------------------------------------------------
 * 引数に指定されたファイルがファイルかどうを調べる
 * 引数
 *  filename        char*        ファイル名
 * 返り値
 *  ファイル        TRUE(1)
 *  ファイルでない  FALSE(0)
 ------------------------------------------------------------*/
BOOL isExistFile( char* filename )
{
    /*  fopen で調べる  */
    FILE*   fp;
    fp = fopen( filename, "w" );
    if( fp ){
        fclose( fp );
        return TRUE;
    }else
        return FALSE;
}
/*------------------------------------------------------------
 * 10進→62進変換
 * 引数
 * a        int     元の１０進
 * out      char *  格納バッファー
 * outlen   int     格納バッファの大きさ
 * 戻り値
 *          成功    outのアドレス
 *          失敗    NULL
 ------------------------------------------------------------*/
char *cnv10to62( int a, char *out, int outlen )
{
#if 1
	int		i, j;
    char    base[] = { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    int     tmp[64];
    int     src;
    int minus;
	int baselen = sizeof( base)-1;
    if( a < 0 ){
		minus = 1;
		a *= -1;
	} else {
		minus = 0;
	}
    /* special case */
    if( a < baselen) {
		if( minus ){
			*(out) = '-';
			*(out+1) = base[a];
			*(out+2) = '\0';
			return (out);
		} else {
			*out = base[a];
			*(out+1) = '\0';
			return( out);
		}
    }
    src = a;
    for( i = 0; src >= baselen; i ++ ) {
        tmp[i] = src % baselen;
        src /= baselen;
    }
    i--;
    if( minus ){
		*out = '-';
     	*(out+1) = base[src];
		for( j = 2; i >= 0; i --, j ++ ) {
			if( j > outlen - 2 ) return NULL;
			*(out+j) = base[tmp[i]];
		}
	} else {
     	*out = base[src];
		for( j = 1; i >= 0; i --, j ++ ) {
			if( j > outlen - 2 ) return NULL;
			*(out+j) = base[tmp[i]];
		}
	}
	*(out+j) = '\0';
    return( out);
#else
/* 古い方 */
#define     CNV_NUMBER  62
	int		i, j;
    char    base[] = { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    int     tmp[64];
    int     src;
    if( a < 0 ) return( NULL);
    if( a < CNV_NUMBER) {
	    *out = base[a];
	    *(out+1) = '\0';
        return( out);
    }
    src = a;
    for( i = 0; src >= CNV_NUMBER; i ++ ) {
        tmp[i] = src % CNV_NUMBER;
        src /= CNV_NUMBER;
    }
    i--;
	*out = base[src];
    for( j = 1; i >= 0; i --, j ++ ) {
		if( j > outlen - 2 ) return NULL;
		*(out+j) = base[tmp[i]];
    }
	*(out+j) = '\0';
    return( out);
#undef      CNV_NUMBER
#endif
}
/* 
 * 与えられた配列の中で，数値が重複しているか調べる関数
 * -1 は対象外という変な関数
 *
 * 引数
 * src		*int		調べる元の配列
 * srclen	int			src の大きさ
 *
 * 戻り値   重複あり	TRUE
 *				なし	FALSE
 */
BOOL checkRedundancy( int *src, int srclen)
{
	int		i,j;
	int		ret = FALSE;
	
	for( i = 0; i < srclen; i ++ ) {
		if( *(src + i) != -1 ) {
			for( j = i+1; j < srclen; j ++ ) {
				if( *(src +i) == *( src+j) ) {
					ret = TRUE;
					break;
				}
			}
		}
	}
	return ret;
}
void
shuffle_ints( int num, int *a, int t )
{
    int i;
    for(i=0;i<t; i++ )
    {
        int x = random() % num;
        int y = random() % num;
        int s;
        s = a[x];
        a[x] = a[y];
        a[y] = s;
    }
}


static unsigned char BitTable[] =	/*	ビットの並び順を逆にするテーブル	*/
{
		0x00 , 0x80 , 0x40 , 0xC0 , 0x20 , 0xA0 , 0x60 , 0xE0 , 
		0x10 , 0x90 , 0x50 , 0xD0 , 0x30 , 0xB0 , 0x70 , 0xF0 , 
		0x08 , 0x88 , 0x48 , 0xC8 , 0x28 , 0xA8 , 0x68 , 0xE8 , 
		0x18 , 0x98 , 0x58 , 0xD8 , 0x38 , 0xB8 , 0x78 , 0xF8 , 
		0x04 , 0x84 , 0x44 , 0xC4 , 0x24 , 0xA4 , 0x64 , 0xE4 , 
		0x14 , 0x94 , 0x54 , 0xD4 , 0x34 , 0xB4 , 0x74 , 0xF4 , 
		0x0C , 0x8C , 0x4C , 0xCC , 0x2C , 0xAC , 0x6C , 0xEC , 
		0x1C , 0x9C , 0x5C , 0xDC , 0x3C , 0xBC , 0x7C , 0xFC , 
		0x02 , 0x82 , 0x42 , 0xC2 , 0x22 , 0xA2 , 0x62 , 0xE2 , 
		0x12 , 0x92 , 0x52 , 0xD2 , 0x32 , 0xB2 , 0x72 , 0xF2 , 
		0x0A , 0x8A , 0x4A , 0xCA , 0x2A , 0xAA , 0x6A , 0xEA , 
		0x1A , 0x9A , 0x5A , 0xDA , 0x3A , 0xBA , 0x7A , 0xFA , 
		0x06 , 0x86 , 0x46 , 0xC6 , 0x26 , 0xA6 , 0x66 , 0xE6 , 
		0x16 , 0x96 , 0x56 , 0xD6 , 0x36 , 0xB6 , 0x76 , 0xF6 , 
		0x0E , 0x8E , 0x4E , 0xCE , 0x2E , 0xAE , 0x6E , 0xEE , 
		0x1E , 0x9E , 0x5E , 0xDE , 0x3E , 0xBE , 0x7E , 0xFE , 
		0x01 , 0x81 , 0x41 , 0xC1 , 0x21 , 0xA1 , 0x61 , 0xE1 , 
		0x11 , 0x91 , 0x51 , 0xD1 , 0x31 , 0xB1 , 0x71 , 0xF1 , 
		0x09 , 0x89 , 0x49 , 0xC9 , 0x29 , 0xA9 , 0x69 , 0xE9 , 
		0x19 , 0x99 , 0x59 , 0xD9 , 0x39 , 0xB9 , 0x79 , 0xF9 , 
		0x05 , 0x85 , 0x45 , 0xC5 , 0x25 , 0xA5 , 0x65 , 0xE5 , 
		0x15 , 0x95 , 0x55 , 0xD5 , 0x35 , 0xB5 , 0x75 , 0xF5 , 
		0x0D , 0x8D , 0x4D , 0xCD , 0x2D , 0xAD , 0x6D , 0xED , 
		0x1D , 0x9D , 0x5D , 0xDD , 0x3D , 0xBD , 0x7D , 0xFD , 
		0x03 , 0x83 , 0x43 , 0xC3 , 0x23 , 0xA3 , 0x63 , 0xE3 , 
		0x13 , 0x93 , 0x53 , 0xD3 , 0x33 , 0xB3 , 0x73 , 0xF3 , 
		0x0B , 0x8B , 0x4B , 0xCB , 0x2B , 0xAB , 0x6B , 0xEB , 
		0x1B , 0x9B , 0x5B , 0xDB , 0x3B , 0xBB , 0x7B , 0xFB , 
		0x07 , 0x87 , 0x47 , 0xC7 , 0x27 , 0xA7 , 0x67 , 0xE7 , 
		0x17 , 0x97 , 0x57 , 0xD7 , 0x37 , 0xB7 , 0x77 , 0xF7 , 
		0x0F , 0x8F , 0x4F , 0xCF , 0x2F , 0xAF , 0x6F , 0xEF , 
		0x1F , 0x9F , 0x5F , 0xDF , 0x3F , 0xBF , 0x7F , 0xFF
};
static unsigned short crctab16[] =	/*	crc　の計算テーブル		*/
{
		0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
		0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
		0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
		0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
		0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
		0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
		0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
		0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
		0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
		0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
		0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
		0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
		0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
		0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
		0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
		0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
		0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
		0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
		0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
		0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
		0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
		0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
		0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
		0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
		0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
		0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
		0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
		0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
		0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
		0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
		0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
		0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0,
};
unsigned short CheckCRC( unsigned char *p , int size )
{
	unsigned short	crc = 0;
	int		i;
	
	for( i = 0 ; i < size ; i ++ ){
		crc = ( crctab16[ ( crc >> 8 ) & 0xFF ] 
					^ ( crc << 8 ) ^ BitTable[ p[ i ] ] );
	}
	return crc;
}

// Add Code By Shan  2001.06.16
INLINE double time_diff(struct timeval subtrahend,
                      struct timeval subtractor)
{
    return( (subtrahend.tv_sec - subtractor.tv_sec)
             + (subtrahend.tv_usec
             - subtractor.tv_usec  ) / (double)1E6 );
}
