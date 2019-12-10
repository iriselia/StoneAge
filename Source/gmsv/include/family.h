#ifndef __FAMILY_H__
#define __FAMILY_H__

#include "version.h"
#include "common.h"
#include "util.h"
#include "net.h"
#include "time.h"

#define FAMILY_MAXNUM			1000	// 家族數量
#ifdef _FMVER21
#define FAMILY_MAXMEMBER		100	// 家族人數
#define FAMILY_MAXCHANNELMEMBER		50	// 頻道人數
#else
#define FAMILY_MAXMEMBER		50	// 家族人數
#define FAMILY_MAXCHANNELMEMBER		10	// 頻道人數
#endif
#define FAMILY_MAXCHANNEL 		5	// 家族頻道

#define CHAR_MAXNAME			20
#define CHAR_MAXID			20
#define MINFMLEVLEFORPOINT      	3       // 3 申請莊園最低等級
#define FMLEADERLV			30	// 族長等級

#ifdef _FAMILY_MANORNUM_CHANGE
#define FAMILY_FMPKFLOOR		14	// 家族ＰＫ圖層
#else
#define FAMILY_FMPKFLOOR		9	// 家族ＰＫ圖層
#define FMPOINTNUM			4       // 有據點家族的最大數量
#define MANORNUM                	4
#define FAMILY_MAXHOME			4	// 家族據點
#endif

enum
{
    FM_TOP_INTEGRATE = 1,    // DPTOP 綜合
    FM_TOP_ADV,              // DPTOP 冒險
    FM_TOP_FEED,             // DPTOP 飼育
    FM_TOP_SYNTHESIZE,       // DPTOP 合成
    FM_TOP_DEALFOOD,         // DPTOP 料理
    FM_TOP_PK,               // DPTOP ＰＫ
#ifdef _NEW_MANOR_LAW
		FM_TOP_MOMENTUM = 8,		 // DPTOP 氣勢
#endif
    FM_TOP_NUM,              // DPTOP 數量
};

enum
{
	FM_FIX_ACCEPTFLAG = 1,
	FM_FIX_FMPK,
	FM_FIX_FMPET,
	FM_FIX_FMRULE,
	FM_FIX_DELFMTIME,
	FM_FIX_FMGOLD,
	FM_FIX_FMADV,
	FM_FIX_FMFEED,
	FM_FIX_FMSYNTHESIZE,
	FM_FIX_FMDEALFOOD,
	FM_FIX_FMLEADERCHANGE,
#ifdef _NEW_MANOR_LAW
	FM_FIX_FMMOMENTUM,
	FM_FIX_FAME,
#endif
};

/*
 * サーバ内部の他の場所との cdkey charname の長さを合せるために
 * CHEKEYLEN, CHARNAMELEN
 * を使うように変更。
 */
    
void CHAR_Family(int fd, int index, char* message);
void ACAddFM(int fd, int result, int fmindex, int index);
void ACJoinFM(int fd, int result, int recv);
void ACLeaveFM(int fd, int result, int resultflag);
void ACDelFM(int fd, int result);
void ACShowFMList(int ret, int fmnum, char *data);
void ACFMDetail(int ret, char *data, int charfdid);
void ACShowMemberList(int result, int index, int fmnumm,
int fmacceptflag, int fmjoinnum, char *data);
void ACShowDpTop(int result,int num, char *data, int kindflag);
void ACShowPointList(int result, char *data);
void ACShowFMMemo(int result, int index, int num, int dataindex, char *data);

#ifdef _PERSONAL_FAME   // Arminius: 家族個人聲望
void ACFMCharLogin(int fd, int ret, int index, int floor, int fmdp,
	int joinflag, int fmsetupflag, int flag, int charindex, int charfame
	#ifdef _NEW_MANOR_LAW
	,int momentum
	#endif
	);
#else
void ACFMCharLogin(int fd, int ret, int index, int floor, int fmdp,
	int joinflag, int fmsetupflag, int flag, int charindex);
#endif

void ACFMPointList(int ret, char *data);
void ACSetFMPoint(int ret, int r, int clifd);
void ACFMAnnounce(int ret, char *fmname, int fmindex, int index, int kindflag,
	char *data, int color);
void ACFixFMPK(int winindex, int loseindex, int data);
void ACFMJob( int fd, int ret, char* data1, char* data2 );

void FAMILY_Add(int fd, int meindex, char *message);
void FAMILY_Join(int fd, int meindex, char *message);
void FAMILY_Leave(int fd, int meindex, char *message);
void FAMILY_Detail(int fd, int meindex, char *message);
void FAMILY_CheckMember(int fd, int meindex, char *message);
void FAMILY_Channel(int fd, int meindex, char *message);
void FAMILY_Bank(int fd, int meindex, char *message);
void FAMILY_SetPoint(int fd, int meindex, char *message);
void FAMILY_Init(void);
void FAMILY_SetAcceptFlag(int fd, int meindex, char *message);
void FAMILY_FixRule( int fd, int meindex, char* message );
void FAMILY_RidePet( int fd, int meindex, char* message );
void FAMILY_LeaderFunc( int fd, int meindex, char* message );

