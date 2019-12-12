#include "version.h"
#include <ctype.h>

#include "object.h"
#include "char_base.h"
#include "char.h"
#include "item.h"
#include "util.h"
#include "handletime.h"
#include "anim_tbl.h"
#include "npc_door.h"
#include "lssproto_serv.h"
#include "npcutil.h"
#include "npccreate.h"
#include "log.h"


/*
 * 標準的な店のルーチン。by nakamura
 *
 * NPCARGUMENTに羅列したアイテムを無限に所有しているお店。
 * 最大SIMPLESHOP_MAXINFINITITEM種類無限に生成することができる。
 * 無限生成アイテムと同じ種類のアイテムを売ったら売ったやつが
 * 世界から消滅する。
 * また、プレイヤーからものを買いとることができるが、ある値段
 * (NPC_SIMPLESHOPMAXBUYPRICE)より高い金を支払わなくてはいけ
 * ない買取はできない。買いとったアイテムはその場でなくなる。
 * また、アイテムの設定でcostが設定されていないものも買いとれない。
 * 2人以上のプレイヤーが店にきたときは、後の客を優先する。そうする
 * ことにより店が永久に誰かを相手していることを防ぐ。
 *
 * タイプ名：SimpleShop
 * 扱うイベント: init, talked, specialtalked
 *
 *
 */
static void NPC_SimpleShopOpenShopWindow( int meindex, int cliindex,
                                          char *npcarg );
static void NPC_SimpleShopGetEscapedItemString( int shopindex,
                                                int cliindex, char *str );
static BOOL NPC_SimpleShopProcessBuyMsg( char *msg, int shop, int cli );
static BOOL NPC_SimpleShopProcessSellMsg( char *msg, int shop, int cli );

static BOOL NPC_LimitBuyInShopProcessSellMsg(char *msg,int shop,int cli);


static BOOL NPC_SimpleShopFillItem( int meindex, char *npcarg );
static int NPC_SimpleShopGetItemNum( int meindex );
static void NPC_SimpleShopNormalWindow( int meindex,
                                        int playerindex, char *str );
static int NPC_ShopWindowIndexToItemTableIndex( int charindex,
                                                int shopwindowindex );
static BOOL NPC_SimpleShopOpenShopOrNot( char *msg, char *openshopstr );
//static void NPC_SimpleShopSetLeakLevel( int meindex );

#define NPC_SIMPLESHOPMAXBUYPRICE 9999
#define SIMPLESHOPTALKBUFSIZE 256

#define SIMPLESHOP_FATALERRSTR "好痛！對不起,最近狀況不太佳。。。"

#define SIMPLESHOP_MAXINFINITITEM 40

enum{
    OPENSHOPTOKEN=1,
        MSGTOKEN,
        MAINTOKEN,
        BUYTOKEN,
        OKBUYTOKEN,
        SELLTOKEN,
        OKSELLTOKEN,
        POORTOKEN,
        ITEMFULLTOKEN,
        ANOTHERPLAYERTOKEN,
        RAREITEMTOKEN,
        THANKYOUTOKEN,
        ITEMLISTTOKEN,

        BUYINITEMLISTTOKEN,
        };
void NPC_SimpleShopTalked( int meindex, int talker, char *msg, int color )
{
#define SHOPRANGE 3
    char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
    if( CHAR_getInt( talker,CHAR_WHICHTYPE) != CHAR_TYPEPLAYER ){
        return;
    }
    if( NPC_Util_charIsInFrontOfChar( talker, meindex, SHOPRANGE )
        && ! CHAR_getFlg( talker,CHAR_ISDIE )  ){
        NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg));
        getStringFromIndexWithDelim( npcarg,"|", OPENSHOPTOKEN,token, sizeof(token));
        if( NPC_SimpleShopOpenShopOrNot( msg, token )
           && NPC_Util_isFaceToFace( meindex, talker, SHOPRANGE ) ){
            CHAR_setWorkInt( meindex, CHAR_WORKSHOPCLIENTINDEX, talker );
            NPC_SimpleShopOpenShopWindow( meindex, talker, npcarg );
        }else{
            int tokennum;
            int i;
            char tmp[NPC_UTIL_GETARGSTR_LINEMAX];
            getStringFromIndexWithDelim( npcarg,"|", MSGTOKEN,token, sizeof(token));
            tokennum = 1;
            for( i=0;token[i]!='\0';i++ ){
                if( token[i] == ',' ) tokennum++;
            }
            getStringFromIndexWithDelim( token,",", rand()%tokennum+1,tmp, sizeof(tmp));
            CHAR_talkToCli( talker, meindex, tmp, CHAR_COLORWHITE );
        }
    }
}

