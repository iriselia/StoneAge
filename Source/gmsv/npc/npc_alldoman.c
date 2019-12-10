#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "npcutil.h"
#include "item.h"
#include "readmap.h"
#include "enemy.h"
#include "chatmagic.h"
#include "log.h"
#include "npc_itemchange.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npc_eventaction.h"
#include "npc_charm.h"

#ifdef _ALLDOMAN
#include "npc_alldoman.h"

#ifdef _FIX_ALLDOMAN
void Send_S_herolist( int ti );
#define MAX_HERO_LIST 50
typedef struct _tagHerolistBase
{
	char strings[3][64];
	int intdata[3];
	int use;
}HerolistBase;
HerolistBase Herolist[MAX_HERO_LIST];
#else
#define MAX_HERO_COLUMN 7
#define MAX_HERO_LIST 100
char Herolist[MAX_HERO_LIST][MAX_HERO_COLUMN][72];
#endif



int sort[MAX_HERO_LIST][2];
int countnum = -1 ;
int showpage = 1 ;
int totalpage = 1;

static void NPC_Alldoman_selectWindow( int meindex, int talker, int seqno,char *data);

void NPC_AlldomanWindowTalked(int meindex, int talkerindex, int seqno, int select,char *data)
{
	char buf2[256];
	char buf[256];
    int i;
    int playernum = CHAR_getPlayerMaxNum();
	//andy_log
	print( "seqno:%d\n", seqno);
	switch(seqno)
	{
	case NPC_ALLDOMAN_MAIN_WND:
		if ( select == WINDOW_BUTTONTYPE_NEXT ) {
			showpage ++ ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex , 4 ,"");
		}
		if ( select == WINDOW_BUTTONTYPE_OK ) {
			//CHAR_talkToCli(talkerindex,meindex,"看清楚了！",CHAR_COLORRED);
			showpage = 1 ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 5 ,"");
		}
		break;
	case NPC_ALLDOMAN_SELECT_WND:
		if(atoi(data)==2) {
			//CHAR_talkToCli(talkerindex,meindex,"１！",CHAR_COLORRED);
			if ( CHAR_getInt( talkerindex , CHAR_FLOOR ) != 8200 ) {
				CHAR_talkToCli(talkerindex,-1,"這裡只能給真正的英雄刻寫！",CHAR_COLORRED);
				break;
			}
			if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) > 132 ||
				CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) <= 0 ) {
				CHAR_setWorkInt( talkerindex  , CHAR_WORKHEROFLOOR , 0 ) ;
				CHAR_talkToCli(talkerindex,-1,"這裡只能給真正的英雄刻寫！",CHAR_COLORRED);
				break;
			}
			sprintf( buf2 , "你拿出了打敗守靈者%3d的證明，試圖將自己的姓名刻上石碑！" ,	 CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) );
			CHAR_talkToCli(talkerindex,-1,buf2,CHAR_COLORRED);
			if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) == 132 ) {
				sprintf( buf2 , "%s" , "尼斯大陸英雄" ) ; 
				//到達頂層對全星系廣播
				sprintf( buf , "%s征服了英雄戰場，由風之精靈將這個消息通知全世界，新的英雄誕生了!!" , CHAR_getChar( talkerindex, CHAR_NAME ) );
				for( i = 0 ; i < playernum ; i++) {
					if( CHAR_getCharUse(i) != FALSE ) {
						CHAR_talkToCli( i, -1, buf, CHAR_COLORBLUE2);
					}
				}
			}
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 130 )
				sprintf( buf2 , "%s" , "尼斯大陸戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 125 )
				sprintf( buf2 , "%s" , "尼斯大陸勇士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 120 )
				sprintf( buf2 , "%s" , "薩伊那斯英雄" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 115 )
				sprintf( buf2 , "%s" , "斯巴達戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 110 )
				sprintf( buf2 , "%s" , "薩姆吉爾戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 100 )
				sprintf( buf2 , "%s" , "瑪麗那絲戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 80 )
				sprintf( buf2 , "%s" , "卡坦戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 60 )
				sprintf( buf2 , "%s" , "霍特爾戰士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 40 )
				sprintf( buf2 , "%s" , "降魔勇士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 20 )
				sprintf( buf2 , "%s" , "聖靈勇士" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 1 )
				sprintf( buf2 , "%s" , "初犢勇士" ) ; 
			else
				sprintf( buf2 , " " );
			saacproto_UpdataStele_send ( 				
				acfd , 
				CHAR_getChar( talkerindex , CHAR_CDKEY ),
				CHAR_getChar( talkerindex , CHAR_NAME ) , 
				buf2 , 
				CHAR_getInt( talkerindex , CHAR_LV ) , 
				CHAR_getInt( talkerindex , CHAR_TRANSMIGRATION) , 
				10 	, 
				CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) 
				) ; 
			//清除WorkFloor
			CHAR_setWorkInt( talkerindex  , CHAR_WORKHEROFLOOR , 0 ) ;
		}
		if(atoi(data)==4) {
			//CHAR_talkToCli(talkerindex,meindex,"２！",CHAR_COLORRED);
			showpage = 1 ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"");
		}
		if(atoi(data)==6) {
			//CHAR_talkToCli(talkerindex,meindex,"３！",CHAR_COLORRED);
		}
		break;
	case NPC_ALLDOMAN_LIST_WND:
		if( select == WINDOW_BUTTONTYPE_OK ){
			print("\nSyu log FloorWorkint => %d" , CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ));
		}
		if( select == WINDOW_BUTTONTYPE_PREV && showpage > 1 ) {
			showpage -- ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"" );
		}
		if( select == WINDOW_BUTTONTYPE_NEXT && showpage < totalpage ) {
			showpage ++ ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"");
		}
		break;
	}
}

