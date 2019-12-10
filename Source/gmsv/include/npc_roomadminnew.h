#ifndef __NPC_ROOMADMINNEW_H__
#define __NPC_ROOMADMINNEW_H__


void NPC_RoomAdminNewTalked( int meindex , int talkerindex , char *msg , int color );
void NPC_RoomAdminNewLoop( int meindex );
BOOL NPC_RoomAdminNewInit( int meindex );

BOOL NPC_RankingInit( int meindex );
void NPC_RankingTalked( int meindex , int talkerindex , char *msg ,int color );

BOOL NPC_PrintpassmanInit( int meindex );
void NPC_PrintpassmanTalked( int meindex , int talkerindex , char *msg ,int color );

#if 0
typedef struct roomadmin_tag
{
    int index;              /* 部屋のindex */
    
    char doorname[256];     /* その部屋のドアの名前 */
    char explanation[256];  /* 説明 */
    char passwd[256];       /* パスワード */
    int expire_time_mod;    /* 位相 */
    time_t expire_time;     /* 期限切れの時間 */
    int least_cost;         /* 最低金額 */
    
}NPC_ROOMINFO;
#endif

typedef struct npc_roomadminnew_tag {
	int		expire;
	char	cdkey[CDKEYLEN];
	char	charaname[32];
	char	passwd[9];
}NPC_ROOMINFO;

typedef struct npc_roomadminnew_ranking_tag {
	int		gold;				/* 落札金額 */
	int		biddate;			/* 落札時間 */
	char	cdkey[CDKEYLEN];	/* 落とした人のＣＤキー */
	char	charaname[32];		/* 落とした人の名前 */
	char	owntitle[32];		/* 称号*/
	
}NPC_RANKING_INFO;

#endif /*__NPC_ROOMADMINNEW_H__*/

BOOL NPC_RoomAdminNew_ReadFile( char *roomname, NPC_ROOMINFO *data);
