#include "version.h"

#ifdef _NPC_WELFARE_2				// WON ADD 職業NPC-2
#ifdef _PROFESSION_SKILL			// WON ADD 人物職業技能

#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "pet_skill.h"
#include "readmap.h"
#include "battle.h"
#include "profession_skill.h"
#include "chatmagic.h"
#include "npc_welfare2.h"


static void NPC_Welfare_selectWindow2(int meindex, int toindex, int num, int select);

//載入好康NPC的初始化
BOOL NPC_WelfareInit2( int meindex )
{
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEWELFARE);
    print("名稱:%s\n",CHAR_getChar( meindex, CHAR_NAME));
    return TRUE;
}

//對話時的處理
void NPC_WelfareTalked2( int meindex , int talkerindex , char *szMes ,int color )
{
    if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) {
    	return;
    }
	if(NPC_Util_isFaceToFace(talkerindex,meindex,2 )==FALSE){
		if( NPC_Util_CharDistance( talkerindex, meindex ) > 1) return;
	}
	NPC_Welfare_selectWindow2( meindex, talkerindex,0,-1);
}

static void NPC_Welfare_selectWindow2( int meindex, int toindex, int num,int select)
{
	switch(num){
	  case 0:
		  //free
		NPC_WelfareMakeStr2(meindex, toindex, select);
		break;
	  case 1:
		break;
	}
}

void NPC_WelfareWindowTalked2( int meindex, int talkerindex, 
		int seqno, int select, char *data)
{
	int skill, i, j;
	int skillID = -1, skillid = -1;
	int ENDEV = -1, NOWEV = -1;
	char buf[64];
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char msg[512];
	int fd = getfdFromCharaIndex( talkerindex );
	double rate= 1.0;
	int skillarray;
	CHAR_HaveSkill* hskill;
	char err_msg[128] = {0};

	if( NPC_Util_CharDistance( talkerindex, meindex ) > 2) {
		return;
	}

	if(NPC_Util_GetArgStr( meindex, argstr, sizeof(argstr))==NULL){
		print("GetArgStrErr");
		return;
	}

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "ENDEV:", msg, sizeof( msg)) != NULL){
		ENDEV=atof(msg);
	}

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "NOWEV:", msg, sizeof( msg)) != NULL){
		NOWEV=atof(msg);
	}

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "err_msg:", err_msg, sizeof( msg)) == NULL){
		print("GetArgStrErr");
		return;		
	}

	if( ENDEV > 0 ){
		if( NPC_EventCheckFlg( talkerindex, ENDEV ) != TRUE ){
			CHAR_talkToCli( talkerindex, -1, err_msg, CHAR_COLORYELLOW);
			return;
		}
	}else if( NOWEV > 0 ){
		if( NPC_EventCheckFlg( talkerindex, NOWEV ) != TRUE ){
			CHAR_talkToCli( talkerindex, -1, err_msg, CHAR_COLORYELLOW);	
			return;
		}
	}

	makeStringFromEscaped( data);
	getStringFromIndexWithDelim(data,"|",1,buf,sizeof(buf));
	skill=atoi(buf);
	
	// 技能id
	skillid = CHAR_getCharSkill( talkerindex, skill - 1 );	
	skillID = skillid;

	if(skillID <= 0) return ;

	if( CHAR_getWorkInt( CONNECT_getCharaindex(fd), CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE) return ;

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "skill_rate", msg, sizeof( msg)) != NULL){
		rate = atof( msg);
	}

	// 不扣錢
	CHAR_send_P_StatusString( talkerindex, CHAR_P_STRING_GOLD);

	skillarray = PROFESSION_SKILL_getskillArray( skillID );

	// 檢查是否有不能刪除的技能
	for( i=0; i<CHAR_SKILLMAXHAVE; i++ ){
		int limit = -1, skillid = -1, skillarray_temp = -1;

		// 技能ID
		skillid = CHAR_getCharSkill( talkerindex, i);
		if( skillid <= 0 ) continue;
	
		skillarray_temp = PROFESSION_SKILL_getskillArray( skillid );

		for( j=0; j<4; j++){
			limit = PROFESSION_SKILL_getInt( skillarray_temp, PROFESSION_SKILL_LIMIT1+j*2 );

			if( limit > 0 && limit == skillID ){
				char token[256] = {0};

				sprintf( token, "需先遺忘%s，才可刪除此技能", PROFESSION_SKILL_getChar( skillarray_temp, PROFESSION_SKILL_NAME ) );
				CHAR_talkToCli( talkerindex, -1, token, CHAR_COLORYELLOW);
				return;
			}
		}
	}

	// 遺忘技能
	{

		int skill_level = -1;
		int count = 0;
		int skill_id[CHAR_SKILLMAXHAVE] = {0};
		int skill_temp[CHAR_SKILLMAXHAVE] = {0};
		
		// 取熟練度
		for( i=0; i<CHAR_SKILLMAXHAVE; i++ ){
			int skillid = -1;

			// 技能ID
			skillid = CHAR_getCharSkill( talkerindex, i);

			// 不儲存需遺忘的技能
			if( skillid <= 0 || skillid == skillID ) continue;


			// 技能等級
			hskill = CHAR_getCharHaveSkill( talkerindex, i );
			skill_level = SKILL_getInt( &hskill->skill, SKILL_LEVEL);
			
			skill_id[count] = skillid;
			skill_temp[count] = skill_level;
			count++;
		}

		// 刪除所有技能
		CHAR_CHAT_DEBUG_delsk( talkerindex, "all" );

		// 還原技能
		for( i=0; i<count; i++ ){
			PROFESSION_SKILL_ADDSK( talkerindex, skill_id[i], skill_temp[i] );
		}

		CHAR_sendStatusString( talkerindex , "S");
		NPC_Welfare_selectWindow2( meindex, talkerindex,0,-1);
		
	}

	// 傳送訊息
	{
		char token[256];
		int next_profession_skill_point = 0;
		
		next_profession_skill_point = CHAR_getInt( talkerindex, PROFESSION_SKILL_POINT ) + 1;			
		CHAR_setInt( talkerindex, PROFESSION_SKILL_POINT, next_profession_skill_point );
		
		memset(token, -1, sizeof(token) );
		sprintf( token, "您遺忘了 %s，剩餘學習點數增加一點"
				,PROFESSION_SKILL_getChar( skillarray, PROFESSION_SKILL_NAME ) );

		CHAR_talkToCli( talkerindex, -1, token, CHAR_COLORYELLOW);
    
		CHAR_sendCToArroundCharacter( CHAR_getWorkInt( talkerindex , CHAR_WORKOBJINDEX ));
	}

}             

