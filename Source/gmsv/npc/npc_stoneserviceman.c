//Terry 2001/09/27
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_stoneserviceman.h"
#include "readmap.h"
#include "battle.h"
#include "log.h"
#include "enemy.h"
#include "configfile.h"

#ifdef _SERVICE
#define LOOP_TIME 100		//0.1秒(進入loop function 的時間)
#define MAX_AP_MSG 128		// 接收AP送來訊息的最大數

// socket struct
struct socketaddr_in
{
  u_short sin_family;
  u_short sin_port;
  u_long  sin_addr;
  char    sin_zero[8];
};

// 接收AP傳來的訊息
typedef struct _MSG
{
  int nAid;		// nAid => 流水號*10 + 訊息代號(_AP_CLIENT_MESSAGE)
  int nIndex;		// NPC的index值
  int nLen;		// 訊息長度
  LPList Data;		// 實際資料
  int bIsUse;		// 這個訊息是否有使用過
}Msg,*pMsg;
Msg g_APMsg[MAX_AP_MSG];
        
// 動作id        
enum _AP_CLIENT_MESSAGE
{
  Cli_CHECKPLAYER,  //要求AP對玩家是否有購買產品的確認  0
  AP_CHECKPLAYER,   //AP回應玩家是否有購買產品
  Cli_YES,          //玩家確定要將獎品拿走
  Cli_CANCEL,	    //通知AP把lock欄位清為0             
  Cli_GET,	    //通知AP給物品
  AP_GET,	    //AP回應物品			5
  Cli_CONNECT,      //測試是否有連到AP
  AP_CONNECT,       //AP回應
  AP_ERROR          //AP有錯誤				
};

// NPC的工作變數
enum
{
  NPC_WORK_FLAG1   	 = CHAR_NPCWORKINT2,	//flag1
  NPC_WORK_FLAG2   	 = CHAR_NPCWORKINT3,	//flag2
  NPC_WORK_FLAG3   	 = CHAR_NPCWORKINT4,	//flag3
  NPC_WORK_TIMEOUT 	 = CHAR_NPCWORKINT5,	//time out 時間
  NPC_WORK_STATE   	 = CHAR_NPCWORKINT6,	//目前執行狀態
  NPC_WORK_START   	 = CHAR_NPCWORKINT7,	//開始計時flag
  NPC_WORK_LEAVE_COUNT   = CHAR_NPCWORKINT8,	//玩家talk時間計時
  NPC_WORK_TOINDEX 	 = CHAR_NPCWORKINT9,	//玩家的index值
  NPC_WORK_SERIALNUM     = CHAR_NPCWORKINT10,   //流水號
  NPC_WORK_ISUSE   	 = CHAR_NPCWORKINT11,	//目前NPC是否有玩家在使用
};

// 執行的狀態
enum
{
  SERVICE_STATE_1,	// check 有沒有和AP連線中 
  SERVICE_STATE_2,	// 向AP取得玩家有無買產品
  SERVICE_STATE_3,	// 取得要給玩家的東西的資料
  SERVICE_STATE_4	// end state
};
                                                                
extern int errno;

int g_nServiceSocket;
int g_EnableService = 0;
char g_ApID[32];
struct socketaddr_in server_sin;
struct sockaddr      temp;
unsigned short g_Port;

static void NPC_StoneServiceMan_selectWindow(int meindex,int toindex,int num,int select);
extern int GetSerialNum(void);
static int flag = 1;

/*********************************
* 初始化
*********************************/
BOOL NPC_StoneServiceManInit(int meindex )
{
  char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
  
  if(NPC_Util_GetArgStr(meindex,npcarg,sizeof(npcarg)) == NULL){
    print("StoneServiceMan:GetArgStrErr");
    return FALSE;
  }

  CHAR_setInt(meindex,CHAR_LOOPINTERVAL,LOOP_TIME);
  CHAR_setInt(meindex,CHAR_WHICHTYPE,CHAR_TYPESTONESERVICEMAN);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
  CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
  CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
  CHAR_setWorkInt(meindex,NPC_WORK_START,0);  
  CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
  CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
  CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
  CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);

  g_EnableService = getEnableService();
  
  //連線到AP在init時只做一次
  if(flag && g_EnableService){
    strcpy(g_ApID,getApID());
		g_Port = getApPort();
    bzero(g_APMsg,sizeof(g_APMsg));
    ConnectToAP();//連線到AP
    flag = 0;
  }
  
  return TRUE;
}