#ifdef _ADD_FAMILY_TAX			   // WON ADD 增加莊園稅收	
void GS_ASK_TAX(void);
void FAMILY_FIX_TAX( int fd, int index, char* message);
#endif

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD 計算線上人數
void GS_SEND_PLAYER_COUNT(void);
#endif

void SetFMPetVarInit(int meindex);
void SetFMVarInit(int meindex);


int CheckFMLeader(int meindex);
int getFmLv(int playerindex);

void getNewFMList( void );
void checkFamilyIndex( void );

int CheckLeaderQ(int meindex);


// shan add Begin
struct FMMEMBER_LIST
{
//  int  fmindex;
    int  fmnum; 
    int  fmjoinnum;
//  BOOL use;                                       // 0->沒使用   1->使用
    int  memberindex[FAMILY_MAXMEMBER];             
    char numberlistarray[FAMILY_MAXMEMBER][64];
    char memo[35][220];                             // family dengon
    int  accept;                                    // 召募成員與否
    int  memonum;
    int  memoindex;
};
// 家族之間的留言板
struct FMS_MEMO
{
    char memo[140][220];
    int  memonum;
    int  memoindex;
};
// 家族強者表
struct FMS_DPTOP
{
    int  num;															// 記錄有多少個家族(綜合)
    char topmemo[FAMILY_MAXNUM][128];
    int  fmtopid[FAMILY_MAXNUM];          // 家族索引
#ifdef _FMVER21    
    int  fmtopdp[FAMILY_MAXNUM];          // 家族綜合聲望
#endif    
#ifdef _NEW_MANOR_LAW
		int	fmMomentum[FAMILY_MAXNUM];		// 家族氣勢
		char	momentum_topmemo[30][96];			// 家族氣勢 top
		int	momentum_topid[FAMILY_MAXNUM];	// 家族氣勢 top id 索引
#endif
    int  adv_num;                // 冒險
    char adv_topmemo[30][96];    
    int  feed_num;               // 伺育
    char feed_topmemo[30][96];        
    int  syn_num;                // 合成
    char syn_topmemo[30][96];        
    int  food_num;               // 料理
    char food_topmemo[30][96];        
    int  pk_num;                 // ＰＫ
    char pk_topmemo[30][96];        
};
// 家族據點
struct FM_POINTLIST
{
  char pointlistarray[FAMILY_MAXHOME][1024];	// Arminius: 32->1024
#ifdef _ADD_FAMILY_TAX			   // WON ADD 增加莊園稅收	
	//andy_fix
	//int fm_tax[4];
	int fm_tax[FAMILY_MAXHOME];
#endif
#ifdef _NEW_MANOR_LAW
	int fm_momentum[FAMILY_MAXHOME];	// 記錄挑戰時期開始時的守莊家族氣勢值
	BOOL fm_inwar[FAMILY_MAXHOME];		// 此莊園是否進行莊園排程中
#endif
};
// 家族PK圖層
struct FM_PKFLOOR
{
    int fl;
};
// End

#ifdef _NEW_MANOR_LAW
typedef struct _ManorSchedule_t{
	int iFmIndex[10];						// 排入挑戰排程的家族索引
	int iFmMomentum[10];				// 家族氣勢
	int iSort[10];							// 排名用
	char szMemo[10][256];				// 記錄: 家族名稱|約戰時間|家族氣勢
	char szFmName[10][32];			// 家族名稱
	struct tm tm1[10];							// 記錄挑戰時間
}ManorSchedule_t;

extern ManorSchedule_t ManorSchedule[MANORNUM];
#endif

#define MAXFAMILYLIST 120000
extern char    familyListBuf[MAXFAMILYLIST];

void JoinMemberIndex( int charaindex, int fmindexi);


#ifdef _DEATH_FAMILY_GM_COMMAND	// WON ADD 家族戰GM指令


#define fm_pk_max 200

typedef struct _fm_pk_struct
{
	int fm_index[fm_pk_max];
	int fm_win[fm_pk_max];
	int fm_lose[fm_pk_max];
	int fm_score[fm_pk_max];
	char fm_name[fm_pk_max][30];
}FM_PK_STRUCT;


enum{
	FM_INDEX =0,
	FM_WIN,
	FM_LOSE,
	FM_SCORE,
	FM_NAME,
};


void setInt_fm_pk_struct( int index, int type, int num );
void setChar_fm_pk_struct( int index, int type, char *msg );
int getInt_fm_pk_struct( int index, int type );
char *getChar_fm_pk_struct( int index, int type );
int get_fm_leader_index( int fm1 );


#endif



#endif