void NPC_WelfareMakeStr2(int meindex,int toindex,int select)
{
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char msg[256];
	int i = 0;// j = 0;
	char token[65530];
	int fd = getfdFromCharaIndex( toindex);
	CHAR_HaveSkill* hskill;
	int skillarray = -1;
	int skillid = -1;
//	int Pskillid = -1;
	char token2[265] = {0};
	int skill_level = -1;
//	int limit = -1;

	if(select==0){
		sprintf(token,"0|0");
		lssproto_WN_send( fd, WINDOW_MESSAGETYPE_PETSKILLSHOP, 
			WINDOW_BUTTONTYPE_NONE, 
			CHAR_WINDOWTYPE_WINDOWPETSKILLSHOP,
			CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
			token);
	}

	if(NPC_Util_GetArgStr( meindex, argstr, sizeof(argstr))==NULL){
		print("GetArgStrErr");
		return;
	}

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "main_msg", msg, sizeof( msg)) == NULL){
		print("mainERR");
		return ;
	}

	sprintf(token,"1|%s|%s", CHAR_getChar(meindex,CHAR_NAME), msg);


	for( i=0; i<CHAR_SKILLMAXHAVE; i++ ){
		// 技能ID
		skillid = CHAR_getCharSkill( toindex, i);
		if( skillid <= 0 ) continue;

		skillarray = PROFESSION_SKILL_getskillArray( skillid );

		// 技能等級
		hskill = CHAR_getCharHaveSkill( toindex, i );
		skill_level = SKILL_getInt( &hskill->skill, SKILL_LEVEL);
				
		sprintf(token2,"|%s|%d|%s|%d",
			PROFESSION_SKILL_getChar( skillarray, PROFESSION_SKILL_NAME ),	// 技能名稱
			skill_level,													// 熟練度
			PROFESSION_SKILL_getChar( skillarray, PROFESSION_SKILL_TXT ),	// 說明
			PROFESSION_SKILL_getInt( skillarray, PROFESSION_SKILL_ICON )	// 圖示
		);

		strcat(token,token2);
	}

	lssproto_WN_send( fd, WINDOW_MESSAGETYPE_PROFESSIONSHOP2, 
				WINDOW_BUTTONTYPE_NONE, 
				WINDOW_MESSAGETYPE_PROFESSIONSHOP,
				CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
				token);

	return;
}


#endif
#endif