void NPC_StoneServiceManTalked(int meindex,int talkerindex,char *szMes,int color)
{
  if(CHAR_getInt(talkerindex,CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) return;
  if(NPC_Util_isFaceToFace(talkerindex,meindex,1) == FALSE) return;
  NPC_StoneServiceMan_selectWindow(meindex,talkerindex,0,-1);
}

static void NPC_StoneServiceMan_selectWindow(int meindex,int toindex,int num,int select)
{
  char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
  char token[1024],buf[128],*pUserid;
  int buttontype = 0, windowtype = 0, windowno = 0,nAid,nLen;
  int fd = getfdFromCharaIndex(toindex);
  char Data[3][32],talk[128];
  int i,j,k,Ret;
  LPList DataList = NULL,Kill = NULL;

	
  if(fd == -1) 
  {
    print("getfd err\n");
    return;
  }
	
  if(NPC_Util_GetArgStr(meindex,npcarg,sizeof(npcarg)) == NULL)
  {
    print("GetArgStrErr");
    return ;
  }
  
  token[0] = '\0';

  //如果不使用石器服務員的功能
  if(!g_EnableService)
  {
    sprintf(token,"\n 請到此星系的最後一顆星球使用此服務，謝謝！");
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_WAIT;
    
    lssproto_WN_send(fd,windowtype,buttontype,windowno,
                     CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		     token);    
    return;
  }
  
  //如果這個玩家是第一個和NPC講話
  if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 0 && 
     CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX) == -1)
  {
    //記住第一個和NPC講話的是誰
    CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,toindex);
    //設定為NPC正在使用中
    CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,1);
  }
  //如果是第二個以後和NPC講話的人
  else if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 1 &&
          CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX) != toindex)
  {
    sprintf(token,"\n 正在服務其他玩家中，請稍等...");
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_WAIT;
    
    lssproto_WN_send(fd,windowtype,buttontype,windowno,
                     CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		     token);    
    return;
  }
  if(flag) 
  {
    ConnectToAP();
	flag = 0;
  }
  switch(num)
  {
  case 0:
  case 1:
    //取得玩家的id
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(CHAR_getWorkInt(meindex,NPC_WORK_FLAG2) == 1)
    {
	  if(pUserid == NULL || SendToAP(Cli_CHECKPLAYER,meindex,1,pUserid,NULL) == 0)
      {
        if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
        sprintf(token,"\n %s",buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,0);
      CHAR_setWorkInt(meindex,NPC_WORK_START,1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_2)
    {
			// 超過五秒time out
      if(CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT) >= 50)
      {
        close(g_nServiceSocket);
        flag = 1;
        sprintf(token,"\n 重新與系統取得連線...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
				CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
				CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        break;
      }
      if((Ret = RecvFromAP(&nAid,meindex,&nLen,&DataList)) == -1)
      {
        sprintf(token,"\n 等待回應...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
        break;
      }
      else 
      {
        for(i=0;i<nLen;i++)
        {
          strcpy(Data[i],DataList->Data);
          Kill = DataList;
          DataList = DataList->Next;
          if(Kill != NULL) free(Kill);
        }
        // 使用過的訊息清空
        bzero(&g_APMsg[Ret],sizeof(Msg));
      }
      if(nAid == AP_CHECKPLAYER)
      {
        switch(atoi(Data[0]))
        {
        case 0: //沒買
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"ErrMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,"\n %s",buf);
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;     
        break;
        case 1: //有買
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"MainMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,"5\n %s"
                        "\n 你要將%s"
												"\n 的獎品給這個角色嗎？\n"
                        "\n"
                        "\n                    《 確  定 》"
  	                    "\n"
        	              "\n                    《 取  消 》",
                        buf,Data[1]);
          buttontype = WINDOW_BUTTONTYPE_NONE;
          windowtype = WINDOW_MESSAGETYPE_SELECT;
          windowno = CHAR_WINDOWTYPE_SERVICE_START; 
        break;
        //另一個角色正在使用中
        case 2:
          sprintf(token,"\n 目前您的另一個角色正在領取.");
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
          break;
				//玩家的獎品已領完
				case 3:
					sprintf(token,"\n 您的獎品已經領完了!!");
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
          break;
        break;
        }
      }
      else
      {
        sprintf(token,"\n 系統訊息錯誤!!(%d)",nAid);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
      }
      CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
    }
    else
    {
      sprintf(token,"\n 等待回應...");
      buttontype = WINDOW_BUTTONTYPE_OK;
      windowtype = WINDOW_MESSAGETYPE_MESSAGE;
      windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
    }
  break;
  case 2:
    //取得玩家的id
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(CHAR_getWorkInt(meindex,NPC_WORK_FLAG3) == 1){
      if(pUserid == NULL || SendToAP(Cli_GET,meindex,1,pUserid,NULL) == 0){
        if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
        sprintf(token,"\n %s",buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
			CHAR_setWorkInt(meindex,NPC_WORK_START,1);
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,0);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_3){
			// 超過五秒time out
      if(CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT) >= 50){
        close(g_nServiceSocket);
        flag = 1;
        sprintf(token,"\n 重新與系統取得連線...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
				CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
				CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        break;
      }
      if((Ret = RecvFromAP(&nAid,meindex,&nLen,&DataList)) == -1){
        sprintf(token,"\n 等待回應...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
        break;
      }
      else
      {
        for(i=0;i<nLen;i++)
        {
          strcpy(Data[i],DataList->Data);
          Kill = DataList;
          DataList = DataList->Next;
          if(Kill != NULL) free(Kill);
        }
        // 使用過的訊息清空
        bzero(&g_APMsg[Ret],sizeof(Msg));
      }
      if(nAid == AP_GET)
      {
        //如果是給錢
        //確定玩家身上有沒有空的欄位
        if(atoi(Data[2]) == 0){
          if(CHAR_getInt(toindex,CHAR_GOLD)+atoi(Data[1]) > CHAR_getMaxHaveGold(toindex) ){
            sprintf(token,"\n"
                          "\n 你身上的空間不足以放入那麼多錢！\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }else{
			CHAR_AddGold( toindex, atoi(Data[1]) );

            CHAR_send_P_StatusString(toindex,CHAR_P_STRING_GOLD);
            LogService(CHAR_getChar(toindex,CHAR_NAME),
                       CHAR_getChar(toindex,CHAR_CDKEY),
                       atoi(Data[1]),
                       "錢",
                       CHAR_getInt(toindex,CHAR_FLOOR),
                       CHAR_getInt(toindex,CHAR_X),
                       CHAR_getInt(toindex,CHAR_Y)
                      );
            sprintf(talk,"%s石幣已經給你嘍!!\n",Data[1]);
          }
        }
        //如果是給物品
        else if(atoi(Data[2]) == 1){
          //確定玩家身上有沒有空的欄位
          int num = 0,itemindex = 0;

          for(i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++)
            if(CHAR_getItemIndex(toindex,i) != -1) num++;
          if(atoi(Data[1])+num > CHAR_MAXITEMNUM){
            sprintf(token,"\n"
                          "\n 你的道具欄空間不夠！\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }
          //有空的欄位
          for(j=0;j<atoi(Data[1]);j++){
            for(i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++){
              if(CHAR_getItemIndex(toindex,i) == -1){
                itemindex = ITEM_makeItemAndRegist(atoi(Data[0]));
                CHAR_setItemIndex(toindex,i,itemindex);
                ITEM_setWorkInt(itemindex,ITEM_WORKOBJINDEX,-1);
                ITEM_setWorkInt(itemindex,ITEM_WORKCHARAINDEX,toindex);
                CHAR_sendItemDataOne(toindex,i);
                LogService(CHAR_getChar(toindex,CHAR_NAME),
                           CHAR_getChar(toindex,CHAR_CDKEY),
                           atoi(Data[0]),
                           "物品",
                           CHAR_getInt(toindex,CHAR_FLOOR),
                           CHAR_getInt(toindex,CHAR_X),
                           CHAR_getInt(toindex,CHAR_Y)
                          );
                break;
              }
            }
          }
          sprintf(talk,"%s %s個已經給你嘍!!\n",ITEM_getAppropriateName(itemindex),Data[1]);
        }
        //如果是給寵物
        else if(atoi(Data[2]) == 2)
        {
          //確定玩家身上有沒有空的欄位
          int num = 0,ret = 0;
          
          for(i=0;i<CHAR_MAXPETHAVE;i++)
            if(CHAR_getCharPet(toindex,i) != -1) num++;
          if(atoi(Data[1])+num > CHAR_MAXPETHAVE)
          {
            sprintf(token,"\n"
                          "\n 你的寵物欄空間不夠！\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }
          //有空的欄位
          for(j=0;j<atoi(Data[1]);j++)
          {
            for(i=0;i<CHAR_MAXPETHAVE;i++)
            {
              if(CHAR_getCharPet(toindex,i) == -1)
              {
                int enemynum,count;
                char msgbuf[64];

                enemynum = ENEMY_getEnemyNum();
                for(k=0;k<enemynum;k++)
                {
                  if(ENEMY_getInt(k,ENEMY_ID) == atoi(Data[0])) break;
                }
                if(k == enemynum) break;
                
                ret = ENEMY_createPetFromEnemyIndex( toindex,k);
                CHAR_setCharPet(toindex,i,ret);
                snprintf(msgbuf,sizeof(msgbuf),"K%d",i);
                CHAR_sendStatusString(toindex,msgbuf);
                snprintf(msgbuf,sizeof(msgbuf),"W%d",i);
                CHAR_sendStatusString(toindex,msgbuf);

								for(count=0;count<CHAR_MAXPETHAVE;count++)
								{
								  if(CHAR_getCharPet(toindex,count) == ret)
									{	
								    CHAR_send_K_StatusString(toindex,count,CHAR_K_STRING_HP|CHAR_K_STRING_AI);
									}
								}
								//CHAR_setInt(petindex,CHAR_VARIABLEAI,CHAR_MAXVARIABLEAI);
                LogService(CHAR_getChar(toindex,CHAR_NAME),
                           CHAR_getChar(toindex,CHAR_CDKEY),
                           atoi(Data[0]),
                           "寵物",
                           CHAR_getInt(toindex,CHAR_FLOOR),
                           CHAR_getInt(toindex,CHAR_X),
                           CHAR_getInt(toindex,CHAR_Y)
                          );
                break;
              }
            }
          }
          sprintf(talk,"%s %s隻已經給你嘍!!\n",CHAR_getChar(ret,CHAR_NAME),Data[1]);
        }
        //玩家資料存檔
        CHAR_charSaveFromConnect(CHAR_getWorkInt(toindex,CHAR_WORKFD),0);
       if(pUserid == NULL || SendToAP(Cli_YES,meindex,1,pUserid,NULL) == 0)
       {
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,"\n %s",buf);
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
					LogService(CHAR_getChar(toindex,CHAR_NAME),
 										 CHAR_getChar(toindex,CHAR_CDKEY),
										 -1,
										 "玩家已領取寵物但Cli_YES沒送到AP",
										 CHAR_getInt(toindex,CHAR_FLOOR),
										 CHAR_getInt(toindex,CHAR_X),
										 CHAR_getInt(toindex,CHAR_Y)
										 );
          break;
        }
        if(NPC_Util_GetStrFromStrWithDelim(npcarg , "ExitMsg" , buf , sizeof(buf)) == NULL) return;
        sprintf(token,"%s %s",talk,buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
        CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_4);
      }
      else 
      {
        sprintf(token,"\n 系統訊息錯誤!!(%d)",nAid);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
    }
    else
    {
      sprintf(token,"\n 等待回應...");
      buttontype = WINDOW_BUTTONTYPE_OK;
      windowtype = WINDOW_MESSAGETYPE_MESSAGE;
      windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
    }
  break;
  case 3:
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(NPC_Util_GetStrFromStrWithDelim(npcarg,"CancelMsg",buf,sizeof(buf)) == NULL) return;
    sprintf(token,"\n %s",buf);
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
  break;
  }
  
  lssproto_WN_send(fd,windowtype,buttontype,windowno,
		   CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		   token);

}

void NPC_StoneServiceManLoop(int meindex)
{
  int nLeaveCount,nTimeout;
  fd_set readfds;
  struct timeval tm;
  LPList kill = NULL;
	//static unsigned int dwLifeTime = 0;
  
  if(g_EnableService)
  {
    // 如果玩家和NPC講話了就開始倒數
    if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 1)
    {
      nLeaveCount = CHAR_getWorkInt(meindex,NPC_WORK_LEAVE_COUNT);
      // 如果玩家超過時間沒完成動作(也有可能是玩家沒有按下ok就離開對話框)
      // 這時要把NPC的工作變數清為零才能讓下一個玩家talk
      if(nLeaveCount > getLoopTime())
      {
        int  talkerindex,i;
        char *pUserid;
       
        talkerindex = CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX);
        pUserid = CHAR_getChar(talkerindex,CHAR_CDKEY);
        if(pUserid != NULL) SendToAP(Cli_CANCEL,meindex,1,pUserid,NULL);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
        CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
        CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
        CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
        CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);
        //把之前收的有關這個NPC的訊息buf清空
        for(i=0;i<MAX_AP_MSG;i++)
        {
          // 檢查訊息欄有沒有被使用
          if(g_APMsg[i].bIsUse == 1)
          {
            // 檢查是不是送給這個NPC的訊息
            if(g_APMsg[i].nIndex == meindex)
            {
              // 使用過的訊息清空
              while(g_APMsg[i].Data != NULL)
              {
                kill = g_APMsg[i].Data;
                g_APMsg[i].Data = g_APMsg[i].Data->Next;
                if(kill != NULL) free(kill);
              }
              bzero(&g_APMsg[i],sizeof(Msg));
            }
          }
        }
        print("Player leave NPC\n");
        return;
      }
      // 未超過時間則計數
      CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,++nLeaveCount);
    }

		/*if(dwLifeTime > 3000) 
		{
			SendToAP(Cli_CONNECT,meindex,0,NULL,NULL);
			dwLifeTime = 0;
			print("\nI'm life!!!!!");
		}
		else dwLifeTime++;*/
		 // NPC_WORK_START為1時計時五秒,超過五秒為time out
    if(CHAR_getWorkInt(meindex,NPC_WORK_START) == 1)
    {
			tm.tv_sec = 0;
			tm.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(g_nServiceSocket,&readfds);
			// 檢查AP有沒有送訊息過來
			if(!select(g_nServiceSocket+1,&readfds,NULL,NULL,&tm))
			{
				nTimeout = CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT);
        CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,++nTimeout);
				return;
			}
			print("\nAP send message.");
			
			// 有送訊息過來接收訊息
			switch(GetMsg(meindex))
			{
				//這個訊息不是給這個NPC
			case  0: 
				print("Msg not send to this npc!!\n");
				nTimeout = CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT);
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,++nTimeout);
				return;
			case -1: break; //重新connect to ap
			case -2: print("Msg buf is full!!\n"); break; 	 // 訊息buffer用光了
			case -3: print("Msg data is wrong!!\n"); break;  // 訊息內容錯誤
			case -4: print("calloc fail!!\n"); break;        // 記憶體配置錯失敗
			case -5: print("\nAP send get I'm life"); break; // AP收到Cli_CONNECT的回應
			}
			CHAR_setWorkInt(meindex,NPC_WORK_START,0);
		}
  }
}

