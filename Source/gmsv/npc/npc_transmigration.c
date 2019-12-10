#include "version.h"
#include <string.h>
#include <math.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "enemy.h"
#include "log.h"
#include "battle.h"
#include "readmap.h"
#include "npc_transmigration.h"

#ifdef _PET_TRANS
#include "npc_pettransman.h"
#endif

static void NPC_Transmigration_selectWindow( int meindex, int toindex, int num);
static int NPC_TransmigrationCheck(int meindex, int talker);
BOOL NPC_TransmigrationAddPet(int meindex, int talker, int petid);
//関数の定義
int NPC_StartpointCheck(int meindex,int talker);
BOOL NPC_TransmigrationMain(int meindex, int toindex, char *buf);
int NPC_TransmigrationFlg_CLS(int meindex, int toindex);
BOOL NPC_TransmigrationDelPet(int meindex, int talker);
BOOL NPC_TransmigrationStatus(int meindex, int toindex,int *work);
BOOL NPC_TransmigratiomWarp(int meindex, int toindex, char *buf);
void s_eventsetend( int charaindex, int shiftbit );

#ifdef _PET_TRANS
int Pet_Select;
#endif

/*********************************
* 初期処理
*********************************/
BOOL NPC_TransmigrationInit( int meindex )
{

	/*--キャラのタイプを設定--*/
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPETRANSMIGRATION );

    return TRUE;

}

/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_TransmigrationTalked( int meindex , int talkerindex , char *szMes ,int color )
{
#ifdef _PET_TRANS
	int i,petindex;
	char buf[256];
#endif	
    /* プレイヤーに対してだけ反応する */
	if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )
	{
    		return;
	}
	/*--目の前にいるかどうか？--*/
	if(NPC_Util_isFaceToFace( meindex ,talkerindex , 2) == FALSE) {
		/* １グリッド以内のみ */
		if(NPC_Util_isFaceToChara( talkerindex, meindex, 1) == FALSE) return;
	}    
//	print("savepoot=%d",CHAR_getInt( talkerindex, CHAR_SAVEPOINT));
//	NPC_StartpointCheck( meindex, talkerindex);


//瑪雷菲雅判斷 成立 寵物轉生 不成立 人物轉生
#ifdef _PET_TRANS
  for(i=0 ; i < CHAR_MAXPETHAVE ; i++)	{
    petindex = CHAR_getCharPet( talkerindex, i);
    if( petindex == -1  )
        continue;
    //檢查寵物的 ID 是否為瑪雷菲雅 1479 ID 為718
    if( CHAR_getInt( petindex, CHAR_PETID) == 718
#ifdef _PET_2TRANS
		|| CHAR_getInt( petindex, CHAR_PETID) == 401
#endif
		)	{
	  if( CHAR_getInt( petindex, CHAR_LV) > 79 )	{
		//continue;
	  }else	if( CHAR_getInt( petindex, CHAR_LV) == 79 ) {
		if( strcmp( CHAR_getChar( petindex, CHAR_OWNERCDKEY), CHAR_getChar( talkerindex, CHAR_CDKEY) ) ||
			strcmp( CHAR_getChar( petindex, CHAR_OWNERCHARANAME), CHAR_getChar( talkerindex, CHAR_NAME) ))	{
			sprintf( buf,"你身上的瑪雷菲雅，並不是你的啊！");
			CHAR_talkToCli( talkerindex, -1, buf,  CHAR_COLORWHITE);
			return;
		}
		break;
	  }
	}
  }

  if(i != CHAR_MAXPETHAVE)	{
    CHAR_setWorkInt(talkerindex,CHAR_WORKSHOPRELEVANT,1);
    NPC_PetTransMan_selectWindow( meindex, talkerindex,0,-1);
  }else {
#endif
    CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT,1);
    NPC_Transmigration_selectWindow( meindex, talkerindex, 0);
#ifdef _PET_TRANS
  }
#endif

	//原轉生
	//CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT,1);
	//NPC_Transmigration_selectWindow( meindex, talkerindex, 0);
	
}


//各処理に分ける
static void NPC_Transmigration_selectWindow( int meindex, int toindex, int num)
{

	char token[1024];
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[2048];
	char buf2[512];
	char tmp[512];
	char s_tensei[64];
	int buttontype = 0;
	int windowtype = 0;
	int windowno = 0;
	int tenseiNo = 1;
	int fd = getfdFromCharaIndex( toindex);
	int i = 1;
	BOOL tenflg = FALSE;
	char *elder[4] = {"samugiru","marinasu","jaja","karutarna"};
	int point;
	int ten_no;
	if(NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg))==NULL){
		print("Tensei:GetArgStrErr");
		return;
	}
  	windowtype = WINDOW_MESSAGETYPE_MESSAGE;
	tenseiNo = CHAR_getInt( toindex, CHAR_TRANSMIGRATION);
	tenseiNo++;
	if(CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT) >= 2){
		tenseiNo--;
	}
	 
	//設定ファイル内での転生回数にあうものを選ぶための準備
	sprintf(s_tensei,"TENSEI:%d",tenseiNo);

	while(getStringFromIndexWithDelim( npcarg,"END",i,buf,sizeof( buf))	!= FALSE){
		i++;
		if(strstr(buf,s_tensei) != NULL) {
		 tenflg = TRUE;
		 break;
		}
	}
#if 0	
	//どれにもあてはまらなかったら終了
	if(tenflg == FALSE){
		print("SettingFile Error ");
		return ;
	}
#endif
        //ttom
        print("\nTrans meindex=%d,toindex=%d,num=%d",meindex,toindex,num);
        //ttom
        CONNECT_set_state_trans(fd,6);
        //ttom end
         
	//それぞれの処理に分ける。
	switch( num){
	  case 0:
	        //ttom
	        //CONNECT_set_state_trans(fd,0);
	        //ttom end
		ten_no = NPC_TransmigrationCheck(meindex, toindex);
 		if(
#ifdef _TRANS_6
			ten_no < 6
#else
#ifdef _PK_SERVER
			ten_no < 3//百萬PK限制到3轉
#else
			ten_no < 5
#endif
#endif//_TRANS_6
			&& ten_no >= 0){
	  		//条件クリアー
	  		if(NPC_Util_GetStrFromStrWithDelim( buf,"startmsg", token,sizeof( token) ) 
				 == NULL)
				{
					print("startmsgErr");
					return;
				}
			  	buttontype = WINDOW_BUTTONTYPE_NEXT;
			  	//ttom
			  	CONNECT_set_state_trans(fd,0);
			  	//ttom end			  	                                                                                                                                
		}else if(
#ifdef _TRANS_6
			ten_no == 6
#else
#ifdef _PK_SERVER
			ten_no == 3//百萬PK限制到3轉
#else
			ten_no == 5
#endif
#endif//_TRANS_6
			){
			//転生を５回やった
		  	if(NPC_Util_GetStrFromStrWithDelim( npcarg,"maxmsg", token,sizeof( token) ) 
			   == NULL)
			{
				print("maxmsgErr");
				return;
			}
			buttontype = WINDOW_BUTTONTYPE_OK;
		}else{
			//条件クリアしてない
			if(NPC_Util_GetStrFromStrWithDelim( buf,"nonemsg", token,sizeof( token) ) 
			   == NULL)
			{
				print("nonemsgErr");
				return;
			}
			buttontype = WINDOW_BUTTONTYPE_OK;

		}
		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_START;
		  	 
	 	break;
	 	
	 	case 1:
		 	if(NPC_Util_GetStrFromStrWithDelim( buf,"mainmsg", token,sizeof( token) ) 
			 == NULL)
			{
				print("mainmsgErr");
				return;
			}
		  	buttontype = WINDOW_BUTTONTYPE_OKCANCEL;
		  	windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_MAIN; 
	 	break;
	 	
	 	case 2:
      	 	        // Robin 2001/03/05 Tensei Check
			//if(!CHAR_getWorkInt(toindex,CHAR_TENSEICHECKED))  return;
	 	       
	 		//転生処理を
	 		point = NPC_StartpointCheck( meindex, toindex);
	 		if(point == -1) {
	 			return;
	 		}
	 		
			NPC_Util_GetStrFromStrWithDelim( npcarg,elder[point],buf2,sizeof( buf2));
			if(NPC_TransmigrationMain(meindex, toindex, buf2) ==  FALSE) 
			{
				return;
			}

	 		
	 		sprintf( tmp, "yesmsg%d", 
	 						CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT));
	 		
	 		//転生終了メッセージをだす。
	 		if(NPC_Util_GetStrFromStrWithDelim( buf,tmp, token,sizeof( token) ) 
			 == NULL)
			{
				print("yesmsgErr");
				return;
			}

			CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 
					CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT) +1
			);
	 		sprintf( tmp, "yesmsg%d", 
	 						CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT));
			
			//転生終了メッセージをだす。
	 		//転生終了メッセージをだす。
	 		if(strstr(buf, tmp) == NULL)	{
				buttontype = WINDOW_BUTTONTYPE_OK;
		  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_END; 
			}else	{
				buttontype = WINDOW_BUTTONTYPE_NEXT;
		  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_END; 
			}
	 	break;
	
	 	case 3:
	 		
	 		/*--選択画面--*/
			/*--キーワードのチェックを行う--*/
			if(NPC_Util_GetStrFromStrWithDelim( buf,"nomsg", token,sizeof( token) ) 
			 == NULL)
			{
				print("nomsgErr");
				return;
			}
		  	buttontype = WINDOW_BUTTONTYPE_OK;
	  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_NONE; 
	 		
	 	break;

	 	case 4:
	 		//転生後のメッセージ処理
	 		sprintf( tmp, "yesmsg%d", 
	 						CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT));
	 		//転生終了メッセージをだす。
	 		if(NPC_Util_GetStrFromStrWithDelim( buf,tmp, token,sizeof( token) ) 
			 == NULL)
			{
				print("yesmsgErr");
				return;
			}

			CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 
					CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT) +1
			);
	 		sprintf( tmp, "yesmsg%d", 
	 						CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT));
			
			//転生終了メッセージをだす。
	 		//転生終了メッセージをだす。
	 		if(strstr(buf, tmp) == NULL)
			{
				buttontype = WINDOW_BUTTONTYPE_OK;
		  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_END; 
			}else{
				buttontype = WINDOW_BUTTONTYPE_NEXT;
		  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_END; 
			}
			
	 	
		break;
		case 5:

                        // Robin 2001/03/05 Tensei Check
                        //if(!CHAR_getWorkInt(toindex,CHAR_TENSEICHECKED))  return;
			//転生処理を
	 		point = NPC_StartpointCheck( meindex, toindex);
	 		if(point == -1) return;
			NPC_Util_GetStrFromStrWithDelim( npcarg,elder[point],buf2,sizeof( buf2));
			NPC_TransmigratiomWarp(meindex, toindex, buf2);
