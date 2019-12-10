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
#define MAXTEAMNUM 1000

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
	int read;
	PkTeamMans MyTeamMans[MAXTEAMMANNUM];
	BattleHistorys BHistory[MAXBATTLENUM];
}PkTeamLists;

#define MAXJOINTEAM 40
#define MAXWATCHMAP 16
typedef struct 
{
	int use;
	int teamnum;
	char cdkey[256];
	char name[256];
	int toindex;
	int fd;
}JoinTeamList;

typedef struct _tagPKProcedureRow
{
	int use;
	int time;
	int type;
	JoinTeamList Team[2];
}PKProcedures;

enum{
	PKTYPE_NONE=0,		//無
	PKTYPE_WAIT,		//等待其他隊伍加入
	PKTYPE_STANDBY,		//準備對戰等待時間
	PKTYPE_PK,			//對戰中
};

void del_rn( char *s );
void PKLIST_ResetOneTeamMan( int ti ); //重置隊員名單
void PKLIST_ResetOneBHistory( int ti ); //重置對戰名單
void PKLIST_ResetOnePkTeamList( int ti ); //重置參賽隊伍資料
int PKLIST_InitPkTeamList( int teamnum ); //重置

int PKLIST_GetPkTeamListArray( int teamnum, char *cdkey);
int PKLIST_GetPkTeamListArrayFromNum( int teamnum);
//確認重複約戰
BOOL PKLIST_CHECKPkTeamSame( int teamnum, int charaindex, char *cdkey, int toteamnum );
//確認對戰場數
int PKLIST_CHECKPkTeamNew( int teamnum, int charaindex, char *cdkey );

//LOAD DATA
BOOL PKLIST_LoadPkTeamListDataSub( int ti, char *data);
BOOL PKLIST_LoadPkTeamListDataMyTeamMans( int ti, char *data);
BOOL PKLIST_LoadPkTeamListDataBHistory( int ti, char *data);
//BOOL PKLIST_LoadPkTeamListData( char *data); //處理ac送來的 pklist
BOOL PKLIST_LoadPkTeamListData(void); // 改成讀檔
void PKLIST_SavePkTeamListData(void);
void PKLIST_LoadInitPkTeamListData(void);	// 讀取最原始的參賽名單
void PKLIST_UpData(char *mycdkey,char *tocdkey,int menum,int tonum,int winer,int flg);
int PKLIST_GetOneBHistory( int ti );
int PKLIST_SetOneBHistory( int ti, int hi, int use, int teamnum, int flg );
int PKLIST_UpdateOnePkTeamData( int ti, int forti, int winerflg);

void PKLIST_InsertTeamNum( int charaindex );
BOOL PKLIST_GetTeamLeaderCdkey( int teamnum, char *buf);
void PKLIST_ShowPkListTeamData( void);

int NPC_PKLIST_Finish_Exit( int menum, int tonum, int winside, int battlemap);


BOOL PKLIST_HandleChartsMess( int fd, char *data, int type, int flg);
BOOL PKLIST_GetChartsListData( int ti, char *data, int sizes );
BOOL PKLIST_GetMyPKListTeamData( int teamnum, char *data, int sizes );


/*
BOOL PKLIST_CheckPKSameTeam( int charaindex );
BOOL PKLIST_JoinPKProcedures( int charaindex );
void PKLIST_DelPKProcedures( int ti, int side );
void PKLIST_CheckPKProcedures( void );

void PKLIST_CheckPKProcedures_PKTYPEWAIT( int ti);
void PKLIST_CheckPKProcedures_PKTYPESTANDBY( int ti);
*/

//賽程
void PKLIST_DelPKProcedures( int ti, int side, int type);
BOOL PKLIST_CheckPklistInServerMap( int ti, int side);
BOOL PKLIST_CheckPKSameTeam( int charaindex );
BOOL PKLIST_CheckPKReapetTeam( int menum, int tonum);
BOOL PKLIST_JoinPKProcedures( int charaindex );
void PKLIST_CheckTeamBeEnable( void);
void PKLIST_warp( int ti, int side, int fl, int x, int y );
void NPC_PKLIST_PlayerLogout_Exit( int charaindex );
int PKLIST_GetPKProcedureArray( int menum );

BOOL PKLIST_CheckLOCKTeam( int menum);
void PKLIST_LOCKTeam( int menum);
void PKLIST_UNLOCKTeam( int menum);
void PKLIST_Sort_PKListSort( void);

//正式賽
typedef struct _tagArrangeBattle
{
	int use;
	int fl;
	int code;
	int teamnum;
	int type; //0 NULL // 1 in battle
	int time;
	char teamname[256];
	int toindex;

	struct _tagArrangeBattle *next[2];
	struct _tagArrangeBattle *top;
}ArrangeBattleC;

#define MAXBAFLOOR 20
#define MAXNOWBATTLE 128

#define MAXBAHEAD 16
#define MAXBATTLEPAGE MAXJOINTEAM

void ABATTLE_InitABattle( int maxnums );
int ABATTLE_CreateNet( ArrangeBattleC *now, int ti, int fl, int maxfl);
void ABATTLE_ShowNet( ArrangeBattleC *now, int fl);
void ABATTLE_ShowBattlefromFl( int ti, int fl);
ArrangeBattleC *ArrangeBattleC_getNew( void);


BOOL ABATTLE_InsertBattle( ArrangeBattleC *aB); //排入賽程
void ABATTLE_EnterBattle( ArrangeBattleC *aB); //入圍
void ABATTLE_EliminateBattlefromFl( ArrangeBattleC *aB);//剔除

BOOL ABATTLE_CheckInABattle( int ti);//確認賽程戰鬥狀態 包含時間
int ABATTLE_FindBattlefromFl( int ti, int fl); //找尋可加入賽程隊伍組合

void ABATTLE_CheckBattlefromFl(int charindex, int ti,int battleindex); //確認層次是否完成 且 排置賽程
int ABATTLE_CheckBattlefromFl_sub(int charindex, int ti, int fl,int battleindex); //確認層次是否完成


ArrangeBattleC *ABATTLE_getInBattle( int teamnum); //取得賽程head form teamnum

void ABATTLE_MakeInABattleString( void); //製作賽程字串

BOOL PKLIST_GetABattlelistDataString( int ti, int *tindex, int *stime,
									 char *buf1, char *buf2, char *buf3, int flg);//取得賽程字串

ArrangeBattleC *ArrangeBattleC_getInBattleArray( int ti);




void ABATTLE_RecordBattle( int ti, char *buf1, char *tstr1,char *buf2, char *tstr2);
void remove_r( char *s );
void ABATTLE_GetRecordBattle( void);

#endif
