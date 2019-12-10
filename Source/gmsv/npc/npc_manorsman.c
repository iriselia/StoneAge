#include "version.h"
#include <time.h>
#include "char.h"
#include "object.h"
#include "lssproto_serv.h"
#include "npcutil.h"
#include "handletime.h"
#include "npc_scheduleman.h"
#include "family.h"
#include "npc_manorsman.h"
#include "saacproto_cli.h"
#include "net.h"
#include "configfile.h"
#include "log.h"

/*
 * 莊園 PK 登記員
 *
 */
#define MAX_MANORSMAN 22
static int FMPK_ManorsmanList[MAX_MANORSMAN]={
-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,
-1,-1 };

enum {
	NPC_WORK_ID = CHAR_NPCWORKINT1,		// 登記員 ID, 從 0 開始
	NPC_WORK_MANORID = CHAR_NPCWORKINT2,	// 莊園編號
	NPC_WORK_CHALLENGEWAIT = CHAR_NPCWORKINT3,	// 挑戰等待時間
	NPC_WORK_PEACEWAIT = CHAR_NPCWORKINT4,	// 休戰時間
	NPC_WORK_PREVLOOPTIME = CHAR_NPCWORKINT5,	// 前一次處理 Loop 的時間
#ifdef _NEW_MANOR_LAW
	NPC_WORK_BETTLETIME = CHAR_NPCWORKINT6	// 記錄開打時的日期
#endif
};

#define SCHEDULEFILEDIR		"./Schedule/"

#ifdef _FIX_FAMILY_PK_LIMIT	   // WON ADD 將可挑戰莊園的由前十大改為前二十大
#define PK_LIMIT	20	       
#endif


extern struct FM_POINTLIST  fmpointlist;
extern void NPC_talkToFloor(int floor, int index1, int index2, char *data);
#ifdef _MANOR_PKRULE
extern struct  FMS_DPTOP fmdptop;
#endif

void NPC_ManorLoadPKSchedule(int meindex);	// Load schedule from disk
#ifdef _NEW_MANOR_LAW
void SortManorSchedule();
int SortManorScheduleMomentum(const void *indexa, const void *indexb);
extern int familyMemberIndex[FAMILY_MAXNUM][FAMILY_MAXMEMBER];
int g_iSortManor;
#endif

int NPC_getManorsmanListIndex( int ID)
{

	if( ID < 0 || ID >= MAX_MANORSMAN ){
		return	FMPK_ManorsmanList[ ID];
	}
	return -1;
}
BOOL NPC_ManorSmanInit( int meindex )
{
  char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
  int meid;
  int a;

  CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TYPEMANORSCHEDULEMAN );

  // 參數
  NPC_Util_GetArgStr(meindex, argstr, sizeof(argstr));
  meid = NPC_Util_GetNumFromStrWithDelim(argstr, "id" );
  if ((meid<0) || (meid>=MAX_SCHEDULEMAN)) {
    print("MANORSMAN init error: invalid ID(%d)\n",meid);
    meid=0;
  }
  CHAR_setWorkInt(meindex, NPC_WORK_ID, meid);

  a = NPC_Util_GetNumFromStrWithDelim(argstr, "loop" );
  if ((a<100) || (a>10000)) a=1000;
  CHAR_setInt(meindex, CHAR_LOOPINTERVAL, a);

  a = NPC_Util_GetNumFromStrWithDelim(argstr, "manorid" );
  if ((a<1) || (a>/*4*/MANORNUM)) {// CoolFish 2002/2/25
    print("MANORSMAN init error: invalid manor id(%d).", a);
    a=1;
  }
  CHAR_setWorkInt(meindex, NPC_WORK_MANORID, a);

  a = NPC_Util_GetNumFromStrWithDelim(argstr, "challengewait");
  if ((a<1) || (a>259200)) {
    print("MANORSMAN init error: invalid challengewait(%d).",a);
    a=259200;
  }
  CHAR_setWorkInt(meindex, NPC_WORK_CHALLENGEWAIT, a);

  a = NPC_Util_GetNumFromStrWithDelim(argstr, "peacewait");
  if ((a<1) || (a>432000)) {
    print("MANORSMAN init error: invalid peacewait(%d).",a);
    a=604800;
  }
  CHAR_setWorkInt(meindex, NPC_WORK_PEACEWAIT, a);

  NPC_ManorLoadPKSchedule(meindex);

  CHAR_setWorkInt(meindex, NPC_WORK_PREVLOOPTIME, NowTime.tv_sec);
#ifdef _NEW_MANOR_LAW
	CHAR_setWorkInt(meindex,NPC_WORK_BETTLETIME,0);
#endif

  if( CHAR_getWorkInt( meindex, NPC_WORK_ID) >= 0 &&
	  CHAR_getWorkInt( meindex, NPC_WORK_ID) <MAX_MANORSMAN ){
		FMPK_ManorsmanList[ CHAR_getWorkInt( meindex, NPC_WORK_ID)] = meindex;
  }

  return TRUE;
}