//			buttontype = WINDOW_BUTTONTYPE_OK;
//	  		windowno = CHAR_WINDOWTYPE_TRANSMIGRATION_END; 
			return;
		break;

	 	
	}
//	makeEscapeString( token, escapedname, sizeof(escapedname));
	/*-ここで送信する--*/
	lssproto_WN_send( fd, windowtype, 
					buttontype, 
					windowno,
					CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
					token);
}
/*-----------------------------------------
クライアントから返ってきた時に呼び出される。
-------------------------------------------*/
void NPC_TransmigrationWindowTalked( int meindex, int talkerindex, 
								int seqno, int select, char *data)
{

	int datanum = -1;

	int fd = getfdFromCharaIndex( talkerindex);
	/*-- ＮＰＣのまわりにいないときは終了 --*/
	if( NPC_Util_CharDistance( talkerindex, meindex ) > 2) {
		return;
	}
       //ttom
       //CONNECT_set_state_trans(fd,6);
       //ttom end
	datanum = atoi( data);
	switch( seqno) {
		/*--最初のメッセージ-*/
	  case CHAR_WINDOWTYPE_TRANSMIGRATION_START:
		if( select == WINDOW_BUTTONTYPE_NEXT) {
		        //ttom
		        if(CONNECT_get_state_trans(fd)==0){ } else return;
		        //ttom end
		        NPC_Transmigration_selectWindow( meindex, talkerindex,1);
			//ttom
			CONNECT_set_state_trans(fd,1);
			//ttom end
			         
		}
	  	break;
		
		//転生するかどうか聞くメッセージの答え）
	  case CHAR_WINDOWTYPE_TRANSMIGRATION_MAIN:
	 	if( select == WINDOW_BUTTONTYPE_OK) {
	 	        //ttom
	 	        if(CONNECT_get_state_trans(fd)==1){ } else return;
	 	        //ttom end
	 	        NPC_Transmigration_selectWindow( meindex, talkerindex, 2);
	 	        //ttom
	 	        CONNECT_set_state_trans(fd,2);
	 	        //ttom end
	 	}else if(select == WINDOW_BUTTONTYPE_CANCEL) {
			NPC_Transmigration_selectWindow( meindex, talkerindex, 3);
		}
	    break;

		/*--最後のメッセージ--*/
	  case CHAR_WINDOWTYPE_TRANSMIGRATION_END:
	 	if( select == WINDOW_BUTTONTYPE_NEXT) {
	 	        //ttom
	 	        if(CONNECT_get_state_trans(fd)==2){ } else return;
	 	         //ttom end
	 	        NPC_Transmigration_selectWindow( meindex, talkerindex, 4);
	 	        //ttom
	 	        CONNECT_set_state_trans(fd,4);
	 	        //ttom end
	 	}else if( select == WINDOW_BUTTONTYPE_OK) {
	 	        //ttom
	 	        if(CONNECT_get_state_trans(fd)==4){ } else return;
	 	        //ttom end
	 	        NPC_Transmigration_selectWindow( meindex, talkerindex, 5);
		}
		break;
		
//ANDY_ADD 10/19
#ifdef _PET_TRANS

	case CHAR_WINDOWTYPE_PETTRANS_START:	//開始對話
		if( select == WINDOW_BUTTONTYPE_NEXT) {
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 1, 0);
		}	
	break;

	case CHAR_WINDOWTYPE_PETTRANS_SELECT:	//是否讓寵物轉生
		if( select == WINDOW_BUTTONTYPE_YES) {	
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 2, 0);
		}else	{
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 5, 0);
		}
	break;

	case CHAR_WINDOWTYPE_PETTRANS_SELPET:	//選寵
		if(select==WINDOW_BUTTONTYPE_CANCEL)	{
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 5,-1 );
		}else	{
			Pet_Select = datanum;	
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 3, datanum );
		}
	break;

	case CHAR_WINDOWTYPE_PETTRANS_MAIN1:	//確定寵物	確定/取消
		if(select==WINDOW_BUTTONTYPE_YES){
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 4, Pet_Select);
		}else	{
			NPC_PetTransMan_selectWindow( meindex, talkerindex, 5, -1);
		}
	break;
       
	case CHAR_WINDOWTYPE_PETTRANS_END:	//寵物轉生結束結束
		CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT, 0);
	break;
	
#endif

	}


}

//************************************************************
//	はじめの長老宅にワープさせる
//************************************************************
BOOL NPC_TransmigratiomWarp(int meindex, int toindex, char *buf)
{
	char buf2[512];
	char buf3[128];
	char buf4[64];
	int floor=0 , x = 0, y = 0;
	int i = 1;
	int ret;
	
	//出身村ごとの条件を導く
	while(getStringFromIndexWithDelim( buf,",",i,buf2,sizeof( buf2))
	!= FALSE)
	{
		i++;
		if(strstr(buf2,"Warp") != NULL)
		{
			getStringFromIndexWithDelim( buf2 ,"=" ,2, buf3 ,sizeof(buf3));
			//フロア
			ret = getStringFromIndexWithDelim( buf3,".",1 ,buf4 ,sizeof( buf4));
			if(ret ==FALSE) return FALSE;
			floor = atoi(buf4);
			//Ｘ座標
			ret = getStringFromIndexWithDelim( buf3,".",2 ,buf4 ,sizeof( buf4));
			if(ret ==FALSE) return FALSE;
			x = atoi(buf4);
			//Ｙ座標
			ret = getStringFromIndexWithDelim( buf3,".",3 ,buf4 ,sizeof( buf4));
			if(ret ==FALSE) return FALSE;
			y = atoi(buf4);

		}
	}
    
    
	//ワープ先のチェック
    if( MAP_IsValidCoordinate( floor, x, y )== FALSE ){
		return FALSE;
	}

        //ttom
        /*
        {
            int tenseiNo;
            tenseiNo=CHAR_getInt( toindex, CHAR_TRANSMIGRATION);
            CHAR_setInt( toindex,CHAR_SKILLUPPOINT,tenseiNo*10);
            print("\n the set the skillpoint %d",tenseiNo*10);
            CHAR_sendStatusString( toindex , "P");
        }
        */
        //ttom
                                                   
	//ワープさせる
	 CHAR_warpToSpecificPoint(toindex, floor, x, y);

	return TRUE;

}

