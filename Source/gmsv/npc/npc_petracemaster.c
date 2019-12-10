#include "version.h"
#include <string.h>
#include <time.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "handletime.h"
 
#ifdef _PETRACE
#include "npc_petracemaster.h"

#define PETRACEPETNUM		3
#define PETRACEMASTER_INITPET_LOOPTIME			3000	// 3秒
#define PETRACEMASTER_WAITDROPI_LOOPTIME		30000	// 30秒
#define PETRACEMASTER_WAITDROPII_LOOPTIME		10000	// 10秒
#define PETRACEMASTER_WAITDROPIII_LOOPTIME	5000	// 5秒
#define PETRACEMASTER_WAITDROPIV_LOOPTIME		1000	// 1秒
#define PETRACEMASTER_PETRACING_LOOPTIME		3000	// 3秒
#define PETRACEMASTER_SHOWWINPET_LOOPTIME		10000	// 10秒
#define PETRACEDROPSTAKETIME						120	// 2分鐘 120
#define PETRACESHOWWINPET							30		// 30秒
#define PETRACESTDPAYRATE							6
#define PETRACESTDGOLD								1		// 計算賠率時基本金額 1000

static void NPC_PetRaceMaster_selectWindow(int meindex, int toindex, int num, int select);
void NPC_findRacePetIndex(int meindex, int floor);
void NPC_givePriceToPlayer(int meindex, int floor, int winpetindex);

