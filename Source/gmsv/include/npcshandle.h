#ifndef _NPCS_HANDLE_H
#define _NPCS_HANDLE_H
#include "version.h"
#include "util.h"

#ifdef _NPCSERVER_NEW
//條件判斷
BOOL NPCS_HandleCheckFreeMess( int npcobjindex, int charaindex, int charobjindex,
				char *CheckfreeMess);

BOOL NPCS_FreePassCheck( int charaindex, char *buf);

//比較判斷
BOOL NPCS_FreeBigSmallCheck( int charaindex,char* buf);
//參數判斷
BOOL NPCS_ArgumentFreeCheck( int charaindex, char* Argument, int amount, int temp, int probjID);
//參數大小判斷
BOOL NPCS_ArgumentBigSmallCheck( int point, int mypoint, int flg);
//取得指定ID寵物數	*reAmount 剩餘欄位
int NPCS_getUserPetAmount( int charaindex, int objID, int flg);
//取得指定ID道具數	*reAmount 剩餘欄位
int NPCS_getUserItemAmount( int charaindex, int objID, int flg);
//取得團隊人數
int NPCS_getUserPartyAmount( int charaindex );
BOOL NPCS_NpcstalkToCli( int charaindex,int npcobjindex, char* message, CHAR_COLOR color );
//事件處理
BOOL NPCS_RunDoEventAction( int charaindex, char *buf1);
#endif
#endif