//************************************************************
//*出発点のチェック
//
//戻り値：
//	０：サムギル
//	１：マリナス
//	２：ジャジャ
//	３：カルターナ
//	−１：エラー
//************************************************************
int NPC_StartpointCheck(int meindex,int talker)
{
	int point;
	int shift = 0;
	point = CHAR_getInt( talker, CHAR_SAVEPOINT);
	for(shift = 0; shift < 4 ;shift++){
		if( (point & ( 1<< shift))  == ( 1<< shift)) {
			return shift;
		}
	}
	return -1;
}

//********************************************************
//
//	転生条件を満たしているかどうかのチェック
//
//戻り値
//	成功：転生回数
//	失敗：-1
//
//********************************************************
static int NPC_TransmigrationCheck(int meindex, int talker)
{
	//	転生条件
	//レベルが８０以上
	//４つの属性イベントが終わっていること
	//最終イベントをこなす。(これはいらんかな）
	int i,j,k=0;
	int ch_no[4] = {39,40,42,46};
	int petindex;
	int num;
//	int petid[4] = {1, 2, 3, 4}; //ペットのＩＤを指定
	int petid[4] = {693, 694, 695, 696}; //ペットのＩＤを指定
//	int petidfinal[4] = {11, 12, 13, 14}; //ペットのＩＤを指定
	int petidfinal[4] = {693, 694, 695, 696}; //ペットのＩＤを指定

	// Robin 2001/03/05
	//CHAR_setWorkInt(talker,CHAR_TENSEICHECKED,FALSE);
	//もし５回以上なら転生できない。
#ifdef _TRANS_6
	if(CHAR_getInt(	talker, CHAR_TRANSMIGRATION) >= 6){
		return 6;
	}
#else
	if(CHAR_getInt(	talker, CHAR_TRANSMIGRATION) >= 5){
		return 5;
	}
#endif

	//レベルが８０以上
	if(CHAR_getInt( talker, CHAR_LV) < 80) return -1;

	//イベントのフラグチェック
#ifdef _TRANS_6
	if(CHAR_getInt(	talker, CHAR_TRANSMIGRATION) < 6){
#else
	if(CHAR_getInt(	talker, CHAR_TRANSMIGRATION) < 5){
#endif
		for(i = 0; i < 4 ;i++)
			if(NPC_EventCheckFlg( talker, ch_no[i] ) == FALSE) return -1;
	}

	num = CHAR_getInt(talker, CHAR_TRANSMIGRATION);

	if(num  < 4 ){
	//指定ペットをもっているかペットチェック
		for(i=0 ; i < CHAR_MAXPETHAVE ; i++){
			petindex = CHAR_getCharPet( talker, i);
			if( petindex == -1  )  continue;
			//ttom
			//print("petid=%d ",CHAR_getInt( petindex, CHAR_PETID));
			//ペットのＩＤは一緒か？
			if(CHAR_getInt( petindex, CHAR_PETID) != petid[num]) continue;

			//ペットのレベルは条件を満たしているか
//			if(CHAR_getInt( petindex, CHAR_LV) < PETLEVEL) continue;
			//ここまできたらそのペットは本物と判断
			break;
		}
		if(i == CHAR_MAXPETHAVE) return -1;
	}else
#ifdef _TRANS_6
    if( num == 4 )
#endif
	{
		for(j = 0; j < 4 ;j++){
			//五回目の場合は４匹をチェック
			for(i=0 ; i < CHAR_MAXPETHAVE ; i++){
				petindex = CHAR_getCharPet( talker, i);
				if( petindex == -1  )  continue;
		//		print("petid=%d ",CHAR_getInt( petindex, CHAR_PETID));
				//ペットのＩＤは一緒か？
				if(CHAR_getInt( petindex, CHAR_PETID) != petidfinal[j]) continue;
				k++;
				break;
			}
		}
		if(k < 4) return -1;
	}
#ifdef _TRANS_6
	else{
		/*int lasttalkelder = 2;//烏力
		if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 1 )//瑪麗娜絲
			lasttalkelder = 112;//凱比
		else if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 2 )//加加
			lasttalkelder = 102;//克克爾
		else if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 3 )//卡魯它那
			lasttalkelder = 34;//威伯
		print("\npetid:%d",lasttalkelder);*/

		//找看看有沒有帶2轉出生寵物
		for(i=0 ; i < CHAR_MAXPETHAVE ; i++){
			petindex = CHAR_getCharPet( talker, i);
			if( petindex == -1  )  continue;
			if( CHAR_getInt( petindex, CHAR_TRANSMIGRATION) == 2 
				&& ( CHAR_getInt( petindex, CHAR_PETID) == 2 
				|| CHAR_getInt( petindex, CHAR_PETID) == 112
				|| CHAR_getInt( petindex, CHAR_PETID) == 102
				|| CHAR_getInt( petindex, CHAR_PETID) == 34 ) ) break;
			//if(CHAR_getInt( petindex, CHAR_TRANSMIGRATION) != 2 || CHAR_getInt( petindex, CHAR_PETID) != lasttalkelder ) continue;		
		}
		if(i == CHAR_MAXPETHAVE){
			CHAR_talkToCli( talker,-1,"你沒有帶2轉的出生寵哦!",CHAR_COLORWHITE);
			return -1;
		}


		if(NPC_EventCheckFlg( talker, 63) == FALSE){ print("\nerr63");return -1;}
		if(NPC_EventCheckFlg( talker, 136) == FALSE){ print("\nerr136");return -1;}
		if(NPC_EventCheckFlg( talker, 175) == FALSE){ print("\nerr175");return -1;}
		if(NPC_EventCheckFlg( talker, 176) == FALSE){ print("\nerr176");return -1;}
		//檢查有無15個魔界之門碎片
		j=0;
		for( i=0 ; i<CHAR_MAXITEMHAVE ; i++ ){
			int itemindex;
			itemindex = CHAR_getItemIndex(talker,i);
			if( ITEM_CHECKINDEX(itemindex) == FALSE )continue;
			if( ITEM_getInt( itemindex, ITEM_ID ) == 20613 ) {
				++j;
			}
		}
		if( j < 15 ){
			CHAR_talkToCli( talker,-1,"你沒有帶15個魔界之門的碎片哦!",CHAR_COLORWHITE);
			print("\nitem:%d",j);
			return -1;
		}

		if( CHAR_getInt(talker, CHAR_HEROCNT) <= 0 ){
			CHAR_talkToCli( talker,-1,"你沒有執行過精靈召喚的任務!",CHAR_COLORWHITE);
			return -1;
		}
	}
#endif
	// Robin 2001/03/05
	//CHAR_setWorkInt(talker,CHAR_TENSEICHECKED,TRUE);
	//最終イベントチェック

	return CHAR_getInt(talker, CHAR_TRANSMIGRATION);
}

BOOL NPC_TransmigrationMain(int meindex, int toindex, char *buf)
{
	int i = 1;
	char buf2[128];
	char buf3[64];
	char buf4[32];
	int item = 10;
	int pet = 5;
	int present = 0;
	int ten_no;
	int work[10];
	int fd = getfdFromCharaIndex( toindex );

#ifdef _ADD_POOL_ITEM			   // WON ADD 增加可寄放的道具	
	int tran_pool_item[5] = { 4, 4, 4, 4, 4 };		// 增加的道具寄放數
	int tran_pool_pet[5] = { 2, 2, 2, 2, 2};		// 增加的寵物寄放數
	int j,item_sum=0, pet_sum=0;

	for(j=0; j<5 ;j++){
		item_sum += tran_pool_item[j] ;			
		pet_sum += tran_pool_pet[j];		
	}

	item_sum += 10;								// 限制最大的寄道具數
	pet_sum  += 5;							    // 限制最大的寄寵數

#endif
	while(getStringFromIndexWithDelim( buf,",",i,buf2,sizeof( buf2))!= FALSE){
		i++;

		if(strstr(buf2,"Item") != NULL){
			getStringFromIndexWithDelim( buf2 ,"=" ,2, buf3 ,sizeof(buf3));
			item = atoi(buf3);
		}else if(strstr(buf2,"Pet") != NULL){
			getStringFromIndexWithDelim( buf2 ,"=" ,2, buf3 ,sizeof(buf3));
			pet = atoi(buf3);
		}else if(strstr(buf2,"Present") != NULL){
			ten_no = CHAR_getInt(toindex,CHAR_TRANSMIGRATION);
			ten_no++;
			getStringFromIndexWithDelim( buf2 ,"=" ,2, buf3 ,sizeof(buf3));
			getStringFromIndexWithDelim( buf3,".",ten_no ,buf4 ,sizeof( buf4));
			present = atoi(buf4);
		}
	}

	CHAR_setInt(toindex ,CHAR_TRANSMIGRATION,
		CHAR_getInt(toindex,CHAR_TRANSMIGRATION) + 1);
#ifdef _TRANS_6
	if(CHAR_getInt(	toindex, CHAR_TRANSMIGRATION) > 6)
		CHAR_setInt(toindex ,CHAR_TRANSMIGRATION,6);
	if( CHAR_getInt(toindex,CHAR_TRANSMIGRATION) == 6 )//6轉後所送的寵
		present = 2546;
#else
	if(CHAR_getInt(	toindex, CHAR_TRANSMIGRATION) > 5)
		CHAR_setInt(toindex ,CHAR_TRANSMIGRATION,5);
#endif
	NPC_TransmigrationStatus(meindex, toindex, work);
	NPC_TransmigrationFlg_CLS(meindex, toindex);
	CHAR_setInt(toindex, CHAR_LV ,1);
	CHAR_setMaxExp( toindex, 0);

#ifdef _TRANS_6
//刪除魔界之門碎片
	for( i=0 ; i<CHAR_MAXITEMHAVE ; i++ ){
		int itemindex;
		itemindex = CHAR_getItemIndex(toindex,i);
		if( ITEM_CHECKINDEX(itemindex) == FALSE )continue;
		if( ITEM_getInt( itemindex, ITEM_ID ) == 20613 ) {
			CHAR_DelItem( toindex, i);
		}
	}
#endif

	if(NPC_TransmigrationDelPet(meindex, toindex) == FALSE) return FALSE;
	if(	NPC_TransmigrationAddPet( meindex, toindex, present)== FALSE) return FALSE;
	// Robin 0718
        {
            int tenseiNo;
            tenseiNo=CHAR_getInt( toindex, CHAR_TRANSMIGRATION);
            CHAR_setInt( toindex,CHAR_SKILLUPPOINT,tenseiNo*10);
            CHAR_Skillupsend( toindex );
            //CHAR_setInt( toindex,CHAR_LEARNRIDE, 0);
            CHAR_setInt( toindex,CHAR_RIDEPET, -1 );
            CHAR_setInt( toindex , CHAR_BASEIMAGENUMBER , CHAR_getInt( toindex , CHAR_BASEBASEIMAGENUMBER) );
            
            //CHAR_sendStatusString( toindex , "P");
        }
	lssproto_WO_send(fd ,0 );
	// 転生ログを取る
	LogTensei(
		CHAR_getChar( toindex, CHAR_NAME ), /* キャラ名 */
		CHAR_getChar( toindex, CHAR_CDKEY),
		"Tensei",
		work[0],
		CHAR_getInt(toindex,CHAR_TRANSMIGRATION),
		work[1],
		NPC_StartpointCheck( meindex, toindex),
		item, pet,
		work[2],work[3],work[4],work[5],
		work[6],work[7],work[8],work[9]
		);

	//更新
	CHAR_complianceParameter( toindex );
	//HPを回復させてあげる
	CHAR_setInt(toindex, CHAR_HP, CHAR_getWorkInt(toindex, CHAR_WORKMAXHP) );
	//ＳＰ先を出身地の村長宅に戻す。
	CHAR_setInt(toindex,CHAR_LASTTALKELDER, NPC_StartpointCheck(meindex,toindex));
	/*  プレイヤー情報を送る  */
	CHAR_sendStatusString( toindex, "P" );
	/*
	CHAR_send_P_StatusString( toindex,
								  CHAR_P_STRING_LV|CHAR_P_STRING_EXP|
								  CHAR_P_STRING_NEXTEXP|
								  CHAR_P_STRING_HP|CHAR_P_STRING_MAXHP|
								  CHAR_P_STRING_MP|CHAR_P_STRING_MAXMP|
								  CHAR_P_STRING_ATK|CHAR_P_STRING_DEF|
								  CHAR_P_STRING_QUICK|CHAR_P_STRING_CHARM|
								  CHAR_P_STRING_LUCK|CHAR_P_STRING_EARTH|
								  CHAR_P_STRING_WATER|CHAR_P_STRING_FIRE|
								  CHAR_P_STRING_WIND|CHAR_P_STRING_VITAL|
								  CHAR_P_STRING_STR|CHAR_P_STRING_DEX|
								  CHAR_P_STRING_TOUGH|
								  CHAR_P_STRING_DUELPOINT //ttom
								  );
	*/			
	return TRUE;
}


static int questTbl[20] = {
1,2,4,5,8,12,14,15,16,17,19,22,27,30,31,34,35,38,45,47
};
//*******************************************************
//クエストの数を数える
//*******************************************************
int NPC_TransmigrationQuestCheck(int toindex)
{
	int i;
	int num = 0;
	
	for(i = 0 ; i < 20; i++){
		if(NPC_EventCheckFlg(toindex ,questTbl[i]) == TRUE){
			num++;
		}
	}
	
	return num;
}

//*******************************************************
//	ステータスを計算する
//*******************************************************
int NPC_TransCalculation(int toindex, int para)
{
	float quest, level, num, tmp;
	float ans;
	int equ;
	equ = CHAR_getInt(toindex, CHAR_TRANSEQUATION);
	//16bit毎で使用
	quest = (float)(( equ >> 16 ) & 0xFFFF);
	level = (float)(( equ >> 0 ) & 0xFFFF);
	//tttom
	//print("\n this test equ=%d,quest=%f,level=%f",equ,quest,level);
	//転生回数
	num = CHAR_getInt(toindex, CHAR_TRANSMIGRATION); // 前面已加1,故0轉1時為1
	tmp = (float)para / 100;
#ifdef _TRANS_6
	if (num == 6) tmp = 620; // 六轉生時舊點數不計
#endif
	ans =(float)tmp/12 + (float)(quest / 4) +( (float)(level- num * 85) /4);
	return (int)ans;
}

//*******************************************************
//	四捨五入
//*******************************************************
float Rounding(float work,int num)
{

	float p;
	
	if(num  <0 ) return work;
	num--;
	p = (float)pow(10,num);
	
	return ( (work * p + 0.5) /p);


}

//*******************************************************
//
//	ステータス関係の更新
//
//*******************************************************
BOOL NPC_TransmigrationStatus(int meindex, int toindex,int work[10])
{
	int vital, str, tgh, dex;
	int quest , level ,equ;
	int t_quest, t_level;
	int total,ans;
	float tmp,lvup;
	
	vital = CHAR_getInt(toindex,CHAR_VITAL);
	str = CHAR_getInt(toindex,CHAR_STR);
	tgh = CHAR_getInt(toindex,CHAR_TOUGH);
	dex = CHAR_getInt(toindex,CHAR_DEX);


	//クエストフラグをチェック
	quest = NPC_TransmigrationQuestCheck( toindex);
	//レベル
	level = CHAR_getInt(toindex,CHAR_LV);
	
	work[0] = level;
	work[1] = quest;
	
	equ = CHAR_getInt(toindex, CHAR_TRANSEQUATION);

	t_quest = (float)(( equ >> 16 ) & 0xFFFF);
	t_level = (float)(( equ >> 0 ) & 0xFFFF);
	
	if( level > 130 ) level = 130;
	t_quest = t_quest + quest;
	t_level = t_level + level;

	CHAR_setInt(toindex, CHAR_TRANSEQUATION,
   		( t_quest << 16 )+( t_level << 0 )
   	);

	total = vital + str + tgh + dex;
	ans = NPC_TransCalculation(toindex, total);

	work[2] = vital;
	tmp = (float)vital/total * ans;
	work[3] = Rounding(tmp,1) * 100;

	work[4] = str;
	tmp = (float)str/total * ans;
	work[5] = Rounding(tmp,1) * 100;

	work[6] = tgh;
	tmp = (float)tgh/total * ans;
	work[7] = Rounding(tmp,1) * 100;
	
	work[8] = dex;
	tmp = (float)dex/total * ans;
	work[9] = Rounding(tmp,1) * 100;

	//vitalセット
	if(work[3] < 0) work[3] = 0;
	CHAR_setInt(toindex, CHAR_VITAL, work[3]);
	//strセット
	if(work[5] < 0) work[5] = 0;
	CHAR_setInt(toindex, CHAR_STR, work[5]);

	//tghrセット
	if(work[7] < 0) work[7] = 0;
	CHAR_setInt(toindex, CHAR_TOUGH, work[7]);
	
	//dexセット
	if(work[9] < 0) work[9] = 0;
	CHAR_setInt(toindex, CHAR_DEX, work[9]);
	
	
	
	//レベルＵＰポイントも変更
	lvup = CHAR_getInt(toindex, CHAR_SKILLUPPOINT );
/*#ifdef _TRANS_6
	if( CHAR_getInt(toindex, CHAR_TRANSMIGRATION) == 6)
		lvup = 620;
#endif*/
	lvup = lvup / 12;
	lvup =  Rounding(lvup, 1);
	if(lvup < 0) lvup = 0;
	CHAR_setInt(toindex, CHAR_SKILLUPPOINT, (int)lvup);
	
	/*  スキルアップできたらこれを送る  */
	if( CHAR_getInt( toindex,CHAR_SKILLUPPOINT ) >=1 )
		CHAR_Skillupsend(toindex);

	
	return TRUE;


}


/*----------------------------
 *  ペットを追加する
 ------------------------------*/
BOOL NPC_TransmigrationAddPet(int meindex, int talker, int petid)
{
	int	ret;
	char msgbuf[64];
	int	enemynum;
	int	i,j;
	int petindex;
	int petindex2;
	for( i = 0 ;i < CHAR_MAXPETHAVE ; i++) {
		petindex = CHAR_getCharPet( talker, i);
		if( petindex == -1  )  break;
	}
	if(i == CHAR_MAXPETHAVE) return FALSE;
	enemynum = ENEMY_getEnemyNum();
	for( i = 0; i < enemynum; i ++ ) {
		if( ENEMY_getInt( i, ENEMY_ID) == petid) {
			break;
		}
	}

	if( i == enemynum ) return FALSE;

	ret = ENEMY_createPetFromEnemyIndex( talker, i);

	for( i = 0; i < CHAR_MAXPETHAVE; i ++ ){
		if( CHAR_getCharPet( talker, i ) == ret )break;
	}
	if( i == CHAR_MAXPETHAVE ) i = 0;
	if( CHAR_CHECKINDEX( ret ) == TRUE ){
		CHAR_setMaxExpFromLevel( ret, CHAR_getInt( ret, CHAR_LV ));
	}

	petindex2 = CHAR_getCharPet(talker, i);
    if( !CHAR_CHECKINDEX( petindex2) )	return FALSE;

	snprintf( msgbuf,sizeof( msgbuf), "拿到%s。",
								CHAR_getChar(petindex2,CHAR_NAME));
	CHAR_talkToCli( talker, -1, msgbuf,  CHAR_COLORWHITE);
	for(j = 0; j < CHAR_MAXPETHAVE; j++){
		petindex = CHAR_getCharPet(talker, j);
	    if( !CHAR_CHECKINDEX( petindex) ) continue;
		CHAR_complianceParameter( petindex );
		snprintf( msgbuf, sizeof( msgbuf ), "K%d", j );
		CHAR_sendStatusString( talker, msgbuf );
		snprintf( msgbuf, sizeof( msgbuf ), "W%d", j );
		CHAR_sendStatusString( talker, msgbuf );
	}
	// ペットを手に入れたログ
	LogPet(
		CHAR_getChar( talker, CHAR_NAME ), /* キャラ名 */
		CHAR_getChar( talker, CHAR_CDKEY ),
		CHAR_getChar( petindex2, CHAR_NAME),
		CHAR_getInt( petindex2, CHAR_LV),
		"TenseiGet",
		CHAR_getInt( talker,CHAR_FLOOR),
		CHAR_getInt( talker,CHAR_X ),
		CHAR_getInt( talker,CHAR_Y ),
		CHAR_getChar( petindex2, CHAR_UNIQUECODE)   // shan 2001/12/14
	);

	return TRUE;

}

#ifdef _TEACHER_SYSTEM
extern int FMAdvTbl[];
#endif

int NPC_TransmigrationFlg_CLS(int meindex, int toindex)
{
#ifdef _TEACHER_SYSTEM
	int iGetFame = 0;
#endif
#ifdef _PET_TRANS
#ifdef _PROSK99
	int endevflg[]={69,70,71,72,109,118,122,131,127,158,159,181,-1};//181:白狼獎品
#else
	int endevflg[]={69,70,71,72,109,118,122,131,127,158,159,-1};
#endif	
	int nowevflg[]={69,70,71,72,-1};
#endif
#ifdef _TRANS_6
	int flg[6] = {48,49,50,51,52,179};
#else
	int flg[5] = {48,49,50,51,52};
#endif
	int num= CHAR_getInt(toindex ,CHAR_TRANSMIGRATION);
	int i;
#ifdef _PET_TRANS
	i=0;
	while(endevflg[i] != -1 )	{
		if( NPC_EventCheckFlg( toindex, endevflg[i] ) == FALSE )	{
			endevflg[i] = 0;
		}
		i++;
	}
	i=0;
	while( nowevflg[i] != -1 )	{
		if( NPC_NowEventCheckFlg( toindex, nowevflg[i] ) == FALSE )	{
			nowevflg[i] = 0;
		}
		i++;
	}
#endif

	CHAR_setInt(toindex,CHAR_ENDEVENT,0);
	CHAR_setInt(toindex,CHAR_ENDEVENT2,0);
	CHAR_setInt(toindex,CHAR_ENDEVENT3,0);
#ifdef _NEWEVENT
	CHAR_setInt(toindex,CHAR_ENDEVENT4,0);
	CHAR_setInt(toindex,CHAR_ENDEVENT5,0);
	CHAR_setInt(toindex,CHAR_ENDEVENT6,0);
#endif
	CHAR_setInt(toindex,CHAR_NOWEVENT,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT2,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT3,0);
#ifdef _NEWEVENT
	CHAR_setInt(toindex,CHAR_NOWEVENT4,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT5,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT6,0);
#endif
#ifdef _ADD_NEWEVENT              // WON 多增任務旗標
	CHAR_setInt(toindex,CHAR_ENDEVENT7,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT7,0);
	CHAR_setInt(toindex,CHAR_ENDEVENT8,0);
	CHAR_setInt(toindex,CHAR_NOWEVENT8,0);
#endif

	//転生フラグを立てる
#ifdef _PET_TRANS
	i=0;
	while( endevflg[i] != -1 )	{
		if( endevflg[i] > 0 )	{
			// shan Begin
			//NPC_EventSetFlg( toindex, endevflg[i]);			
			s_eventsetend(toindex, endevflg[i]);
			// End
		}
		i++;
	}
	i=0;
	while( nowevflg[i] != -1 )	{
		if( nowevflg[i] > 0 )	{
			NPC_NowEventSetFlg( toindex, nowevflg[i]);
		}
		i++;
	}

#endif

	for(i = 0 ;i < num ;i++)	{
		NPC_EventSetFlg(toindex, flg[i]);
		// CoolFish: Family Adv 2001/8/4
		AddFMAdv(toindex, flg[i]);
#ifdef _TEACHER_SYSTEM
		// 累加所得聲望
		if(flg[i] < 179) iGetFame += FMAdvTbl[flg[i]];
#endif
	}
#ifdef _TEACHER_SYSTEM
	iGetFame /= 20; // 導師獲得學生所得聲望的 5% (1/20)
	// 檢查所得的聲望有沒有大於0
	if(iGetFame > 0){
		// 檢查有沒有導師
		if(strlen(CHAR_getChar(toindex,CHAR_TEACHER_ID)) > 0 && strlen(CHAR_getChar(toindex,CHAR_TEACHER_NAME)) > 0){
			int iPlayernum = CHAR_getPlayerMaxNum();
			char szMsg[128];

			// 檢查導師在不在線上
			for(i=0;i<iPlayernum;i++){
				if(CHAR_getCharUse(i) == FALSE) continue;
				if(strcmp(CHAR_getChar(toindex,CHAR_TEACHER_ID),CHAR_getChar(i,CHAR_CDKEY)) == 0 &&
					strcmp(CHAR_getChar(toindex,CHAR_TEACHER_NAME),CHAR_getChar(i,CHAR_NAME)) == 0){
					float fGetFame = (float)iGetFame/100;
					// 導師在線上
					CHAR_setWorkInt(i,CHAR_WORK_GET_TEACHER_FAME,CHAR_getWorkInt(i,CHAR_WORK_GET_TEACHER_FAME) + iGetFame);
					sprintf(szMsg,"獲得學生 %s %.2f 點聲望",CHAR_getChar(toindex,CHAR_NAME),fGetFame);
					CHAR_talkToCli(i,-1,szMsg,CHAR_COLORYELLOW);
					break;
				}
			}
		}
	}
#endif

#ifdef _TRANS_6
	if( num == 6 ){
		NPC_EventSetFlg(toindex, 63); //不給玩家解英雄的祝福
		NPC_EventSetFlg(toindex, 136);//不給玩家解淚之石
		NPC_EventSetFlg(toindex, 175);//雷爾任務
		NPC_EventSetFlg(toindex, 176);//雷爾任務
	}
#endif

	return 1;
}

//******************************************************************
//ペットを消す
//******************************************************************
BOOL NPC_TransmigrationDelPetDel(int meindex,int talker,int petsel)
{

	int petindex;
	int fd = getfdFromCharaIndex( talker );
	char msgbuf[64];
	char szPet[128];
	int defpet;
	
	petindex = CHAR_getCharPet( talker, petsel);

    if( !CHAR_CHECKINDEX(petindex) ) return FALSE;
	
	/*--そのキャラが戦闘中なら、ペットをうらず終了--*/
	if( CHAR_getWorkInt( CONNECT_getCharaindex( fd),
   	                     CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE) return FALSE;
	/*--そのペットがバトルかどうかのチェック--*/
	defpet = CHAR_getInt( talker, CHAR_DEFAULTPET);
	if(defpet == petsel){
		CHAR_setInt( talker, CHAR_DEFAULTPET, -1);
		lssproto_KS_send( fd, -1, TRUE);
	}

	snprintf( msgbuf,sizeof( msgbuf), "交出%s。",
									CHAR_getChar( petindex, CHAR_NAME));	CHAR_talkToCli( talker, -1, msgbuf,  CHAR_COLORWHITE);
	// ペットを手渡したログ
	LogPet(
		CHAR_getChar( talker, CHAR_NAME ), /* キャラ名 */
		CHAR_getChar( talker, CHAR_CDKEY ),
		CHAR_getChar( petindex, CHAR_NAME),
		CHAR_getInt( petindex, CHAR_LV),
		"TenseiDel",
		CHAR_getInt( talker,CHAR_FLOOR),
		CHAR_getInt( talker,CHAR_X ),
		CHAR_getInt( talker,CHAR_Y ),
		CHAR_getChar( petindex, CHAR_UNIQUECODE)   // shan 2001/12/14
	);
	/* 持ち物から消す */
	CHAR_setCharPet( talker, petsel, -1);
	CHAR_endCharOneArray( petindex );
	snprintf( szPet, sizeof( szPet ), "K%d", petsel);
	// 新しいペットステータス送りつける
	CHAR_sendStatusString( talker, szPet );
	return TRUE;
}

//***********************************************************
//	消すペットを探す。
//***********************************************************
BOOL NPC_TransmigrationDelPet(int meindex, int talker)
{
	int petsel;
	int petindex;
	int num;
	int j, k;
//	int petid[4] = {1, 2, 3, 4}; //ペットのＩＤを指定
	int petid[4] = {693, 694, 695, 696}; //ペットのＩＤを指定
//	int petidfinal[4] = {11, 12, 13, 14}; //ペットのＩＤを指定
	int petidfinal[4] = {693, 694, 695, 696}; //ペットのＩＤを指定
	int petwork[4] ={ -1, -1, -1, -1};
	
	num = CHAR_getInt(talker, CHAR_TRANSMIGRATION);
	num--;
	if(num  < 4 ){
		//指定ペットをもっているかペットチェック
		for(petsel=0 ; petsel < CHAR_MAXPETHAVE ; petsel++){
			petindex = CHAR_getCharPet( talker, petsel);
			if( petindex == -1  )  continue;
			
			//ペットのＩＤは一緒か？
			if(CHAR_getInt( petindex, CHAR_PETID) != petid[num]) continue;
			//ここまできたらそのペットは本物と判断
			break;
		}
	
		if(petsel  == CHAR_MAXPETHAVE) return FALSE;
		if(NPC_TransmigrationDelPetDel(meindex, talker, petsel) == FALSE) return FALSE;
	
	}
#ifdef _TRANS_6
	else if( num == 4 )
#else
	else
#endif
	{
		//五回目
		for(j = 0; j < 4; j++){
			//指定ペットをもっているかペットチェック
			for(petsel=0 ; petsel < CHAR_MAXPETHAVE ; petsel++){
				petindex = CHAR_getCharPet( talker, petsel);
				if( petindex == -1  )  continue;
				
				//ペットのＩＤは一緒か？
				if(CHAR_getInt( petindex, CHAR_PETID) != petidfinal[j]) continue;
				
				petwork[j] = petsel;
				//ここまできたらそのペットは本物と判断
				break;
			}
		}
		for(k=0; k < 4 ;k++)
		{
			if(petwork[k] == -1) return FALSE;
		}
		for(k = 0 ; k < 4 ;k++){
			if(NPC_TransmigrationDelPetDel(meindex, talker, petwork[k]) == FALSE) return FALSE;
		}
	}
#ifdef _TRANS_6
	else{
		/*int lasttalkelder = 2;//烏力
		if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 1 )//瑪麗娜絲
			lasttalkelder = 112;//凱比
		else if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 2 )//加加
			lasttalkelder = 102;//克克爾
		else if( CHAR_getInt( talker, CHAR_LASTTALKELDER) == 3 )//卡魯它那
			lasttalkelder = 34;//威伯
			*/

		//指定ペットをもっているかペットチェック
		for(petsel=0 ; petsel < CHAR_MAXPETHAVE ; petsel++){
			int petid;
			petindex = CHAR_getCharPet( talker, petsel);
			if( petindex == -1  )  continue;
			
			//ペットのＩＤは一緒か？
			if(CHAR_getInt( petindex, CHAR_TRANSMIGRATION) != 2) continue;
			petid=CHAR_getInt( petindex, CHAR_PETID );
			if (petid == 2) break;
			if (petid == 112) break;
			if (petid == 102) break;
			if (petid == 34) break;
			//ここまできたらそのペットは本物と判断
		}
		if(petsel  == CHAR_MAXPETHAVE) return FALSE;
		if(NPC_TransmigrationDelPetDel(meindex, talker, petsel) == FALSE) return FALSE;
	}
#endif
	return TRUE;
}

#ifdef _PET_TRANS
void NPC_PetTransMan_selectWindow(int meindex,int toindex,int num,int select)
{ 
	char token[1024],buf[1024],buf1[256];
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
	char s_tensei[128];
	char errchar[][56]={"errormy","erroritem","errorpet","failmsg"};
	int buttontype = 0, windowtype = 0, windowno = 0, errtype = 0;
	int petindex = 0,i=1;
	char *petname;
	BOOL tenflg = FALSE;
	int fd = getfdFromCharaIndex( toindex);

	if( fd == -1 ) {
		print( "getfd err\n");
		return;
	}

	if(NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg))==NULL){
		print("Tensei:GetArgStrErr");
		return;
	}

	sprintf(s_tensei,"PETTRANS:");

	//比對npc談話
	while(getStringFromIndexWithDelim( npcarg,"END",i,buf,sizeof( buf)) != FALSE)	{
		i++;
  		//如果條件符合則 tenflg 設為TRUE
  		if( strstr(buf,s_tensei) != NULL) {
			tenflg = TRUE;
			break;
  		}
	}
  
	if( CHAR_getWorkInt( toindex, CHAR_WORKSHOPRELEVANT) < 0 )
		CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
  
	token[0] = '0';		
 
	switch(num)	{
		case 0:		//對話
			if( NPC_Util_GetStrFromStrWithDelim( buf,"mainmsg", token,sizeof( token)) == NULL )	{
				print("\n ERROR buf str!!");
				return;
			}
			buttontype = WINDOW_BUTTONTYPE_NEXT;
			windowno = CHAR_WINDOWTYPE_PETTRANS_START;
			break;
		
		case 1:	//是否讓寵物轉生				
			//轉生需要物品或寵物檢查
			errtype = NPC_PetTransManCheck( meindex, toindex, 0);
			if( errtype >= 0 )
				errtype = 3;
	
			if( errtype >= 0 )	{
				if( NPC_Util_GetStrFromStrWithDelim( buf,errchar[ errtype], token,sizeof( token)) == NULL )	{
					print("\n ERROR str errchar !!");
					return;
				}
				buttontype = WINDOW_BUTTONTYPE_OK;
				windowtype = WINDOW_MESSAGETYPE_MESSAGE;
				windowno =  CHAR_WINDOWTYPE_PETTRANS_END;
				break;                                                
			}
			if( NPC_Util_GetStrFromStrWithDelim( buf,"selectmsg", token,sizeof( token)) == NULL )	{
				print("\n ERROR selectmsg !!");
				return;
			}
			CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 1);
			windowno = CHAR_WINDOWTYPE_PETTRANS_SELECT;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			buttontype=WINDOW_BUTTONTYPE_YESNO;
			break;

		case 2:		//選寵
			if( NPC_Util_GetStrFromStrWithDelim( buf,"setpetmsg", token,sizeof( token)) == NULL )	{
				print("\n ERROR setpetmsg !!");
				return;
			}
			buttontype = WINDOW_BUTTONTYPE_NONE;
			windowtype = WINDOW_MESSAGETYPE_PETSELECT;
			windowno = CHAR_WINDOWTYPE_PETTRANS_SELPET;
			break;

		case 3:		//寵物轉生	確定/取消
			if( NPC_Util_GetStrFromStrWithDelim( buf,"transmsg", buf1,sizeof( buf1)) == NULL ){
				print("\n ERROR transmsg !!");
				return;
			}
			strcpy( token,"\0");
			petindex = CHAR_getCharPet( toindex, select-1);
			petname = CHAR_getUseName( petindex);
			sprintf(token,	"\n是[%s]沒錯吧？ \n\n %s",petname,buf1);

			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			buttontype = WINDOW_BUTTONTYPE_YESNO;
			windowno = CHAR_WINDOWTYPE_PETTRANS_MAIN1;
			break;

		case 4:	//寵物轉生 CHAR_TRANSMIGRATION
			//轉生條件判斷
			petindex = CHAR_getCharPet( toindex, select-1);
			if( !CHAR_CHECKINDEX( petindex) ) {
				return;
			}
			if( CHAR_getInt( petindex, CHAR_FUSIONBEIT ) == 1 ) {
				sprintf(token, "\n\n融合過的寵物不能轉生喔！"); 
				CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
			}else
#ifdef _PET_2TRANS
			if( CHAR_getInt( toindex, CHAR_RIDEPET ) == select-1 /*&& CHAR_getInt( petindex, CHAR_TRANSMIGRATION ) > 0*/ ){//騎乘中的寵物不能轉生
				sprintf(token, "\n\n騎乘中的寵物不能轉生喔！"); 	
			}else
#endif
			if( CHAR_getInt( petindex, CHAR_LV) < 80 )	{
				sprintf(token, "\n\n接受轉生祝福的寵物必需要等級80以上喔！"); 
				CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
			}else if( CHAR_getInt( petindex, CHAR_PETID) == 718 
#ifdef _PET_2TRANS
					  || CHAR_getInt( petindex, CHAR_PETID) == 401 
#endif
			){
				sprintf(token, "\n\n這隻寵物不能轉生喔！"); 
				CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
#ifdef _PET_2TRANS
			}else if( CHAR_getInt( petindex, CHAR_TRANSMIGRATION ) > 1 )	{
#else
			}else if( CHAR_getInt( petindex, CHAR_TRANSMIGRATION ) > 0 )	{
#endif
				sprintf(token, "\n\n這隻寵物已經接受過祝福了喔！"); 
				CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
				//檢查寵物是否重複轉生
			}else	{	//	轉生ＯＫ
				petindex = CHAR_getCharPet( toindex, select-1);
				if( NPC_PetTransManStatus( meindex, toindex, petindex) == FALSE )
					return;
				if( NPC_Util_GetStrFromStrWithDelim( buf,"main1msg", token,sizeof( token)) == NULL )
					return;
			}

			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno =  CHAR_WINDOWTYPE_PETTRANS_END;	//結束
			break;
  
		case 5:		//寵物轉生取消	
			if( NPC_Util_GetStrFromStrWithDelim( buf,"cancelmsg", token,sizeof( token)) == NULL )
				return;
			CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, 0);
			buttontype = WINDOW_BUTTONTYPE_OK;
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
			windowno =  CHAR_WINDOWTYPE_PETTRANS_END;	//結束
			break;	
	
		default:
			break;
	}
	lssproto_WN_send( fd, windowtype, buttontype, windowno, CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX), token);

}