enum
{
	NPC_WORK_STATE =  CHAR_NPCWORKINT1,
	NPC_WORK_PET1 = CHAR_NPCWORKINT2,
	NPC_WORK_PET2 = CHAR_NPCWORKINT3,
	NPC_WORK_PET3 = CHAR_NPCWORKINT4,
	NPC_WORK_PETGROUP = CHAR_NPCWORKINT5,
	NPC_WORK_STATECHANGE = CHAR_NPCWORKINT6,
	NPC_WORK_FINDPETFLAG = CHAR_NPCWORKINT7,
	NPC_WORK_WINPET = CHAR_NPCWORKINT8,
	NPC_WORK_PETGOAL = CHAR_NPCWORKINT9,
};
enum
{
	NPC_State_Init,
	NPC_State_WaitDropStake,
	NPC_State_PetRacing,
	NPC_State_ShowWinPet,
};
enum
{
	NPC_State_Wait,
	NPC_State_Run,
	NPC_State_ShowWin,
};
typedef struct tagPetRaceGraTable
{
	int pet1gra;
	int pet2gra;
	int pet3gra;
	char pet1name[64];
	char pet2name[64];
	char pet3name[64];
	int pet1win;
	int pet2win;
	int pet3win;
#ifdef _DROPSTAKENEW
#else
	float payrate1;
	float payrate2;
	float payrate3;
#endif
	char state1[64];
	char state2[64];
	char state3[64];
}PetRaceGraTable;
#ifdef _DROPSTAKENEW
PetRaceGraTable petracegra[27] =
{
	{100820, 100821, 100822, "撲滿烏力", "烏力萊德", "烏力固力", 0, 0, 0, "良好", "良好", "良好"},
	{100823, 100824, 100825, "布依布魯", "布依布依", "布依胖", 0, 0, 0, "良好", "良好", "良好"},
	{100842, 100843, 100844, "尼加斯", "尼基斯", "特洛可斯", 0, 0, 0, "良好", "良好", "良好"},
	{100852, 100853, 100855, "克拉爾", "拉斯基", "克達達", 0, 0, 0, "良好", "良好", "良好"},
	{100859, 100856, 100857, "可卡特", "卡卡特", "昆依特", 0, 0, 0, "良好", "良好", "良好"},
	{100864, 100865, 100866, "藍寶", "瑞德寶", "毛寶", 0, 0, 0, "良好", "良好", "良好"},
	{100829, 100826, 100827, "比比加", "加加", "加斯奧", 0, 0, 0, "良好", "良好", "良好"},
	{100830, 100832, 100833, "烏寶依", "威比", "烏拉拉", 0, 0, 0, "良好", "良好", "良好"},
	{100883, 100884, 100885, "尤里蛙", "裘里蛙", "艾爾蛙", 0, 0, 0, "良好", "良好", "良好"},
	{100898, 100900, 100901, "可可恩", "布蘭恩", "迪布恩", 0, 0, 0, "良好", "良好", "良好"},
	{100834, 100836, 100837, "貝洛金", "貝洛貝", "貝洛波波", 0, 0, 0, "良好", "良好", "良好"},
	{100838, 100840, 100841, "藍龜", "龜之鋼", "卡拉龜", 0, 0, 0, "良好", "良好", "良好"},
	{100879, 100881, 100880, "迪加", "波波頓", "砂鯊", 0, 0, 0, "良好", "良好", "良好"},
	{100888, 100887, 100889, "莫拉司", "歐瑟菲", "瑪斯特", 0, 0, 0, "良好", "良好", "良好"},
	{100913, 100914, 100915, "萊姆洛斯", "朱利洛斯", "辛普洛斯", 0, 0, 0, "良好", "良好", "良好"},
	{100845, 100846, 100847, "呼波波", "呼魯魯", "魯拉其斯", 0, 0, 0, "良好", "良好", "良好"},
	{100849, 100850, 100851, "邦諾斯娜", "楊格斯", "迪基格斯", 0, 0, 0, "良好", "良好", "良好"},
	{100878, 100876, 100877, "玻洛布斯", "加耶布斯", "迪米布斯", 0, 0, 0, "良好", "良好", "良好"},
	{100908, 100909, 100906, "阿米朵", "邦司涼朵", "朵巴奈特", 0, 0, 0, "良好", "良好", "良好"},
	{100916, 100918, 100919, "立杜魯斯", "巴克亞司", "雷德力克斯", 0, 0, 0, "良好", "良好", "良好"},
	{100860, 100862, 100861, "司爾頓", "梅爾頓", "格爾希洛", 0, 0, 0, "良好", "良好", "良好"},
	{100867, 100870, 100868, "歐林吉魯", "芭拉芭", "史凱魯", 0, 0, 0, "良好", "良好", "良好"},
	{100871, 100873, 100874, "普魯夏", "薩格魯", "瑪斯貝卡", 0, 0, 0, "良好", "良好", "良好"},
	{100894, 100895, 100896, "固力摩", "摩吉摩吉", "摩米索拉", 0, 0, 0, "良好", "良好", "良好"},
	{100902, 100903, 100905, "格里蘭", "摩里", "塔斯夫", 0, 0, 0, "良好", "良好", "良好"},
	{100912, 100911, 100910, "伊夫霍恩", "布依倫斯", "布萊茲", 0, 0, 0, "良好", "良好", "良好"},
	{100891, 100892, 100893, "嘎吱拉", "哥斯哥斯", "蒙哥拉斯", 0, 0, 0, "良好", "良好", "良好"},
};
#else
PetRaceGraTable petracegra[27] =
{
	{100820, 100821, 100822, "撲滿烏力", "烏力萊德", "烏力固力", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100823, 100824, 100825, "布依布魯", "布依布依", "布依胖", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100842, 100843, 100844, "尼加斯", "尼基斯", "特洛可斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100852, 100853, 100855, "克拉爾", "拉斯基", "克達達", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100859, 100856, 100857, "可卡特", "卡卡特", "昆依特", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100864, 100865, 100866, "藍寶", "瑞德寶", "毛寶", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100829, 100826, 100827, "比比加", "加加", "加斯奧", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100830, 100832, 100833, "烏寶依", "威比", "烏拉拉", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100883, 100884, 100885, "尤里蛙", "裘里蛙", "艾爾蛙", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100898, 100900, 100901, "可可恩", "布蘭恩", "迪布恩", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100834, 100836, 100837, "貝洛金", "貝洛貝", "貝洛波波", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100838, 100840, 100841, "藍龜", "龜之鋼", "卡拉龜", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100879, 100881, 100880, "迪加", "波波頓", "砂鯊", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100888, 100887, 100889, "莫拉司", "歐瑟菲", "瑪斯特", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100913, 100914, 100915, "萊姆洛斯", "朱利洛斯", "辛普洛斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100845, 100846, 100847, "呼波波", "呼魯魯", "魯拉其斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100849, 100850, 100851, "邦諾斯娜", "楊格斯", "迪基格斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100878, 100876, 100877, "玻洛布斯", "加耶布斯", "迪米布斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100908, 100909, 100906, "阿米朵", "邦司涼朵", "朵巴奈特", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100916, 100918, 100919, "立杜魯斯", "巴克亞司", "雷德力克斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100860, 100862, 100861, "司爾頓", "梅爾頓", "格爾希洛", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100867, 100870, 100868, "歐林吉魯", "芭拉芭", "史凱魯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100871, 100873, 100874, "普魯夏", "薩格魯", "瑪斯貝卡", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100894, 100895, 100896, "固力摩", "摩吉摩吉", "摩米索拉", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100902, 100903, 100905, "格里蘭", "摩里", "塔斯夫", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100912, 100911, 100910, "伊夫霍恩", "布依倫斯", "布萊茲", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
	{100891, 100892, 100893, "嘎吱拉", "哥斯哥斯", "蒙哥拉斯", 0, 0, 0, 2, 2, 2, "良好", "良好", "良好"},
};
#endif
BOOL NPC_PetRaceMasterInit( int meindex )
{
	CHAR_setInt(meindex, CHAR_LOOPINTERVAL,	PETRACEMASTER_INITPET_LOOPTIME);
	CHAR_setInt(meindex, CHAR_WHICHTYPE, CHAR_PETRACEMASTER);
	CHAR_setWorkInt(meindex, NPC_WORK_STATE, NPC_State_Init);
	CHAR_setWorkInt(meindex, NPC_WORK_PETGROUP, 0);
#ifdef _DROPSTAKENEW
#else
	petracegra[0].payrate1 = 0;
	petracegra[0].payrate2 = 0;
	petracegra[0].payrate3 = 0;
#endif
	return	TRUE;
}

void NPC_PetRaceMasterTalked( int meindex , int talkerindex , char *szMes , int color )
{
	// 尚須新增說明以及寵物狀況
	if (CHAR_getInt(talkerindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER)
		return;
	if (NPC_Util_isFaceToFace(talkerindex, meindex, 2) == FALSE)
		if (NPC_Util_CharDistance(talkerindex, meindex) > 1)
			return;
	NPC_PetRaceMaster_selectWindow(meindex, talkerindex, 0, -1);
}

static void NPC_PetRaceMaster_selectWindow(int meindex, int toindex, int num, int select)
{
	char token[1024];
	int buttontype = 0, windowtype = 0, windowno = 0;
	int fd = getfdFromCharaIndex(toindex);
	if (fd == -1) return;
	token[0] = '\0';
	switch(num)
	{
		case 0:
			sprintf(token, "4\n\n	          ★寵物競速場★\n"
				"你好，想要知道什麼情報嗎？\n"
				"\n	          《察看遊戲規則》"
				"\n             《寵物狀況》"
				"\n               《離開》");
			buttontype = WINDOW_BUTTONTYPE_NONE;
			windowtype = WINDOW_MESSAGETYPE_SELECT;
			windowno = 	CHAR_WINDOWTYPE_PETRACEMASTER_START;
			break;
		case 1:
#ifdef _DROPSTAKENEW
			sprintf(token, 
				"\n競速場規則："
				"\n參加競速的寵物總共有三隻，您可以依照寵物"
				"\n的狀況或個人喜好下注，請將兌換得到的彩券"
				"\n放置於欲下注的號碼前方。當競速結束時，主"
				"\n持人將會自動幫您計算積分。"
				"\n請注意：當您離開此房間或登出時，下注的彩"
				"\n卷及積分將無法取回唷！");
#else
			sprintf(token, 
				"\n競速場規則：參加競速的寵物共有三隻，您可"
				"\n以依照寵物狀況或個人喜好下注，每個寵物有"
				"\n自己的賠率。請將石幣丟置於欲下注的號碼前"
				"\n方。當競速結束時，主持人會自動地將金額放"
				"\n回你的身上或存入個人銀行。"
				"\n請注意：銀行金額請勿超過一千萬！當您離開"
				"\n此房間或登出時，下注的金額將無法取回唷！");
#endif
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_PETRACEMASTER_RULE;
		break;
		case 2:
		{
			int petgroup = CHAR_getWorkInt(meindex, NPC_WORK_PETGROUP);
			int petindex1 = CHAR_getWorkInt(meindex, NPC_WORK_PET1);
			int petindex2 = CHAR_getWorkInt(meindex, NPC_WORK_PET2);
			int petindex3 = CHAR_getWorkInt(meindex, NPC_WORK_PET3);
			int pet1win = petracegra[petgroup].pet1win;
			int pet2win = petracegra[petgroup].pet2win;
			int pet3win = petracegra[petgroup].pet3win;
			int total = pet1win + pet2win + pet3win;
			float winrate1 = 0, winrate2 = 0, winrate3 = 0;
#ifdef _DROPSTAKENEW
#else
			char cpayrate1[64], cpayrate2[64], cpayrate3[64];
			if (petracegra[petgroup].payrate1 == 0)
				snprintf(cpayrate1, sizeof(cpayrate1), "???");
			else
				snprintf(cpayrate1, sizeof(cpayrate1), "%6.2f", petracegra[petgroup].payrate1);
			if (petracegra[petgroup].payrate2 == 0)
				snprintf(cpayrate2, sizeof(cpayrate2), "???");
			else
				snprintf(cpayrate2, sizeof(cpayrate2), "%6.2f", petracegra[petgroup].payrate2);
			if (petracegra[petgroup].payrate3 == 0)
				snprintf(cpayrate3, sizeof(cpayrate3), "???");
			else
				snprintf(cpayrate3, sizeof(cpayrate3), "%6.2f", petracegra[petgroup].payrate3);
#endif
			if (total == 0)
			{
				winrate1 = 0; winrate2 = 0; winrate3 = 0;
			}
			else
			{
				winrate1 = (float)pet1win / (float)total;
				winrate2 = (float)pet2win / (float)total;
				winrate3 = (float)pet3win / (float)total;
			}
#ifdef _DROPSTAKENEW
			sprintf(token, "寵物狀況\n\n  寵物    勝場  敗場  狀況    勝場率\n"
				"========================================"
				"%-10s%-6d%-6d%-8s%-6.2f\n"
				"%-10s%-6d%-6d%-8s%-6.2f\n"
				"%-10s%-6d%-6d%-8s%-6.2f\n",
				CHAR_getChar(petindex1, CHAR_NAME), pet1win, total - pet1win,
				petracegra[petgroup].state1, winrate1,
				CHAR_getChar(petindex2, CHAR_NAME), pet2win, total - pet2win,
				petracegra[petgroup].state2, winrate2,
				CHAR_getChar(petindex3, CHAR_NAME), pet3win, total - pet3win,
				petracegra[petgroup].state3, winrate3);
#else
			sprintf(token, "寵物狀況\n\n  寵物    勝場  敗場  狀況  勝場率  賠率\n"
				"========================================"
				"%-10s%-6d%-6d%-6s%-6.2f%-4s\n"
				"%-10s%-6d%-6d%-6s%-6.2f%-4s\n"
				"%-10s%-6d%-6d%-6s%-6.2f%-4s\n",
				CHAR_getChar(petindex1, CHAR_NAME), pet1win, total - pet1win,
				petracegra[petgroup].state1, winrate1, cpayrate1,
				CHAR_getChar(petindex2, CHAR_NAME), pet2win, total - pet2win,
				petracegra[petgroup].state2, winrate2, cpayrate2,
				CHAR_getChar(petindex3, CHAR_NAME), pet3win, total - pet3win,
				petracegra[petgroup].state3, winrate3, cpayrate3);
#endif
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno = CHAR_WINDOWTYPE_PETRACEMASTER_PET;
		}
		break;
		case 3:
		default:
			break;
	}
	lssproto_WN_send(fd, windowtype, buttontype, windowno,
		CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX), token);
}

void NPC_PetRaceMasterWindowTalked(int meindex, int talkerindex, int seqno, int select, char *data)
{
	int datanum = 0;
	makeStringFromEscaped(data);
	datanum = atoi(data);
	switch(seqno)
	{
		case CHAR_WINDOWTYPE_PETRACEMASTER_START:
			if (datanum == 1)
				NPC_PetRaceMaster_selectWindow(meindex, talkerindex, 1, -1);
			else if (datanum == 2)
				NPC_PetRaceMaster_selectWindow(meindex, talkerindex, 2, -1);
		break;
		case CHAR_WINDOWTYPE_PETRACEMASTER_RULE:
			if (datanum == 3)
				NPC_PetRaceMaster_selectWindow(meindex, talkerindex, 1, -1);
		case CHAR_WINDOWTYPE_PETRACEMASTER_PET:
		case CHAR_WINDOWTYPE_PETRACEMASTER_LEAVE:
		default:
		break;
	}
}

void NPC_PetRaceMasterLoop( int meindex)
{
	int floor = CHAR_getInt(meindex, CHAR_FLOOR);
	time_t t1;
	time(&t1);
	switch(CHAR_getWorkInt(meindex, NPC_WORK_STATE))
	{
		case NPC_State_Init:	
				// 尋找寵物index
				NPC_findRacePetIndex(meindex, floor);
			break;
		case NPC_State_WaitDropStake:
			{
				char tmpbuf[256];
				int statechangetime = CHAR_getWorkInt(meindex, NPC_WORK_STATECHANGE);
				if (t1 < statechangetime)
				{
					if (statechangetime - t1 <= 5)
					{
						CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_WAITDROPIV_LOOPTIME);
						snprintf(tmpbuf, sizeof(tmpbuf), "還有%2d秒就要開跑了，請大家踴躍下注！",
							(int)(statechangetime - t1) % 60);
					}
					else if (statechangetime - t1 <= 10)
					{
						CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_WAITDROPIII_LOOPTIME);
						snprintf(tmpbuf, sizeof(tmpbuf), "還有%2d秒就要開跑了，請大家踴躍下注！",
							(int)(statechangetime - t1) % 60);
					}
					else if (statechangetime - t1 <= 30)
					{
						CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_WAITDROPII_LOOPTIME);
						snprintf(tmpbuf, sizeof(tmpbuf), "還有%2d秒就要開跑了，請大家踴躍下注！",
							(int)(statechangetime - t1) % 60);
					}
					else
						snprintf(tmpbuf, sizeof(tmpbuf), "還有%2d分鐘%2d秒就要開跑了，請大家踴躍下注！",
							(int)(statechangetime - t1) / 60, (int)(statechangetime - t1) % 60);
				}
				if (t1 >= statechangetime)
				{
#ifdef _DROPSTAKENEW
#else
					float payrate1 = 0, payrate2 = 0, payrate3 = 0;
					int pet1gold = 0, pet2gold = 0, pet3gold = 0, totalgold = 0;
					int petindex1 = CHAR_getWorkInt(meindex, NPC_WORK_PET1);
					int petindex2 = CHAR_getWorkInt(meindex, NPC_WORK_PET2);
					int petindex3 = CHAR_getWorkInt(meindex, NPC_WORK_PET3);
					int randpet	= CHAR_getWorkInt(meindex, NPC_WORK_PETGROUP);
					float droprate1 = 0, droprate2 = 0, droprate3 = 0, totaldroprate, Npayrate = PETRACESTDPAYRATE;
#endif
					int i = 0;
					CHAR_setWorkInt(meindex, NPC_WORK_STATE, NPC_State_PetRacing);
					// 設定地面不可下注
					SetCasinoMap(meindex, 0, FALSE);
#ifdef _DROPSTAKENEW
					snprintf(tmpbuf, sizeof(tmpbuf), "請大家停止下注，寵物賽跑開始囉！");
#else
					// 告知大家賠率（取得寵物下注金額計算賠率）
					GetCasinoMapGold(meindex, PET_RACE1, &pet1gold);
					GetCasinoMapGold(meindex, PET_RACE2, &pet2gold);
					GetCasinoMapGold(meindex, PET_RACE3, &pet3gold);
					totalgold = pet1gold + pet2gold + pet3gold;
					if (totalgold <= 0)
					{
						payrate1 = 2; payrate2 = 2; payrate3 = 2;
					}
					else
					{
						if (pet1gold <= PETRACESTDGOLD)
							pet1gold = PETRACESTDGOLD;
						if (pet2gold <= PETRACESTDGOLD)
							pet2gold = PETRACESTDGOLD;
						if (pet3gold <= PETRACESTDGOLD)
							pet3gold = PETRACESTDGOLD;
						totalgold = pet1gold + pet2gold + pet3gold;
						droprate1 = totalgold / pet1gold;
						droprate2 = totalgold / pet2gold;
						droprate3 = totalgold / pet3gold;
						totaldroprate = droprate1 + droprate2 + droprate3;
						if (totaldroprate / droprate1 > PETRACESTDPAYRATE)
						{
							payrate1 = 1; Npayrate -= 1;
						}
						if (totaldroprate / droprate2 > PETRACESTDPAYRATE)
						{
							payrate2 = 1; Npayrate -= 1;
						}
						if (totaldroprate / droprate3 > PETRACESTDPAYRATE)
						{
							payrate3 = 1; Npayrate -= 1;
						}
						if (payrate1 != 1)
							payrate1 = droprate1 / totaldroprate * Npayrate;
						if (payrate2 != 1)
							payrate2 = droprate2 / totaldroprate * Npayrate;
						if (payrate3 != 1)
							payrate3 = droprate3 / totaldroprate * Npayrate;
					}
					petracegra[randpet].payrate1 = payrate1;
					petracegra[randpet].payrate3 = payrate2;
					petracegra[randpet].payrate2 = payrate3;
					SetCasinoPayRate(meindex, PET_RACE1, payrate1);
					SetCasinoPayRate(meindex, PET_RACE2, payrate2);
					SetCasinoPayRate(meindex, PET_RACE3, payrate3);
					snprintf(tmpbuf, sizeof(tmpbuf), "請大家停止下注，寵物賽跑開始囉！%s的賠率為%5.2f；%s的賠率為%5.2f；%s的賠率為%5.2f",
						CHAR_getChar(petindex1, CHAR_NAME), payrate1,
						CHAR_getChar(petindex2, CHAR_NAME), payrate2,
						CHAR_getChar(petindex3, CHAR_NAME), payrate3);
#endif
					CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_PETRACING_LOOPTIME);
					// 設定寵物狀態
					for (i = 0; i < PETRACEPETNUM; i++)
					{
						CHAR_setWorkInt(CHAR_getWorkInt(meindex, NPC_WORK_PET1 + i), NPC_WORK_STATE, NPC_State_Run);
						CHAR_setInt(CHAR_getWorkInt(meindex, NPC_WORK_PET1 + i), CHAR_LOOPINTERVAL, 1000);
					}
				}
				CHAR_talkToFloor(floor, meindex, tmpbuf, CHAR_COLORWHITE);
			}
			break;
		case NPC_State_PetRacing:
			if (CHAR_getWorkInt(meindex, NPC_WORK_PETGOAL) == PETRACEPETNUM)
			{
				char tmpbuf[256];
				int winpetindex = CHAR_getWorkInt(meindex, NPC_WORK_WINPET);
				int petgroup = CHAR_getWorkInt(meindex, NPC_WORK_PETGROUP);
				snprintf(tmpbuf, sizeof(tmpbuf), "競賽結束，勝利的寵物是：%s\n", 
					CHAR_getChar(winpetindex, CHAR_NAME));
				CHAR_talkToFloor(floor, meindex, tmpbuf, CHAR_COLORYELLOW);
				// 設定輸贏，以計算賠率
				if (CHAR_getWorkInt(meindex, NPC_WORK_PET1) == winpetindex)
					petracegra[petgroup].pet1win = petracegra[petgroup].pet1win + 1;
				else if (CHAR_getWorkInt(meindex, NPC_WORK_PET2) == winpetindex)
					petracegra[petgroup].pet2win = petracegra[petgroup].pet2win + 1;
				else if (CHAR_getWorkInt(meindex, NPC_WORK_PET3) == winpetindex)
					petracegra[petgroup].pet3win = petracegra[petgroup].pet3win + 1;
				CHAR_setWorkInt(meindex, NPC_WORK_STATE, NPC_State_ShowWinPet);
				CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_SHOWWINPET_LOOPTIME);
				CHAR_setWorkInt(meindex, NPC_WORK_STATECHANGE, t1 + PETRACESHOWWINPET);	
				// 發獎金
				NPC_givePriceToPlayer(meindex, floor, winpetindex);
			}
			break;
		case NPC_State_ShowWinPet:
			{
				int statechangetime = CHAR_getWorkInt(meindex, NPC_WORK_STATECHANGE);
				if (t1 >= statechangetime)
				{
					int randpet = RAND(0, arraysizeof(petracegra) - 1);
					int petindex1 = CHAR_getWorkInt(meindex, NPC_WORK_PET1);
					int petindex2 = CHAR_getWorkInt(meindex, NPC_WORK_PET2);
					int petindex3 = CHAR_getWorkInt(meindex, NPC_WORK_PET3);
					char tmpbuf[256];
					CHAR_setWorkInt(meindex, NPC_WORK_PETGROUP, randpet);
					CHAR_setWorkInt(meindex, NPC_WORK_STATE, NPC_State_WaitDropStake);
					CHAR_setWorkInt(meindex, NPC_WORK_WINPET, 0);
					CHAR_setWorkInt(meindex, NPC_WORK_PETGOAL, 0);
					CHAR_setWorkInt(meindex, NPC_WORK_STATECHANGE, t1 + PETRACEDROPSTAKETIME);
					CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_WAITDROPI_LOOPTIME);
					// 設定地面可下注
					SetCasinoMap(meindex, 0, TRUE);
					// 設定寵物
					CHAR_setWorkInt(petindex1, NPC_WORK_STATE, NPC_State_Wait);
					CHAR_setInt(petindex1, CHAR_LOOPINTERVAL, 1000);
					CHAR_setInt(petindex1, CHAR_BASEIMAGENUMBER, petracegra[randpet].pet1gra);
					CHAR_setChar(petindex1, CHAR_NAME, petracegra[randpet].pet1name);

					CHAR_setWorkInt(petindex2, NPC_WORK_STATE, NPC_State_Wait);
					CHAR_setInt(petindex2, CHAR_LOOPINTERVAL, 1000);
					CHAR_setInt(petindex2, CHAR_BASEIMAGENUMBER, petracegra[randpet].pet2gra);
					CHAR_setChar(petindex2, CHAR_NAME, petracegra[randpet].pet2name);

					CHAR_setWorkInt(petindex3, NPC_WORK_STATE, NPC_State_Wait);
					CHAR_setInt(petindex3, CHAR_LOOPINTERVAL, 1000);
					CHAR_setInt(petindex3, CHAR_BASEIMAGENUMBER, petracegra[randpet].pet3gra);
					CHAR_setChar(petindex3, CHAR_NAME, petracegra[randpet].pet3name);
					// 設定寵物狀況
#ifdef _DROPSTAKENEW
#else
					petracegra[randpet].payrate1 = 0;
					petracegra[randpet].payrate3 = 0;
					petracegra[randpet].payrate2 = 0;
#endif
					{
						int state1 = RAND(0, 3);
						int state2 = RAND(0, 3);
						int state3 = RAND(0, 3);
						if (state1 == 0)
							sprintf(petracegra[randpet].state1, "良好");
						else if (state1 == 1)
							sprintf(petracegra[randpet].state1, "普通");
						else if (state1 == 2)
							sprintf(petracegra[randpet].state1, "不佳");
						else if (state1 == 3)
							sprintf(petracegra[randpet].state1, "未知");
						if (state2 == 0)
							sprintf(petracegra[randpet].state2, "良好");
						else if (state2 == 1)
							sprintf(petracegra[randpet].state2, "普通");
						else if (state2 == 2)
							sprintf(petracegra[randpet].state2, "不佳");
						else if (state2 == 3)
							sprintf(petracegra[randpet].state2, "未知");
						if (state3 == 0)
							sprintf(petracegra[randpet].state3, "良好");
						else if (state3 == 1)
							sprintf(petracegra[randpet].state3, "普通");
						else if (state3 == 2)
							sprintf(petracegra[randpet].state3, "不佳");
						else if (state3 == 3)
							sprintf(petracegra[randpet].state3, "未知");
					}
					snprintf(tmpbuf, sizeof(tmpbuf), "寵物競速準備中，請大家踴躍下注！");
					CHAR_talkToFloor(floor, meindex, tmpbuf, CHAR_COLORYELLOW);
				}
			}
			break;
		default:
			break;
	}
}