#ifndef _NEW_MANOR_LAW
void NPC_ManorSmanTalked(int meindex, int talkerindex, char *msg, int color)
{
  char buf[4096];
  int fd;
  int fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID)*MAX_SCHEDULE;

  if (NPC_Util_CharDistance(meindex, talkerindex) > 2) return;
  if (!NPC_Util_isFaceToFace(meindex, talkerindex, 2)) return;
  fd = getfdFromCharaIndex(talkerindex);
  if (fd == -1) return;

  switch (fmpks[fmpks_pos+1].flag) {
  case FMPKS_FLAG_NONE:
    // 沒有約定戰鬥，此時可以下戰書
    saacproto_ACFMPointList_send(acfd);
#ifdef _MANOR_PKRULE
	sprintf(buf, "莊園所有權爭奪戰的挑戰資格\n\n"
                 "一、沒有擁有莊園的家族\n"
	#ifdef _FIX_FAMILY_PK_LIMIT	   // WON ADD 將可挑戰莊園的由前十大改為前二十大
				 "二、家族排行必需為前二十大家族\n"
	#else
                 "二、家族排行必需為前十大家族\n"
	#endif
				 "三、踢館時間：下午六點至凌晨三點\n\n"
                 "請稍待，我將確認你的資格。");
#else
    sprintf(buf, "莊園所有權爭奪戰的挑戰資格\n\n"
                 "一、沒有擁有莊園的家族\n"
                 "二、家族的聲望高於莊園家族的聲望\n\n"
                 "請稍待，我將確認你的資格。");
#endif
    lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
	    		WINDOW_BUTTONTYPE_YESNO,
    			CHAR_WINDOWTYPE_CHECKMAN_START,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			buf);
    break;
  case FMPKS_FLAG_MANOR_PREPARE:
    // 目前已經定好戰鬥，準備中
    {
      int timeleft=fmpks[fmpks_pos+1].dueltime-NowTime.tv_sec;
      int dd,hh,mm;
      char buf2[4096];
      dd = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 86400;
      hh = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 3600 - dd*24;
      mm = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 60 - dd*24*60 - hh*60;
      memset(buf2,0,sizeof(buf2));
      if (dd>0) sprintf(buf, " %d 天", dd); else strcpy(buf, "");
      strcat(buf2, buf);
      if (hh>0) sprintf(buf, " %d 小時", hh); else strcpy(buf, "");
      strcat(buf2, buf);
      if (mm>0) sprintf(buf, " %d 分鍾", mm); else strcpy(buf, "");
      strcat(buf2, buf);
      if (strlen(buf2)==0)
        sprintf(buf, "莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n"
                    "即將開始，請準備入場。",
    		fmpks[fmpks_pos+1].guest_name,
    		fmpks[fmpks_pos+1].host_name);
      else
        sprintf(buf, "莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n"
                    "預定將在%s後開始。",
    		fmpks[fmpks_pos+1].guest_name,
    		fmpks[fmpks_pos+1].host_name,
    		buf2);
      lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			CHAR_WINDOWTYPE_CHECKMAN_END,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			buf);
    }
    break;
  case FMPKS_FLAG_MANOR_PEACE:
    // 戰鬥已經結束的和平時期
    {
      int timeleft=fmpks[fmpks_pos+1].dueltime-NowTime.tv_sec;
      int dd,hh,mm;
      char buf2[4096];
      dd = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 86400;
      hh = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 3600 - dd*24;
      mm = /*fmpks[fmpks_pos+1].dueltime*/ timeleft / 60 - dd*24*60 - hh*60;
      strcpy(buf2,"");
      if (dd>0) sprintf(buf, " %d 天", dd); else strcpy(buf, "");
      strcat(buf2, buf);
      if (hh>0) sprintf(buf, " %d 小時", hh); else strcpy(buf, "");
      strcat(buf2, buf);
      if (mm>0) sprintf(buf, " %d 分鍾", mm); else strcpy(buf, "");
      strcat(buf2, buf);

      if (strlen(buf2)==0)
        strcpy(buf, "請稍等，讓我準備一下申請踢館的表格。");
      else
        sprintf(buf, "現在是休戰時期，要踢館的話\n請%s後再來申請。",buf2);
      lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			CHAR_WINDOWTYPE_CHECKMAN_END,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX), buf);
    }
    break;
  case FMPKS_FLAG_MANOR_BATTLEBEGIN:
    // 目前正在進行踢館
    sprintf(buf, "莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n"
                 "開∼打∼囉∼\n還沒進場的人趕快進場吧。",
    		fmpks[fmpks_pos+1].guest_name,
    		fmpks[fmpks_pos+1].host_name);
    lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			CHAR_WINDOWTYPE_CHECKMAN_END,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			buf);
    break;
  case FMPKS_FLAG_MANOR_OTHERPLANET:
    // 在別的星球進行戰鬥
    sprintf(buf, "莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n"
    		 "決鬥地點在 %s 。",
              fmpks[fmpks_pos+1].guest_name,
              fmpks[fmpks_pos+1].host_name,
              fmpks[fmpks_pos+2].host_name);
    lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			CHAR_WINDOWTYPE_CHECKMAN_END,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			buf);
    break;
  }
}
#else
void NPC_ManorSmanTalked(int meindex, int talkerindex, char *msg, int color)
{
  char buf[4096];
  int fd;
  int fmpks_pos = CHAR_getWorkInt(meindex,NPC_WORK_ID)*MAX_SCHEDULE;
	int manorid = CHAR_getWorkInt(meindex,NPC_WORK_MANORID);

	if(CHAR_CHECKINDEX(meindex) == FALSE){
		printf("\nNPC_ManorSmanTalked error!(meindex:%d)",meindex);
		return;
	}
  if(NPC_Util_CharDistance(meindex,talkerindex) > 2) return;
  if(!NPC_Util_isFaceToFace(meindex,talkerindex,2)) return;
  fd = getfdFromCharaIndex(talkerindex);
  if(fd == -1) return;

	if(fmpks[fmpks_pos+1].flag == FMPKS_FLAG_CHALLENGE) fmpks[fmpks_pos+1].flag = FMPKS_FLAG_NONE;

  switch(fmpks[fmpks_pos+1].flag){
	// 挑戰期 1800~2200
  case FMPKS_FLAG_NONE:
	case FMPKS_FLAG_WAIT:
    saacproto_ACFMPointList_send(acfd);
		sprintf(buf,"莊園所有權爭奪戰的挑戰資格\n\n"
								"一、沒有莊園的家族\n"
								"二、家族的氣勢高於莊園家族的氣勢\n"
								"三、約戰時間終止時，氣勢排名第一者獲得挑戰資格\n"
								"四、氣勢相同者以最先報名者獲得資格\n"
								"按 OK 我將確認你的資格，按 NO 觀看挑戰排名");
    lssproto_WN_send(fd,WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_YESNO,CHAR_WINDOWTYPE_CHECKMAN_START,
    								 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),buf);
		if(fmpointlist.fm_momentum[manorid-1] <= -1){
			int hadfmindex,index;
			char token[256];

			getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",5,token,sizeof(token));
			hadfmindex = atoi(token);
			if(hadfmindex != -1){
				for(index=0;index<FAMILY_MAXNUM;index++){
					// 找出守莊家族的索引
					if(fmdptop.fmtopid[index] == (hadfmindex - 1)) break;
				}
				if(index >= FAMILY_MAXNUM){
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",6,token,sizeof(token));
					printf("\nNPC_ManorSmanTalked():save fm_momentum error(%d:%s)",hadfmindex,token);
					break;
				}
				// 記錄這個時刻守莊家族的氣勢
				else fmpointlist.fm_momentum[manorid-1] = fmdptop.fmMomentum[index];
			}
		}
		SortManorSchedule();
    break;
	// 挑戰準備期 2200~取得挑戰資格的家族當時挑戰的時間(隔天的 1800~2200 之間)
  case FMPKS_FLAG_MANOR_PREPARE:
    // 目前已經定好戰鬥，準備中
    {
      int timeleft = fmpks[fmpks_pos+1].dueltime - NowTime.tv_sec;
      int mm,hh,dd;
      char buf2[4096];

			dd = timeleft / 86400;
			hh = timeleft / 3600 - dd * 24;
      mm = timeleft / 60 - dd * 24 * 60 - hh * 60;
      memset(buf2,0,sizeof(buf2));
			if(dd > 0) sprintf(buf," %d 天",dd); else strcpy(buf,"");
      strcat(buf2,buf);
			if(hh > 0) sprintf(buf," %d 小時",hh); else strcpy(buf,"");
      strcat(buf2,buf);
      if(mm > 0) sprintf(buf," %d 分鍾",mm); else strcpy(buf,"");
      strcat(buf2,buf);
      if(strlen(buf2) == 0)
        sprintf(buf,"莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n即將開始，請準備入場。",
    						fmpks[fmpks_pos+1].guest_name,fmpks[fmpks_pos+1].host_name);
      else
				sprintf(buf,"莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n預定將在%s後開始。",
    						fmpks[fmpks_pos+1].guest_name,fmpks[fmpks_pos+1].host_name,buf2);
      lssproto_WN_send(fd,WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
    									 CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),buf);
    }
    break;
	// 休戰期及準備期,五天
  case FMPKS_FLAG_MANOR_PEACE:
	case FMPKS_FLAG_MANOR_PEACE_SAVE:
    {
      int timeleft = fmpks[fmpks_pos+1].dueltime-NowTime.tv_sec;
      int dd,hh,mm;
      char buf2[4096];
      dd = timeleft / 86400;
      hh = timeleft / 3600 - dd*24;
      mm = timeleft / 60 - dd*24*60 - hh*60;
      memset(buf2,0,sizeof(buf2));
      if(dd > 0) sprintf(buf," %d 天",dd); else strcpy(buf,"");
      strcat(buf2,buf);
      if(hh > 0) sprintf(buf," %d 小時",hh); else strcpy(buf,"");
      strcat(buf2,buf);
      if(mm > 0) sprintf(buf," %d 分鍾",mm); else strcpy(buf,"");
      strcat(buf2,buf);

      if(strlen(buf2) == 0) strcpy(buf, "請稍等，讓我準備一下申請踢館的表格。");
      else sprintf(buf, "現在非挑戰時期，要挑戰的話\n請%s後再來申請。",buf2);
      lssproto_WN_send(fd,WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
											 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),buf);
    }
    break;
	// 挑戰中
  case FMPKS_FLAG_MANOR_BATTLEBEGIN:
    sprintf(buf,"莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n開∼打∼囉∼\n還沒進場的人趕快進場吧。",
    				fmpks[fmpks_pos+1].guest_name,fmpks[fmpks_pos+1].host_name);
    lssproto_WN_send(fd,WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
    								 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),buf);
    break;
  case FMPKS_FLAG_MANOR_OTHERPLANET:
    // 在別的星球進行戰鬥
    sprintf(buf,"莊園所有權爭奪戰\n『%s ＶＳ %s』\n\n決鬥地點在 %s 。",
            fmpks[fmpks_pos+1].guest_name,fmpks[fmpks_pos+1].host_name,fmpks[fmpks_pos+2].host_name);
    lssproto_WN_send(fd,WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
    								 CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),buf);
    break;
  }
}
#endif