void NPC_SimpleShopSpecialTalked( int meindex, int talker, char *msg, int color )
{
    if( CHAR_getInt(talker,CHAR_WHICHTYPE) == CHAR_TYPEPLAYER
        && NPC_Util_isFaceToFace( meindex, talker, SHOPRANGE ) ){
        char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
        NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg));
        switch( tolower(msg[0]) ){
        case 's':
        {
            int     ret;
            if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX)!= talker ){
                getStringFromIndexWithDelim( npcarg,"|",ANOTHERPLAYERTOKEN,
                                             token, sizeof(token));
                NPC_SimpleShopNormalWindow( meindex, talker, token );
                return;
            }
            ret = getStringFromIndexWithDelim( msg, "|", 3, token, sizeof(token));
            if( ret == FALSE ){
                token[0] = 'e';
                token[1] = '\0';
            }
            switch( tolower(token[0])){
            case 'b':
                if( (ret=NPC_SimpleShopProcessBuyMsg(msg,meindex,talker))
                    <0){
                    switch( ret ){
                    case -2:
                        getStringFromIndexWithDelim( npcarg,"|",POORTOKEN,
                                                     token,sizeof(token));
                        NPC_SimpleShopNormalWindow( meindex,talker,token);
                        break;
                    case -3:
                        getStringFromIndexWithDelim( npcarg,"|",
                                                     ITEMFULLTOKEN,
                                                     token,sizeof(token));
                        NPC_SimpleShopNormalWindow(meindex,talker,token);
                        break;
                    case -1:
                    default:
                        NPC_SimpleShopNormalWindow(meindex, talker,
                                                   SIMPLESHOP_FATALERRSTR
                            );
                        break;
                    }
                }else{
                    NPC_SimpleShopFillItem( meindex, npcarg );
                    getStringFromIndexWithDelim(npcarg,"|",THANKYOUTOKEN,
                                                token, sizeof(token));
                    NPC_SimpleShopNormalWindow(meindex,talker,token);
                }
                break;
            case 's':
                if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX) != talker ){
                    getStringFromIndexWithDelim( npcarg,"|",
                                                 ANOTHERPLAYERTOKEN,
                                                 token, sizeof(token));
                    NPC_SimpleShopNormalWindow( meindex, talker, token );
                    return;
                }
                if((ret=NPC_SimpleShopProcessSellMsg(msg,meindex,talker)) <0 ){
                    switch(ret){
                    case -2:
                        getStringFromIndexWithDelim( npcarg,"|",
                                                     RAREITEMTOKEN,
                                                     token, sizeof(token) );
                        NPC_SimpleShopNormalWindow( meindex,talker,token);
                        break;
                    case -1:
                    default:
                        NPC_SimpleShopNormalWindow(meindex, talker,
                                                   SIMPLESHOP_FATALERRSTR );
                        break;
                    }
                }else{
                    getStringFromIndexWithDelim(npcarg,"|",THANKYOUTOKEN,
                                                token, sizeof(token));
                    NPC_SimpleShopNormalWindow(meindex,talker,token);
                }
                break;
            case 'e':
            default:
                CHAR_setWorkInt( meindex, CHAR_WORKSHOPCLIENTINDEX, -1 );
                break;
            }
            break;
        }
        case 'i':
            if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX)
                == talker ){
                NPC_SimpleShopOpenShopWindow( meindex, talker, npcarg );
            }
            break;
        default:
            break;
        }
    }
}