static void NPC_Alldoman_selectWindow( int meindex, int talker, int num, char *data)
{
	 int buttontype = 0,windowtype = 0,windowno = 0 , i ;
     int fd = getfdFromCharaIndex( talker);
	 char token[1024];
	 char token2[100 * 100];


	 //andy_log
	 print( "NPC_Alldoman_selectWindow( num:%d)\n", num);
     switch(num)
	 {
	 case 4:
		 if ( showpage == 1 ) {
			 buttontype = WINDOW_BUTTONTYPE_NEXT;
			 sprintf( token, "４８５４４７聖餅時，吉魯出現大批機暴橫行，所到之處皆無一倖免，尼斯各村的長老達成共識，調集各村勇士，前往消滅這群發狂的機暴，這群勇士在海底隧道入口跟這群機暴激戰，不料，實力相差懸殊，勇士們傷亡慘重，殘餘的勇士，選擇在霍特爾海底通路的入口做最");
		 }
		 else if ( showpage == 2 ) {
			 buttontype = WINDOW_BUTTONTYPE_NEXT;
			 sprintf( token, "後的的抵抗，各村長老召集大批人力，封死所有通往吉魯的海底通道，終於在大批機暴湧入前，將入口封死，此地為４８５５５０聖餅時所建造，為紀念４８５４４７聖餅時所犧牲的勇士，立此石碑，建此地洞，命名為“英雄戰場”，望後世人效法先烈的行徑，達到強種強");
		 }
		 else if ( showpage == 3) {
			 buttontype = WINDOW_BUTTONTYPE_OK;
			 sprintf( token, "族為目的，讓尼斯大陸的人們生生不息，上面的英雄是最近勇闖“英雄戰場”所留下來的，特此詔告全尼斯英雄！\n\n                      英雄戰場製作委員會");
		 }
		 windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		 windowno = NPC_ALLDOMAN_MAIN_WND;
		 lssproto_WN_send(fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX), token );
		 break;
	 case 5:
		 buttontype = WINDOW_BUTTONTYPE_NONE;
		 windowtype = WINDOW_MESSAGETYPE_SELECT;
		 windowno = NPC_ALLDOMAN_SELECT_WND;
		 sprintf(token,
			"0\n\n　　　　　　　　      刻寫石碑"
			 "\n\n　　　      　　　　檢視其他勇者"
			 "\n\n　　　　　      　　　　取消"
			 );
		 lssproto_WN_send( fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),token);
		 break;
	 case 6:
		 if ( totalpage != 1 && showpage == 1 ) 
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_NEXT;
		 else if ( totalpage == 1 )
			 buttontype = WINDOW_BUTTONTYPE_OK ; 
		 else if ( totalpage != 1 && showpage == totalpage )
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_PREV;
		 else if ( totalpage != 1 && showpage != 1 && showpage != totalpage ) 
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT;
		 windowtype = WINDOW_MESSAGETYPE_WIDEMESSAGEANDLINEINPUT;
		 windowno = NPC_ALLDOMAN_LIST_WND;
		 sprintf ( token2 , "            姓名          頭銜    樓層    等級    轉生\n");
		 for ( i=0+(showpage-1 )*15; i<showpage*15; i++) {
#ifdef _FIX_ALLDOMAN
			if( i >= MAX_HERO_LIST || i<0 ) break;
			if( Herolist[i].use == 0 ) continue;
			sprintf( token , "%16s %13s %7d %7d %7d\n" , 
				Herolist[i].strings[1], Herolist[i].strings[2],
				Herolist[i].intdata[3], Herolist[i].intdata[0],
				Herolist[i].intdata[1] );
			strcat( token2, token);
#else
			 if( atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][6] ) == 0 || 
				 atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][3] ) == 0 )
				 continue ; 
			sprintf( token , "%16s %13s %7d %7d %7d\n" , 
				Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][1] , 
				Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][2] , 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][6] ), 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][3] ), 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][4] ) 
				); 
			strcat ( token2 , token ) ; 
			if ( i + 1 == countnum )
				break;
