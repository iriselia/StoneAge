#ifndef __NPC_SCHEDULEMAN_H__
#define __NPC_SCHEDULEMAN_H__

#define CHARNAME_MAX	32

// 最大 scheduleman 數量
#ifdef _FAMILY_MANORNUM_CHANGE
#define MAX_SCHEDULEMAN	20
#else
#define MAX_SCHEDULEMAN	12
#endif

// 每一個 scheduleman 控管的 schedule 數量
#define MAX_SCHEDULE	24

// schedule 列表的一頁所能列出的最大數量
#define MAXSCHEDULEINONEWINDOW	4

// family 列表的一頁所能列出的最大數量
#define MAXFAMILYINONEWINDOW	8

// 每一個 scheduleman 所使用的 schedule 儲存位置
// 對應在 fmpks 的 ID*MAX_SCHEDULE 上 (ID=0~MAX_SCHEDULEMAN-1)
// ID 是寫在 data/npc 中指定給 "id:" 的值

// dueltime = (隔天?10000:0) + (小時*100)

typedef struct tagFamilyPKSchedule {
	int	dueltime;	// 比賽時刻
	int	host_index;	// 主隊家族
	char host_name[CHARNAME_MAX];	// 主隊家族名稱
	int	guest_index;	// 客隊家族
	char guest_name[CHARNAME_MAX];	// 客隊家族名稱
	int	prepare_time;	// 準備時間 (1~40 分鐘)
	int	max_player;	// 最大出場人數 (1~家族人數上限)
	int	flag;		// 狀態
	int	win;		// 勝利條件設定
	int	challenge_timeout;	// 挑戰時限
	int	setting_timeout;	// 設定狀態的 timeout
	char gmsv_name[256];
} FamilyPKSchedule;

// 紀錄在 FamilyPKSchedule (fmpks) 的 flag 值
#define FMPKS_FLAG_NONE		-1	// 沒有任何排程
#define FMPKS_FLAG_CHALLENGE	0	// 等待客隊同意中
#define FMPKS_FLAG_SETTING	1	// 主隊正在設定排程 (取消時變成 NONE)
#define FMPKS_FLAG_CONFIRMING	2	// 客隊正在同意中
#define FMPKS_FLAG_SCHEDULED	3	// 已經排好排程，尚未開打
#define FMPKS_FLAG_DUEL		4	// 開打中
#define FMPKS_FLAG_HOSTWIN	5	// 主隊勝
#define FMPKS_FLAG_GUESTWIN	6	// 客隊勝
#define FMPKS_FLAG_MANOR_BATTLEBEGIN	7	// 莊園挑戰 戰鬥中
#define FMPKS_FLAG_MANOR_PREPARE	8	// 莊園挑戰 準備中
#define FMPKS_FLAG_MANOR_PEACE	9	// 莊園挑戰 休戰中
#define FMPKS_FLAG_MANOR_OTHERPLANET	10	// 莊園挑戰 在別的星球決戰
#define FMPKS_FLAG_MANOR_BATTLEEND	11	// 莊園挑戰 戰鬥結束
#define FMPKS_FLAG_MANOR_PEACE_SAVE	12	// 將莊園挑戰存檔
#define FMPKS_FLAG_MANOR_READYTOFIGHT	13	// (GM: manorpk) 全星系變成可約戰狀態
#define FMPKS_FLAG_MANOR_CLEANFLAG	14	// (GM: manorpk) 本星球清除狀態
#ifdef _NEW_MANOR_LAW
#define FMPKS_FLAG_WAIT		15	// 進入挑戰期,已記錄家族氣勢,等待挑戰排程
#endif

// 傳給 client 的 flag
#define FLAG_NONE	-1
#define FLAG_MODIFY	0
#define FLAG_ACCEPT	1
#define FLAG_SETTING	2
#define FLAG_CONFIRM	3
#define FLAG_SCHEDULED	4

extern FamilyPKSchedule fmpks[MAX_SCHEDULE*MAX_SCHEDULEMAN];

BOOL NPC_SchedulemanInit(int meindex);
void NPC_SchedulemanTalked(int meindex, int talkerindex, char *msg, int color);
void NPC_SchedulemanWindowTalked(int meindex, int talkerindex,
				 int seqno, int select, char *data);
void NPC_SchedulemanLoop(int meindex);

#endif 

/*__NPC_SCHEDULEMAN_H__*/
