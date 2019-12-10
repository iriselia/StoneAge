#include "version.h"
#include <time.h>
#include "char.h"
#include "object.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npcutil.h"
#include "handletime.h"
#include "npc_fmdengon.h"
#include "family.h"

#define DENGONFILELINENUM      35     // 整個公佈欄的資料筆數
#define FMSDENGONFILELINENUM   140    // 家族間的留言板資料筆數
#define DENGONFILEENTRYSIZE    128    // 本文大小
#ifdef _NEW_MANOR_LAW
#define MESSAGEINONEWINDOW     9      // 每頁所顯示的筆數
#else
#define MESSAGEINONEWINDOW     7      // 每頁所顯示的筆數
#endif
#define FMMAXNUM               1000   // 家族數量的最大值
#define FM_MEMBERLIST          2      // 家族成員列表    (主功能表的按鍵)
#define FM_MEMBERMEMO          3      // 家族留言        (主功能表的按鍵)
#ifdef _UN_FMMEMO
#define FM_FMPOINT             4      // 申請家族據點    (主功能表的按鍵)
#define FM_FMDPTOP             5      // 家族間強者表    (主功能表的按鍵)
#else
#define FM_FMMEMO              4      // 家族之間留言板  (主功能表的按鍵)
#ifdef _UN_FMPOINT
#define FM_FMDPTOP			   5      // 家族間強者表    (主功能表的按鍵)
#else
#define FM_FMPOINT             5      // 申請家族據點    (主功能表的按鍵)
#define FM_FMDPTOP             6      // 家族間強者表    (主功能表的按鍵)
#endif//_UN_FMPOINT
#endif//_UN_FMMEMO
#define FM_WAITTIME            (3*60)
#define FMSDENGON_SN           10000  // 家族之間的留言板的識別碼

extern struct  FMMEMBER_LIST memberlist[FMMAXNUM];         // 接收 AC 成員列表資料的 ARRAY 
extern struct  FMS_MEMO fmsmemo;                           // 家族之間的留言板
extern struct  FM_POINTLIST fmpointlist;                   // 家族據點
extern struct  FMS_DPTOP fmdptop;                          // 家族強者表
extern int leaderdengonindex;                              // 777 家族公佈欄 index
char NPC_sendbuf[DENGONFILEENTRYSIZE*MESSAGEINONEWINDOW];  // 一整頁的大小
char enlistbuf[4096];                                        // 家族成員召募 BUF(顯示用的)

unsigned long READTIME1 = 0,
              READTIME2 = 0,
              READTIME3 = 0,
              READTIME4 = 0;

// 公佈欄的初始化(when gmsv start)
BOOL NPC_FmDengonInit( int meindex)
{
    int i;
    
    CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TYPEDENGON);
    
    if( CHAR_getInt(meindex, CHAR_FLOOR) == 777 ){
        leaderdengonindex = meindex;
    }
    
    if( READTIME1 == 0 || READTIME2 == 0 || READTIME3 == 0 || READTIME4 == 0 ){
        READTIME1 = NowTime.tv_sec+FM_WAITTIME,
        READTIME2 = NowTime.tv_sec+FM_WAITTIME,
        READTIME3 = NowTime.tv_sec+FM_WAITTIME,
        READTIME4 = NowTime.tv_sec+FM_WAITTIME;
       
        // 取得家族的成員列表(memberlist struct)，以及家族的留言板
        for( i=0; i<FMMAXNUM; i++){
            saacproto_ACShowMemberList_send( acfd, i);
            saacproto_ACFMReadMemo_send( acfd, i);
        }
        // 家族之間的留言板所傳的值預設為 FMSDENGON_SN
        saacproto_ACFMReadMemo_send( acfd, FMSDENGON_SN);
        saacproto_ACFMPointList_send(acfd);
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_INTEGRATE);
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_ADV);    
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_FEED);
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_SYNTHESIZE);
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_DEALFOOD);
        saacproto_ACShowTopFMList_send(acfd, FM_TOP_PK);
#ifdef _NEW_MANOR_LAW
				saacproto_ACShowTopFMList_send(acfd, FM_TOP_MOMENTUM);
#endif
    }
    return TRUE;
}

