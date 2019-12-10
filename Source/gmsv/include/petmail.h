#ifndef  __PETMAIL_H__
#define __PETMAIL_H__
#include "version.h"
/*
 * ペット関連の設定です。
 */
#define PETMAIL_OFFMSG_MAX          10000
#define PETMAIL_OFFMSG_TIMEOUT      ( 3 * 24 * 3600 )
#define PETMAIL_CHECK_OFFMSG_EXPIRE_INTERVAL  3600
#define PETMAIL_OFFMSG_TEXTLEN 512

/* ペットメールの出現効果の数。実際の数より１少なくすること */
#define		 PETMAIL_EFFECTMAX	1			

#define		PETMAIL_SPOOLFLOOR		777
#define		PETMAIL_SPOOLX			30
#define		PETMAIL_SPOOLY			30

#define		PETMAIL_LOOPINTERVAL1	500
#define		PETMAIL_LOOPINTERVAL2	5000

/* オフラインのキャラへのメッセージを覚える */
typedef struct
{
    int use;
    time_t send_tm;     /* ユーザがメッセージを送信した時間+TIMEOUT。 */
    int color;							/* テキストの色 */
    char text[PETMAIL_OFFMSG_TEXTLEN];     /* テキストの内容 */
    char destcd[CDKEYLEN];				/* 送信先CDKEY */
    char destcharname[CHARNAMELEN];		/* 送信先のキャラ名 */
    char srccd[CDKEYLEN];				/* 送信元のcdkey */
    char srccharname[CHARNAMELEN];		/* 送信元のキャラ名 */
	
} PETMAIL_offmsg;

void PETMAIL_Loopfunc( int index);
BOOL PETMAIL_CheckPlayerExist( int index, int mode);

BOOL PETMAIL_initOffmsgBuffer( int count );
BOOL PETMAIL_addOffmsg( int fromindex, char *tocdkey, char *tocharaname,
                            char *text, int color );
PETMAIL_offmsg *PETMAIL_getOffmsg( int offmsgindex);
BOOL PETMAIL_deleteOffmsg( int offmsgindex);
void PETMAIL_proc( void );
BOOL storePetmail( void);
BOOL PETMAIL_sendPetMail( int cindex, int aindex, 
					int havepetindex, int haveitemindex, char* text , int color );

#ifdef _PETMAIL_DEFNUMS
void CHAR_AutoPickupMailPet( int charaindex, int petindex );
#endif
int PETMAIL_getPetMailTotalnums( void);
void PETMAIL_delPetMailTotalnums( int numflg);
void PETMAIL_setPetMailTotalnums( int numflg);
int PETMAIL_CheckIsMyOffmsg( int fromindex, char *tocdkey, char *tocharaname);

#endif