int NPC_PetTransManCheck( int meindex, int toindex, int select)
{
	//char errchar[][56]={"errormy","erroritem","errorpet","failmsg"};
	int i;
	int petindex = 0;
	char token[256];
	int PetEvent_no[3]={4,69,70};
	//int PetEvent_no[3]={4,4,4};

	int petNum = 0;
	//寵物轉生條件
	if( CHAR_getInt( toindex, CHAR_LV ) < 80 )	{
	  return 0;
	}else	{
		//檢查任務的FLAG 是否已完成寵轉需要的任務
	  for(i = 0; i < 3 ;i++)	{
		  if( NPC_EventCheckFlg( toindex, PetEvent_no[i] ) == FALSE)	{
			return 0;
		  }
	  }
	//檢查所需(寵物/精靈) & 檢查所需物品
	  for(i=0 ; i < CHAR_MAXPETHAVE ; i++)	{
		petindex = CHAR_getCharPet( toindex, i);
		if( petindex == -1  )
			continue;
		//檢查寵物的 ID 是否為瑪雷菲雅 1479 ID 為718
		if(CHAR_getInt( petindex, CHAR_PETID) == 718
#ifdef _PET_2TRANS
			|| CHAR_getInt( petindex, CHAR_PETID) == 401
#endif
			){
			petNum ++;
		  if( CHAR_getInt( petindex, CHAR_LV) > 79 )	{
			return 0;
		  }else	if( CHAR_getInt( petindex, CHAR_LV) == 79 ) {
			if( strcmp( CHAR_getChar( petindex, CHAR_OWNERCDKEY), CHAR_getChar( toindex, CHAR_CDKEY) ) ||
				strcmp( CHAR_getChar( petindex, CHAR_OWNERCHARANAME), CHAR_getChar( toindex, CHAR_NAME) ))	{
				sprintf( token,"你身上的瑪雷菲雅，並不是你的啊！");
				CHAR_talkToCli( toindex, -1, token,  CHAR_COLORWHITE);
				return 0;
			}
		  }
		}
	  }   

	  if( petNum > 1 )	{
		sprintf( token,"你怎會有那麼多瑪雷菲雅呢？");
		CHAR_talkToCli( toindex, -1, token,  CHAR_COLORWHITE);
		print("\n error PetTrans : (PETID = 718) > 1 !!");
		return 0;
	  }else if( petNum != 1 )	{
		  print("\n 寵轉發生錯誤 !!");
		  return 0;
	  }
	}
	return -1;
}