void NPC_StoneServiceManWindowTalked(int meindex,int talkerindex,int seqno,int select,char *data)
{
  int datanum = -1,i;
  char *pUserid;
  LPList Kill = NULL;

  makeStringFromEscaped(data);
  datanum = atoi( data);
  switch(seqno)
  {
  case CHAR_WINDOWTYPE_SERVICE_START:
    if(datanum == 1) 
    {
      CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_3);
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,2,-1);
    }
    else 
    if(datanum == 3) NPC_StoneServiceMan_selectWindow(meindex,talkerindex,3,-1);
  break;
  case CHAR_WINDOWTYPE_SERVICE_EXIT:
    pUserid = CHAR_getChar(talkerindex,CHAR_CDKEY);
    if(pUserid != NULL) SendToAP(Cli_CANCEL,meindex,1,pUserid,NULL);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
    CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
    CHAR_setWorkInt(meindex,NPC_WORK_START,0);
    CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
    CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
    CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
    CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
    CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);
    //把之前收的有關這個NPC的訊息buf清空
    for(i=0;i<MAX_AP_MSG;i++)
    {
      // 檢查訊息欄有沒有被使用
      if(g_APMsg[i].bIsUse == 1)
      {
        // 檢查是不是送給這個NPC的訊息
        if(g_APMsg[i].nIndex == meindex)
        {
          // 使用過的訊息清空
          while(g_APMsg[i].Data != NULL)
          {
            Kill = g_APMsg[i].Data;
						if(g_APMsg[i].Data->Next != NULL) 
						{
							g_APMsg[i].Data = g_APMsg[i].Data->Next;
							if(Kill != NULL) free(Kill);
						}
						else
						{
							if(Kill != NULL) free(Kill);
							break;
						}
          }
          bzero(&g_APMsg[i],sizeof(Msg));
        }
      }
    }
  break;
  case CHAR_WINDOWTYPE_SERVICE_CONTINUE:
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_1)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,0,-1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_2)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,1,-1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_3)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,2,-1);
    }
  break;
  case CHAR_WINDOWTYPE_SERVICE_WAIT:
  break;
  default:
    print("ERROR type\n");
  break;
  }
}