void NPC_LimitBuyInShopSpecialTalked( int meindex, int talker, char *msg, int color )
{
    if( CHAR_getInt(talker,CHAR_WHICHTYPE) == CHAR_TYPEPLAYER
        && NPC_Util_isFaceToFace( meindex, talker, SHOPRANGE ) ){
        char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
        NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg));
        switch( tolower(msg[0]) ){
        case 's':
        {
            int     ret;
            if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX) != talker ){
                getStringFromIndexWithDelim( npcarg,"|",
                                             ANOTHERPLAYERTOKEN,
                                             token, sizeof(token));
                NPC_SimpleShopNormalWindow( meindex, talker, token );
                return;
            }
            ret = getStringFromIndexWithDelim( msg, "|", 3, token, sizeof( token));
            if( ret == FALSE ){
                token[0] = 'e';
                token[1] = '\0';
            }
            switch( tolower(token[0])){
            case 'b':
                if( ( ret = NPC_SimpleShopProcessBuyMsg( msg, meindex, talker)) < 0){
                    switch( ret ){
                    case -2:
                        getStringFromIndexWithDelim( npcarg,"|",POORTOKEN,
                                                     token,sizeof(token));
                        NPC_SimpleShopNormalWindow( meindex,talker,token);
                        break;
                    case -3:
                        getStringFromIndexWithDelim( npcarg,"|",
                                                     ITEMFULLTOKEN,
                                                     token,sizeof(token));
                        NPC_SimpleShopNormalWindow(meindex,talker,token);
                        break;
                    case -1:
                    default:
                        NPC_SimpleShopNormalWindow(meindex, talker,
                                                   SIMPLESHOP_FATALERRSTR
                            );
                        break;
                    }
                }else{
                    NPC_SimpleShopFillItem( meindex, npcarg );
                    getStringFromIndexWithDelim(npcarg,"|",THANKYOUTOKEN,
                                                token, sizeof(token));
                    NPC_SimpleShopNormalWindow(meindex,talker,token);

                }
                break;
            case 's':
                if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX) != talker ){
                    getStringFromIndexWithDelim( npcarg,"|",
                                                 ANOTHERPLAYERTOKEN,
                                                 token, sizeof(token));
                    NPC_SimpleShopNormalWindow( meindex, talker, token );
                    return;
                }
                if((ret=NPC_LimitBuyInShopProcessSellMsg(
                    msg,meindex,talker))<0){
                    switch(ret){
                    case -2:
                        getStringFromIndexWithDelim( npcarg,"|",
                                                     RAREITEMTOKEN,
                                                     token,sizeof(token));
                        NPC_SimpleShopNormalWindow( meindex,talker,token);
                        break;
                    case -1:
                    default:
                        NPC_SimpleShopNormalWindow(meindex, talker, SIMPLESHOP_FATALERRSTR
                            );
                        break;
                    }
                }else{
                    getStringFromIndexWithDelim(npcarg,"|",THANKYOUTOKEN, token, sizeof(token));
                    NPC_SimpleShopNormalWindow(meindex,talker,token);

                }
                break;
            case 'e':
            default:
                CHAR_setWorkInt( meindex, CHAR_WORKSHOPCLIENTINDEX, -1 );
                break;
            }
            break;
        }
        case 'i':
            if( CHAR_getWorkInt(meindex,CHAR_WORKSHOPCLIENTINDEX)
                == talker ){
                NPC_SimpleShopOpenShopWindow( meindex, talker, npcarg );
            }
            break;
        default:
            break;
        }
    }
}





/*
 * 店がノーマルウインドウを出す。ウインドウのぼたんを押すと、
 * 最初に店に話しかけたときと同じ状態にもどる。
 * int meindex : 店のindex
 * int playerindex : ウインドウを出すプレイヤーのindex
 * char *str :
 */
static void NPC_SimpleShopNormalWindow( int meindex, int playerindex,
                                        char *str )
{
    int fd;
    int objindex;
    char tmp[512], *name;

    objindex = CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX);
    name = CHAR_getChar( meindex, CHAR_NAME );
    snprintf(tmp,sizeof(tmp),"W|%d|N|%s|123|%s", objindex, name, str );
    fd = getfdFromCharaIndex(playerindex);
    if( fd == -1 ) return;
    lssproto_TK_send(fd,-1,tmp,CHAR_COLORWHITE );
}

