
#ifndef __ITEM_H__
#define __ITEM_H__

#include "char.h"

#define NULLITEM    "0"

typedef enum
{
    ITEM_FIST =0,
    ITEM_AXE,
    ITEM_CLUB,
    ITEM_SPEAR,
    ITEM_BOW,
    ITEM_SHIELD,
    ITEM_HELM,
    ITEM_ARMOUR,

	ITEM_BRACELET =8,
	ITEM_MUSIC,
	ITEM_NECKLACE,
	ITEM_RING,
	ITEM_BELT,
	ITEM_EARRING,
	ITEM_NOSERING,
	ITEM_AMULET,
    /* ****** */
    ITEM_OTHER =16,
    ITEM_BOOMERANG,		// 迴旋標
    ITEM_BOUNDTHROW,	// 投擲斧頭
    ITEM_BREAKTHROW,	// 投擲石
    ITEM_DISH =20,
#ifdef _ITEM_INSLAY
	ITEM_METAL,
	ITEM_JEWEL,
#endif
#ifdef _ITEM_CHECKWARES
	ITEM_WARES,			//貨物
#endif

#ifdef _ITEM_EQUITSPACE
	ITEM_WBELT,			//腰帶
	ITEM_WSHIELD,		//盾
	ITEM_WSHOES,		//鞋子
#endif
#ifdef _EQUIT_NEWGLOVE 
	ITEM_WGLOVE,		//手套
#endif

#ifdef _ALCHEMIST
	ITEM_ALCHEMIST =30,
#endif

#ifdef _ANGEL_SUMMON
	//ITEM_ANGELTOKEN,
	//ITEM_HEROTOKEN,
#endif

    ITEM_CATEGORYNUM,
    
}ITEM_CATEGORY;

typedef enum
{
	ITEM_FIELD_ALL,
	ITEM_FIELD_BATTLE,
	ITEM_FIELD_MAP,
}ITEM_FIELDTYPE;

typedef enum
{
	ITEM_TARGET_MYSELF,
	ITEM_TARGET_OTHER,
	ITEM_TARGET_ALLMYSIDE,
	ITEM_TARGET_ALLOTHERSIDE,
	ITEM_TARGET_ALL,
}ITEM_TARGETTYPE;