void NPC_findRacePetIndex(int meindex, int floor)
{
	int i = 0, j = 0;
	time_t t1;
	time(&t1);

	if (CHAR_getWorkInt(meindex, NPC_WORK_FINDPETFLAG) == 0)
	{
		for (i = 0; i < CHAR_getCharNum(); i++)
		{
			if (CHAR_getInt(i, CHAR_FLOOR) == floor)
			{
				if (CHAR_getInt(i, CHAR_WHICHTYPE) == CHAR_PETRACEPET)
				{
					for (j = 0; j < PETRACEPETNUM; j++)
					{
						if (CHAR_getWorkInt(meindex, NPC_WORK_PET1 + j) == 0)
						{
							CHAR_setWorkInt(meindex, NPC_WORK_PET1 + j, i);
							CHAR_setWorkInt(i, CHAR_NPCWORKINT2, meindex);
							break;
						}
					}
				}
			}
		}
		if (CHAR_getWorkInt(meindex, NPC_WORK_PET1) > 0
			&& CHAR_getWorkInt(meindex, NPC_WORK_PET2) > 0
			&& CHAR_getWorkInt(meindex, NPC_WORK_PET3) > 0)
		{
			CHAR_setWorkInt(meindex, NPC_WORK_FINDPETFLAG, 1);
			CHAR_setWorkInt(meindex, NPC_WORK_STATE, NPC_State_WaitDropStake);
			CHAR_setInt(meindex, CHAR_LOOPINTERVAL, PETRACEMASTER_WAITDROPI_LOOPTIME);
			CHAR_setWorkInt(meindex, NPC_WORK_STATECHANGE, t1 + PETRACEDROPSTAKETIME);
		}
		else CHAR_setWorkInt(meindex, NPC_WORK_FINDPETFLAG, 0);
	}
}

void NPC_givePriceToPlayer(int meindex, int floor, int winpetindex)
{
	int wincasinotype = 0;
	if (CHAR_getWorkInt(meindex, NPC_WORK_PET1) == winpetindex)
		wincasinotype = PET_RACE1;
	else if (CHAR_getWorkInt(meindex, NPC_WORK_PET2) == winpetindex)
		wincasinotype = PET_RACE2;
	else if (CHAR_getWorkInt(meindex, NPC_WORK_PET3) == winpetindex)
		wincasinotype = PET_RACE3;
	if (wincasinotype == 0) return;
	else 
		CasinoPay(meindex, wincasinotype);
}
#endif