// Select Event
void NPC_FmDengonWindowTalked( int index, int talker, int seqno, int select, char *data)
{
//    char buf[DENGONFILEENTRYSIZE*MESSAGEINONEWINDOW*2];
    char buf[4096];
    int  buttonevent;
    int  buttontype = 0;
    struct timeval recvtime;
    
    if (!CHAR_CHECKINDEX(talker)) return;
    
    CONNECT_getLastrecvtime_D( getfdFromCharaIndex( talker), &recvtime);
    if( time_diff( NowTime, recvtime) < 0.5 ){
        return;
    }
    
    CONNECT_setLastrecvtime_D( getfdFromCharaIndex( talker), &NowTime);
#ifndef _FM_MODIFY
    // 距離超出 DENGONDISTANCE 的範圍內時，即取消動作
#define DENGONDISTANCE 3	
    if( CHAR_getInt(index, CHAR_FLOOR) != 777 )
        if(NPC_Util_CharDistance( index, talker) > DENGONDISTANCE) return;
#endif
    
    // 家族留言板
    if( seqno == CHAR_WINDOWTYPE_FM_DENGON)
    {
			int dengonindex;
			int fmindex_wk;
			char tmp_buffer[4096],tmp[4096];
			
			getStringFromIndexWithDelim(data,"|",1,tmp_buffer,sizeof(tmp_buffer));
			dengonindex = atoi(tmp_buffer);
			
			fmindex_wk = CHAR_getWorkInt( talker, CHAR_WORKFMINDEXI);
			
			if( fmindex_wk < 0 || fmindex_wk >= FMMAXNUM) return;
			
			switch( select){
			case WINDOW_BUTTONTYPE_NEXT:
			case WINDOW_BUTTONTYPE_PREV:
				{
					int fd,i;
					fd = getfdFromCharaIndex( talker);
					if( fd == -1) return;
					
					dengonindex += 7 * (( select == WINDOW_BUTTONTYPE_NEXT) ? 1 : -1);
					if( dengonindex > memberlist[fmindex_wk].memoindex && memberlist[fmindex_wk].memonum < DENGONFILELINENUM)
						dengonindex = memberlist[fmindex_wk].memoindex;
					else if( dengonindex < 6 && memberlist[fmindex_wk].memonum < DENGONFILELINENUM) 
						dengonindex = 6;
					else if( dengonindex < 1 && memberlist[fmindex_wk].memonum >= DENGONFILELINENUM)
						dengonindex = memberlist[fmindex_wk].memonum+dengonindex;
					else if( dengonindex > memberlist[fmindex_wk].memonum && memberlist[fmindex_wk].memonum >= DENGONFILELINENUM)
						dengonindex -= memberlist[fmindex_wk].memonum;
					
					buttontype = WINDOW_BUTTONTYPE_OKCANCEL;
					if( dengonindex==memberlist[fmindex_wk].memoindex && memberlist[fmindex_wk].memonum >= DENGONFILELINENUM) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;
					else if( (dengonindex - 7)<=memberlist[fmindex_wk].memoindex && (dengonindex - 7)>=(memberlist[fmindex_wk].memoindex - 7) &&
						memberlist[fmindex_wk].memonum >= DENGONFILELINENUM) 
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else if( dengonindex==memberlist[fmindex_wk].memoindex) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;    
					else if( dengonindex == 6) buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else{
						buttontype |= WINDOW_BUTTONTYPE_PREV;
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					}
					if(dengonindex >= 6){
						strcpy( NPC_sendbuf, memberlist[fmindex_wk].memo[dengonindex - 6]);
						strcat( NPC_sendbuf, "\n");
						for( i=(dengonindex - 5); i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					if(dengonindex < 6){
						strcpy( NPC_sendbuf, memberlist[fmindex_wk].memo[memberlist[fmindex_wk].memonum+(dengonindex - 6)]);
						strcat( NPC_sendbuf, "\n");
						for( i=memberlist[fmindex_wk].memonum+(dengonindex - 5); i<memberlist[fmindex_wk].memonum; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						for( i=0; i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DENGON,
						buttontype,
						CHAR_WINDOWTYPE_FM_DENGON,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
				}
				break;
			case WINDOW_BUTTONTYPE_OK:
				{
					int    fd,i;
					struct tm tm1;
					char   m_buf[4096];
					
					fd = getfdFromCharaIndex( talker);
					if( fd == -1 ) return;
					if( strlen(data) == 0 ) break;
					
					memberlist[fmindex_wk].memonum++;
					if( memberlist[fmindex_wk].memonum > DENGONFILELINENUM) 
						memberlist[fmindex_wk].memonum = DENGONFILELINENUM;
					
					memberlist[fmindex_wk].memoindex++;
					if( memberlist[fmindex_wk].memoindex >= DENGONFILELINENUM) 
						memberlist[fmindex_wk].memoindex=0;
					
					dengonindex = memberlist[fmindex_wk].memoindex;
					
					if( dengonindex < 6 && memberlist[fmindex_wk].memonum<DENGONFILELINENUM)
						dengonindex = 6;
					
					memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof(struct tm));
					getStringFromIndexWithDelim(data,"|",2,tmp_buffer,sizeof(tmp_buffer));
					sprintf( m_buf,"%s|%2d/%02d %2d:%02d %s",
						tmp_buffer,
						tm1.tm_mon +1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min,
						CHAR_getChar( talker, CHAR_NAME));
					strcpy(memberlist[fmindex_wk].memo[memberlist[fmindex_wk].memoindex], m_buf);
					
					// send acsv 
					saacproto_ACFMWriteMemo_send( acfd, CHAR_getChar( talker, CHAR_FMNAME), 
						CHAR_getInt(talker, CHAR_FMINDEX),
						makeEscapeString( memberlist[fmindex_wk].memo[memberlist[fmindex_wk].memoindex], buf, sizeof(buf)),
						fmindex_wk);
					
					if( dengonindex >= 6){
						strcpy( NPC_sendbuf, memberlist[fmindex_wk].memo[dengonindex - 6]);
						strcat( NPC_sendbuf, "\n");
						for( i=(dengonindex-5); i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					if( dengonindex < 6){
						strcpy( NPC_sendbuf, memberlist[fmindex_wk].memo[memberlist[fmindex_wk].memonum+(dengonindex - 6)]);
						strcat( NPC_sendbuf, "\n");
						for( i=memberlist[fmindex_wk].memonum+(dengonindex - 5); i<memberlist[fmindex_wk].memonum; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						for( i=0; i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DENGON,
						WINDOW_BUTTONTYPE_OKCANCEL|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DENGON,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
				}
				break;
			default:
				break;
      }  // Switch End
    }  // If End
        
    // 家族之間留言板
    else if(seqno == CHAR_WINDOWTYPE_FM_FMSDENGON)
    {
			int dengonindex;
			char tmp_buffer[4096],tmp[4096];
			getStringFromIndexWithDelim(data,"|",1,tmp_buffer,sizeof(tmp_buffer));
			dengonindex = atoi(tmp_buffer);
			
			switch( select ){
			case WINDOW_BUTTONTYPE_NEXT:
			case WINDOW_BUTTONTYPE_PREV:
				{
					int fd,i;
					fd = getfdFromCharaIndex( talker);
					if( fd == -1 ) return;
					
					dengonindex += 7 * (( select == WINDOW_BUTTONTYPE_NEXT) ? 1 : -1);
					if( dengonindex > fmsmemo.memoindex && fmsmemo.memonum < FMSDENGONFILELINENUM)
						dengonindex = fmsmemo.memoindex;
					else if( dengonindex < 6 && fmsmemo.memonum < FMSDENGONFILELINENUM) 
						dengonindex = 6;
					else if( dengonindex < 1 && fmsmemo.memonum >= FMSDENGONFILELINENUM)
						dengonindex = fmsmemo.memonum+dengonindex;
					else if( dengonindex > fmsmemo.memonum && fmsmemo.memonum >= FMSDENGONFILELINENUM)
						dengonindex -= fmsmemo.memonum;
					
					buttontype = WINDOW_BUTTONTYPE_OKCANCEL;
					if( dengonindex==fmsmemo.memoindex && fmsmemo.memonum >= FMSDENGONFILELINENUM) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;
					else if( (dengonindex-7)<=fmsmemo.memoindex && (dengonindex - 7)>=(fmsmemo.memoindex - 7) &&
						fmsmemo.memonum >= FMSDENGONFILELINENUM) 
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else if( dengonindex == fmsmemo.memoindex) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;    
					else if( dengonindex == 6 ) 
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else{
						buttontype |= WINDOW_BUTTONTYPE_PREV;
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					}
					
					if( dengonindex >= 6 ){
						if( (dengonindex - 6)  >= 140 ) return;
						strcpy( NPC_sendbuf, fmsmemo.memo[dengonindex - 6]);
						strcat( NPC_sendbuf, "\n");
						for( i=(dengonindex - 5); i<=dengonindex; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					if(dengonindex < 6){
						if( (fmsmemo.memonum+(dengonindex - 6))  >= 140 ||
							(fmsmemo.memonum+(dengonindex - 6))  < 0 ) return;
						
						strcpy( NPC_sendbuf, fmsmemo.memo[fmsmemo.memonum+(dengonindex - 6)]);
						
						strcat( NPC_sendbuf, "\n");
						for( i=fmsmemo.memonum+(dengonindex - 5); i<fmsmemo.memonum; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						for( i=0; i<=dengonindex; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_FMSDENGON,
						buttontype,
						CHAR_WINDOWTYPE_FM_FMSDENGON,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
				}
				break;
			case WINDOW_BUTTONTYPE_OK:
				{
					int    fd,i;
					struct tm tm1;
					char   m_buf[4096];
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
#ifdef _FMVER21  
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) != FMMEMBER_LEADER){              
#else
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) != 1){              
#endif              
							sprintf( NPC_sendbuf, "              『警       告』\n 抱歉！你不是族長，所以僅能查看。");
							lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
								WINDOW_BUTTONTYPE_OK,
								-1,
								-1,
								makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
							return;
						}
						
						if( strlen( data) == 0 )  break;
						
						fmsmemo.memonum++;
						if( fmsmemo.memonum > FMSDENGONFILELINENUM) 
							fmsmemo.memonum = FMSDENGONFILELINENUM;
            
						fmsmemo.memoindex++;
						if( fmsmemo.memoindex >= FMSDENGONFILELINENUM) 
							fmsmemo.memoindex = 0;
						
						dengonindex = fmsmemo.memoindex;
						if( dengonindex < 6 && fmsmemo.memonum<FMSDENGONFILELINENUM)
							dengonindex = 6;
						
						memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
						getStringFromIndexWithDelim(data,"|",2,tmp_buffer,sizeof(tmp_buffer));
						sprintf( m_buf,"%s|%2d/%02d %2d:%02d %s",
							tmp_buffer,
							tm1.tm_mon +1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min,
							CHAR_getChar( talker, CHAR_NAME));
						strcpy( fmsmemo.memo[fmsmemo.memoindex], m_buf);
						
						// send acsv 
						saacproto_ACFMWriteMemo_send( acfd, "FMS", 
							FMSDENGON_SN,
							makeEscapeString( fmsmemo.memo[fmsmemo.memoindex], buf, sizeof(buf)),
							FMSDENGON_SN);
						
						if( dengonindex >= 6){
							strcpy( NPC_sendbuf, fmsmemo.memo[dengonindex - 6]);
							strcat( NPC_sendbuf, "\n");
							for( i=(dengonindex - 5); i<=dengonindex; i++){
								strcat( NPC_sendbuf, fmsmemo.memo[i]);
								strcat( NPC_sendbuf, "\n");
							}
							sprintf(tmp, "%d\n", dengonindex);
							strcat( NPC_sendbuf, tmp);
						}
						if(dengonindex < 6){
							strcpy( NPC_sendbuf, fmsmemo.memo[fmsmemo.memonum+(dengonindex - 6)]);
							strcat( NPC_sendbuf, "\n");
							for( i=fmsmemo.memonum+(dengonindex - 5); i<fmsmemo.memonum; i++){
								strcat( NPC_sendbuf, fmsmemo.memo[i]);
								strcat( NPC_sendbuf, "\n");
							}
							for( i=0; i<=dengonindex; i++){
								strcat( NPC_sendbuf, fmsmemo.memo[i]);
								strcat( NPC_sendbuf, "\n");
							}
							sprintf(tmp, "%d\n", dengonindex);
							strcat( NPC_sendbuf, tmp);
						}
						lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_FMSDENGON,
							WINDOW_BUTTONTYPE_OKCANCEL|
							WINDOW_BUTTONTYPE_PREV,
							CHAR_WINDOWTYPE_FM_FMSDENGON,
#ifndef _FM_MODIFY
							CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
							-1,
#endif
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
					}
					break;
				default: break;
      }  // Switch End
    }  // If End
        
    // 說明視窗(家族據點)
    else if( seqno == CHAR_WINDOWTYPE_FM_MESSAGE1)
    {
			int fd,i;
			char pointbuf[4096];
			
			fd = getfdFromCharaIndex( talker );
			if( fd == -1 )  return;
			
			switch( select ){
			case WINDOW_BUTTONTYPE_OK:
				{
					strcpy( pointbuf, fmpointlist.pointlistarray[0]);
					strcat( pointbuf, "\n");
					for( i=1; i<=FMPOINTNUM; i++){
						strcat( pointbuf, fmpointlist.pointlistarray[i]);
						strcat( pointbuf, "\n");
					}
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_POINTLIST,
						WINDOW_BUTTONTYPE_OK,
						CHAR_WINDOWTYPE_FM_POINTLIST,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( pointbuf, buf, sizeof(buf)));
				}
				break;
			default:
				break;
			}
    }        

    // 說明視窗(成員列表)
    else if( seqno == CHAR_WINDOWTYPE_FM_MESSAGE2)
    {
			int fd,i;
			char numberlistbuf[4096];
			int fmindex_wk;
			fmindex_wk = CHAR_getWorkInt( talker, CHAR_WORKFMINDEXI);
			
			if( fmindex_wk < 0 || fmindex_wk >= FMMAXNUM) return;
			
			fd = getfdFromCharaIndex( talker );
			if( fd == -1 )  return;
			
			switch( select ){
			case WINDOW_BUTTONTYPE_OK:
				{
					strcpy( numberlistbuf, memberlist[fmindex_wk].numberlistarray[0]);
					strcat( numberlistbuf, "\n");
					for( i=1; i<10; i++){
						strcat( numberlistbuf, memberlist[fmindex_wk].numberlistarray[i]);
						strcat( numberlistbuf, "\n");
					}
					// 增加程式碼(向AC要召募人員的值)
					sprintf(enlistbuf, "是否繼續召募家族人員|0|%d",memberlist[fmindex_wk].accept);
					strcat( numberlistbuf, enlistbuf);
					strcat( numberlistbuf, "\n");
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_SELECT,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_NEXT,
						CHAR_WINDOWTYPE_FM_MEMBERLIST,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( numberlistbuf, buf, sizeof(buf)));
				}
				break;
			default:
				break;
			}
    }        

    // 強者表的選項視窗
    else if( seqno == CHAR_WINDOWTYPE_FM_DPSELECT)
    {
			int fmindex_wk;
			fmindex_wk = CHAR_getWorkInt( talker, CHAR_WORKFMINDEXI);
			
			if( CHAR_getInt(talker, CHAR_FMINDEX) > 0 ){
				if( fmindex_wk < 0 || fmindex_wk >= FMMAXNUM){
					print("FamilyNumber Data Error!!");
					return;
				}
			}
			
			buttonevent = atoi(data);
			switch( buttonevent ){
			case 1:				// 前三十大家族綜合聲望列表
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.topmemo[i]);
						strcat( listbuf, "\n");
					}
					strcat( listbuf, "0\n");
#ifdef _FMVER21              
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_TOP30DP,
#else
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
#endif              
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_NEXT,
						CHAR_WINDOWTYPE_FM_DPTOP,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
			case 2:				// 前十大家族冒險列表
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.adv_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.adv_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
			case 3:				// 前十大家族伺育列表
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.feed_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.feed_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#ifndef _NEW_MANOR_LAW
			case 4:				// 前十大家族合成列表
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.syn_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.syn_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
			case 5:				// 前十大家族料理列表
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.food_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.food_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#endif
#ifdef _NEW_MANOR_LAW
			case 4:				// 前十大家族ＰＫ列表
#else
			case 6:				// 前十大家族ＰＫ列表
#endif
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.pk_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.pk_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#ifdef _NEW_MANOR_LAW
			case 5:						// 十大氣勢家族
				{
					int  fd,i;
					char listbuf[4096];
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( listbuf, fmdptop.momentum_topmemo[0]);
					strcat( listbuf, "\n");
					for( i=1; i<10; i++){
						strcat( listbuf, fmdptop.momentum_topmemo[i]);
						strcat( listbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_10_MEMONTUM,
						WINDOW_BUTTONTYPE_OK|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#endif
#ifndef _NEW_MANOR_LAW
			case 7:				// 自己家族聲望排行榜
#else
			case 6:
#endif
				{
					int  fd,i,h,k,fmid;
					char listbuf[4096];
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					fmid = CHAR_getWorkInt(talker, CHAR_WORKFMINDEXI);
					if( fmid < 0 ){
						sprintf( NPC_sendbuf, "              『警       告』\n 抱歉！你不是家族人員，無法查看。");
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE, WINDOW_BUTTONTYPE_OK,
							-1,
							-1,
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
						return;
					}
					
					for( h=0; h<FMMAXNUM; h++)
						if( fmdptop.fmtopid[h] == fmid ) 
							break;
						
						k = h;                 // 標示顏色用(多傳一個1，以供Client之用)
						if(h <= 4) h = 0;
						else if(h >= 994 ) h = 990;
						else h -= 4;
						
						strcpy( listbuf, fmdptop.topmemo[h]);
						if( k == h ) strcat( listbuf, "|1");
						strcat( listbuf, "\n");
						for( i = h + 1; i < h + 10; i++){
							strcat( listbuf, fmdptop.topmemo[i]);
							if(i == k) strcat( listbuf, "|1");
							strcat( listbuf, "\n");
						}
#ifdef _FMVER21              
						lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_TOP30DP,
#else
            lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
#endif              
							WINDOW_BUTTONTYPE_OK|
							WINDOW_BUTTONTYPE_PREV,
							CHAR_WINDOWTYPE_FM_DPME,
#ifndef _FM_MODIFY
							CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
							-1,
#endif
							makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#ifdef _NEW_MANOR_LAW
			case 7:		// 自己家族氣勢排名
				{
					int  fd,h,fmid;
					char listbuf[4096];
					char szTempbuf[12];
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					fmid = CHAR_getWorkInt(talker, CHAR_WORKFMINDEXI);
					if( fmid < 0 ){
						sprintf( NPC_sendbuf, "              『警       告』\n 抱歉！你不是家族人員，無法查看。");
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE, WINDOW_BUTTONTYPE_OK,
							-1,
							-1,
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
						return;
					}
					
					for( h=0; h<FMMAXNUM; h++)
						if( fmdptop.momentum_topid[h] == fmid ) 
							break;
						
						strcpy( listbuf, fmdptop.momentum_topmemo[h]);
						sprintf(szTempbuf,"|%d",CHAR_getInt(talker,CHAR_MOMENTUM)/100);
						strcat(listbuf,szTempbuf);
						lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_FM_MEMONTUM,
							WINDOW_BUTTONTYPE_OK|
							WINDOW_BUTTONTYPE_PREV,
							CHAR_WINDOWTYPE_FM_DPME,
							-1,
							makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
#endif
			default:
				break;
        }
    }
        
    // 選項視窗
    else if( seqno == CHAR_WINDOWTYPE_FM_SELECT)
    {
			int fmindex_wk;
			fmindex_wk = CHAR_getWorkInt( talker, CHAR_WORKFMINDEXI);
			
			if( CHAR_getInt(talker, CHAR_FMINDEX) > 0 ){
				if( fmindex_wk < 0 || fmindex_wk >= FMMAXNUM){
					print("FamilyNumber Data Error!!");
					return;
				}
			}
			
			buttonevent = atoi(data);
			
			switch( buttonevent ){
			case FM_MEMBERLIST:
				{
					int fd;
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					if( CHAR_getInt(talker, CHAR_FMINDEX) <= 0){  
						sprintf( NPC_sendbuf, "              『警       告』\n 抱歉！你不是家族人員，不得使用公佈欄。");
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE, WINDOW_BUTTONTYPE_OK,
							-1, -1, makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
						return;
					}
					/*
					#ifdef _FMVER21                   
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER ||
					CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_ELDER ){
					#else
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) == 1){
					#endif              
					saacproto_ACShowMemberList_send( acfd, fmindex_wk);
					READTIME1 = NowTime.tv_sec+FM_WAITTIME;
					}else
					*/
					if( NowTime.tv_sec > READTIME1 ){
						saacproto_ACShowMemberList_send( acfd, fmindex_wk);
						READTIME1 = NowTime.tv_sec+FM_WAITTIME;
					}
					
#ifdef _FMVER21                   
					//              if( CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER || 
					//                  CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_ELDER ||
					//                  CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_VICELEADER ){
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER || 
						CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_ELDER ){
#else
						if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) == 1 ){
#endif              
							sprintf( NPC_sendbuf, "               『族 長 需 知』\n請小心處理族員的資料，一經修改後就無法回復原態，敬請小心。");
						}else{
							sprintf( NPC_sendbuf, "               『列 表 需 知』\n 此表族長可作修改，族員僅能查看。");
						}
						
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
							WINDOW_BUTTONTYPE_OK,
							CHAR_WINDOWTYPE_FM_MESSAGE2,
#ifndef _FM_MODIFY
							CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
							-1,
#endif
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
          }
          break;
#ifdef _UN_FMPOINT
#else
			case FM_FMPOINT:
				{
					int fd;
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
#ifdef _FMVER21                   
					if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) == FMMEMBER_LEADER ){
#else
						if( CHAR_getInt( talker, CHAR_FMLEADERFLAG) == 1 ){
#endif              
							saacproto_ACFMPointList_send(acfd);
							sprintf( NPC_sendbuf, "               『族 長 需 知』\n請小心慎選所申請的據點，一但選取據點後就無法回復原態，敬請小心。");
							READTIME4 = NowTime.tv_sec+FM_WAITTIME;
						}
						else{
							sprintf( NPC_sendbuf, "               『列 表 需 知』\n此表族長可以申請，其餘僅能查看。");
						}
						
						if( NowTime.tv_sec > READTIME4 ){
							saacproto_ACFMPointList_send(acfd);
							READTIME4 = NowTime.tv_sec+FM_WAITTIME;
						}
						
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
							WINDOW_BUTTONTYPE_OK,
							CHAR_WINDOWTYPE_FM_MESSAGE1,
#ifndef _FM_MODIFY
							CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
							-1,
#endif
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
          }
          break;
#endif //_UN_FMPOINT
			case FM_FMDPTOP:
				{
					int  fd;
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					if( NowTime.tv_sec > READTIME3 ){
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_INTEGRATE );
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_ADV );    
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_FEED );
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_SYNTHESIZE );
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_DEALFOOD );
						saacproto_ACShowTopFMList_send( acfd, FM_TOP_PK );                           
#ifdef _NEW_MANOR_LAW
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_MOMENTUM);
#endif
						READTIME3 = NowTime.tv_sec+FM_WAITTIME;
					}
					memset(NPC_sendbuf,0,sizeof(NPC_sendbuf));
					strcpy( NPC_sendbuf, "\n              三十大家族聲望列表\n");
					strcat( NPC_sendbuf, "              十大冒險家族\n");
					strcat( NPC_sendbuf, "              十大飼育家族\n");
#ifndef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              十大合成家族\n");
					strcat( NPC_sendbuf, "              十大料理家族\n");
#endif
					strcat( NPC_sendbuf, "              十大戰鬥家族\n");
#ifdef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              十大氣勢家族\n");
#endif
					strcat( NPC_sendbuf, "              自己家族聲望列表\n");
#ifdef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              自己家族氣勢排名\n");
#endif					
						
					lssproto_WN_send( fd, WINDOW_MESSAGETYPE_SELECT,
						WINDOW_BUTTONTYPE_NONE,
						CHAR_WINDOWTYPE_FM_DPSELECT,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
					
				}
				break;
			case FM_MEMBERMEMO:
				{
					int fd,i,dengonindex;
					char tmp[4096];
					fd = getfdFromCharaIndex( talker );
					
					if( fd == -1 )  return;
					
					if( CHAR_getInt(talker, CHAR_FMINDEX) <= 0){  
						sprintf( NPC_sendbuf, "              『警       告』\n 抱歉！你不是家族人員，不得使用公佈欄。");
						
						lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE,
							WINDOW_BUTTONTYPE_OK,
							-1,
							-1,
							makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
						return;
					}
					
					if( NowTime.tv_sec > READTIME2 ){
						saacproto_ACFMReadMemo_send( acfd, fmindex_wk);
						READTIME2 = NowTime.tv_sec+FM_WAITTIME;
					}
					
					dengonindex = memberlist[fmindex_wk].memoindex;
					if( memberlist[fmindex_wk].memoindex < 6 && memberlist[fmindex_wk].memonum < DENGONFILELINENUM ){
						dengonindex = 6;
					}    
					
					if( dengonindex >= 6 ){
						strcpy( NPC_sendbuf, memberlist[fmindex_wk].memo[dengonindex - 6]);
						strcat( NPC_sendbuf, "\n");
						for( i=(dengonindex - 5); i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					if( dengonindex < 6 ){
						strcpy( NPC_sendbuf,
							memberlist[fmindex_wk].memo[memberlist[fmindex_wk].memonum + (dengonindex - 6)]);
						strcat( NPC_sendbuf, "\n");
						for( i=memberlist[fmindex_wk].memonum + (dengonindex - 5); i<memberlist[fmindex_wk].memonum; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						for( i=0; i<=dengonindex; i++){
							strcat( NPC_sendbuf, memberlist[fmindex_wk].memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DENGON,
						WINDOW_BUTTONTYPE_OKCANCEL|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_DENGON,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
				}
				break;
#ifdef _UN_FMMEMO
#else
			case FM_FMMEMO:
				{
					int fd,i,dengonindex;
					char tmp[4096];
					fd = getfdFromCharaIndex( talker );
					
					if( fd == -1 )  return;
					
					if( NowTime.tv_sec > READTIME3 ){
						saacproto_ACFMReadMemo_send( acfd, FMSDENGON_SN);
						READTIME3 = NowTime.tv_sec+FM_WAITTIME;
					}
					dengonindex = fmsmemo.memoindex;
					if( fmsmemo.memoindex<6 || fmsmemo.memonum>FMSDENGONFILELINENUM ){
						dengonindex = 6; 
					}
					if( dengonindex >= 6 ){
						strcpy( NPC_sendbuf, fmsmemo.memo[dengonindex - 6]);
						strcat( NPC_sendbuf, "\n");
						for( i=(dengonindex - 5); i<=dengonindex; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					if( dengonindex < 6 ){
						if( (fmsmemo.memonum + (dengonindex - 6)) < 0 || (fmsmemo.memonum + (dengonindex - 6)) >= 140 )
							return;
						strcpy( NPC_sendbuf, fmsmemo.memo[fmsmemo.memonum + (dengonindex - 6)]);
						strcat( NPC_sendbuf, "\n");
						for( i=fmsmemo.memonum + (dengonindex - 5); i<fmsmemo.memonum; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						for( i=0; i<=dengonindex; i++){
							strcat( NPC_sendbuf, fmsmemo.memo[i]);
							strcat( NPC_sendbuf, "\n");
						}
						sprintf(tmp, "%d\n", dengonindex);
						strcat( NPC_sendbuf, tmp);
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_FMSDENGON,
						WINDOW_BUTTONTYPE_OKCANCEL|
						WINDOW_BUTTONTYPE_PREV,
						CHAR_WINDOWTYPE_FM_FMSDENGON,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));
				}   
#endif//_UN_FMMEMO
				break;
			default:
				break;
        }
    }
        
    // 成員列表
    else if( seqno == CHAR_WINDOWTYPE_FM_MEMBERLIST)
    {
			char numberlistbuf[4096],tmp_buffer[4096],dutybuf[64];
			int  numberlistindex;
			int  fmindex_wk;
			fmindex_wk = CHAR_getWorkInt( talker, CHAR_WORKFMINDEXI);
			if( fmindex_wk < 0 || fmindex_wk >= FMMAXNUM) return;
			
			getStringFromIndexWithDelim(data,"|",1,tmp_buffer,sizeof(tmp_buffer));
			numberlistindex = atoi(tmp_buffer);
			getStringFromIndexWithDelim(data,"|",2,tmp_buffer,sizeof(tmp_buffer));
			buttonevent = atoi(tmp_buffer);
			getStringFromIndexWithDelim(data,"|",3,dutybuf,sizeof(dutybuf));
			
#ifdef _FMVER21
			//        if( buttonevent>=1 && buttonevent<=11 && 
			//            ( CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER || 
			//              CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_ELDER ||
			//              CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_VICELEADER ))
			if( buttonevent>=1 && buttonevent<=11 && 
				( CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER || 
				CHAR_getInt( talker, CHAR_FMLEADERFLAG ) == FMMEMBER_ELDER ))
#else
        if( buttonevent>=1 && buttonevent<=11 && CHAR_getInt( talker, CHAR_FMLEADERFLAG) == 1 )
#endif        
        {
					int fd,i;
					int int_status;
					char getstatus[4096];
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					strcpy( getstatus, memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]
						+ (strlen( memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]) - 1));
					
					int_status = atoi(getstatus);
					
					// 家族的加入、退出、申請等選項
					if( buttonevent!=11 )
#ifdef _FMVER21            
						strcpy( memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]
						+ (strlen(memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]) - 1), dutybuf);                			    
#else
					switch( int_status ){
					case 1:
					case 3:
						strcpy( memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]
							+ (strlen(memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]) - 1), "4");                      
						break;
					case 2:
						strcpy( memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]
							+ (strlen(memberlist[fmindex_wk].numberlistarray[numberlistindex+buttonevent - 1]) - 1), "1"); 					  
						break;
					default:
						break;
					}              
#endif                 
					// 家族的召募選項
					if( buttonevent == 11 )
					{
						strcpy( getstatus, enlistbuf + (strlen(enlistbuf) - 1));
						int_status = atoi(getstatus);
						
						switch( int_status ){
						case 1:
							memberlist[fmindex_wk].accept = 0;
							sprintf(enlistbuf, "是否繼續召募家族人員|%d|%d",numberlistindex,memberlist[fmindex_wk].accept);
							break;
						case 0:
							memberlist[fmindex_wk].accept = 1;
							sprintf(enlistbuf, "是否繼續召募家族人員|%d|%d",numberlistindex,memberlist[fmindex_wk].accept);
							break;
						default:
							break;    
						}
					}
					
					strcpy( numberlistbuf, memberlist[fmindex_wk].numberlistarray[numberlistindex]);
					strcat( numberlistbuf, "\n");
					for( i=(numberlistindex + 1); i<numberlistindex + 10; i++){
						strcat( numberlistbuf, memberlist[fmindex_wk].numberlistarray[i]);
						strcat( numberlistbuf, "\n");
					}
					sprintf(enlistbuf, "是否繼續召募家族人員|%d|%d",numberlistindex,memberlist[fmindex_wk].accept);
					strcat( numberlistbuf, enlistbuf);
					strcat( numberlistbuf, "\n");
					
					buttontype = WINDOW_BUTTONTYPE_OK;
					if( (numberlistindex + 10) > memberlist[fmindex_wk].fmnum) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;
					else if( numberlistindex == 0 ) 
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else{
						buttontype |= WINDOW_BUTTONTYPE_PREV;
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_SELECT,
						buttontype,
						CHAR_WINDOWTYPE_FM_MEMBERLIST,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( numberlistbuf, buf, sizeof(buf)));
        } // end if
        switch( select ){
				case WINDOW_BUTTONTYPE_NEXT:
				case WINDOW_BUTTONTYPE_PREV:
          {
						int fd,i;
						
						fd = getfdFromCharaIndex( talker );
						if( fd == -1 )  return;
						
						numberlistindex += 10 * (( select == WINDOW_BUTTONTYPE_NEXT) ? 1 : -1);
						
						if( numberlistindex >= memberlist[fmindex_wk].fmnum) 
							numberlistindex -= 10;
						else if( numberlistindex < 1 ) 
							numberlistindex = 0;
						
						buttontype = WINDOW_BUTTONTYPE_OK;
						if( (numberlistindex + 10) >= memberlist[fmindex_wk].fmnum) 
							buttontype |= WINDOW_BUTTONTYPE_PREV;
						else if( numberlistindex==0 )
							buttontype |= WINDOW_BUTTONTYPE_NEXT;
						else{
							buttontype |= WINDOW_BUTTONTYPE_PREV;
							buttontype |= WINDOW_BUTTONTYPE_NEXT;
						}
						
						strcpy( numberlistbuf, memberlist[fmindex_wk].numberlistarray[numberlistindex]);
						strcat( numberlistbuf, "\n");
						for( i=(numberlistindex+1); i<numberlistindex+10; i++){
							strcat( numberlistbuf, memberlist[fmindex_wk].numberlistarray[i]);
							strcat( numberlistbuf, "\n");
						}
						sprintf(enlistbuf, "是否繼續召募家族人員|%d|%d",numberlistindex,memberlist[fmindex_wk].accept);
						strcat( numberlistbuf, enlistbuf);
						strcat( numberlistbuf, "\n");
						lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_SELECT,
							buttontype,
							CHAR_WINDOWTYPE_FM_MEMBERLIST,
#ifndef _FM_MODIFY
							CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
							-1,
#endif
							makeEscapeString( numberlistbuf, buf, sizeof(buf)));
          }
          break;
				case WINDOW_BUTTONTYPE_OK:
          break;
				default:
          break;
        }
    }
    
    // 家族強者表(前三十)
    else if( seqno == CHAR_WINDOWTYPE_FM_DPTOP)
    {
			char listbuf[4096],tmp_buffer[4096];
			int  listindex;
			getStringFromIndexWithDelim(data,"|",1,tmp_buffer,sizeof(tmp_buffer));
			listindex = atoi(tmp_buffer);
			
			switch( select ){
			case WINDOW_BUTTONTYPE_NEXT:
			case WINDOW_BUTTONTYPE_PREV:
				{
					int fd,i;
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					listindex += 10 * (( select == WINDOW_BUTTONTYPE_NEXT) ? 1 : -1);
					
					if( listindex >= 30) 
						return;
					//listindex = 20;
					//listindex -= 10;
					//else if( listindex < 1 ) 
					//    listindex = 0;
					if (listindex < 0) return;
					
					buttontype = WINDOW_BUTTONTYPE_OK;
					if( (listindex + 10) >= 30) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;
					else if( listindex==0 )
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else{
						buttontype |= WINDOW_BUTTONTYPE_PREV;
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					}
					
					strcpy( listbuf, fmdptop.topmemo[listindex]);
					strcat( listbuf, "\n");
					for( i=(listindex+1); i<listindex+10; i++){
						strcat( listbuf, fmdptop.topmemo[i]);
						strcat( listbuf, "\n");
					}
					sprintf(tmp_buffer, "%d\n", listindex);
					strcat( listbuf, tmp_buffer);
					
#ifdef _FMVER21              
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_TOP30DP,
#else
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_DP,
#endif              
						buttontype,
						CHAR_WINDOWTYPE_FM_DPTOP,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( listbuf, buf, sizeof(buf)));
				}
				break;
			case WINDOW_BUTTONTYPE_OK:
				break;
			default:
				break;
			}
    }

    // 據點列表
    else if( seqno == CHAR_WINDOWTYPE_FM_POINTLIST)
    {
			char pointlistbuf[4096];
			int  pointlistindex;
			
			pointlistindex = 0;
			buttonevent = atoi(data);
			
			switch( select ){
			case WINDOW_BUTTONTYPE_NEXT:
			case WINDOW_BUTTONTYPE_PREV:
				{
					int fd,i;
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					pointlistindex += 10 * (( select == WINDOW_BUTTONTYPE_NEXT) ? 1 : -1);
					
					if( pointlistindex > FMPOINTNUM) 
						pointlistindex -= 10;
					else if( pointlistindex < 1 ) 
						pointlistindex = 0;
					
					buttontype = WINDOW_BUTTONTYPE_OK;
					if( (pointlistindex + 10) > FMPOINTNUM) 
						buttontype |= WINDOW_BUTTONTYPE_PREV;
					else if( pointlistindex==0 )
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					else{
						buttontype |= WINDOW_BUTTONTYPE_PREV;
						buttontype |= WINDOW_BUTTONTYPE_NEXT;
					}
					
					strcpy( pointlistbuf, fmpointlist.pointlistarray[pointlistindex]);
					strcat( pointlistbuf, "\n");
					for( i=(pointlistindex+1); i<pointlistindex+10; i++){
						strcat( pointlistbuf, fmpointlist.pointlistarray[i]);
						strcat( pointlistbuf, "\n");
					}
					
					lssproto_WN_send( fd, WINDOW_FMMESSAGETYPE_POINTLIST,
						buttontype,
						CHAR_WINDOWTYPE_FM_POINTLIST,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( pointlistbuf, buf, sizeof(buf)));
				}
				break;
			case WINDOW_BUTTONTYPE_OK:
				break;
			default:
				break;
			}
    }        

    // 家族強者表(自己及前十大)
    else if( seqno == CHAR_WINDOWTYPE_FM_DPME )
    {
			switch( select ){
			case WINDOW_BUTTONTYPE_PREV:
				{
					int  fd;
					
					fd = getfdFromCharaIndex( talker );
					if( fd == -1 )  return;
					
					if( NowTime.tv_sec > READTIME3 ){
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_INTEGRATE);
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_ADV);    
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_FEED);
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_SYNTHESIZE);
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_DEALFOOD);
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_PK);                           
#ifdef _NEW_MANOR_LAW
						saacproto_ACShowTopFMList_send(acfd, FM_TOP_MOMENTUM);
#endif
						READTIME3 = NowTime.tv_sec+FM_WAITTIME;
					}
					
					strcpy( NPC_sendbuf, "\n              三十大家族聲望列表\n");
					strcat( NPC_sendbuf, "              十大冒險家族\n");
					strcat( NPC_sendbuf, "              十大飼育家族\n");
#ifndef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              十大合成家族\n");
					strcat( NPC_sendbuf, "              十大料理家族\n");
#endif
					strcat( NPC_sendbuf, "              十大戰鬥家族\n");
#ifdef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              十大氣勢家族\n");
#endif
					strcat( NPC_sendbuf, "              自己家族聲望列表\n");
#ifdef _NEW_MANOR_LAW
					strcat( NPC_sendbuf, "              自己家族氣勢排名\n");
#endif					
					lssproto_WN_send( fd, WINDOW_MESSAGETYPE_SELECT,
						WINDOW_BUTTONTYPE_NONE,
						CHAR_WINDOWTYPE_FM_DPSELECT,
#ifndef _FM_MODIFY
						CHAR_getWorkInt( index, CHAR_WORKOBJINDEX),
#else
						-1,
#endif
						makeEscapeString( NPC_sendbuf, buf, sizeof(buf)));                
				}
				break;
			case WINDOW_BUTTONTYPE_OK:
				break;
			default:
				break;
			}
			
    }        
}