/*
 * 店ウインドウのインデクスから、アイテムテーブルのインデクスに変換
 * 引数
 * int charindex:キャラのインデクス
 * int shopwindowindex:店ウインドウのインデクス。0からはじまる。
 * 返り値
 * アイテムテーブルのインデクス。エラーは-1。
 */
static int NPC_ShopWindowIndexToItemTableIndex( int charindex,
                                                int shopwindowindex )
{
    int i, counter;

    counter = 0;
    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        if( ITEM_CHECKINDEX( CHAR_getItemIndex(charindex,i) ) ){
            if( shopwindowindex == counter ) return i;
            counter++;
        }
    }
    return -1;
}

static BOOL NPC_SimpleShopProcessBuyMsg(char *msg, int shopindex,
                                        int playerindex )
{
    int shopwindowindex, itemtableindex,itemind;
    int price, tmpgold;
    char buf[64];

    if(!CHAR_CHECKINDEX(shopindex)||!CHAR_CHECKINDEX(playerindex))
        return -1;

    if( !getStringFromIndexWithDelim( msg,"|",4,buf,sizeof(buf) )){
        return -1;
    }
    shopwindowindex = atoi(buf);
    itemtableindex=NPC_ShopWindowIndexToItemTableIndex( shopindex, shopwindowindex );
    itemind = CHAR_getItemIndex(shopindex,itemtableindex );
    if( ! ITEM_CHECKINDEX(itemind) ) return -1;
    price = ITEM_getInt( itemind, ITEM_COST )
        * NPC_Util_buyRate(playerindex);

    tmpgold = CHAR_getInt(playerindex,CHAR_GOLD);
    if( tmpgold < price ){
        return -2;
    }

    if( ! NPC_Util_moveItemToChar(playerindex,itemind,TRUE ) ){
        return -3;
    }
	CHAR_AddGold( playerindex, price );

    CHAR_sendStatusString( playerindex , "P");
    return 1;
}