void NPC_ManorSmanWindowTalked(int meindex, int talkerindex,
				int seqno, int select, char *data)
{
  int fd, fmpks_pos, manorid, tkfmindex, tkfmdp;
  char buf[4096],token[256];
  int hadfmindex;
#ifndef _ACFMPK_LIST
  char hadfmname[256];
#endif
  int hadfmpopular;
  
  if (NPC_Util_CharDistance(meindex,talkerindex) > 2) return;
  if (!NPC_Util_isFaceToFace(meindex, talkerindex, 2)) return;
  fd = getfdFromCharaIndex(talkerindex);
  if (fd == -1) return;
  fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID)*MAX_SCHEDULE;
  manorid = CHAR_getWorkInt(meindex, NPC_WORK_MANORID);
  tkfmindex = CHAR_getWorkInt(talkerindex, CHAR_WORKFMINDEXI);
  tkfmdp = CHAR_getWorkInt(talkerindex, CHAR_WORKFMDP);
  getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1], "|", 5, token, sizeof(token));
  hadfmindex = atoi(token);
  getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1], "|", 7, token, sizeof(token));
  hadfmpopular = atoi(token);
  
  switch (seqno){
  case CHAR_WINDOWTYPE_CHECKMAN_START:
    if (select==WINDOW_BUTTONTYPE_YES){
#ifdef _ACFMPK_NOFREE
		sprintf(buf, "莊園挑戰會有一天的準備期\n"
				"你可以隨時詢問我以得知剩下的時間\n"
				"挑戰結束後不論勝負將會有二天的休戰期\n"
				"休戰期間不能挑戰\n\n"
				"你確定要挑戰這個莊園嗎？");
		lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
				WINDOW_BUTTONTYPE_YESNO,
				CHAR_WINDOWTYPE_CHECKMAN_MAIN,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX), buf);
				return;
#endif

#ifdef _FMVER21      
		  if ((CHAR_getInt(talkerindex,CHAR_FMLEADERFLAG)==FMMEMBER_LEADER) &&
#else
      if ((CHAR_getInt(talkerindex,CHAR_FMLEADERFLAG)==1) &&
#endif      
				(CHAR_getWorkInt(talkerindex, CHAR_WORKFMSETUPFLAG)==1)){
        if (hadfmindex != -1) {
          if (hadfmindex-1 != tkfmindex){
            int check=0,i;
            // Arminius 2.25 fix: fmpks 中第 1~"MANORNUM" 組一定要給 manorsman
            for (i=0; i<=/*3*/MANORNUM-1; i++) {	// 9個莊園
              getStringFromIndexWithDelim(fmpointlist.pointlistarray[i], "|", 5, token, sizeof(token));
              if (tkfmindex==atoi(token)-1) check=1;
            }
            for (i=1; i<=/*4*/MANORNUM; i++) {	// 是否已經挑戰其他莊園
              if ((fmpks[i*MAX_SCHEDULE+1].guest_index==tkfmindex) &&
								(strcmp(fmpks[i*MAX_SCHEDULE+1].guest_name,
								CHAR_getChar(talkerindex, CHAR_FMNAME))==0)
								) {
                check=2;
              }
            }
            if (check==0){
							int won1;
#ifdef _MANOR_PKRULE		    // WON ADD 2002/01/22
	#ifdef _NEW_MANOR_LAW
		//-----------------------------------------
							int index;
							won1 = 1;

							for(index=0;index<FAMILY_MAXNUM;index++){
								// 找出挑戰家族的索引
								if(fmdptop.fmtopid[index] == tkfmindex) break;
							}
							if(index >= FAMILY_MAXNUM){
								printf("\nNPC_ManorSmanWindowTalked():find tkfmIndex error (%d)",tkfmindex);
							}
							else
							// 如果挑戰家族氣勢不足守莊家族氣勢的九成,不能挑戰
							if(fmdptop.fmMomentum[index] < fmpointlist.fm_momentum[manorid-1]*0.9) won1 = 0;
		//-----------------------------------------
	#else
		//-----------------------------------------
							int i;
							char won2[256];
							won1 = 0;
		#ifdef _FIX_FAMILY_PK_LIMIT	   // WON ADD 將可挑戰莊園的由前十大改為前二十大		
							for(i=0; i< PK_LIMIT; i++){
		#else
							for(i=0; i<10; i++){
		#endif
								if( getStringFromIndexWithDelim( fmdptop.topmemo[i], "|", 3, won2, sizeof(won2)) == FALSE )	{
									print("err Get fmdptop.topmemo[%d] if FALSE !!\n", i);
									break;
								}
								if( strcmp(CHAR_getChar(talkerindex, CHAR_FMNAME) ,won2) == 0 ){
									won1= 1;
									break;
								}
		//-----------------------------------------
							}
	#endif

							if(won1 == 1){		
			// WON END
	#ifdef _NEW_MANOR_LAW
							sprintf(buf,"確定莊園挑戰資格後會有一天的準備時間\n"
													"你可以隨時詢問我以得知剩下的時間\n"
													"挑戰結束後至下次挑戰期將會有一天的休戰期\n"
													"和四天的準備期，在這五天期間內不能挑戰\n\n"
													"你確定要挑戰這個莊園嗎？");
	#else
							sprintf(buf,"莊園挑戰會有一天的準備期\n"
													"你可以隨時詢問我以得知剩下的時間\n"
													"挑戰結束後不論勝負將會有二天的休戰期\n"
													"休戰期間不能挑戰\n\n"
													"你確定要挑戰這個莊園嗎？");
	#endif
#else
              if(tkfmdp >= hadfmpopular){
								sprintf(buf,"莊園挑戰會有一天的準備期\n"
                            "你可以隨時詢問我以得知剩下的時間\n"
												    "挑戰結束後不論勝負將會有五天的休戰期\n"
														"休戰期間不能挑戰\n\n"
														"你確定要挑戰這個莊園嗎？");
#endif
                lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    							WINDOW_BUTTONTYPE_YESNO,
    															CHAR_WINDOWTYPE_CHECKMAN_MAIN,
    															CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    															buf);
    					}
							else{
#ifdef _NEW_MANOR_LAW
								sprintf(buf, "你的家族氣勢尚未到達莊園家族氣勢的九成，請再加油。");
#else
	#ifdef _MANOR_PKRULE
		#ifdef _FIX_FAMILY_PK_LIMIT	   // WON ADD 將可挑戰莊園的由前十大改為前二十大
                sprintf(buf, "你的家族聲望尚未進入前二十大家族，請再加油。");
		#else
                sprintf(buf, "你的家族聲望尚未進入前十大家族，請再加油。");
		#endif
	#else
                sprintf(buf, "你的家族聲望尚不足以挑戰\n"
                             "擁有這個莊園的家族，請再加油。");
	#endif
#endif
                lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
						              	    	WINDOW_BUTTONTYPE_OK,
													   			CHAR_WINDOWTYPE_CHECKMAN_END,
												    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
													   			buf);
		   	      }
            }
						else if(check == 2){
              sprintf(buf, "你的家族正在挑戰其他莊園，\n"
                           "請把機會留給其他家族吧！");
              lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
													    	WINDOW_BUTTONTYPE_OK,
											    			CHAR_WINDOWTYPE_CHECKMAN_END,
											    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
											    			buf);
            }
						else{
              sprintf(buf, "一個家族只能擁有一個莊園，\n"
                           "如果莊園家族之間需要切磋\n"
                           "請到家族ＰＫ場。");
              lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    						WINDOW_BUTTONTYPE_OK,
											    			CHAR_WINDOWTYPE_CHECKMAN_END,
											    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
										    				buf);
            }
          }
					else{
            sprintf(buf, "這個莊園已經是你的家族的喔。");
            lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              						  	WINDOW_BUTTONTYPE_OK,
									    				CHAR_WINDOWTYPE_CHECKMAN_END,
											   			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
										    			buf);
          }
        }
				else{
          sprintf(buf, "現在並沒有莊園家族，\n"
                       "直接去申請遷入就可以了喲。");
          lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
			              	    	WINDOW_BUTTONTYPE_OK,
    												CHAR_WINDOWTYPE_CHECKMAN_END,
									    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
									    			buf);
        }
      }
			else{
        sprintf(buf, "只有族長可以下戰書喔。");
        lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    			WINDOW_BUTTONTYPE_OK,
									   			CHAR_WINDOWTYPE_CHECKMAN_END,
								    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
								    			buf);
      }
    }
