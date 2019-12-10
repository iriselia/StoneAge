#include "version.h"
#include <stdio.h>

#include "readmap.h"
#include "object.h"
#include "char.h"
#include "char_base.h"
#include "battle.h"
#include "lssproto_serv.h"
#include "npcutil.h"
#include "npc_bus.h"
#include "npc_airplane.h"       // Arminius 7.10 Airplane
#include "family.h"             // shan

#ifdef _ITEM_QUITPARTY
#include "init.h"
#endif
// shan add 
extern struct FM_PKFLOOR fmpkflnum[FAMILY_FMPKFLOOR];

/*------------------------------------------------------------
 * パーティ関連のソース
 ------------------------------------------------------------*/

/*------------------------------------------------------------
 * 空いているパーティ欄を探す
 * なければ-1を返す。
 ------------------------------------------------------------*/
int CHAR_getEmptyPartyArray( int charaindex)
{
	int     i = -1;
	int     rc = FALSE;
	int		toindex;
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_NONE ) {
		toindex = charaindex;
	}
	else {
		toindex = CHAR_getPartyIndex( charaindex, 0);
	}
	if( CHAR_CHECKINDEX( toindex)){
		for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
			if( CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1) == -1 ) {
				rc = TRUE;
				break;
			}
		}
	}
	return( rc ? i: -1);
}
/*------------------------------------------------------------
 * 実際にパーティに入る処理
 *
 *  charaindex		int		自分
 *  targetindex		int		入る相手の人
 ------------------------------------------------------------*/
void CHAR_JoinParty_Main( int charaindex, int targetindex)
{
	int		firstflg = FALSE;
	int		i;
	char	c[3];
	char buf[64];
	int		toindex;
	int		parray;

	/* 親がいたら引っ張り出す */
	if( CHAR_getWorkInt( targetindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_NONE ) {
		toindex = targetindex;
	}
	else {
		toindex = CHAR_getPartyIndex( targetindex, 0);
		if( !CHAR_CHECKINDEX( toindex) ) {
			print( " %s:%d err\n", __FILE__, __LINE__);
			return;
		}
	}


	/* 相手パーティの人数はＯＫか？ */
	parray = CHAR_getEmptyPartyArray( toindex) ;
	if( parray == -1 ) {
		print( "%s : %d err\n", __FILE__,__LINE__);
		return;
	}
	/* 何も無し→親の時は親になったCAを送信する */
	if( CHAR_getWorkInt( toindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_NONE ) {
		CHAR_sendLeader( CHAR_getWorkInt( toindex, CHAR_WORKOBJINDEX), 1);
		/* 相手の状態の書き換え */
		/* 親になる */
		CHAR_setWorkInt( toindex, CHAR_WORKPARTYMODE, 1);
		CHAR_setWorkInt( toindex, CHAR_WORKPARTYINDEX1, toindex);
		firstflg = TRUE;
	}
	CHAR_setWorkInt( toindex, parray + CHAR_WORKPARTYINDEX1, charaindex);

	CHAR_setWorkChar( charaindex, CHAR_WORKWALKARRAY, "");

	CHAR_setWorkInt( charaindex, CHAR_WORKPARTYMODE, CHAR_PARTY_CLIENT);

	CHAR_setWorkInt( charaindex, CHAR_WORKPARTYINDEX1, toindex);

	if( firstflg ) {
		CHAR_sendStatusString( toindex, "N0");
	}


	for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
		int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
		if( CHAR_CHECKINDEX(index)) {
				snprintf( c, sizeof(c), "N%d", i);
				CHAR_sendStatusString( charaindex, c);
		}
	}

	snprintf( buf,sizeof( buf), "%s 加入團隊！",
			  CHAR_getChar( charaindex, CHAR_NAME));

	for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
		int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
		if( CHAR_CHECKINDEX(index)) {
			if( index != charaindex ) {
				snprintf( c, sizeof(c), "N%d", parray);
				CHAR_sendStatusString( index, c);
				CHAR_talkToCli( index, -1, buf, CHAR_COLORYELLOW);
			}
			else {
				CHAR_talkToCli( index, -1, "加入團隊！", CHAR_COLORYELLOW);
			}
		}
	}
}
/*------------------------------------------------------------
 * パーティに入ろうとする。
 ------------------------------------------------------------*/