BOOL NPC_PetTransManStatus( int meindex, int toindex, int petNo)
{
	int petindex;
	int LevelUpPoint = 0,petrank = 0;
	int i,j,k,itemindex,itemID,count;
	int vital1,str1,tgh1,dex1;
	int vital2,str2,tgh2,dex2;
	char token[128];
	int total1,total2,total,petLV,ans;
	int petID,enemynum,ret;
	int work[4]={0,0,0,0};
	struct PetTransDelItem {
		int minItemID;
		int maxItemID;
	}DelItem[3] = { {19629,19632} , {19672,19687} , {19625,19628} };
#define RAND(x,y)   ((x-1)+1+ (int)( (double)(y-(x-1))*rand()/(RAND_MAX+1.0)) )
#ifdef _PET_2TRANS
	int tran = CHAR_getInt ( petNo, CHAR_TRANSMIGRATION );
	int tpetidx1,tpetidx2;
	if( tran == 0 ){
		tpetidx1 = 718;
		tpetidx2 = 401;
	}
	else{
		tpetidx1 = 401;
		tpetidx2 = 401;
	}
#endif

	for(i=0 ; i < CHAR_MAXPETHAVE ; i++)	{
		petindex = CHAR_getCharPet( toindex, i);
		if( petindex == -1  )
			continue;
		if(
#ifdef _PET_2TRANS
			CHAR_getInt( petindex, CHAR_PETID) == tpetidx1
			|| CHAR_getInt( petindex, CHAR_PETID) == tpetidx2
#else
			CHAR_getInt( petindex, CHAR_PETID) == 718
#endif
			){
			if( CHAR_getInt( petindex, CHAR_LV) == 79 ){
				if( strcmp( CHAR_getChar( petindex, CHAR_OWNERCDKEY), CHAR_getChar( toindex, CHAR_CDKEY) ) ||
					strcmp( CHAR_getChar( petindex, CHAR_OWNERCHARANAME), CHAR_getChar( toindex, CHAR_NAME) ))	{
					continue;
				}
				break;
			}
		}
	}
	if(i == CHAR_MAXPETHAVE)	{
		print("\n 寵物轉生發生錯誤 !!");
		return FALSE;
	}

	LevelUpPoint = CHAR_getInt( petindex, CHAR_ALLOCPOINT );
	petrank = CHAR_getInt( petindex, CHAR_PETRANK );

	vital1 = (float)(( LevelUpPoint >> 24 ) & 0xFF);
	str1 = (float)(( LevelUpPoint >> 16 ) & 0xFF);
	tgh1 = (float)(( LevelUpPoint >> 8 ) & 0xFF);
	dex1 = (float)(( LevelUpPoint >> 0 ) & 0xFF);
	total1 = ( vital1 + str1 + tgh1 + dex1 );
	if( total1 > 150 )	total1 = 150;
	if( total1 < 0 ) total1 = 0;
	LevelUpPoint = CHAR_getInt( petNo, CHAR_ALLOCPOINT );
	petrank = CHAR_getInt( petNo, CHAR_PETRANK );
	petLV = CHAR_getInt( petNo, CHAR_LV);
	vital2 = (float)(( LevelUpPoint >> 24 ) & 0xFF);
	str2 = (float)(( LevelUpPoint >> 16 ) & 0xFF);
	tgh2 = (float)(( LevelUpPoint >> 8 ) & 0xFF);
	dex2 = (float)(( LevelUpPoint >> 0 ) & 0xFF);
	total2 = ( vital2 + str2 + tgh2 + dex2 );

#ifdef _PET_2TRANS
	ans = NPC_PetTransManGetAns( total1, total2, petLV, petrank, CHAR_getInt( petNo, CHAR_TRANSMIGRATION ) );
#else
	ans = NPC_PetTransManGetAns( total1, total2, petLV, petrank);
#endif
	total = total1 + (total2*4);

	work[0] = ( ans * ( vital1 + (vital2*4) ) ) / total;
	work[1] = ( ans * ( str1 + (str2*4) ) ) / total;
	work[2] = ( ans * ( tgh1 + (tgh2*4) ) ) / total;
	work[3] = ( ans * ( dex1 + (dex2*4) ) ) / total;
//取array=========================
	enemynum = ENEMY_getEnemyNum();
	for(i=0;i<CHAR_MAXPETHAVE;i++)	{
		if( CHAR_getCharPet( toindex , i) == petNo)	{
			break;
		}
	}

	if( CHAR_MAXPETHAVE == i )	{
		return FALSE;
	}

	petID = CHAR_getInt( petNo, CHAR_PETID);
	for(k=0;k<enemynum;k++)	{
		if(ENEMY_getInt(k, ENEMY_TEMPNO) == petID )
			break;
	}
	if(k == enemynum)	{
		print("\n test *-7 : petID=%d", petID);
		return FALSE;
	}
	{
		BOOL FINDs = FALSE;
#ifdef _PET_2TRANS
		tpetidx1 = CHAR_getInt( petindex, CHAR_PETID);
#endif		
		//刪除身上所有瑪寵
		for(j=0 ; j < CHAR_MAXPETHAVE ; j++){
			int pet_index = CHAR_getCharPet( toindex, j);
			if( pet_index == -1  )  continue;
			if( 
#ifdef _PET_2TRANS
				CHAR_getInt( pet_index, CHAR_PETID) == tpetidx1
#else
				CHAR_getInt( pet_index, CHAR_PETID) == 718
#endif
				){
				if( NPC_TransmigrationDelPetDel( meindex, toindex, j) == FALSE){
					return FALSE;
				}else	{
					FINDs = TRUE;
				}
			}
		}
		if( FINDs == FALSE )
			return FALSE;
	}

	ret = GetNewPet( toindex , petNo, k, work);
	if( ret < 0 )
		return FALSE;
	for(count=0;count<CHAR_MAXPETHAVE;count++)	{
		if(CHAR_getCharPet(toindex,count) == ret)	{	
			CHAR_send_K_StatusString(toindex,count,CHAR_K_STRING_HP|CHAR_K_STRING_AI);
		}
	}
	{
		
			LogPetTrans( 
				CHAR_getChar( toindex, CHAR_CDKEY ),
				CHAR_getChar( petNo, CHAR_UNIQUECODE),
				CHAR_getChar( petindex, CHAR_UNIQUECODE),//瑪蕾
				CHAR_getChar( toindex, CHAR_NAME),
				CHAR_getInt( toindex, CHAR_FLOOR),
				CHAR_getInt( toindex, CHAR_X), 
				CHAR_getInt( toindex, CHAR_Y), 
				CHAR_getInt( petNo, CHAR_PETID),
				CHAR_getChar( petNo, CHAR_NAME),
				petLV, petrank,
				vital2, str2, tgh2, dex2, total2,
				CHAR_getInt( petindex, CHAR_PETID),
				CHAR_getChar( petindex, CHAR_NAME),
				vital1, str1, tgh1, dex1, total1,
				work[0], work[1], work[2], work[3], ans, CHAR_getInt ( petNo, CHAR_TRANSMIGRATION ) );
	}
    sprintf(token,"%s 接受了精靈王的祝福！", CHAR_getUseName( ret));
	CHAR_talkToCli( toindex, -1,token,CHAR_COLORWHITE);
	sprintf( token, "K%d", i );
	CHAR_sendStatusString( toindex , token );
 
//--------------------------------------------------------------------------
	//刪除物品	itemindex,itemID;
	for( i =0 ; i < CHAR_MAXITEMHAVE ; i++ ){
		itemindex = CHAR_getItemIndex( toindex , i );
		if( ITEM_CHECKINDEX(itemindex) ){
			itemID = ITEM_getInt(itemindex ,ITEM_ID );
			for( j = 0;j < 3;j++)	{
				if( ( itemID >= DelItem[j].minItemID ) && ( itemID <= DelItem[j].maxItemID ) )	{
							LogItem(
								CHAR_getChar( toindex, CHAR_NAME ),
								CHAR_getChar( toindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD 在item的log中增加item名稱
								itemindex,
#else
								ITEM_getInt( itemindex, ITEM_ID),
#endif
								"WarpManDelItem",
								CHAR_getInt( toindex, CHAR_FLOOR),
								CHAR_getInt( toindex, CHAR_X ),
 								CHAR_getInt( toindex, CHAR_Y ),
								ITEM_getChar( itemindex, ITEM_UNIQUECODE),
								ITEM_getChar( itemindex, ITEM_NAME),
								ITEM_getInt( itemindex, ITEM_ID)
							);
					CHAR_DelItem( toindex, i);
				}

			}//for(j)
		}
	}

	NPC_NowEventSetFlg( toindex, 71 );

	return TRUE;
}



// shan add
void s_eventsetend( int charaindex, int shiftbit )
{
#ifdef _ADD_NEWEVENT              // WON 多增任務旗標
	int event_num = 8;
#else
	int event_num = 6;
#endif
	int point;	
	int array;
	int shift;

	array = shiftbit / 32;
	shift = shiftbit % 32;
		
	if( array>=event_num ){
		//print("錯誤！！所設的任務旗標編號已超過範圍(0~%d)。",32*event_num-1);			
		return;
	}
	point = CHAR_getInt( charaindex, CHAR_ENDEVENT+array);
	point = point | (1<<shift);
	CHAR_setInt( charaindex, CHAR_ENDEVENT+array, point);
}
#endif