#ifdef _NEW_MANOR_LAW
		else if(select == WINDOW_BUTTONTYPE_NO){
			int i,index,manorindex;
			char szMsg[3072];

			sprintf(buf,"              莊園挑戰排名表\n名次 家族名稱         時間          氣勢\n");
			manorindex = CHAR_getWorkInt(meindex,NPC_WORK_MANORID) - 1;
			// 送出挑戰排名
			for(i=0;i<10;i++){
				index = ManorSchedule[manorindex].iSort[i];
				if(ManorSchedule[manorindex].iFmIndex[index] != -1){
					sprintf(szMsg,"%2d   %s\n",i+1,ManorSchedule[manorindex].szMemo[index]);
					strcat(buf,szMsg);
				}
			}
			lssproto_WN_send(fd,WINDOW_FMMESSAGETYPE_MANOR_SCHEDULE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
								    	 CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),buf);
		}
#endif
    break;
  case CHAR_WINDOWTYPE_CHECKMAN_MAIN:	  
    if (select==WINDOW_BUTTONTYPE_YES) {

#ifdef _ACFMPK_NOFREE
	#ifndef _NEW_MANOR_LAW
		NPC_ManorSavePKSchedule(meindex, talkerindex, 0);
		sprintf( buf, "莊園挑戰登陸確認中，請稍後。");
		CHAR_talkToCli( talkerindex, meindex, buf, CHAR_COLORYELLOW);
	#else
		NPC_ManorAddToSchedule(meindex,talkerindex);
	#endif
		return;
#endif

#ifdef _FMVER21    
      if ((CHAR_getInt(talkerindex,CHAR_FMLEADERFLAG)==FMMEMBER_LEADER) && 
#else
      if ((CHAR_getInt(talkerindex,CHAR_FMLEADERFLAG)==1) &&     
#endif      
          (CHAR_getWorkInt(talkerindex, CHAR_WORKFMSETUPFLAG)==1) &&   
          (hadfmindex != -1) &&
		  (hadfmindex - 1 != tkfmindex)    
#ifndef _MANOR_PKRULE
          && (tkfmdp >= hadfmpopular)
#endif
		  ){
#ifndef _ACFMPK_LIST
		  struct tm tm1; 
#endif

#ifndef _NEW_MANOR_LAW
	#ifdef _MANOR_PKRULE
		  time_t timep;
		  struct tm *p;		  
		  time(&timep);
		  p = gmtime(&timep);		
		  if( p->tm_hour+8 > 24)
		  	  p->tm_hour = p->tm_hour-16;
		  else
			  p->tm_hour = p->tm_hour+8;		 
		  if(p->tm_hour<18 && p->tm_hour>2){
			  sprintf(buf, "請於下午６：００至凌晨３：００再來約戰吧！");
			  lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			        CHAR_WINDOWTYPE_CHECKMAN_END,
    			        CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			        buf);
			  break;
		  }		  
	#endif
			// WON ADD 修正家族pk場的約戰問題
			if( fmpks[fmpks_pos+1].flag != FMPKS_FLAG_NONE ){
			     sprintf(buf, "這個莊園已經有人約戰了喔。");
					 lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
						 WINDOW_BUTTONTYPE_OK,
						 CHAR_WINDOWTYPE_CHECKMAN_END,
						 CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
						 buf);
					 return;
			}
#endif
#ifdef _ACFMPK_LIST
	#ifndef _NEW_MANOR_LAW
		NPC_ManorSavePKSchedule(meindex, talkerindex, 0);
		sprintf( buf, "莊園挑戰登陸確認中，請稍後。");
		CHAR_talkToCli( talkerindex, meindex, buf, CHAR_COLORYELLOW);
	#else
		NPC_ManorAddToSchedule(meindex,talkerindex);
	#endif
#else
	    memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
        getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1], "|", 6, token, sizeof(token));
        strcpy(hadfmname, token);

        fmpks[fmpks_pos+1].dueltime = CHAR_getWorkInt(meindex, NPC_WORK_CHALLENGEWAIT) + NowTime.tv_sec
                                      - tm1.tm_min*60 + 1800; // Arminius 11.1 改成一律 xx:30 開打
        fmpks[fmpks_pos+1].host_index = hadfmindex-1;
        strcpy(fmpks[fmpks_pos+1].host_name, hadfmname);
        fmpks[fmpks_pos+1].guest_index = tkfmindex;
        strcpy(fmpks[fmpks_pos+1].guest_name, CHAR_getChar(talkerindex, CHAR_FMNAME));        
        fmpks[fmpks_pos+1].prepare_time = 15;
        fmpks[fmpks_pos+1].max_player = 50;
        fmpks[fmpks_pos+1].win = -1;
        fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_OTHERPLANET;
        strcpy(fmpks[fmpks_pos+2].host_name, getGameserverID());

	#ifndef _NEW_MANOR_LAW
		NPC_ManorSavePKSchedule(meindex, talkerindex, 0);
		sprintf(buf, "莊園挑戰已經設定完成，請好好準備。");
        lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
              	    	WINDOW_BUTTONTYPE_OK,
    			CHAR_WINDOWTYPE_CHECKMAN_END,
    			CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),
    			buf);
	#else
		NPC_ManorAddToSchedule(meindex,talkerindex);
	#endif
    fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_PREPARE;
