#include "version.h"

#ifdef _DEATH_CONTEND
//團隊人物名單
typedef struct 
{
	int use;
	char cdkey[64];
	char name[64];
}PkTeamMans;
//戰鬥紀錄
typedef struct 
{
	int use;
	int teamnum;
	int flg;	//0 1
}BattleHistorys;

#define MAXTEAMMANNUM 5		//隊伍最高人數
#define MAXBATTLENUM 100	//隊伍最高戰鬥紀錄

#define DEFMAXBATTLENUM 50 //最高決鬥場次
#define DEFWINSCORE 90
#define DEFLOSERATE 0.4
//決鬥參賽隊伍info
typedef struct _tagPkTeamLists
{
	int use;					//flg
	int teamnum;				//隊伍序號
	char teamname[64];			//隊伍名稱
	char pathdir[64];			//隊伍資料目錄
	char leadercdkey[64];		//隊長CDKEY
	int win;					//勝
	int lost;					//負
	int battleplay;				//總場次
	int score;
	int inside;					//錄取旗標

	int updata;					//更新時間

	PkTeamMans MyTeamMans[MAXTEAMMANNUM];
	BattleHistorys BHistory[MAXBATTLENUM];
}PkTeamLists;

//勿動-------------------------------------------------------
int PKLIST_getStringPkTeamData( int ti, char *data, int sizes);
int PKLIST_SetOneBHistory( int ti, int hi, int use, int teamnum, int flg );
int PKLIST_GetOneBHistory( int ti );
void PKLIST_ResetOneTeamMan( int ti );
void PKLIST_ResetOneBHistory( int ti );
void PKLIST_ResetOnePkTeamList( int ti );
int PKLIST_UpdateOnePkTeamData( int ti, int forti, int winerflg);
//-----------------------------------------------------------
int PKLIST_InitPkTeamList( int maxteam );
void PKLIST_UpdatePkTeamList( int fd, int menum, char *mecdkey, int tonum, char *tocdkey, int winer, int flg);
void PKLIST_GetPkTeamListData( int fd, int startTi, int count);
void PKLIST_GetPkTeamListDataOne( int fd, int ti);
//-----------------------------------------------------------
int PKLIST_LoadPkTeamListfromFile( char *dirpath, char *listfilename );
int PKLIST_BackupPkTeamListfromFile( char *dirpath, char *listfilename );
void PKLIST_SortTheCharts( int ti);//排行榜處理
void PKLIST_TheCharts( int fd, int type, int flg);

int PKLIST_LoadUserfromFile( char *dirpath, char *listfilename );
int PKLIST_CutStartUserfromFile( char *dirpath, char *listfilename );

void PKLIST_CheckRepeatBattleHistory( void);
int PKLIST_alteUserfromFile( char *dirpath, char *listfilename );
void PKLIST_Sort_PKListSort( void);

void PKLIST_CheckAllUser( void);

#endif