// call FmDengon NPC event
#ifndef _FM_MODIFY
void NPC_FmDengonLooked( int meindex, int lookedindex )
{
    char buf[DENGONFILEENTRYSIZE*MESSAGEINONEWINDOW*2];
    char menubuf[4096];
    int  fd;
    
    if (!CHAR_CHECKINDEX(lookedindex)) return;
    
    fd = getfdFromCharaIndex( lookedindex );
    if( fd == -1 )  return;
    
    // 必須站在佈告欄的前面一格
    if( NPC_Util_CharDistance( lookedindex, meindex ) > 1) return;
    // 空白處請勿更動
    strcpy( menubuf, "                『家族佈告欄』\n\n");
	strcat( menubuf, "                 家族成員列表\n");
	strcat( menubuf, "                   家族留言\n");
#ifdef _UN_FMMEMO
#else
	strcat( menubuf, "                家族之間留言板\n");
#endif
#ifdef _UN_FMPOINT
#else
	strcat( menubuf, "                 申請家族據點\n");
#endif
	strcat( menubuf, "                家族之間強者表");
        
    lssproto_WN_send(fd, WINDOW_MESSAGETYPE_SELECT,
        	     WINDOW_BUTTONTYPE_CANCEL,
                     CHAR_WINDOWTYPE_FM_SELECT,
 		     CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
		     makeEscapeString( menubuf, buf, sizeof(buf)));        
}
#else
void NPC_FmDengonLooked( int meindex, int lookedindex )
{
    char buf[DENGONFILEENTRYSIZE*MESSAGEINONEWINDOW*2];
    char menubuf[4096];
    int  fd;
    
    if (!CHAR_CHECKINDEX(lookedindex)) return;
    
    fd = getfdFromCharaIndex( lookedindex );
    if( fd == -1 )  return;
    
    // 空白處請勿更動
    strcpy( menubuf, "                『家族佈告欄』\n\n");
		strcat( menubuf, "                 家族成員列表\n");
		strcat( menubuf, "                   家族留言\n");
#ifdef _UN_FMMEMO
#else
		strcat( menubuf, "                家族之間留言板\n");
#endif
#ifdef _UN_FMPOINT
#else
		strcat( menubuf, "                 申請家族據點\n");
#endif
		strcat( menubuf, "                家族之間強者表");
        
    lssproto_WN_send(fd,
										 WINDOW_MESSAGETYPE_SELECT,
        						 WINDOW_BUTTONTYPE_CANCEL,
                     CHAR_WINDOWTYPE_FM_SELECT,
 										 -1,
										 makeEscapeString( menubuf, buf, sizeof(buf)));        
}
#endif