#endif
        
      }
    }
    break;
  }
}

void NPC_CleanPkList( int ti)
{
// Terry fix 要清為-1不能清為0
/*	fmpks[ ti+1 ].host_index = 0;
	fmpks[ ti+1].guest_index=0;
	fmpks[ ti].host_index=0;
	fmpks[ ti].guest_index=0;*/
	fmpks[ ti+1 ].host_index = -1;
	fmpks[ ti+1].guest_index=-1;
	fmpks[ ti].host_index=-1;
	fmpks[ ti].guest_index=-1;

	strcpy(fmpks[ ti+1].host_name,"");
	strcpy(fmpks[ ti+1].guest_name,"");
	strcpy(fmpks[ ti].host_name,"");
	strcpy(fmpks[ ti].guest_name,"");
	strcpy(fmpks[ ti+2].host_name,"");
}

void NPC_ManorSmanLoop(int meindex)
{
	struct tm tm1;
	int fmpks_pos;
#ifdef _NEW_MANOR_LAW
	int iOffsetTime;
#endif

	if(CHAR_CHECKINDEX(meindex) == FALSE){
		printf("\nNPC_ManorSmanLoop error!(meindex:%d)",meindex);
		return;
	}

	fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID)*MAX_SCHEDULE;
  memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));

  switch (fmpks[fmpks_pos+1].flag) {
  case FMPKS_FLAG_NONE:
#ifdef _NEW_MANOR_LAW
		{
			int hadfmindex,index,manorid;
			char token[256];
			//此時可以開始挑戰
			manorid = CHAR_getWorkInt(meindex,NPC_WORK_MANORID);
			getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",5,token,sizeof(token));
			hadfmindex = atoi(token);
			if(hadfmindex != -1){
				for(index=0;index<FAMILY_MAXNUM;index++){
					// 找出守莊家族的索引
					if(fmdptop.fmtopid[index] == (hadfmindex - 1)) break;
				}
				if(index >= FAMILY_MAXNUM){
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",6,token,sizeof(token));
					printf("\nNPC_ManorSmanLoop():save fm_momentum error(%d:%s)",hadfmindex,token);
					break;
				}
				// 記錄這個時刻守莊家族的氣勢
				else fmpointlist.fm_momentum[manorid-1] = fmdptop.fmMomentum[index];
				// 進入下一個狀態
				fmpks[fmpks_pos+1].flag = FMPKS_FLAG_WAIT;
				// 挑戰期共4小時(1800~2200)
				fmpks[fmpks_pos+1].dueltime = NowTime.tv_sec + 3600 * 4;
				NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_WAIT,fmpks[fmpks_pos+1].dueltime,tm1);
			}
			SortManorSchedule();
		}
#endif    
    break;
#ifdef _NEW_MANOR_LAW
	// 進入挑戰期,已記錄家族氣勢,等待挑戰排程
	case FMPKS_FLAG_WAIT:
		{
			int manorid,i,iPlayerNum = CHAR_getPlayerMaxNum();
			char szMsg[256];

			manorid = CHAR_getWorkInt(meindex,NPC_WORK_MANORID);
			// 還沒開打,氣勢改變要去向AC要即時資料
			fmpointlist.fm_inwar[manorid-1] = TRUE;
			if(fmpks[fmpks_pos+1].dueltime <= NowTime.tv_sec){
				// 挑戰家族中氣勢最高的獲得挑戰權
				int manorindex = CHAR_getWorkInt(meindex,NPC_WORK_MANORID) - 1;
				int iNo1FmIndexSort = ManorSchedule[manorindex].iSort[0];
				int index = ManorSchedule[manorindex].iFmIndex[iNo1FmIndexSort],iFmIndex1,iCharindex;
				char token[256],fmname[256];
				// 沒人挑戰
				if(index < 0){
					// 直接進入到挑戰結束
					fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_BATTLEEND;
					CHAR_setWorkInt(meindex,NPC_WORK_BETTLETIME,tm1.tm_mday);
					// 通知所有人
					for(i=0;i<iPlayerNum;i++){
						if(CHAR_getCharUse(i) != FALSE){
							CHAR_talkToCli(i,-1,"由於無家族挑戰莊園，莊園進入休戰時期",CHAR_COLORBLUE2);
						}
					}
					getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",5,token,sizeof(token));
					iFmIndex1 = atoi(token) - 1;
					if(iFmIndex1 != -1){
						getStringFromIndexWithDelim(fmpointlist.pointlistarray[manorid-1],"|",6,fmname,sizeof(token));
						saacproto_ACFixFMPoint_send(acfd,fmname,iFmIndex1 + 1,iFmIndex1,
							fmname,iFmIndex1 + 1,iFmIndex1,CHAR_getWorkInt(meindex,NPC_WORK_ID));
						// 增加莊園戰勝負Log
						sprintf(token," (%d:%d) %d/%d/%d",tm1.tm_hour,tm1.tm_min,tm1.tm_year+1900,tm1.tm_mon+1,tm1.tm_mday);
						Logfmpk(fmname,iFmIndex1,-1,"無人挑戰",-1,-1,token,"","",2);
						// 原家族守住了莊園,家族成員可得到石幣
						for(i=0;i<FAMILY_MAXMEMBER;i++){
							iCharindex = familyMemberIndex[iFmIndex1][i];
							// 若在線上才給錢 
							if(iCharindex >= 0 && CHAR_getCharUse(iCharindex)){
								// 獲得金錢 = 個人氣勢 * 5000
								int iAddGold = ((float)CHAR_getInt(iCharindex,CHAR_MOMENTUM)/100.0f) * 5000.0f;
								int iGold = CHAR_getInt(iCharindex,CHAR_BANKGOLD),iMaxGold;
								// 先放入個人銀行
								if(iGold + iAddGold > CHAR_MAXBANKGOLDHAVE){
									CHAR_setInt(iCharindex,CHAR_BANKGOLD,CHAR_MAXBANKGOLDHAVE);
									// 個人銀行放不下了,放到個人身上
									iAddGold = iGold + iAddGold - CHAR_MAXBANKGOLDHAVE;
									iGold = CHAR_getInt(iCharindex,CHAR_GOLD);
									iMaxGold = CHAR_getMaxHaveGold(iCharindex);
									if(iGold + iAddGold > iMaxGold) CHAR_setInt(iCharindex,CHAR_GOLD,iMaxGold);
									else CHAR_setInt(iCharindex,CHAR_GOLD,iGold + iAddGold);
									LogFMPKGetMomey(CHAR_getChar(iCharindex,CHAR_FMNAME),
																	CHAR_getChar(iCharindex,CHAR_CDKEY),
																	CHAR_getChar(iCharindex,CHAR_NAME),
																	CHAR_getInt(iCharindex,CHAR_MOMENTUM),iAddGold,0);
								}
								else{
									CHAR_setInt(iCharindex,CHAR_BANKGOLD,iGold + iAddGold);
									LogFMPKGetMomey(CHAR_getChar(iCharindex,CHAR_FMNAME),
																	CHAR_getChar(iCharindex,CHAR_CDKEY),
																	CHAR_getChar(iCharindex,CHAR_NAME),
																	CHAR_getInt(iCharindex,CHAR_MOMENTUM),iAddGold,1);
								}
								CHAR_talkToCli(iCharindex,-1,"辛苦了!守護住莊園的獎金已匯入你的個人銀行",CHAR_COLORRED);
								// 家族的個人及家族氣勢都要歸零
								CHAR_setInt(iCharindex,CHAR_MOMENTUM,0);
								CHAR_talkToCli(iCharindex,-1,"莊園戰後個人及家族氣勢歸零",CHAR_COLORRED);
							}
						}
					}
				}
				else{
					// 挑戰時間已過,選出的挑戰家族和守莊家族有約一天的時間可以準備
					NPC_ManorSavePKSchedule(meindex,index,0,-1,ManorSchedule[manorindex].tm1[iNo1FmIndexSort]);
					// 開打了,氣勢改變也不用去向AC要即時資料了
					fmpointlist.fm_inwar[manorid-1] = FALSE;
					// 通知所有人
					sprintf(szMsg,"%s 家族獲得挑戰莊園資格",ManorSchedule[manorindex].szFmName[iNo1FmIndexSort]);
					
					for(i=0;i<iPlayerNum;i++){
						if(CHAR_getCharUse(i) != FALSE){
							CHAR_talkToCli(i,-1,szMsg,CHAR_COLORBLUE2);
						}
					}
				}
			}
		}
		break;