//送訊息到AP

//nAid (Cli_????) nIndex (npc Index) nLen (後續資料筆數) Data1 (user cdkey)
int SendToAP(int nAid,int nIndex,int nLen,void *Data1,void *Data2)
{
  char		buf[128];
  char		*pData[2];
  int			i,j,bufnum;
  fd_set	fdset;
  struct  timeval tmv;
  int     ret;
 // Syu ADD 修正石器服務員星系確認
  char *sags="";
  if( strcmp ( getAccountservername() , "210.64.50.31") == 0 ) sags = "sags31"; //北斗
  if( strcmp ( getAccountservername() , "210.64.50.41") == 0 ) sags = "sags41"; //天神
  if( strcmp ( getAccountservername() , "210.64.50.51") == 0 ) sags = "sags51"; //太陽
  if( strcmp ( getAccountservername() , "210.64.50.61") == 0 ) sags = "sags61"; //紫微
  if( strcmp ( getAccountservername() , "210.64.50.71") == 0 ) sags = "sags71"; //蒼龍
  if( strcmp ( getAccountservername() , "210.64.50.81") == 0 ) sags = "sags81"; //銀河
  if( strcmp ( getAccountservername() , "210.64.50.91") == 0 ) sags = "sags91"; //天鷹
  if( strcmp ( getAccountservername() , "210.64.50.101") == 0 ) sags = "sags101"; //網路家庭
  if( strcmp ( getAccountservername() , "210.64.50.106") == 0 ) sags = "sags106"; //星樂園
  if( strcmp ( getAccountservername() , "210.64.50.111") == 0 ) sags = "sags111"; //聖獸
  if( strcmp ( getAccountservername() , "152.104.1.206") == 0 ) sags = "hkgs206"; //香港
  if( strcmp ( getAccountservername() , "202.134.122.130") == 0 ) sags = "hkgs130"; //新界

  
  bzero(buf,sizeof(buf));
  pData[0] = (char*)Data1;
  pData[1] = (char*)Data2;
  
  // 以下為編碼
  // 取得流水號
  nAid = GetSerialNum()*10+nAid;
// Syu ADD 修正石器服務員星系確認
  nLen=2;	

  sprintf(buf,"&;%d;%d;%d;",nAid,nIndex,nLen);
  CHAR_setWorkInt(nIndex,NPC_WORK_SERIALNUM,nAid/10);
  if(nLen > 0)
  {
 // Syu ADD 修正石器服務員星系確認
    for(j = 0 ; j < 1 ; j++)

    {
      for(i = strlen(buf) ; i < 128 ; i++)
      {
      	if(*pData[j] == '\0')
      	  break;
      	else if(*pData[j] == ';')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = ';';
      	}
      	else if(*pData[j] == '\\')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = '\\';
      	}
      	else if(*pData[j] == '&')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = '&';
      	}
      	else buf[i] = *pData[j];
      	pData[j]++;
      }