static BOOL NPC_SimpleShopProcessSellMsg(char *msg, int shopindex,
                                         int playerindex )
{
    int sellwindowindex, itemtableindex,itemind, id;
    int price, tmpgold;
    char buf[64];
    if(!CHAR_CHECKINDEX(shopindex)||!CHAR_CHECKINDEX(playerindex))
        return -1;
    if( !getStringFromIndexWithDelim( msg,"|",4,buf,sizeof(buf) )){
        return -1;
    }
    sellwindowindex = atoi(buf);
    itemtableindex = sellwindowindex + CHAR_STARTITEMARRAY;
    itemind = CHAR_getItemIndex(playerindex,itemtableindex );
    if( ! ITEM_CHECKINDEX(itemind) ) return -1;

    price = ITEM_getInt( itemind, ITEM_COST );

    if( price <= 0 || price > NPC_SIMPLESHOPMAXBUYPRICE ){
        return -2;
    }
    price *= NPC_Util_sellRate(playerindex);

	CHAR_AddGold( playerindex, price );

	CHAR_sendStatusString( playerindex , "P");

    id = ITEM_getInt(itemind,ITEM_ID);
	{
		LogItem(
			CHAR_getChar( playerindex, CHAR_NAME ), /* キャラ名 */
			CHAR_getChar( playerindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD 在item的log中增加item名稱
			itemind,
#else
       		ITEM_getInt( itemind, ITEM_ID ),  /* アイテム番号 */
#endif
			"Sell",
			CHAR_getInt( playerindex,CHAR_FLOOR),
			CHAR_getInt( playerindex,CHAR_X ),
 	      	CHAR_getInt( playerindex,CHAR_Y ),
	        ITEM_getChar( itemind, ITEM_UNIQUECODE),
			ITEM_getChar( itemind, ITEM_NAME),
			ITEM_getInt( itemind, ITEM_ID)
		);
	}

    if( ! NPC_Util_moveItemToChar( shopindex,itemind,TRUE ) ){
        return -1;
    }

	//TODO: FIX SHOP
    //NPC_Util_RemoveItemByID(shopindex,id,FALSE);
    return 1;
}

static BOOL NPC_LimitBuyInShopProcessSellMsg(char *msg,
                                 int shopindex, int playerindex )
{
    int sellwindowindex, itemtableindex,itemind, id,i,buyokflag, setid;
    int price, tmpgold;
    char buf[BUFSIZ];
    char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];

    if(!CHAR_CHECKINDEX(shopindex)||!CHAR_CHECKINDEX(playerindex))
        return -1;
    if( !getStringFromIndexWithDelim( msg,"|",4,buf,sizeof(buf) )){
        return -1;
    }
    sellwindowindex = atoi(buf);
    itemtableindex = sellwindowindex + CHAR_STARTITEMARRAY;
    itemind = CHAR_getItemIndex(playerindex,itemtableindex );
    if( ! ITEM_CHECKINDEX(itemind) ) return -1;
    id = ITEM_getInt(itemind,ITEM_ID);
    NPC_Util_GetArgStr( shopindex, npcarg, sizeof(npcarg));
    getStringFromIndexWithDelim( npcarg, "|", BUYINITEMLISTTOKEN, token, sizeof( token) );
    buyokflag = FALSE;
    for( i=1;getStringFromIndexWithDelim(token,",",i,buf,sizeof(buf));  i++){

	  char *tok1 = NULL;
	  char *tok2 = NULL;
	  int no1=0, no2=0;
	  tok1 = strtok( buf, "-" );
	  tok2 = strtok( NULL, "-" );

	  setid = atoi(buf);

	  if( tok1 == NULL || tok2 == NULL ){
		if( setid == 0 ) return -2;
		if( setid == id ) buyokflag = TRUE;
	  } else {
		no1 = atoi( tok1 );
		no2 = atoi( tok2 );
		if( no1 <= id && id <= no2 ){
		  buyokflag = TRUE;
		}
	  }
    }
    if( buyokflag == FALSE ) return -2;
    price = ITEM_getInt( itemind, ITEM_COST );
    price *= NPC_Util_sellRate(playerindex);

	CHAR_AddGold( playerindex, price );

    CHAR_sendStatusString( playerindex , "P");
    if( ! NPC_Util_moveItemToChar( shopindex,itemind,TRUE ) ){
        return -1;
    }

	{
		LogItem(
			CHAR_getChar( playerindex, CHAR_NAME ), /* キャラ名 */
			CHAR_getChar( playerindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD 在item的log中增加item名稱
			itemind,
#else
       		ITEM_getInt( itemind, ITEM_ID ),  /* アイテム番号 */
#endif
			"Sell",
			CHAR_getInt( playerindex,CHAR_FLOOR),
			CHAR_getInt( playerindex,CHAR_X ),
 	      	CHAR_getInt( playerindex,CHAR_Y ),
	        ITEM_getChar( itemind, ITEM_UNIQUECODE),
			ITEM_getChar( itemind, ITEM_NAME),
			ITEM_getInt( itemind, ITEM_ID)
		);
	}

    /* 買い取ったアイテムを消去 */
	//TODO: FIX SHOP
    //NPC_Util_RemoveItemByID(shopindex,id,FALSE);

    return 1;

}

/*
 * 店にあるアイテムの数を得る
 */
static int NPC_SimpleShopGetItemNum( int meindex )
{
    int counter, i;

    counter = 0;
    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        /* アイテム表を全部サーチしてもってるやつをカウントする */
        if( ITEM_CHECKINDEX( CHAR_getItemIndex(meindex,i) ) ){
            counter++;
        }
    }
    return counter;

}

/*
 * あるキャラインデクスのキャラに、おみせwindowを開くための
 * 情報を与える。
 * int meindex:自分の(店の)インデクス
 * int cliindex:客のインデクス
 */