BOOL CHAR_JoinParty( int charaindex )
{

	int     result = -1;
	int     x,y;
	OBJECT  object;
	int     found = FALSE;
	int     fd;
	int		cnt;
	int	i;

	fd = getfdFromCharaIndex( charaindex );
	if( fd == -1 ) {
		print( "%s : %d err\n", __FILE__, __LINE__);
		return FALSE;
	}

        /* 自分がパーティ組んでたら駄目 */
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE ) {
		lssproto_PR_send( fd, 1, FALSE);
		return FALSE;
	}

	/* 目の前の座標を得る */
	CHAR_getCoordinationDir( CHAR_getInt( charaindex, CHAR_DIR ) ,
							 CHAR_getInt( charaindex , CHAR_X ),
							 CHAR_getInt( charaindex , CHAR_Y ) ,
							 1 , &x , &y );

	/* 初期化する */
	for( i = 0; i < CONNECT_WINDOWBUFSIZE; i ++ ) {
        CONNECT_setJoinpartycharaindex(fd,i,-1);
    }
	cnt = 0;

	/*自分の目の前のキャラを取得する */

	for( object = MAP_getTopObj( CHAR_getInt( charaindex, CHAR_FLOOR),x,y) ;
		 object ;
		 object = NEXT_OBJECT(object ) )
	{
		int toindex;
		int parray;
		int objindex = GET_OBJINDEX(object);
		int targetindex = -1;

		/* キャラクターじゃない */
		if( OBJECT_getType( objindex) != OBJTYPE_CHARA) continue;
		toindex = OBJECT_getIndex( objindex);
	
                // shan begin
                if( CHAR_getInt(charaindex, CHAR_FMINDEX) > 0 && CHAR_getInt(toindex, CHAR_FMINDEX) >0){
                    for( i = 0; i < FAMILY_FMPKFLOOR; i++){
                        if( fmpkflnum[i].fl == CHAR_getInt( charaindex, CHAR_FLOOR) )
                            if( CHAR_getInt(charaindex, CHAR_FMINDEX) != CHAR_getInt(toindex, CHAR_FMINDEX) ){
                                lssproto_PR_send( fd, 1, FALSE);
                                return FALSE;
                            }
                    }
                }
                // shan end
	
		/* プレイヤーの時 */
		if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER ){
			found = TRUE;
			/* 相手が子だったら親を引っ張り出す */
			if( CHAR_getWorkInt( toindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT ) {
				targetindex = CHAR_getWorkInt( toindex, CHAR_WORKPARTYINDEX1);
				if( !CHAR_CHECKINDEX( targetindex) ) {
					print( " %s:%d err\n", __FILE__, __LINE__);
					continue;
				}
				if( CHAR_getInt( targetindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS) {
					continue;
				}
			}
			else {
				targetindex = toindex;
			}

			/* （親と）１歩以内にいるか */
			if( NPC_Util_CharDistance( charaindex, targetindex ) > 1) {
				continue;
			}

			/* 戦闘中はでない事。*/
			if( CHAR_getWorkInt( targetindex, CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE ){
				continue;
			}
			/* 仲間許可モードか */
			if( !CHAR_getFlg( targetindex, CHAR_ISPARTY) ) continue;

#ifdef _ANGEL_SUMMON
			if( CHAR_getWorkInt( targetindex, CHAR_WORKANGELMODE) == TRUE) {
				CHAR_talkToCli( charaindex, -1, "使者不可以當領隊。", CHAR_COLORYELLOW);
				continue;
			}
#endif
#ifdef _ESCAPE_RESET // 使用惡寶逃跑後x分鐘內不可與人組隊
			if( getStayEncount( getfdFromCharaIndex(targetindex) ) ) {
				//print(" 惡寶中組隊 ");
				if( time(NULL) - CHAR_getWorkInt( targetindex, CHAR_WORKLASTESCAPE) < 5*60 ) {
					//print(" 惡寶逃跑後組隊 ");
					CHAR_talkToCli( charaindex, -1, "此人暫時不可以當領隊。", CHAR_COLORYELLOW);
					continue;
				}
			}
#endif
		}
		/* マンモスバスがいる時は，人間より優先する。 */
		else if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS ) {
			targetindex = toindex;
			cnt = 0;
			if( !NPC_BusCheckJoinParty( toindex, charaindex, TRUE)) {
				/* 条件を満たさなかった。仲間入るのは終わる。人間の処理もしない。
				 * ややこしいので。
				 */
				break;
			}
			{	// Arminius 7.10 Airplane
			  int busimg=CHAR_getInt(toindex, CHAR_BASEIMAGENUMBER);
		          if ((busimg!=100355) && (busimg!=100461)) {
		            CHAR_setInt(charaindex,CHAR_BASEIMAGENUMBER,busimg);
			    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex ,
				CHAR_WORKOBJINDEX ));
				// Robin debug 01/11/21
				if( CHAR_getInt( charaindex, CHAR_RIDEPET) != -1 ) {
					CHAR_setInt( charaindex, CHAR_RIDEPET, -1);
					CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_RIDEPET);
				}
			/*
			    CHAR_sendPMEToArroundCharacterFLXY(charaindex,
			        CHAR_getInt( charaindex, CHAR_FLOOR),
			        CHAR_getInt( charaindex, CHAR_X),
			        CHAR_getInt( charaindex, CHAR_Y),
			        0,1,CHAR_getInt( charaindex, CHAR_PETMAILEFFECT)
			        );
			*/
		          }
		        }
		}
		/* プレイヤー又はマンモスバス以外は無視する */
		else {
			continue;
		}
		/* 相手パーティの人数はＯＫか？ */
		parray = CHAR_getEmptyPartyArray( targetindex) ;
		if( parray == -1 ) continue;

		/* ここまでくればＯＫ */
        CONNECT_setJoinpartycharaindex( fd,cnt,toindex);
		cnt++;
		if( cnt == CONNECT_WINDOWBUFSIZE ) break;
		
		/* マンモスバス発見しだい，ループを抜ける。 */
		if( CHAR_getInt( targetindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS ) break;

	}

	if( cnt == 0 ) {
		if( found == TRUE) {
			CHAR_talkToCli( charaindex, -1, "無法加入團隊。", CHAR_COLORYELLOW);
		}
		result = FALSE;
	}else if( cnt == 1 ) {
#ifdef _DEATH_CONTEND
		int toindex = CONNECT_getJoinpartycharaindex( fd, 0);
		if(CHAR_getInt(toindex,CHAR_PKLISTTEAMNUM) == -1 && CHAR_getInt(charaindex,CHAR_PKLISTTEAMNUM) == -1){
		}else if( CHAR_getInt( charaindex, CHAR_PKLISTLEADER ) > 0 ||
			CHAR_getInt( toindex, CHAR_PKLISTTEAMNUM) < 0 ||
			CHAR_getInt( charaindex, CHAR_PKLISTTEAMNUM) < 0 ||
			CHAR_getInt( toindex, CHAR_PKLISTTEAMNUM) != CHAR_getInt( charaindex, CHAR_PKLISTTEAMNUM) ||
			CHAR_getInt(toindex,CHAR_WHICHTYPE) != CHAR_TYPEPLAYER){

			CHAR_talkToCli( charaindex, -1, "隊伍不同，無法加入團隊。", CHAR_COLORYELLOW);
			result = FALSE;
		}else{
#endif
			CHAR_JoinParty_Main( charaindex, CONNECT_getJoinpartycharaindex(fd,0));
			result = TRUE;
#ifdef _DEATH_CONTEND
		}
#endif
	}else {
		int		strlength;
		char	msgbuf[1024];
		char	escapebuf[2048];
#ifdef _DEATH_CONTEND
		int toindex = CONNECT_getJoinpartycharaindex( fd, 0);
		if(CHAR_getInt(toindex,CHAR_PKLISTTEAMNUM) == -1 && CHAR_getInt(charaindex,CHAR_PKLISTTEAMNUM) == -1){
		}else if( CHAR_getInt( charaindex, CHAR_PKLISTLEADER ) > 0 ||
			CHAR_getInt( toindex, CHAR_PKLISTTEAMNUM) < 0 ||
			CHAR_getInt( charaindex, CHAR_PKLISTTEAMNUM) < 0 ||
			CHAR_getInt( toindex, CHAR_PKLISTTEAMNUM) != CHAR_getInt( charaindex, CHAR_PKLISTTEAMNUM) ||
			CHAR_getInt(toindex,CHAR_WHICHTYPE) != CHAR_TYPEPLAYER){

			CHAR_talkToCli( charaindex, -1, "隊伍不同，無法加入團隊。", CHAR_COLORYELLOW);
			result = FALSE;
		}
#endif
		strcpy( msgbuf, "1\n和誰組成團隊呢？\n");
		strlength = strlen( msgbuf);
		for( i = 0;
             CONNECT_getJoinpartycharaindex( fd,i ) != -1
			&& i< CONNECT_WINDOWBUFSIZE;
			i ++ ){
			char	*a = CHAR_getChar(
                CONNECT_getJoinpartycharaindex(fd,i) , CHAR_NAME);
			char	buf[256];
			snprintf( buf, sizeof( buf),"%s\n", a);
			if( strlength + strlen( buf) > arraysizeof( msgbuf)){
				print( "%s:%d視窗訊息buffer不足。\n",
						__FILE__,__LINE__);
				break;
			}
			strcpy( &msgbuf[strlength], buf);
			strlength += strlen(buf);
		}
		lssproto_WN_send( fd, WINDOW_MESSAGETYPE_SELECT,
						WINDOW_BUTTONTYPE_CANCEL,
						CHAR_WINDOWTYPE_SELECTPARTY,
						-1,
					makeEscapeString( msgbuf, escapebuf, sizeof(escapebuf)));


	}

	if( result != -1 ) {
		lssproto_PR_send( fd, 1, result);
	}

	return result;
}