// Syu ADD 修正石器服務員星系確認
	  strcat(buf,";");
	  strcat(buf,sags);

      buf[strlen(buf)] = ';';

    }
  }
  else strcat(buf,"0;0;");


  memset(&buf[strlen(buf)],'&',sizeof(buf)-strlen(buf));

	// 檢查網路可不可以傳送訊息
	FD_ZERO(&fdset);
  FD_SET(g_nServiceSocket,&fdset);
  tmv.tv_sec = tmv.tv_usec = 0;	
  ret = select(g_nServiceSocket + 1,(fd_set*)NULL,&fdset,(fd_set*)NULL,&tmv);
  print("ret1=%d ",ret);


  if(ret > 0)
	{
		// 送訊息到AP
		print("\n**SendToAP**:[%s]",buf);  
		if((bufnum = write(g_nServiceSocket,buf,sizeof(buf))) < 0)
		{
			//與主機失去連線
			print("reconnect to server!\n");
			close(g_nServiceSocket);
			flag = 1;
			return 0;
		}
		print("\nSend %d bytes to AP.",bufnum);

	}
	else
	{
		print("reconnect to server!\n");
		close(g_nServiceSocket);
		flag = 1;
			
		return 0;
	}
	
  return 1;
}

//收到AP回應的訊息
int RecvFromAP(int *nAid,int nIndex,int *nLen,LPList *Data)
{
  int  i;
  
  for(i=0;i<MAX_AP_MSG;i++)
  {
    // 檢查訊息欄有沒有被使用
    if(g_APMsg[i].bIsUse == 1)
    {
      // 檢查是不是送給這個NPC的訊息
      if(g_APMsg[i].nIndex == nIndex)
      {
        *nAid = g_APMsg[i].nAid;
        //檢查流水號對不對
        if((*nAid)/10 == CHAR_getWorkInt(nIndex,NPC_WORK_SERIALNUM))
        {
          *nAid = *nAid%10;
          *nLen = g_APMsg[i].nLen;
          *Data = g_APMsg[i].Data;
          
          return i;
        }
      }
    }
  }
  return -1;
}