#endif
  case FMPKS_FLAG_MANOR_PREPARE:
    if(fmpks[fmpks_pos+1].dueltime <= NowTime.tv_sec){
      memcpy(&fmpks[fmpks_pos], &fmpks[fmpks_pos+1], sizeof(FamilyPKSchedule));
      fmpks[fmpks_pos].dueltime = tm1.tm_hour * 100 + tm1.tm_min;
      fmpks[fmpks_pos].flag = FMPKS_FLAG_SCHEDULED;
      fmpks[fmpks_pos+1].dueltime = 0;
      fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_BATTLEBEGIN;
      NPC_talkToFloor(CHAR_getInt(meindex, CHAR_FLOOR) , fmpks[fmpks_pos].host_index,
         	fmpks[fmpks_pos].guest_index, "莊園爭奪戰已經開始，請盡快入場！");
#ifdef _NEW_MANOR_LAW
			CHAR_setWorkInt(meindex,NPC_WORK_BETTLETIME,tm1.tm_mday);
#endif
    }
    break;
  case FMPKS_FLAG_MANOR_PEACE:
    if (fmpks[fmpks_pos+1].dueltime <= NowTime.tv_sec) {
      fmpks[fmpks_pos+1].dueltime = 0;
      fmpks[fmpks_pos+1].flag = FMPKS_FLAG_NONE;
#ifndef _NEW_MANOR_LAW
      NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_NONE);
#else
			NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_NONE,fmpks[fmpks_pos+1].dueltime,tm1);
#endif
    }
    break;
  case FMPKS_FLAG_MANOR_BATTLEBEGIN:
    break;
  case FMPKS_FLAG_MANOR_BATTLEEND:
		{
#ifdef _NEW_MANOR_LAW
			int manorindex = CHAR_getWorkInt(meindex,NPC_WORK_MANORID) - 1;
#endif

			fmpks[fmpks_pos].flag = FMPKS_FLAG_NONE;
#ifndef _NEW_MANOR_LAW
			fmpks[fmpks_pos+1].dueltime = CHAR_getWorkInt(meindex, NPC_WORK_PEACEWAIT) + NowTime.tv_sec;
#else
			// 打完後要再過五天才能再挑戰
			// 若開打的日期和結束戰鬥的日期不一樣表示打超過了一天以上,以一天計
			if(CHAR_getWorkInt(meindex,NPC_WORK_BETTLETIME) != tm1.tm_mday){
				iOffsetTime = tm1.tm_hour * 3600 * -1;	// 扣回00:00
			}
			else{	// 戰鬥結束時未超過一天
				iOffsetTime = (24 - tm1.tm_hour) * 3600;	// 補成00:00
			}
			//#ifdef _75_TEST
			fmpks[fmpks_pos+1].dueltime = NowTime.tv_sec - tm1.tm_min*60 + iOffsetTime + 172800 + (18 * 3600);	// 二天
			//#else
			//	fmpks[fmpks_pos+1].dueltime = NowTime.tv_sec - tm1.tm_min*60 + iOffsetTime + 432000 + (18 * 3600);
			//														|-----------取整點------------| |-補成00:00-| |-五天-|  |18:00開始報名|
			//#endif
#endif
			fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_PEACE_SAVE;
			NPC_CleanPkList( fmpks_pos);
#ifndef _NEW_MANOR_LAW
			NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_MANOR_PEACE_SAVE);
#else
			NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_MANOR_PEACE_SAVE,fmpks[fmpks_pos+1].dueltime,tm1);
			// 清空排程
			memset(&ManorSchedule[manorindex],0,sizeof(ManorSchedule[manorindex]));
			{
				int i;
				for(i=0;i<10;i++) ManorSchedule[manorindex].iSort[i] = ManorSchedule[manorindex].iFmIndex[i] = -1;
				for(i=0;i<FAMILY_MAXHOME;i++) fmpointlist.fm_momentum[i] = -1;
			}
		}
#endif
    break;
  case FMPKS_FLAG_MANOR_PEACE_SAVE:
    fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_PEACE;
    break;
  case FMPKS_FLAG_MANOR_OTHERPLANET:
    break;
  case FMPKS_FLAG_MANOR_READYTOFIGHT:
  case FMPKS_FLAG_MANOR_CLEANFLAG:
    fmpks[fmpks_pos].flag = FMPKS_FLAG_NONE;
    fmpks[fmpks_pos+1].dueltime = CHAR_getWorkInt(meindex, NPC_WORK_PEACEWAIT) + NowTime.tv_sec;
    fmpks[fmpks_pos+1].flag = FMPKS_FLAG_NONE;

		NPC_CleanPkList( fmpks_pos);
#ifndef _NEW_MANOR_LAW
    NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_NONE);
#else
		NPC_ManorSavePKSchedule(meindex, -1, FMPKS_FLAG_NONE,fmpks[fmpks_pos+1].dueltime,tm1);
#endif
    break;
  }
  CHAR_setWorkInt(meindex, NPC_WORK_PREVLOOPTIME, NowTime.tv_sec);
}