typedef enum
{
    ITEM_ID,
    ITEM_BASEIMAGENUMBER,
    ITEM_COST,
    ITEM_TYPE,
	ITEM_ABLEUSEFIELD,
	ITEM_TARGET,
    ITEM_LEVEL,                     /*  LEVEL  */
#ifdef _ITEM_MAXUSERNUM
	ITEM_DAMAGEBREAK,				//物品使用次數
#endif

#ifdef _ITEMSET4_TXT
	ITEM_USEPILENUMS,				//物品堆疊次數
	ITEM_CANBEPILE,					//是否可堆疊

	ITEM_NEEDSTR,
	ITEM_NEEDDEX,
	ITEM_NEEDTRANS,
	ITEM_NEEDPROFESSION,
#endif

#ifdef _TAKE_ITEMDAMAGE
	ITEM_DAMAGECRUSHE,
	ITEM_MAXDAMAGECRUSHE,
#endif

#ifdef _ADD_DEAMGEDEFC
	ITEM_OTHERDAMAGE,
	ITEM_OTHERDEFC,
#endif

#ifdef _SUIT_ITEM
	ITEM_SUITCODE,
#endif

    ITEM_ATTACKNUM_MIN,             /*  最低攻撃回数  */
    ITEM_ATTACKNUM_MAX,             /*  最高攻撃回数  */
    ITEM_MODIFYATTACK,              /*  攻撃力変化量  */
    ITEM_MODIFYDEFENCE,             /*  防御力変化量  */
    ITEM_MODIFYQUICK,               /*  QUICK変化量  */

    ITEM_MODIFYHP,                  /*  HP変化量    */
    ITEM_MODIFYMP,                  /*  MP変化量    */
    ITEM_MODIFYLUCK,                /*  LUCK変化量    */
    ITEM_MODIFYCHARM,               /*  CHARM変化量    */
    ITEM_MODIFYAVOID,               /*  回避率修正    */
	ITEM_MODIFYATTRIB,				/*  属性修正 */
	ITEM_MODIFYATTRIBVALUE,			/*  属性修正値 */
	ITEM_MAGICID,					/*  呪術番号 */
	ITEM_MAGICPROB,					/*  呪術発動率 */
	ITEM_MAGICUSEMP,				/*  消費MP */

#ifdef _ITEMSET5_TXT
	ITEM_MODIFYARRANGE,
	ITEM_MODIFYSEQUENCE,

	ITEM_ATTACHPILE,
	ITEM_HITRIGHT,	//額外命中
#endif
#ifdef _ITEMSET6_TXT
	ITEM_NEGLECTGUARD,
//	ITEM_BEMERGE,
#endif
    /*  ステータス修正値。*/
    ITEM_POISON,					/* 毒歩くたびにダメージ          */
    ITEM_PARALYSIS,          		/* しびれ、1部の行動ができない。 */
    ITEM_SLEEP,              		/* 眠り。行動できない            */
    ITEM_STONE,              		/* 石。行動できない              */
    ITEM_DRUNK,              		/* 酔う。命中率が下がる     */
    ITEM_CONFUSION,          		/* 混乱。攻撃目標を誤る     */

	ITEM_CRITICAL,					/* クリティカル率修正 */

	ITEM_USEACTION,					/* 使った時のアクション */
    ITEM_DROPATLOGOUT,              /* ログアウトする時に落すかどうか  */
    ITEM_VANISHATDROP,              /* 落した時に消えるかどうか */
    ITEM_ISOVERED,                  /*  上に乗っからられるかどうか。*/
	ITEM_CANPETMAIL,				/* ペットメールで送れるか */
	ITEM_CANMERGEFROM,				/* 合成元になれるか */
	ITEM_CANMERGETO,				/* 合成先になれるか */

    ITEM_INGVALUE0,                 /* 成分(5個分) */
    ITEM_INGVALUE1,
    ITEM_INGVALUE2,
    ITEM_INGVALUE3,
    ITEM_INGVALUE4,
    
	ITEM_PUTTIME,					/*  アイテムが置かれた時間 */
    ITEM_LEAKLEVEL,                 /*  秘密がどれだけばれたか  */
	ITEM_MERGEFLG,					/*  合成されたアイテムかどうか */
	ITEM_CRUSHLEVEL,				/*  壊れ度合い 0〜2 ０は壊れてない 2は全壊 */

    ITEM_VAR1,              	/*  汎用作業領域   */
    ITEM_VAR2,              	/*  汎用作業領域   */
    ITEM_VAR3,              	/*  汎用作業領域   */
    ITEM_VAR4,              	/*  汎用作業領域   */

	ITEM_DATAINTNUM,

}ITEM_DATAINT;