static void NPC_SimpleShopOpenShopWindow( int meindex, int cliindex,
                                          char *npcarg )
{
    int fd;
    int objindex;

    char tmp[1024*12], itemstr[1024 * 8];

    char maincaption[SIMPLESHOPTALKBUFSIZE];
    char buycaption[SIMPLESHOPTALKBUFSIZE];
    char okbuycaption[SIMPLESHOPTALKBUFSIZE];
    char sellcaption[SIMPLESHOPTALKBUFSIZE];
    char oksellcaption[SIMPLESHOPTALKBUFSIZE];
    char poorcaption[SIMPLESHOPTALKBUFSIZE];
    char itemfullcaption[SIMPLESHOPTALKBUFSIZE];

    getStringFromIndexWithDelim( npcarg,"|",MAINTOKEN,
                                 maincaption,sizeof(maincaption) );
    getStringFromIndexWithDelim( npcarg,"|",BUYTOKEN,
                                 buycaption,sizeof(buycaption) );
    getStringFromIndexWithDelim( npcarg,"|",OKBUYTOKEN,
                                 okbuycaption,sizeof(okbuycaption) );
    getStringFromIndexWithDelim( npcarg,"|",SELLTOKEN,
                                 sellcaption,sizeof(sellcaption) );
    getStringFromIndexWithDelim( npcarg,"|",OKSELLTOKEN,
                                 oksellcaption,sizeof(oksellcaption) );
    getStringFromIndexWithDelim( npcarg,"|",POORTOKEN,
                                 poorcaption,sizeof(poorcaption));
    getStringFromIndexWithDelim( npcarg,"|",ITEMFULLTOKEN,
                                 itemfullcaption,sizeof(itemfullcaption));

    tmp[0] = 0;
    NPC_SimpleShopGetEscapedItemString( meindex, cliindex, itemstr );
    objindex = CHAR_getWorkInt( meindex,CHAR_WORKOBJINDEX );
    snprintf( tmp,sizeof(tmp),"S|%d|%s|%s|%s|"
              "%s|%s|%s|%s|"
              "-1|%d%s", objindex,
              maincaption,
              buycaption,
              okbuycaption,
              sellcaption,
              oksellcaption,
              poorcaption,
              itemfullcaption,
              NPC_SimpleShopGetItemNum( meindex ),
              itemstr );
    fd = getfdFromCharaIndex( cliindex );

    if( fd == -1 )return;

    lssproto_TK_send(fd, -1 ,tmp, CHAR_COLORWHITE );


}


/*
 * TKで送るための文字列をつくる。
 */
static void NPC_SimpleShopGetEscapedItemString( int shopindex,
                                                int cliindex, char *str )
{
    int i, cost, cl, imageno, itemindex;
    char *itemname;
    char tmp[1024];
    char escapedname[256];
//    char *oli; /* oli means onelineinfo */

    str[0] = '\0';

    /* まず店の持ち物のリスト */
    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        itemindex=CHAR_getItemIndex( shopindex , i );
        if( ITEM_CHECKINDEX(itemindex) ){
            itemname = ITEM_getChar( itemindex, ITEM_SECRETNAME );
            makeEscapeString( itemname, escapedname, sizeof(escapedname));
            cost = ITEM_getInt( itemindex, ITEM_COST );
            /* 金額の調整。MERCHANTLEVELによって変わる。 */
            cost *= NPC_Util_buyRate(cliindex);
            cl = ITEM_getInt( itemindex, ITEM_LEVEL );
            imageno = ITEM_getInt( itemindex, ITEM_BASEIMAGENUMBER );
            //oli =  ITEM_getMemo( itemindex );
#if 0
            snprintf( tmp, sizeof(tmp), "|%s|%d|%d|%d|%s",
                      escapedname, cost, cl, imageno, oli );
#endif
            snprintf( tmp, sizeof(tmp), "|%s|%d|%d|%d|",
                      escapedname, cost, cl, imageno );
            strcat( str, tmp );
        }
    }

    /* 次に、客の持ち物のリスト */
    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        itemindex=CHAR_getItemIndex( cliindex , i );
        if( ITEM_CHECKINDEX(itemindex) ){
            cost = ITEM_getInt( itemindex, ITEM_COST );
            /* 金額の調整。MERCHANTLEVELによって変わる。 */
            cost *=NPC_Util_sellRate(cliindex);
            snprintf( tmp, sizeof(tmp), "|%d", cost );
            strcat( str, tmp );
        }else{
            /* アイテムを持ってない場所には0をいれる     */
            strcat( str, "|0" );
        }
    }

}

/*
 * 店が無限に持っているアイテムを持たせる。初期化時と取り引きの後
 * に呼ぶ。
 */