void NPC_ManorLoadPKSchedule(int meindex)
{
#ifdef _ACFMPK_LIST
	int fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID);
	saacproto_ACLoadFmPk_send(acfd, fmpks_pos);
#else
	char filename[256], tmp[4096], token[256];
	FILE *f;
	int fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID)*MAX_SCHEDULE;

	snprintf(filename,sizeof(filename), "%s%d_%d_%d",
		       SCHEDULEFILEDIR, CHAR_getInt(meindex, CHAR_FLOOR),
			   CHAR_getInt(meindex, CHAR_X),
			CHAR_getInt(meindex, CHAR_Y) );

	if( ! (f=fopen( filename, "r" )) ){	// create new schedule file
		f = fopen( filename, "w" );
		if( !f ){
			print( "ERROR:Can't create Schedule file %s!\n",filename );
			return;
		}
		strcpy(tmp,"0|-1|-1|-1|-1|-1|-1|-1|-1\n");
		fwrite(tmp, strlen(tmp), 1, f);
	}

	fseek(f, 0, SEEK_SET);	// 檔案開頭
	fgets(tmp, sizeof(tmp), f);
	fmpks[fmpks_pos].flag=-1;

	// 時間
	if (getStringFromIndexWithDelim(tmp,"|",1,token,sizeof(token))) {
		fmpks[fmpks_pos+1].dueltime=atoi(token);
	}
	// 主隊 familyindex
	if (getStringFromIndexWithDelim(tmp,"|",2,token,sizeof(token))) {
	    fmpks[fmpks_pos+1].host_index=atoi(token);
	}
	// 主隊 家族名
	if (getStringFromIndexWithDelim(tmp,"|",3,token,sizeof(token))) {
		strcpy(fmpks[fmpks_pos+1].host_name,makeStringFromEscaped(token));
	}
	// 客隊 familyindex
	if (getStringFromIndexWithDelim(tmp,"|",4,token,sizeof(token))) {
	    fmpks[fmpks_pos+1].guest_index=atoi(token);
	}
	// 客隊 家族名
	if (getStringFromIndexWithDelim(tmp,"|",5,token,sizeof(token))) {
	    strcpy(fmpks[fmpks_pos+1].guest_name,makeStringFromEscaped(token));
	}
	// 準備時間
	if (getStringFromIndexWithDelim(tmp,"|",6,token,sizeof(token))) {
		fmpks[fmpks_pos+1].prepare_time=atoi(token);
	}
	// 最大人數
	if (getStringFromIndexWithDelim(tmp,"|",7,token,sizeof(token))) {
		fmpks[fmpks_pos+1].max_player=atoi(token);
	}
	// 旗標
	if (getStringFromIndexWithDelim(tmp,"|",8,token,sizeof(token))) {
		fmpks[fmpks_pos+1].flag=atoi(token);
	}
	// 對戰星球
	if (getStringFromIndexWithDelim(tmp,"|",9,token,sizeof(token))) {
		strcpy(fmpks[fmpks_pos+2].host_name,makeStringFromEscaped(token));
	}
  
	// no schedule
	fmpks[fmpks_pos].flag=-1;
	// prepare time expired?
	if ((fmpks[fmpks_pos+1].flag==FMPKS_FLAG_MANOR_PREPARE) &&
		  (fmpks[fmpks_pos+1].dueltime<NowTime.tv_sec)) {
		fmpks[fmpks_pos].flag=-1;
// Terry fix 要清為-1不能清為0
//		fmpks[fmpks_pos].host_index=0;
		fmpks[fmpks_pos].host_index=-1;
		strcpy(fmpks[fmpks_pos].host_name,"");
// Terry fix 要清為-1不能清為0
//		fmpks[fmpks_pos].guest_index=0;
		fmpks[fmpks_pos].guest_index=-1;
		strcpy(fmpks[fmpks_pos].guest_name,"");
		fmpks[fmpks_pos+1].flag=-1;
		fmpks[fmpks_pos+1].dueltime=0;
	//    fmpks[fmpks_pos+1].dueltime=NowTime.tv_sec+30*60;	// add more 30 minutes
	}
	fclose(f);
#endif // end AC_SEND_FM_PK

}

#ifdef _ACFMPK_LIST
#ifndef _NEW_MANOR_LAW
void NPC_ManorSavePKSchedule(int meindex, int toindex, int flg)
#else
void NPC_ManorSavePKSchedule(int meindex, int toindex, int flg,int setTime,struct tm tm2)
#endif
{
	int fmpks_pos, manorid, dueltime;
	char msg[1024], n1[256], n2[256], n3[256];
	char token[256], hadfmname[256];
	int hadfmindex=0, hadfmpopular=0, tkfmindex=0;
	int PkFlg=0;

	if( toindex == -1 ){
#ifndef _NEW_MANOR_LAW
		dueltime = CHAR_getWorkInt(meindex, NPC_WORK_PEACEWAIT) + NowTime.tv_sec;
		sprintf(msg, "%d|%d|%s|%d|%s|%d|%d|%d|%s",
			dueltime, 0, "", 0, "", 0, 0, flg, "");
#else
		sprintf(msg, "%d|%d|%s|%d|%s|%d|%d|%d|%s",
			setTime, 0, "", 0, "", 0, 0, flg, "");
#endif
		PkFlg = 0;
	}else{
		struct tm tm1;
		int index;
#ifndef _NEW_MANOR_LAW
		tkfmindex = CHAR_getWorkInt(toindex, CHAR_WORKFMINDEXI);
#else
		// toindex 變成 家族的index
		tkfmindex = toindex;
#endif
		fmpks_pos = CHAR_getWorkInt(meindex, NPC_WORK_ID) * MAX_SCHEDULE;
		manorid = CHAR_getWorkInt(meindex, NPC_WORK_MANORID);
		memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
#ifndef _NEW_MANOR_LAW
		dueltime = CHAR_getWorkInt(meindex, NPC_WORK_CHALLENGEWAIT) + NowTime.tv_sec - tm1.tm_min*60 + 1800;
#else
		// 以挑戰時的時間為開打的時間,中間所經過的時間為準備時間
		dueltime = NowTime.tv_sec - tm1.tm_min * 60 + ((24 - tm1.tm_hour) * 3600) + 3600 * tm2.tm_hour;
		//				|-------------取整點-------------|  |--------加到0000時-------|  |挑戰時的時間(算整點)| 
#endif
		getStringFromIndexWithDelim( fmpointlist.pointlistarray[manorid-1], "|", 5, token, sizeof(token));
		hadfmindex = atoi( token);
		getStringFromIndexWithDelim( fmpointlist.pointlistarray[manorid-1], "|", 7, token, sizeof(token));
		hadfmpopular = atoi( token);
		getStringFromIndexWithDelim( fmpointlist.pointlistarray[manorid-1], "|", 6, token, sizeof(token));
		strcpy( hadfmname, token);
		makeEscapeString( hadfmname, n1, sizeof(n1));
#ifndef _NEW_MANOR_LAW
		makeEscapeString( CHAR_getChar(toindex, CHAR_FMNAME), n2, sizeof(n2));		
#else
		// 找出挑戰家族中氣勢最高家族索引
		index = ManorSchedule[manorid-1].iSort[0];
		makeEscapeString(ManorSchedule[manorid-1].szFmName[index],n2,sizeof(n2));
#endif
		makeEscapeString( getGameserverID(), n3, sizeof(n3));

		sprintf(msg, "%d|%d|%s|%d|%s|%d|%d|%d|%s",
			dueltime, hadfmindex-1, n1, tkfmindex, n2, 15, 50, FMPKS_FLAG_MANOR_OTHERPLANET, n3);
		PkFlg = 1;
		{
			char buf1[256];
			sprintf( buf1,"%d", CHAR_getWorkInt( meindex, NPC_WORK_ID) );
			Logfmpk(
				 n1, hadfmindex-1, 0,
				 n2, tkfmindex, 0,
				 "", buf1, n3, 1);
		}
	}
#ifndef _NEW_MANOR_LAW
	saacproto_ACSendFmPk_send( acfd, toindex, PkFlg, CHAR_getWorkInt( meindex, NPC_WORK_ID), msg);
#else
	// toindex 用不到設為 -1
	saacproto_ACSendFmPk_send( acfd, -1, PkFlg, CHAR_getWorkInt( meindex, NPC_WORK_ID), msg);
#endif
}

