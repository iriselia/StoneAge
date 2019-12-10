#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "npc_action.h"

/* 
 * プレイヤーのアクションに反応するNPC。
 * 単に喋り返したりするだけだが。
 * 歩く，または立っているのアクションには反応しない。
 *
 * 引数：
 *      msgcol:		メッセージの色。デフォルトは黄色
 *      normal:		普通に喋ってきた時や無効なアクションに対しての返答
 *		attack:		攻撃アクションに対する返答
 *		damage:		ダメージを受けたアクションに対する返答
 *		down:		倒れるアクションに対する返答
 *		sit:		座るアクションに対しての返答
 *		hand:		手を振るアクションに対しての返答
 *		pleasure:	喜ぶアクションに対しての返答
 *		angry:		怒るアクションに対しての返答
 *		sad:		悲しむアクションに対しての返答
 *		guard:		ガードするアクションに対しての返答
 */
 

#define		NPC_ACTION_MSGCOLOR_DEFAULT		CHAR_COLORYELLOW

enum {
	CHAR_WORK_MSGCOLOR	= CHAR_NPCWORKINT1,
};

/*********************************
* 初期処理
*********************************/
BOOL NPC_ActionInit( int meindex )
{
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int		tmp;
	
	tmp = NPC_Util_GetNumFromStrWithDelim( argstr, "msgcol");
	if( tmp == -1 ) tmp = NPC_ACTION_MSGCOLOR_DEFAULT;
	CHAR_setWorkInt( meindex, CHAR_WORK_MSGCOLOR, tmp);
    
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEACTION );
	
	
    return TRUE;
}




/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_ActionTalked( int meindex , int talkerindex , char *szMes ,
                     int color )
{
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char	buf[64];

    /* プレイヤーに対してだけ反応する */
    if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) {
    	return;
    }
	/* １グリッド以内のみ */
	if( !NPC_Util_charIsInFrontOfChar( talkerindex, meindex, 1 )) return; 

	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));

	if( NPC_Util_GetStrFromStrWithDelim( argstr, 
										"normal",
										buf, sizeof( buf))
    	!= NULL )
	{
    	CHAR_talkToCli( talkerindex, meindex ,buf , 
    					CHAR_getWorkInt( meindex, CHAR_WORK_MSGCOLOR ));
	}
	
}
/*********************************
* watch処理
*********************************/
void NPC_ActionWatch( int meobjindex, int objindex, CHAR_ACTION act,
                    int x,int y,int dir, int* opt,int optlen )
{
	int		meindex;
	int		index;
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char	buf[64];
	int		i;
    struct  {
    	CHAR_ACTION		act;
    	char			*string;
    }searchtbl[] = {
	    { CHAR_ACTATTACK,	"attack"},
	    { CHAR_ACTDAMAGE,	"damage"},
		{ CHAR_ACTDOWN,		"down"},
		{ CHAR_ACTSIT,		"sit"},
		{ CHAR_ACTHAND,		"hand"},
		{ CHAR_ACTPLEASURE,	"pleasure"},
		{ CHAR_ACTANGRY,	"angry"},
		{ CHAR_ACTSAD,		"sad"},
		{ CHAR_ACTGUARD,	"guard"},
		{ CHAR_ACTNOD,		"nod"},
		{ CHAR_ACTTHROW,	"throw"},
    };
	
	if( OBJECT_getType( objindex) != OBJTYPE_CHARA) return;
	index = OBJECT_getIndex( objindex);
	/* プレイヤーにのみ反応する */
	if( CHAR_getInt( index, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) return;
    
    meindex = OBJECT_getIndex( meobjindex);
    
    /* 向き合って１グリッドでないと反応しない */
    if( NPC_Util_isFaceToFace( meindex, index, 1 ) != TRUE ) return;
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	for( i = 0; i < arraysizeof( searchtbl); i ++ ) {
		if( searchtbl[i].act == act) {
			if( NPC_Util_GetStrFromStrWithDelim( argstr, 
												searchtbl[i].string,
												buf, sizeof( buf))
		    	!= NULL )
			{
		    	CHAR_talkToCli( index, meindex ,buf , 
		    					CHAR_getWorkInt( meindex, CHAR_WORK_MSGCOLOR ));
				break;
			}
		}
	}
}