static BOOL CHAR_DischargePartySub( int charaindex, int msgflg)
{
	char buf[64], c[3];
	int toindex,flg,i;
#ifdef _ITEM_QUITPARTY
    int j = 0,k;
#endif
	

	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_LEADER ) {
		int pindex, airplaneflag=0;
		// Arminius 7.10 Airplane
		if( CHAR_getInt(charaindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS ) {
		  if ((CHAR_getInt(charaindex, CHAR_BASEIMAGENUMBER) !=100355) &&
		      (CHAR_getInt(charaindex, CHAR_BASEIMAGENUMBER) !=100461)){
		    airplaneflag=1;
		  }
		}
		for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
			pindex = CHAR_getWorkInt( charaindex, i + CHAR_WORKPARTYINDEX1);
			if( CHAR_CHECKINDEX( pindex) ) {
				int     fd = getfdFromCharaIndex( pindex );
				CHAR_setWorkInt( pindex, CHAR_WORKPARTYINDEX1, -1);
				CHAR_setWorkInt( pindex, CHAR_WORKPARTYMODE, CHAR_PARTY_NONE);
				if( msgflg ){
					CHAR_talkToCli( pindex, -1, "團隊已解散！", CHAR_COLORYELLOW);
#ifdef _ITEM_QUITPARTY
					// won fix
	                for( j=0;j<CHAR_MAXITEMHAVE;j++ ){
						int del_item_index = CHAR_getItemIndex( pindex , j );
                        if( ITEM_CHECKINDEX(del_item_index) ){ //格子內有道具
                            for( k=0;k<itemquitparty_num;k++ ){
							    if( ITEM_getInt( del_item_index, ITEM_ID) == atoi(Disappear_Item[k].string) ){ //若等於所設定的道具ID
			                        CHAR_setItemIndex( pindex, j, -1); //格子內道具消失
									ITEM_endExistItemsOne( del_item_index );
				                    CHAR_sendItemDataOne( pindex, j);
								}
							}
						}
					}
#endif
				}
				if( fd != -1 ) {
					lssproto_PR_send( fd, 0, 1);
				}
				// Arminius 7.10 Airplane
				if (airplaneflag && (CHAR_getInt(pindex,CHAR_WHICHTYPE)!=CHAR_TYPEBUS)) {
				  int bi,bbi,ii,category;
				  bbi=CHAR_getInt(pindex,CHAR_BASEBASEIMAGENUMBER);
				  ii=CHAR_getItemIndex(pindex,CHAR_ARM);
				  if (!ITEM_CHECKINDEX(ii))
				    category=ITEM_FIST;
				  else
				    category=ITEM_getInt(ii,ITEM_TYPE);
				  bi=CHAR_getNewImagenumberFromEquip(bbi,category);
				  if (bi==-1) bi=bbi;
				  CHAR_setInt(pindex,CHAR_BASEIMAGENUMBER,bi);
				  // Robin 0810 debug
				  CHAR_complianceParameter( pindex );				  
				  CHAR_sendCToArroundCharacter(CHAR_getWorkInt(pindex ,
				    CHAR_WORKOBJINDEX));
				}
			}
			CHAR_setWorkInt( charaindex, i + CHAR_WORKPARTYINDEX1, -1);
		}
		CHAR_sendLeader( CHAR_getWorkInt( charaindex, CHAR_WORKOBJINDEX), 0);
	}else if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT ) {
		int		myarray = -1;
		int     fd = getfdFromCharaIndex( charaindex );
		CHAR_setWorkInt( charaindex, CHAR_WORKPARTYMODE, CHAR_PARTY_NONE);
		toindex = CHAR_getWorkInt( charaindex, CHAR_WORKPARTYINDEX1);
		if( !CHAR_CHECKINDEX(toindex ) ) return FALSE;
		if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEBUS ) {
			NPC_BusCheckAllowItem( toindex, charaindex, TRUE);
		  // Arminius 7.9 Airplane
		  if ((CHAR_getInt( toindex, CHAR_BASEIMAGENUMBER) !=100355) &&
		      (CHAR_getInt( toindex, CHAR_BASEIMAGENUMBER) !=100461)){
		    int bi,bbi,ii,category;
		    
		    bbi=CHAR_getInt(charaindex,CHAR_BASEBASEIMAGENUMBER);
		    ii=CHAR_getItemIndex(charaindex,CHAR_ARM);
		    if (!ITEM_CHECKINDEX(ii))
		      category=ITEM_FIST;
		    else
		      category=ITEM_getInt(ii,ITEM_TYPE);
		    bi=CHAR_getNewImagenumberFromEquip(bbi,category);
		    if (bi==-1) bi=bbi;
		    CHAR_setInt(charaindex,CHAR_BASEIMAGENUMBER,bi);

		    // Robin 0810 debug
		    CHAR_complianceParameter( charaindex );		    

		    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));
		    if(CHAR_getWorkInt(toindex,CHAR_NPCWORKINT5)==1) {
		      if( CHAR_getInt( charaindex, CHAR_LASTTALKELDER)>=0){
		        int fl,x,y;
		        CHAR_getElderPosition( CHAR_getInt( charaindex, CHAR_LASTTALKELDER),
		          &fl, &x, &y );
		        CHAR_warpToSpecificPoint(charaindex, fl, x, y);
		      }
		    }
		  }
		}
		CHAR_setWorkInt( charaindex, CHAR_WORKPARTYINDEX1, -1);
		for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
			int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
			if( CHAR_CHECKINDEX(index) ){
				if( index == charaindex) {
					myarray = i;
					break;
				}
			}
		}
		if( myarray == CHAR_PARTYMAX) {
			print( "DischargeParty(): 真奇怪！");
			return FALSE;
		}
		CHAR_setWorkInt( toindex, CHAR_WORKPARTYINDEX1 + myarray, -1);
		snprintf( buf,sizeof( buf), "%s 脫離團隊！",
				  CHAR_getChar( charaindex, CHAR_NAME));
		if( msgflg ){
			CHAR_talkToCli( charaindex, -1, "脫離團隊！", CHAR_COLORYELLOW);
#ifdef _ITEM_QUITPARTY
			// won fix
	        for( i=0;i<CHAR_MAXITEMHAVE;i++ ){
				int del_item_index = CHAR_getItemIndex( charaindex , j );
				if( ITEM_CHECKINDEX(del_item_index) ){ //格子內有道具
                   for( j=0;j<itemquitparty_num;j++ ){
					    if( ITEM_getInt( del_item_index, ITEM_ID) == atoi(Disappear_Item[j].string) ){ //若等於所設定的道具ID
			                CHAR_setItemIndex( charaindex, i, -1); //格子內道具消失
							ITEM_endExistItemsOne( del_item_index );
			                CHAR_sendItemDataOne( charaindex, i);
						}
					}
				}
			}
#endif
		}
		snprintf( c, sizeof(c), "N%d", myarray);
		if( fd != -1 ) {
			lssproto_PR_send( fd, 0, 1);
		}
		for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
			int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
			if( CHAR_CHECKINDEX(index) ){
#ifdef _ITEM_QUITPARTY
				// won fix
	            for( j=0;j<CHAR_MAXITEMHAVE;j++ ){
					int del_item_index = CHAR_getItemIndex( index , j );	
                    if( ITEM_CHECKINDEX(del_item_index) ){ //格子內有道具
                        for( k=0;k<itemquitparty_num;k++ ){
						    if( ITEM_getInt( del_item_index, ITEM_ID) == atoi(Disappear_Item[k].string) ){ //若等於所設定的道具ID
			                    CHAR_setItemIndex( index, j, -1); //格子內道具消失
								ITEM_endExistItemsOne( del_item_index );
						        CHAR_sendItemDataOne( index, j);
							}
						}
					}
				}
#endif
				if( msgflg ){
					CHAR_talkToCli( index, -1, buf, CHAR_COLORYELLOW);
				}
				CHAR_sendStatusString( index, c);
			}
		}
		flg = FALSE;
		for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
			int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
			if( CHAR_CHECKINDEX(index) ){
				flg = TRUE;
				break;
			}
		}
		if( !flg) {
			CHAR_setWorkInt( toindex, CHAR_WORKPARTYMODE, CHAR_PARTY_NONE);
			CHAR_sendLeader( CHAR_getWorkInt( toindex, CHAR_WORKOBJINDEX), 0);
		}else {

			POINT	start,end;
			int 	previndex = toindex;
			end.x = CHAR_getInt( charaindex, CHAR_X);
			end.y = CHAR_getInt( charaindex, CHAR_Y);
			for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
				int index = CHAR_getWorkInt( toindex, i + CHAR_WORKPARTYINDEX1);
				if( CHAR_CHECKINDEX( index) ) {
					if( NPC_Util_CharDistance( index, previndex) > 1) {
						int		parent_dir;
						start.x = CHAR_getInt( index, CHAR_X);
						start.y = CHAR_getInt( index, CHAR_Y);
						parent_dir = NPC_Util_getDirFromTwoPoint( &start,&end );
						end = start;
						if( parent_dir != -1 ) {
							CHAR_walk( index, parent_dir, 0);
						}
					}
					previndex = index;
				}
			}
		}
		
	}

	return TRUE;
}
BOOL CHAR_DischargeParty( int charaindex, int flg)
{
	return CHAR_DischargePartySub( charaindex, 1);
}