typedef enum
{
    ITEM_NAME,                      /*  名前 （本当の名前）   */
    ITEM_SECRETNAME,                /*  名前（変更される可能性有り）  */
    ITEM_EFFECTSTRING,              /*  効果文字列  */
    ITEM_ARGUMENT,                  /*  アイテムの引数  */
#ifdef _ITEM_INSLAY
	ITEM_TYPECODE,
	ITEM_INLAYCODE,
#endif
	ITEM_CDKEY,						/*  アイテムの名前を最初に変更した人のＣＤＫＥＹ */
#ifdef _ITEM_FORUSERNAMES
	ITEM_FORUSERNAME,
	ITEM_FORUSERCDKEY,
#endif
// CoolFish: 2001/10/11
#ifdef _UNIQUE_P_I
    ITEM_UNIQUECODE,		  /* 物品編碼 */
#endif

    ITEM_INGNAME0,                  /*  成分の名前(5個分) */
    ITEM_INGNAME1,
    ITEM_INGNAME2,
    ITEM_INGNAME3,
    ITEM_INGNAME4,


    ITEM_INITFUNC,                  /* 引数
                                     * ITEM_Item*
                                     * 返り値 BOOL
                                     * 返り値の意味は CHAR_INITFUNC
                                     * と同じ  */
    ITEM_FIRSTFUNCTION = ITEM_INITFUNC,
    ITEM_PREOVERFUNC,               /* CHAR_PREOVERFUNC を参照 */
    ITEM_POSTOVERFUNC,              /* CHAR_POSTOVERFUNC を参照*/
    ITEM_WATCHFUNC,                 /* CHAR_WATCHFUNC を参照 */
    ITEM_USEFUNC,                   /* 引数は、
                                     * int charaindex キャラインデックス
                                     * int charitemindex 自分の
                                     *              アイテム配列の何番目
                                     *              を使ったか
                                     */
    ITEM_ATTACHFUNC,                /* 引数は、
                                     * int charaindex キャラインデックス
                                     * int itemindex  アイテムインデックス
                                     *      キャラクタの持っているアイテム
                                     *      のアイテム欄でのインデックス
                                     *      ではない事に注意。
                                     */
    ITEM_DETACHFUNC,                /* 引数は、
                                     * int charaindex キャラインデックス
                                     * int itemindex  アイテムインデックス
                                     *      キャラクタの持っているアイテム
                                     *      のアイテム欄でのインデックス
                                     *      ではない事に注意。
                                     */
    ITEM_DROPFUNC, 		            /* 落としたとき
                                     * 引数は
                                     *  int charaindex 落としたキャラ
                                     *  int itemindex アイテムインデックス
                                     */
    ITEM_PICKUPFUNC,              /* アイテムを拾った時
                                     * 引数は
                                     *  int charaindex  拾ったキャラindex
                                     *  int itemindex アイテムインデックス
                                     */
#ifdef _Item_ReLifeAct
	ITEM_DIERELIFEFUNC,					/*ANDY_ADD
										復活道具	
									 */
#endif

#ifdef _CONTRACT
	ITEM_CONTRACTTIME,
	ITEM_CONTRACTARG,
#endif

    ITEM_LASTFUNCTION,

    ITEM_DATACHARNUM = ITEM_LASTFUNCTION,

#ifdef _ANGEL_SUMMON
	ITEM_ANGELMISSION = ITEM_INGNAME0,
	ITEM_ANGELINFO = ITEM_INGNAME1,
	ITEM_HEROINFO = ITEM_INGNAME2,
#endif

}ITEM_DATACHAR;

typedef enum
{
    ITEM_WORKOBJINDEX,
    ITEM_WORKCHARAINDEX,
#ifdef _MARKET_TRADE
	ITEM_WORKTRADEINDEX,
	ITEM_WORKTRADETYPE,
	ITEM_WORKTRADESELLINDEX,
#endif
#ifdef _ITEM_ORNAMENTS
	ITEM_CANPICKUP,
#endif
#ifdef _ITEM_TIME_LIMIT
	ITEM_WORKTIMELIMIT,
#endif
    ITEM_WORKDATAINTNUM,
}ITEM_WORKDATAINT;



typedef struct tagItem
{
    int         data[ITEM_DATAINTNUM];
    STRING64    string[ITEM_DATACHARNUM];
    int         workint[ITEM_WORKDATAINTNUM];

    void*       functable[ITEM_LASTFUNCTION-ITEM_FIRSTFUNCTION];
}ITEM_Item;

typedef struct tagITEM_table
{
    int         use; 
    ITEM_Item   itm;
    int         randomdata[ITEM_DATAINTNUM];
}ITEM_table;


typedef struct tagITEM_exists
{
    BOOL        use;
    ITEM_Item   itm;
}ITEM_exists;

#ifdef _CONTRACT
#define MAX_CONTRACTTABLE	10
typedef struct tagITEM_contract
{
    int         used;
    char		detail[2048];
    int			argnum;
}ITEM_contractTable;
#endif


#define		ITEM_CHECKINDEX(index)		\
	_ITEM_CHECKINDEX( __FILE__, __LINE__, index)
INLINE BOOL _ITEM_CHECKINDEX( char *file, int line, int index);


BOOL ITEM_initExistItemsArray( int num );
BOOL ITEM_endExistItemsArray( void );
#define		ITEM_initExistItemsOne( itm) \
	_ITEM_initExistItemsOne( __FILE__, __LINE__, itm)
int _ITEM_initExistItemsOne( char *file, int line, ITEM_Item* itm );

#define		ITEM_endExistItemsOne( index ) \
			_ITEM_endExistItemsOne( index, __FILE__, __LINE__)

void _ITEM_endExistItemsOne( int index , char *file, int line);

#define ITEM_getInt( Index, element) _ITEM_getInt( __FILE__, __LINE__, Index, element )
INLINE int _ITEM_getInt( char *file, int line, int index ,ITEM_DATAINT element);