#endif

#ifdef _NEW_MANOR_LAW
// 加入挑戰排程
void NPC_ManorAddToSchedule(int meindex,int charaindex)
{
	int i,j,iEmpty = -1,iFmIndex,manorindex = CHAR_getWorkInt(meindex,NPC_WORK_MANORID) - 1;
	struct tm tm1;

	iFmIndex = CHAR_getWorkInt(charaindex,CHAR_WORKFMINDEXI);
	// 檢查是否已排入排程,檢查所有的莊園約戰
	for(j=0;j<MANORNUM;j++){
		for(i=0;i<10;i++){
			if(ManorSchedule[j].iFmIndex[i] != -1){
				if(ManorSchedule[j].iFmIndex[i] == iFmIndex){
					lssproto_WN_send(getfdFromCharaIndex(charaindex),WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
						CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),"你已經報名過挑戰莊園了喔∼");
					return;
				}
			}
			else{
				if(j == manorindex) iEmpty = i;	// 記錄空的位置
			}
		}
	}
	// 如果程式執行到這裡,表示沒有加入排程,檢查有沒有空的位置
	if(iEmpty != -1){ // 有空位置,把報名者加入
		// 記錄家族索引
		ManorSchedule[manorindex].iFmIndex[iEmpty] = iFmIndex;
		// 記錄家族名稱
		sprintf(ManorSchedule[manorindex].szFmName[iEmpty],"%s",CHAR_getChar(charaindex,CHAR_FMNAME));
		// 記錄挑戰時間
		memcpy(&tm1,localtime((time_t *)&NowTime.tv_sec),sizeof(tm1));
		memset(ManorSchedule[manorindex].szMemo[iEmpty],0,sizeof(ManorSchedule[manorindex].szMemo[iEmpty]));
		memcpy(&ManorSchedule[manorindex].tm1[iEmpty],&tm1,sizeof(tm1));
		lssproto_WN_send(getfdFromCharaIndex(charaindex),WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
				CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),"加入挑戰排程");
	}
	// 滿了,和挑戰排程的最後一名作比較,如果後來的氣勢比最後一名高,替換掉最後一名,否則通知目前氣勢無法擠進排程
	else{
		// 取得最後一名在ManorSchedule的位置
		int index = ManorSchedule[manorindex].iSort[9];

		for(i=0;i<FAMILY_MAXNUM;i++){
			// 找出挑戰家族的索引
			if(fmdptop.fmtopid[i] == iFmIndex) break;
		}
		// 找不到此家族
		if(i >= FAMILY_MAXNUM){
			lssproto_WN_send(getfdFromCharaIndex(charaindex),WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
				CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),"抱歉，找不到你的家族氣勢資料");
			return;
		}
		// 比較氣勢
		if(ManorSchedule[manorindex].iFmMomentum[index] < fmdptop.fmMomentum[i]){
			int fmpks_pos,iPlayerNum = CHAR_getPlayerMaxNum();
			char szMsg[256];

			// 通知所有人
			fmpks_pos = CHAR_getWorkInt(meindex,NPC_WORK_ID) * MAX_SCHEDULE;
			sprintf(szMsg,"%s 家族被 %s 家族擠下挑戰排程",ManorSchedule[manorindex].szFmName[index],CHAR_getChar(charaindex,CHAR_FMNAME));
			for(i=0;i<iPlayerNum;i++){
				if(CHAR_getCharUse(i) != FALSE){
					CHAR_talkToCli(i,-1,szMsg,CHAR_COLORBLUE2);
				}
			}
			// 最後一名被擠掉
			ManorSchedule[manorindex].iFmIndex[index] = iFmIndex;
			// 記錄家族名稱
			sprintf(ManorSchedule[manorindex].szFmName[index],"%s",CHAR_getChar(charaindex,CHAR_FMNAME));
			// 記錄挑戰時間
			memcpy(&tm1,localtime((time_t *)&NowTime.tv_sec),sizeof(tm1));
			memcpy(&ManorSchedule[manorindex].tm1[index],&tm1,sizeof(tm1));
			lssproto_WN_send(getfdFromCharaIndex(charaindex),WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
				CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),"加入挑戰排程");
		}
		else{
			lssproto_WN_send(getfdFromCharaIndex(charaindex),WINDOW_MESSAGETYPE_MESSAGE,WINDOW_BUTTONTYPE_OK,CHAR_WINDOWTYPE_CHECKMAN_END,
				CHAR_getWorkInt(meindex, CHAR_WORKOBJINDEX),"抱歉，你的家族氣勢不足以排進挑戰排程");
			return;
		}
	}
	// 排序
	SortManorSchedule();
}

// 排序挑戰排程
void SortManorSchedule()
{
	int i,j,k;

	// 先取得最新家族氣勢
	for(k=0;k<MANORNUM;k++){
		for(i=0;i<10;i++){
			ManorSchedule[k].iSort[i] = i;
			if(ManorSchedule[k].iFmIndex[i] != -1){
				for(j=0;j<FAMILY_MAXNUM;j++){
					// 找出挑戰家族的索引
					if(fmdptop.fmtopid[j] == ManorSchedule[k].iFmIndex[i]){
						// 取得氣勢值
						ManorSchedule[k].iFmMomentum[i] = fmdptop.fmMomentum[j];
						// 作成memo
						sprintf(ManorSchedule[k].szMemo[i],"%-16s %2d:%2d %12d",
							ManorSchedule[k].szFmName[i],ManorSchedule[k].tm1[i].tm_hour,ManorSchedule[k].tm1[i].tm_min,
							ManorSchedule[k].iFmMomentum[i]/100);
						break;
					}
				}
			}
		}
	}
	// 排序
	for(i=0;i<MANORNUM;i++){
		g_iSortManor = i;
		qsort(&ManorSchedule[g_iSortManor].iSort,10,sizeof(int),&SortManorScheduleMomentum);
	}
}

int SortManorScheduleMomentum(const void *indexa, const void *indexb)
{
  int momentumA,momentumB;
  
  momentumA = ManorSchedule[g_iSortManor].iFmMomentum[*(int*)indexa];
  momentumB = ManorSchedule[g_iSortManor].iFmMomentum[*(int*)indexb];
  return (momentumA > momentumB) ? -1:1;
}
#endif