BOOL CHAR_DischargePartyNoMsg( int charaindex)
{
	return CHAR_DischargePartySub( charaindex, 0);
}


/*------------------------------------------------------------
 * 自分がリーダーかどうかを送信する。
 ------------------------------------------------------------*/
void CHAR_sendLeader( int objindex, int leader)
{
	int		opt[1];
	opt[0] = leader;
	CHAR_sendWatchEvent( objindex,CHAR_ACTLEADER,opt,1,TRUE);
}
/*------------------------------------------------------------
 * 仲間を順番（CHAR_WORKPARTYINDEX)指定でキャラindexを引っ張る。
 * 自分が親でも子でもＯＫ。
 ------------------------------------------------------------*/
int CHAR_getPartyIndex( int index, int num)
{
	int	nindex = -1;

	/* 仲間のインデックスを取得 */
	/* 親の場合 */
	if( CHAR_getWorkInt( index, CHAR_WORKPARTYMODE) == CHAR_PARTY_LEADER ) {
		nindex = CHAR_getWorkInt( index, CHAR_WORKPARTYINDEX1 + num );
	}
	/* 子の場合 */
	else {
		int oyaindex = CHAR_getWorkInt( index, CHAR_WORKPARTYINDEX1);
		if( CHAR_CHECKINDEX( oyaindex)) {
			nindex = CHAR_getWorkInt( oyaindex, CHAR_WORKPARTYINDEX1+num);
		}
	}
	return nindex;
}
/*------------------------------------------------------------
 * メッセージを送信する。
 * 仲間がいればその仲間にもメッセージを送信する。
 ------------------------------------------------------------*/
void CHAR_talkToCliAndParty( int talkedcharaindex,int talkcharaindex,
					 char* message, CHAR_COLOR color )
{
	int		i;
	/* まず自分 */
	CHAR_talkToCli( talkedcharaindex, talkcharaindex, message, color);

	for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
		int index = CHAR_getPartyIndex( talkedcharaindex, i);
		if( CHAR_CHECKINDEX( index) &&
			index != talkedcharaindex)
		{
			CHAR_talkToCli( index, talkcharaindex, message, color);
		}
	}
}