#endif
		 }
		 lssproto_WN_send( fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),token2);
		 break;
    }
}

static int loadherolist = 0;
BOOL NPC_AlldomanInit( int meindex )
{
//andy_log
//print( "\n\nNPC_AlldomanInit( %d)\n\n", meindex);
	if( loadherolist == 0){
		loadherolist = 1;
		saacproto_UpdataStele_send ( acfd , "FirstLoad", "LoadHerolist" , "華義" , 0 , 0 , 0 , 999 ) ; 
	}
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEALLDOMAN);
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );
    return TRUE;
}

void NPC_AlldomanTalked( int meindex , int talker , char *msg ,int color )
{
     if( NPC_Util_CharDistance( talker, meindex ) > 1) return;
     if( NPC_Util_isFaceToFace( talker, meindex , 1 ) == FALSE ) return;
	 showpage = 1 ; 
	 NPC_Alldoman_selectWindow(meindex, talker , 4 ,"");
}
#ifdef _FIX_ALLDOMAN
void NPC_rePlaceHeroList( int fti, int tti)
{
	int i;
	if( tti<0 || tti>=MAX_HERO_LIST ) return;
	if( fti<0 || fti>=MAX_HERO_LIST ) return;
	for( i=0; i<3; i++ ){
		sprintf( Herolist[tti].strings[i], "%s", Herolist[fti].strings[i]);
	}
	for( i=0; i<4; i++ ){
		Herolist[tti].intdata[i] = Herolist[fti].intdata[i];
	}
	Herolist[tti].use = Herolist[fti].use;
	Herolist[fti].use = 0;
}
#endif
void NPC_AlldomanWriteStele ( char *token )
{
/*#ifdef _FIX_ALLDOMAN
	char buf1[512], buf2[256];
	int k=0, ti=-1, i;
	BOOL rePlace = FALSE;

	if( token==NULL || strlen( token)<=0 ) return;

//andy_log
print( "token:[%s]\n", token);

	if( getStringFromIndexWithDelim( token, ",", 1, buf1, sizeof( buf1) ) == FALSE ) return;
//andy_log
print( "buf1:[%s]\n", buf1);
	if( !strcmp( buf1, "A|") ){
		for( i=0; i<MAX_HERO_LIST; i++ )
			Herolist[i].use = 0;
	}else{
		rePlace = TRUE;
	}
	k = 2;
	while( getStringFromIndexWithDelim( token, ",", k, buf1, sizeof( buf1)) != FALSE ){
		k++;
		if( getStringFromIndexWithDelim( buf1, "|", 1, buf2, sizeof( buf2) ) == FALSE ) continue;
		ti = atoi( buf2);
		if( ti<0 || ti>=MAX_HERO_LIST ) continue;
		if( rePlace == TRUE ){
			for( i=MAX_HERO_LIST-1; i>ti; i++ ){
				NPC_rePlaceHeroList( i-1, i);
			}
			Herolist[ti].use = 0;
		}
		if( getStringFromIndexWithDelim( buf1, "|", 2, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[0], "%s", buf2 );
		if( getStringFromIndexWithDelim( buf1, "|", 3, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[1], "%s", buf2 );
		if( getStringFromIndexWithDelim( buf1, "|", 4, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[3], "%s", buf2 );

		if( getStringFromIndexWithDelim( buf1, "|", 5, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[0] = atoi( buf2);
		if( getStringFromIndexWithDelim( buf1, "|", 6, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[1] = atoi( buf2);
		if( getStringFromIndexWithDelim( buf1, "|", 7, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[3] = atoi( buf2);
		Herolist[ti].use = 1;
//andy_log
print( "ti:[%d]\n", ti);
	}
	//andy_log
	for( i=0; i<MAX_HERO_LIST; i++){
		if( Herolist[i].use == 0 ) continue;
		print( "hero[%d]:[%s,%s,%s,%d,%d,%d,%d]\n", i,
			Herolist[i].strings[0], Herolist[i].strings[1],
			Herolist[i].strings[2],
			Herolist[i].intdata[0], Herolist[i].intdata[1],
			Herolist[i].intdata[2], Herolist[i].intdata[3] );
	}
#else*/
	int linenum = 0 , i , flag , temp , temp1 , j  ; 
	int lens=0;
	char *addr;
	if( token == NULL || (lens = strlen( token)) <= 0 ) return;
	while( linenum < 100 ){
		for ( i = 0 ; i < MAX_HERO_COLUMN ; i ++ ) {
			if( getStringFromIndexWithDelim( token, "|", i + 1 , Herolist[ linenum ][ i ] , sizeof( Herolist[ linenum ][ i ] ) ) == FALSE ){
#ifdef _ALLDOMAN_DEBUG
				countnum = 0 ; 
	            for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		            sort[i][0] = atoi ( Herolist[i][6] ) ;
		            sort[i][1] = i ; 
		            if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			            countnum ++ ; 
				}

	            for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		            flag = 0 ; 
		            for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			            if ( sort[j][0] > sort[j+1][0] ){
				            flag = 1 ; 
				            temp = sort[j][0];
				            temp1 = sort[j][1];
				            sort[j][0] = sort[j + 1][0];
				            sort[j][1] = sort[j + 1][1];
				            sort[j+1][0] = temp;
				            sort[j+1][1] = temp1;
						}
					}
		            if ( flag != 1 ) 
			            break;
				}
                totalpage = countnum / 15 ;
	            if ( ( countnum % 15 ) != 0 )
		            totalpage ++;
#endif
				return;
			}
		}
		linenum ++;