#define ITEM_setInt( Index, element, data) _ITEM_setInt( __FILE__, __LINE__, Index, element, data)
INLINE int _ITEM_setInt( char *file, int line, int index ,ITEM_DATAINT element, int data);


INLINE char* ITEM_getChar( int index ,ITEM_DATACHAR element );
INLINE BOOL ITEM_setChar( int index ,ITEM_DATACHAR element , char* input);

INLINE int ITEM_getWorkInt( int index ,ITEM_WORKDATAINT element);
INLINE int ITEM_setWorkInt( int index ,ITEM_WORKDATAINT element, int input);
INLINE int ITEM_getITEM_itemnum( void );
INLINE int ITEM_getITEM_UseItemnum( void );
INLINE BOOL ITEM_getITEM_use( int index );
void ITEM_constructFunctable( int itemindex );
void* ITEM_getFunctionPointer( int itemindex, int functype );
INLINE ITEM_Item *ITEM_getItemPointer( int index );
int ITEM_getItemMaxIdNum( void);


char* ITEM_makeStringFromItemData( ITEM_Item* one, int mode );
char* ITEM_makeStringFromItemIndex( int index, int mode );

BOOL ITEM_makeExistItemsFromStringToArg( char* src , ITEM_Item* item, int mode );
void ITEM_getDefaultItemSetting( ITEM_Item* itm);


INLINE BOOL ITEM_CHECKITEMTABLE( int number );
BOOL    ITEM_readItemConfFile( char* filename );


CHAR_EquipPlace ITEM_getEquipPlace( int charaindex, int itmid );


char*  ITEM_makeItemStatusString( int haveitemindex, int itemindex );
char*   ITEM_makeItemFalseString( void );
char*   ITEM_makeItemFalseStringWithNum( int haveitemindex );


BOOL ITEM_makeItem( ITEM_Item* itm, int number );
int ITEM_makeItemAndRegist( int number );


void ITEM_equipEffect( int index );

void Other_DefcharWorkInt( int index);

char* ITEM_getAppropriateName(int itemindex);
char* ITEM_getEffectString( int itemindex );


int ITEM_getcostFromITEMtabl( int itemid );

#define ITEM_getNameFromNumber( id) _ITEM_getNameFromNumber( __FILE__, __LINE__, id)
INLINE char* _ITEM_getNameFromNumber( char *file, int line, int itemid );


int ITEM_getlevelFromITEMtabl( int itemid );
int ITEM_getgraNoFromITEMtabl( int itemid );
char *ITEM_getItemInfoFromNumber( int itemid );

int ITEM_getdropatlogoutFromITEMtabl( int itemid );
int ITEM_getvanishatdropFromITEMtabl( int itemid );
int ITEM_getcanpetmailFromITEMtabl( int itemid );
int ITEM_getmergeItemFromFromITEMtabl( int itemid );

#ifdef _ITEM_CHECKWARES
BOOL CHAR_CheckInItemForWares( int charaindex, int flg);
#endif

BOOL ITEM_canuseMagic( int itemindex);
// Nuke +1 08/23 : For checking the validity of item target
int ITEM_isTargetValid( int charaindex, int itemindex, int toindex);


#ifdef _IMPOROVE_ITEMTABLE
BOOL ITEMTBL_CHECKINDEX( int ItemID);
int ITEM_getSIndexFromTransList( int ItemID);
int ITEM_getMaxitemtblsFromTransList( void);
int ITEM_getTotalitemtblsFromTransList( void);
#endif

int ITEMTBL_getInt( int ItemID, ITEM_DATAINT datatype);
char *ITEMTBL_getChar( int ItemID, ITEM_DATACHAR datatype);

int ITEM_getItemDamageCrusheED( int itemindex);
void ITEM_RsetEquit( int charaindex);//自動卸除裝備位置錯誤之物品
void ITEM_reChangeItemToPile( int itemindex);
void ITEM_reChangeItemName( int itemindex);


#ifdef _SIMPLIFY_ITEMSTRING
void ITEM_getDefaultItemData( int itemID, ITEM_Item* itm);
#endif

#ifdef _CONTRACT
BOOL ITEM_initContractTable( );
#endif

#endif