static BOOL NPC_SimpleShopFillItem( int meindex, char *npcarg )
{
    char token[256], buf[16];
    int i,itemid, num;

    getStringFromIndexWithDelim( npcarg,"|",ITEMLISTTOKEN,token,
                                 sizeof(token) );

    for( i=1; i<=SIMPLESHOP_MAXINFINITITEM;i++ ){
        if( getStringFromIndexWithDelim( token,",",i,buf,sizeof(buf) )){
            itemid = atoi( buf );
            if( itemid == 0 ){
				//TODO: FIX SHOP
                //NPC_Util_ReleaseHaveItemAll(meindex);
                return FALSE;
            }
            num=NPC_Util_countHaveItem(meindex,itemid);
            if( num < 0 ){
                return FALSE;/* 不正なidなどのfatal err */
            }else if( num == 0 ){ /* もってなかったら補充 */
                NPC_Util_createItemToChar( meindex,itemid, FALSE);

            }else if( num>=2 ){ /* もちすぎの場合削除 */
				//TODO: FIX SHOP
                //NPC_Util_RemoveItemByID(meindex,itemid,FALSE);
            }
        }else{
            break;
        }
    }

    /* 一回しきべつされた状態にする */
    //NPC_SimpleShopSetLeakLevel( meindex );

    return TRUE;
}


/*
 * 初期化する。
 */
BOOL NPC_SimpleShopInit( int meindex )
{
    unsigned int mlevel;
    char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];


    /* 一回失敗したあとは2度と作らないようにする */
    int createindex = CHAR_getInt( meindex, CHAR_NPCCREATEINDEX );

/*    print("shopinit start\n" );*/
    if( NPC_CHECKCREATEINDEX(createindex) ){
        NPC_create[createindex].intdata[NPC_CREATETIME] = -1;
    }

    CHAR_setInt( meindex , CHAR_HP , 0 );
    CHAR_setInt( meindex , CHAR_MP , 0 );
    CHAR_setInt( meindex , CHAR_MAXMP , 0 );
    CHAR_setInt( meindex , CHAR_STR , 0 );
    CHAR_setInt( meindex , CHAR_TOUGH, 0 );
    CHAR_setInt( meindex , CHAR_LV , 0 );
    mlevel = ((100<<16)|(100<<0)); /* 上位2バイトが買うときの倍率、
                                    下位2バイトが売るときの倍率。 */
    CHAR_setInt( meindex, CHAR_MERCHANTLEVEL, mlevel );

    CHAR_setWorkInt( meindex, CHAR_WORKSHOPCLIENTINDEX, -1 );

    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPESHOP );
    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );

    NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg));

    /* アイテム持たせる。失敗したらreturn FALSE */
    if( ! NPC_SimpleShopFillItem( meindex,npcarg ) ){
        print( "SHOP INIT ERROR: npcarg=%s\n", npcarg );
        return FALSE;
    }

/*    print("shopinit end\n" );*/


    return TRUE;
}

/*
 * 店ウインドウを開くかどうか判定する。
 * npcargのいっこめのトークンに
 * こんにちは,ごめんください,くださいな
 * のように羅列された文字列をもとに判定する。このときのデリミタは","。
 *
 * 引数
 * msg:プレイヤーがしゃべった文字列
 * openshopstr:npcargのいっこめのトークン。
 * 返り値
 * 開くならTRUE, 開かないならFALSE
 */
static BOOL NPC_SimpleShopOpenShopOrNot( char *msg, char *openshopstr )
{
    int i;
    char buf[256];

    i=1;
    while(getStringFromIndexWithDelim(openshopstr,",",i,buf,sizeof(buf))){
        if( strstr( msg, buf ) ) return TRUE;
        i++;
    }
    return FALSE;
}

/*
 * すべてのもちものを一回識別された状態にする。
 */
#if 0
static void NPC_SimpleShopSetLeakLevel( int meindex )
{
    int i, itemindex;

    for( i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++ ){
        itemindex=CHAR_getItemIndex(meindex,i);
        if( ITEM_CHECKINDEX(itemindex)){
            ITEM_setInt(itemindex,ITEM_LEAKLEVEL, 1 );
        }
    }
}
#endif