int GetMsg(int meindex)
{
  char buf[128];
  char temp[3][128];
  char *p;
  int  i = 0,j;
  pMsg Msg = NULL;
  LPList tempList = NULL;
  
	bzero(buf,sizeof(buf));
  // 接收AP傳來的訊息
  if(read(g_nServiceSocket,buf,sizeof(buf)) <= 0)
  {
    print("can't read message!\n");
    //重連AP
	close(g_nServiceSocket);
    flag = 1;
    return -1;
  }
  print("**GetFromAP**:[%s]\n",buf);
  // 取得一個沒用過的訊息欄來用
  for(j=0;j<MAX_AP_MSG;j++)
  {
    if(g_APMsg[j].bIsUse == 0) 
    {
      Msg = &g_APMsg[j];
      if((Msg->Data = (LPList)calloc( 1,sizeof(List))) != NULL)
      {
        Msg->Data->Next = NULL;
        tempList = Msg->Data;
        break;
      }
      else return -4;
    }
  }
  // 超過MAX_AP_MSG表示訊息欄都滿了
  if(j == MAX_AP_MSG) return -2;
	Msg->bIsUse = 1;
  
  // 以下為解碼
  if(buf[i] == '&')
  {
    if(buf[++i] == ';')
    {
      bzero(temp[0] , sizeof(temp[0]));
      bzero(temp[1] , sizeof(temp[1]));
      bzero(temp[2] , sizeof(temp[2]));
      for(j = 0 ; j < 3 ; j++)
      {
        p = strchr(&buf[i+1] , ';');
        strncpy(temp[j] , &buf[i+1] , p - (&buf[i+1]));
        i += strlen(temp[j]) + 1;
      }
      Msg->nAid   = atoi(temp[0]);
      Msg->nIndex = atoi(temp[1]);
      Msg->nLen   = atoi(temp[2]);
			// 如果收到的是AP_CONNECT不理這個訊息
			if(Msg->nAid%10 == AP_CONNECT)
			{
				free(Msg->Data);
				bzero(Msg,sizeof(Msg));
				return -5;
			}
      j = 0;
      
      bzero(temp[0] , sizeof(temp[0]));
      
      if(Msg->nLen > 0)
      {
        while(buf[++i] != '\0')
        {
          switch(buf[i])
          {
          case '\\':
            switch(buf[i + 1])
            {
            case ';':
            case '&':
            case '\\':
              temp[0][j++] = buf[++i];
              break;
            default:
              temp[0][j++] = buf[i++];
              break;
            }
          break;
          case ';':
            j = 0;
            strcpy(tempList->Data,temp[0]);
            tempList->Next = (LPList)calloc( 1,sizeof(List));
            tempList = tempList->Next;
            bzero(temp[0],sizeof(temp[0]));
          break;
          case '&':
            buf[i--] = '\0';
          break;
          default:
            temp[0][j++] = buf[i];
          break;
          }
        }
        tempList->Next = NULL;
        // 如果這個訊息是給這個NPC return 1
        if(Msg->nIndex == meindex) return 1;
        else 											 return 0;
      }
    }
  }
  else return -3;
  
  return 1;
}

int ConnectToAP(void)
{
  //make socket and connect to ap
  if((g_nServiceSocket = socket(PF_INET,SOCK_STREAM,0)) < 0)
  //if((g_nServiceSocket = socket(PF_INET,SOCK_DGRAM,0)) < 0)
  {
    print("can't create socket!\n");
    return 0;
  }
  bzero((char*)&server_sin , sizeof(server_sin));
  server_sin.sin_family = AF_INET;
  server_sin.sin_port = htons(g_Port);
  server_sin.sin_addr = inet_addr(g_ApID);
  memcpy(&temp , &server_sin , sizeof(server_sin));
  if(connect(g_nServiceSocket,&temp,sizeof(temp)) < 0)
  {
//	print("connect server error: %s\n" , sys_errlist[errno]);
	print("connect server error !!\n");
    print("ip:%s,port:%d\n",g_ApID,g_Port);
    return 0;
  }
  else print("connect AP server success!\n");
  
  return 1;
}

//取得流水號
int GetSerialNum(void)
{
  static int i = 0;
  
  if(++i > 65535)
  {
    i = 0;
  }
  return i;
}

#endif