//#endif

		if ( (addr = strstr ( token , "\n" )) == NULL ) break;
		strcpy ( token , addr + 1 ) ;
		if( token == NULL || (lens = strlen( token)) <= 0 ) {
#ifdef _ALLDOMAN_DEBUG	    
			countnum = 0 ; 
	        for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		        sort[i][0] = atoi ( Herolist[i][6] ) ;
		        sort[i][1] = i ; 
		        if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			        countnum ++ ; 
			}
	        for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		        flag = 0 ; 
		        for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			        if ( sort[j][0] > sort[j+1][0] ){
				        flag = 1 ; 
				        temp = sort[j][0];
				        temp1 = sort[j][1];
				        sort[j][0] = sort[j + 1][0];
				        sort[j][1] = sort[j + 1][1];
				        sort[j+1][0] = temp;
				        sort[j+1][1] = temp1;
					}
				}
		        if ( flag != 1 ) 
			        break;
			}
	        totalpage = countnum / 15 ;
	        if ( ( countnum % 15 ) != 0 )
		        totalpage ++;
#endif
	        return;
		}		
	}

	countnum = 0 ; 
	for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		sort[i][0] = atoi ( Herolist[i][6] ) ;
		sort[i][1] = i ; 
		if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			countnum ++ ; 
	}

	for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		flag = 0 ; 
		for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			if ( sort[j][0] > sort[j+1][0] ){
				flag = 1 ; 
				temp = sort[j][0];
				temp1 = sort[j][1];
				sort[j][0] = sort[j + 1][0];
				sort[j][1] = sort[j + 1][1];
				sort[j+1][0] = temp;
				sort[j+1][1] = temp1;
			}
		}
		if ( flag != 1 ) 
			break;
	}


	totalpage = countnum / 15 ;
	if ( ( countnum % 15 ) != 0 )
		totalpage ++;
//#endif
}

void NPC_Alldoman_S_WriteStele( char *ocdkey , char *oname , char *ncdkey , 
		   char *nname , char *title , int level , int trns , int floor ) 
{
#ifdef _FIX_ALLDOMAN
	return;
#else
	int i , j , temp , temp1 , flag ; 
	for ( i = 0 ; i < MAX_HERO_COLUMN ; i ++ ) {
		if ( ( strcmp( Herolist[i][0] , ocdkey ) == 0 ) && ( strcmp( Herolist[i][1] , oname ) == 0 ) ) {
			sprintf( Herolist[i][0] , "%s" , ncdkey ) ; 
			sprintf( Herolist[i][1] , "%s" , nname ) ; 			
			sprintf( Herolist[i][2] , "%s" , title ) ; 			
			sprintf( Herolist[i][3] , "%d" , level ) ; 			
			sprintf( Herolist[i][4] , "%d" , trns ) ; 			
			sprintf( Herolist[i][6] , "%d" , floor ) ; 		
			break;
		}
	}
	countnum = 0 ; 
	for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		sort[i][0] = atoi ( Herolist[i][6] ) ;
		sort[i][1] = i ; 
		if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			countnum ++ ; 
	}
	for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		flag = 0 ; 
		for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			if ( sort[j][0] > sort[j+1][0] ){
				flag = 1 ; 
				temp = sort[j][0];
				temp1 = sort[j][1];
				sort[j][0] = sort[j + 1][0];
				sort[j][1] = sort[j + 1][1];
				sort[j+1][0] = temp;
				sort[j+1][1] = temp1;
			}
		}
		if ( flag != 1 ) 
			break;
	}
	totalpage = countnum / 15 ;
	if ( ( countnum % 15 ) != 0 )
		totalpage ++ ; 

#endif
}

#endif

