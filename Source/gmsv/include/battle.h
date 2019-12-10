#ifndef __BATTLE_H__
#define __BATTLE_H__

#define	BATTLE_ENTRY_MAX 10
#define	BATTLE_PLAYER_MAX 5
#define SIDE_OFFSET 	10

#define BATTLE_STRING_MAX	4096
#define BATTLE_TIME_LIMIT	(60*60)


#define DUELPOINT_RATE	(0.1)

enum{
	BATTLE_MODE_NONE = 0,	// 無戰鬥狀態
	BATTLE_MODE_INIT,		// 戰鬥初始化
	BATTLE_MODE_BATTLE,		// 戰鬥中
	BATTLE_MODE_FINISH,		// 戰鬥結束
	BATTLE_MODE_STOP,		// 戰鬥結束(未使用)
	BATTLE_MODE_WATCHBC,	// 觀戰初始化
	BATTLE_MODE_WATCHPRE,	// 觀戰(沒作用)
	BATTLE_MODE_WATCHWAIT,	// 觀戰(沒作用)
	BATTLE_MODE_WATCHMOVIE, // 觀戰(沒作用)
	BATTLE_MODE_WATCHAFTER, // 觀戰(沒作用)
	BATTLE_MODE_END
};


enum{
	BATTLE_TYPE_NONE = 0,
	BATTLE_TYPE_P_vs_E = 1,
	BATTLE_TYPE_P_vs_P = 2,
	BATTLE_TYPE_E_vs_E = 3,
	BATTLE_TYPE_WATCH = 4,
	BATTLE_TYPE_DP_BATTLE = 5,
	BATTLE_TYPE_BOSS_BATTLE = 6,
	BATTLE_TYPE_END
};


enum{
	BATTLE_CHARMODE_NONE = 0,		// 無戰鬥狀態
	BATTLE_CHARMODE_INIT,			// 戰鬥初始化
	BATTLE_CHARMODE_C_WAIT,			// 等待戰鬥指令
	BATTLE_CHARMODE_C_OK,			// 已輸入戰鬥指令
	BATTLE_CHARMODE_BATTLE,			// 未使用
	BATTLE_CHARMODE_RESCUE,			// 由help狀態進入的
	BATTLE_CHARMODE_FINAL,			// 戰鬥結束
	BATTLE_CHARMODE_WATCHINIT,		// 觀戰初始化
	BATTLE_CHARMODE_COMMAND,		// 未使用
	BATTLE_CHARMODE_END
};




typedef enum{
	BATTLE_ERR_NONE = 0,
	BATTLE_ERR_NOTASK,
	BATTLE_ERR_NOUSE,
	BATTLE_ERR_PARAM,
	BATTLE_ERR_ENTRYMAX,
	BATTLE_ERR_TYPE,
	BATTLE_ERR_CHARAINDEX,
	BATTLE_ERR_BATTLEINDEX,
	BATTLE_ERR_NOENEMY,
	BATTLE_ERR_ALREADYBATTLE,
	BATTLE_ERR_SAMEPARTY,
	BATTLE_ERR_END
}BATTLE_ERR;


enum{
	BATTLE_S_TYPE_PLAYER = 0,
	BATTLE_S_TYPE_ENEMY,
	BATTLE_S_TYPE_END
};


typedef enum{
	BATTLE_COM_NONE,
	BATTLE_COM_ATTACK,
	BATTLE_COM_GUARD,
	BATTLE_COM_CAPTURE,
	BATTLE_COM_ESCAPE,
	BATTLE_COM_PETIN,
	BATTLE_COM_PETOUT,
	BATTLE_COM_ITEM,
	BATTLE_COM_BOOMERANG,
	BATTLE_COM_COMBO,
	BATTLE_COM_COMBOEND,
	BATTLE_COM_WAIT,

	BATTLE_COM_SEKIBAN = 1000,
	BATTLE_COM_S_RENZOKU,
	BATTLE_COM_S_GBREAK,
	BATTLE_COM_S_GUARDIAN_ATTACK,
	BATTLE_COM_S_GUARDIAN_GUARD,
	BATTLE_COM_S_CHARGE,
	BATTLE_COM_S_MIGHTY,
	BATTLE_COM_S_POWERBALANCE,
	BATTLE_COM_S_STATUSCHANGE,
	BATTLE_COM_S_EARTHROUND0,
	BATTLE_COM_S_EARTHROUND1,
	BATTLE_COM_S_LOSTESCAPE,
	BATTLE_COM_S_ABDUCT,
	BATTLE_COM_S_STEAL,
	BATTLE_COM_S_NOGUARD,
	BATTLE_COM_S_CHARGE_OK,
	BATTLE_COM_JYUJYUTU = 2000,

	BATTLE_COM_COMPELESCAPE, //強制離開

#ifdef __ATTACK_MAGIC
  BATTLE_COM_S_ATTACK_MAGIC,		// 寵物魔法
#endif

#ifdef _PSKILL_FALLGROUND
	BATTLE_COM_S_FALLRIDE,			//落馬術
#endif
#ifdef _PETSKILL_EXPLODE
	BATTLE_COM_S_EXPLODE,
#endif
#ifdef _PETSKILL_TIMID
	BATTLE_COM_S_TIMID,
#endif
#ifdef _PETSKILL_2TIMID
	BATTLE_COM_S_2TIMID,
#endif
#ifdef _PETSKILL_ANTINTER
	BATTLE_COM_S_ANTINTER,
#endif
#ifdef _PETSKILL_PROPERTY
	BATTLE_COM_S_PROPERTYSKILL,
#endif
#ifdef _PETSKILL_TEAR
	BATTLE_COM_S_PETSKILLTEAR,
#endif
#ifdef _BATTLE_LIGHTTAKE
	BATTLE_COM_S_LIGHTTAKE,
#endif
#ifdef _BATTLE_ATTCRAZED		// ANDY 瘋狂暴走
	BATTLE_COM_S_ATTCRAZED,
#endif
#ifdef _SHOOTCHESTNUT			// Syu ADD 寵技：丟栗子
	BATTLE_COM_S_ATTSHOOT,
#endif
#ifdef _BATTLESTEAL_FIX
	BATTLE_COM_S_STEALMONEY,
#endif
#ifdef _PRO_BATTLEENEMYSKILL
	BATTLE_COM_S_ENEMYRELIFE,		//NPC ENEMY 復活技能
	BATTLE_COM_S_ENEMYREHP,			//NPC ENEMY 補血技能
	BATTLE_COM_S_ENEMYHELP,			//NPC ENEMY 招人
#endif
#ifdef _SKILL_DAMAGETOHP
	BATTLE_COM_S_DAMAGETOHP,		//嗜血技
#endif
#ifdef _Skill_MPDAMAGE
	BATTLE_COM_S_MPDAMAGE,			//MP傷害
#endif
#ifdef _SKILL_WILDVIOLENT_ATT
    BATTLE_COM_S_WILDVIOLENTATTACK, //狂暴攻擊  vincent add 2002/05/16
#endif

#ifdef _SKILL_SPEEDY_ATT
    BATTLE_COM_S_SPEEDYATTACK,		//疾速攻擊  vincent add 2002/05/20
#endif
#ifdef _SKILL_GUARDBREAK2
    BATTLE_COM_S_GBREAK2,			//破除防禦2 vincent add 2002/05/20
#endif
#ifdef _SKILL_SACRIFICE
    BATTLE_COM_S_SACRIFICE,			//救援		vincent add 2002/05/30
#endif
#ifdef _SKILL_WEAKEN  
	    BATTLE_COM_S_WEAKEN,		//虛弱		vincent add 2002/07/11
#endif
#ifdef _SKILL_DEEPPOISON  
	    BATTLE_COM_S_DEEPPOISON,	//劇毒		vincent add 2002/07/16
#endif
#ifdef _SKILL_BARRIER  
	    BATTLE_COM_S_BARRIER,		//魔障		vincent add 2002/07/16
#endif
#ifdef _SKILL_NOCAST 
	    BATTLE_COM_S_NOCAST,		//沉默		vincent add 2002/07/16
#endif
#ifdef _SKILL_ROAR 
    BATTLE_COM_S_ROAR,			    //大吼		vincent add 2002/07/11
#endif
#ifdef _BATTLENPC_WARP_PLAYER
	BATTLE_COM_WARP,                // npc warp player
#endif
#ifdef _SKILL_TOOTH
	BATTLE_COM_S_TOOTHCRUSHE,
#endif
#ifdef _PSKILL_MODIFY
	BATTLE_COM_S_MODIFYATT,
#endif
#ifdef _PSKILL_MDFYATTACK
	BATTLE_COM_S_MDFYATTACK,
#endif
#ifdef _MAGIC_SUPERWALL
	BATTLE_COM_S_SUPERWALL,
#endif
#ifdef _SKILL_REFRESH
    BATTLE_COM_S_REFRESH,
#endif
#ifdef _VARY_WOLF
	BATTLE_COM_S_VARY,
#endif
#ifdef _PETSKILL_SETDUCK
	BATTLE_COM_S_SETDUCK,
#endif
#ifdef _MAGICPET_SKILL
	BATTLE_COM_S_SETMAGICPET,
#endif
#ifdef _PROFESSION_SKILL			// WON ADD 人物職業技能

// 巫師
	BATTLE_COM_S_VOLCANO_SPRINGS,	// 火山泉	
	BATTLE_COM_S_FIRE_BALL,			// 火星球
	BATTLE_COM_S_FIRE_SPEAR,		// 火龍槍
	BATTLE_COM_S_SUMMON_THUNDER,	// 召雷術
	BATTLE_COM_S_CURRENT,			// 電流術
	BATTLE_COM_S_STORM,				// 暴風雨
	BATTLE_COM_S_ICE_ARROW,			// 冰箭術
	BATTLE_COM_S_ICE_CRACK,			// 冰爆術
	BATTLE_COM_S_ICE_MIRROR,		// 冰鏡術
	BATTLE_COM_S_DOOM,				// 世界末日
	BATTLE_COM_S_BLOOD,				// 嗜血成性
	BATTLE_COM_S_BLOOD_WORMS,		// 嗜血蠱
	BATTLE_COM_S_SIGN,				// 一針見血
	BATTLE_COM_S_FIRE_ENCLOSE,		// 火附體	
	BATTLE_COM_S_ICE_ENCLOSE,		// 冰附體
	BATTLE_COM_S_THUNDER_ENCLOSE,	// 雷附體
	BATTLE_COM_S_ENCLOSE,			// 附身術
	BATTLE_COM_S_TRANSPOSE,			// 移形換位
// 勇士
	BATTLE_COM_S_BRUST,				// 爆擊			
	BATTLE_COM_S_CHAIN_ATK,			// 連環攻擊
	BATTLE_COM_S_AVOID,				// 迴避
	BATTLE_COM_S_RECOVERY,			// 補血
	BATTLE_COM_S_WEAPON_FOCUS,		// 武器專精
	BATTLE_COM_S_REBACK,			// 狀態回復
	BATTLE_COM_S_CHAIN_ATK_2,		// 雙重攻擊
	BATTLE_COM_S_SCAPEGOAT,			// 捨已為友
	BATTLE_COM_S_ENRAGE,			// 激化攻擊
	BATTLE_COM_S_COLLECT,			// 能量聚集
	BATTLE_COM_S_FOCUS,				// 專注戰鬥
	BATTLE_COM_S_SHIELD_ATTACK,		// 盾擊
	BATTLE_COM_S_DUAL_WEAPON,		// 二刀流
	BATTLE_COM_S_DEFLECT,			// 格檔
	BATTLE_COM_S_THROUGH_ATTACK,	// 貫穿攻擊
	BATTLE_COM_S_CAVALRY,			// 座騎攻擊
	BATTLE_COM_S_DEAD_ATTACK,		// 瀕死攻擊
	BATTLE_COM_S_CONVOLUTE,			// 迴旋攻擊
	BATTLE_COM_S_CHAOS,				// 混亂攻擊
// 獵人
	BATTLE_COM_S_TRAP,				// 陷阱			
	BATTLE_COM_S_TRACK,				// 追尋敵蹤
	BATTLE_COM_S_DOCILE,			// 馴伏寵物	
	BATTLE_COM_S_ENRAGE_PET,		// 激怒寵物	
	BATTLE_COM_S_DRAGNET,			// 天羅地網
	BATTLE_COM_S_ENTWINE,			// 樹根纏繞
	BATTLE_COM_S_AUTARKY,			// 自給自足
	BATTLE_COM_S_PLUNDER,			// 屍體掠奪
	BATTLE_COM_S_TOXIN_WEAPON,		// 毒素武器
	BATTLE_COM_S_RESIST_FIRE,		// 火抗性提升
	BATTLE_COM_S_RESIST_ICE,		// 冰抗性提升
	BATTLE_COM_S_RESIST_THUNDER,	// 雷抗性提升
	BATTLE_COM_S_G_RESIST_FIRE,		// 團體火抗性提升
	BATTLE_COM_S_G_RESIST_ICE,		// 團體冰抗性提升
	BATTLE_COM_S_G_RESIST_THUNDER,	// 團體雷抗性提升
	BATTLE_COM_S_ATTACK_WEAK,		// 弱點攻擊
	BATTLE_COM_S_INSTIGATE,			// 挑撥
	BATTLE_COM_S_OBLIVION,			// 遺忘
#ifdef _PROFESSION_ADDSKILL
	BATTLE_COM_S_RESIST_F_I_T,      // 自然威能
	BATTLE_COM_S_CALL_NATURE,       // 號召自然
    BATTLE_COM_S_BOUNDARY,          // 四屬性結界
#endif
#endif	

#ifdef _PET_SKILL_SARS				// WON ADD 毒煞蔓延
	BATTLE_COM_S_SARS,
#endif
#ifdef _SONIC_ATTACK				// WON ADD 音波攻擊
	BATTLE_COM_S_SONIC,
	BATTLE_COM_S_SONIC2,
#endif
#ifdef _PETSKILL_REGRET
	BATTLE_COM_S_REGRET,
	BATTLE_COM_S_REGRET2,
#endif
#ifdef _PETSKILL_GYRATE
    BATTLE_COM_S_GYRATE,
#endif
#ifdef _PETSKILL_ACUPUNCTURE
    BATTLE_COM_S_ACUPUNCTURE,
#endif
#ifdef _PETSKILL_RETRACE
	BATTLE_COM_S_RETRACE,
#endif
#ifdef _PETSKILL_HECTOR
	BATTLE_COM_S_HECTOR,
#endif
#ifdef _PETSKILL_FIREKILL
    BATTLE_COM_S_FIREKILL,
#endif
#ifdef _PETSKILL_DAMAGETOHP
	BATTLE_COM_S_DAMAGETOHP2, //暗月狂狼(嗜血技的變體) 
#endif
#ifdef _PETSKILL_BECOMEFOX
    BATTLE_COM_S_BECOMEFOX,
#endif
#ifdef _PETSKILL_BECOMEPIG
    BATTLE_COM_S_BECOMEPIG,
#endif
#ifdef _PETSKILL_SHOWMERCY
    BATTLE_COM_S_SHOWMERCY,
#endif
#ifdef _PETSKILL_LER
		BATTLE_COM_S_BAT_FLY,					// 雷爾技 - 群蝠四竄
		BATTLE_COM_S_DIVIDE_ATTACK,		// 雷爾技 - 分身地裂
#endif
#ifdef _PETSKILL_BATTLE_MODEL
		BATTLE_COM_S_BATTLE_MODEL,		// 寵物技能戰鬥模組
#endif

	BATTLE_COM_END
}BATTLE_COM;


enum{
	BATTLE_RET_NORMAL,
	BATTLE_RET_CRITICAL,
	BATTLE_RET_MISS,
	BATTLE_RET_DODGE,
	BATTLE_RET_ALLGUARD,
#ifdef _EQUIT_ARRANGE
	BATTLE_RET_ARRANGE,
#endif
	BATTLE_RET_END
}BATTLE_RET;


#define BC_FLG_NEW			(1<<0)
#define BC_FLG_DEAD			(1<<1)
#define BC_FLG_PLAYER		(1<<2)
#define BC_FLG_POISON		(1<<3)
#define BC_FLG_PARALYSIS	(1<<4)
#define BC_FLG_SLEEP		(1<<5)
#define BC_FLG_STONE		(1<<6)
#define BC_FLG_DRUNK		(1<<7)
#define BC_FLG_CONFUSION	(1<<8)
#define BC_FLG_HIDE			(1<<9)
#define BC_FLG_REVERSE		(1<<10)
#ifdef _MAGIC_WEAKEN
#define BC_FLG_WEAKEN		(1<<11) // 虛弱
#endif
#ifdef _MAGIC_DEEPPOISON
#define BC_FLG_DEEPPOISON	(1<<12) // 劇毒
#endif
#ifdef _MAGIC_BARRIER
#define BC_FLG_BARRIER		(1<<13) // 魔障
#endif
#ifdef _MAGIC_NOCAST
#define BC_FLG_NOCAST		(1<<14) // 沉默
#endif

#ifdef _PET_SKILL_SARS			// WON ADD 毒煞蔓延
#define BC_FLG_SARS			(1<<15) // 毒煞
#endif

#ifdef _PROFESSION_SKILL			// WON ADD 人物職業技能
#define BC_FLG_DIZZY		(1<<16)	// 暈眩	
#define BC_FLG_ENTWINE		(1<<17)	// 樹根纏繞
#define BC_FLG_DRAGNET		(1<<18)	// 天羅地網	
#define BC_FLG_ICECRACK	    (1<<19)	// 冰爆術	
#define BC_FLG_OBLIVION  	(1<<20)	// 遺忘
#define BC_FLG_ICEARROW		(1<<21)	// 冰箭
#define BC_FLG_BLOODWORMS 	(1<<22)	// 嗜血蠱
#define BC_FLG_SIGN 		(1<<23)	// 一針見血
#define BC_FLG_CARY			(1<<24) // 挑撥
#define BC_FLG_F_ENCLOSE	(1<<25) // 火附體
#define BC_FLG_I_ENCLOSE	(1<<26) // 冰附體
#define BC_FLG_T_ENCLOSE	(1<<27) // 雷附體
#ifdef _PROFESSION_ADDSKILL
#define BC_FLG_WATER	    (1<<28) // 水附體
#define BC_FLG_FEAR			(1<<29) // 恐懼
#endif
#ifdef _PETSKILL_LER
#define BC_FLG_CHANGE			(1<<30) // 雷爾變身
#endif
#endif




#define BP_FLG_JOIN					(1<<0)
#define BP_FLG_PLAYER_MENU_OFF		(1<<1)
#define BP_FLG_BOOMERANG			(1<<2)
#define BP_FLG_PET_MENU_OFF			(1<<3)
#define BP_FLG_ENEMY_SURPRISAL		(1<<4)
#define BP_FLG_PLAYER_SURPRISAL		(1<<5)


#define CHAR_BATTLEFLG_ULTIMATE		(1<<0)
#define CHAR_BATTLEFLG_AIBAD		(1<<1)
#define CHAR_BATTLEFLG_REVERSE		(1<<2)
#define CHAR_BATTLEFLG_GUARDIAN		(1<<3)
#define CHAR_BATTLEFLG_NORETURN		(1<<4)
#define CHAR_BATTLEFLG_RECOVERY		(1<<5)
#define CHAR_BATTLEFLG_ABIO			(1<<6)
#define CHAR_BATTLEFLG_NODUCK		(1<<7)

#define GETITEM_MAX	3
typedef struct
{
	int attacker;
}INVADER;

typedef struct
{
	int		charaindex;
	int		bid;
	int		escape;
	int		flg;
	int		guardian;
	int		duelpoint;
	int		getitem[GETITEM_MAX];
}BATTLE_ENTRY;
#define BENT_FLG_ULTIMATE	(1<<0)

typedef struct
{
	int		type;
	int		flg;
	int		common_dp;
	BATTLE_ENTRY	Entry[BATTLE_ENTRY_MAX];
}BATTLE_SIDE;

#define BSIDE_FLG_SURPRISE ( 1 << 0 )
#define BSIDE_FLG_HELP_OK ( 1 << 1 )



typedef struct _Battle
{
	BOOL	use;	/* 使っているかどうか */
	int		battleindex; /* バトル番号 */
	int		mode;	/* 現在の戦闘モード */
	int		type;	/* 戦闘タイプ (0:通常)(1:DUEL)(2:ボス戦) */
	int		dpbattle;	/* DPバトルか？ */
	int		norisk;	/* 死んでもリスクの無いバトルか？ */
	int		turn;	/* ターン数 */
	int		timer;	/* ウエイト等に使うタイマ */
	int		leaderindex; /* エンカウントを起こしたキャラのインデックス */
	int		winside; /* 勝利したサイド */
	int		field_att;	/* フィールドの属性 */
	int		att_count;	/* フィールドの属性変化中のタイマ */
	int		att_pow;	/* フィールドの属性変化中のパワー */
	int		field_no;
	int		flg;		/* 各種フラグ */
	BATTLE_SIDE		Side[2];
#ifdef _BATTLE_TIMESPEED
	unsigned int CreateTime;
	unsigned int EndTime;
	unsigned int PartTime;
	int flgTime;
	
#endif

#ifdef _PROFESSION_ADDSKILL
	//冰爆術存放
	int ice_count;//暫存數量
	int ice_bout[20];//回合計數
	BOOL ice_use[20];//是否使用
	int ice_toNo[20];//對象
	int ice_level[20];//攻擊力
	int ice_array[20];
	int ice_charaindex[20];
	int ice_attackNo[20];
#endif

#ifdef _DEATH_CONTEND
	int menum;
	int tonum;
#endif

#ifdef _ACTION_BULLSCR
	int enemynum;
#endif
	int		iEntryBack[BATTLE_ENTRY_MAX*2];		// 前ターン登録されていたメンバー
	int		iEntryBack2[BATTLE_ENTRY_MAX*2];		// 前ターン登録されていたメンバー
	int		createindex;	/* この戦闘を作り出したキャラインデックス(NPCなど) */
	int    (*WinFunc)( int battleindex, int charaindex );
#ifdef _DEATH_CONTEND//計算勝敗func
	int    (*PkFunc)( int menum, int tonum, int winside, int battlemap);
	int battlemap;
#endif
	struct _Battle *pNext;		// 次の観戦グループ
	struct _Battle *pBefore;	// 前の観戦グループ
}BATTLE;

#define BATTLE_FLG_FREEDP ( 1 << 0 )	// 逃げてもＤＰ減らない
#define BATTLE_FLG_CHARALOST   ( 1 << 1 )	// キャラが死んだり抜けたりしている



enum{	// 上のfield_att に入れる値
	BATTLE_ATTR_NONE = 0,	// 無属性
	BATTLE_ATTR_EARTH,		// 地
	BATTLE_ATTR_WATER,		// 水
	BATTLE_ATTR_FIRE,		// 火
	BATTLE_ATTR_WIND,		// 風
	BATTLE_ATTR_END
};


//krynn 2001/12/28
enum
{
	BATTLE_SIDE_RIGHT,		// 戰鬥中右側	right side of battle
	BATTLE_SIDE_LEFT,		// 戰鬥中左側	left side of battle
	BATTLE_SIDE_WATCH,		// 觀戰			watch battle player
};
//krynn end

#define TARGET_SIDE_0	20	// 右側（サイド０）指定
#define TARGET_SIDE_1	21	// 左側（サイド１）指定
#define TARGET_ALL	22	// 全体指定

#ifdef __ATTACK_MAGIC

#define TARGET_SIDE_0_B_ROW     26      // 右下後一列
#define TARGET_SIDE_0_F_ROW     25      // 右下前一列
#define TARGET_SIDE_1_F_ROW     24      // 左上前一列
#define TARGET_SIDE_1_B_ROW     23      // 左上後一列

// won add
#define	TARGER_THROUGH			27

#endif

extern int	gItemCrushRate;		// 破壊率デフォルト
extern BATTLE *BattleArray; 	/* 戦闘タスク配列 */
extern int BATTLE_battlenum;	/* 配列の数 */
extern char szAllBattleString[BATTLE_STRING_MAX];	/* 戦闘に使うコマンド文字列 */
extern char *pszBattleTop, *pszBattleLast;	/* 戦闘に使う文字列の位置 */
extern char szBadStatusString[];	// ステータス異常用文字列
extern int gWeponType;	// 現在の武器の種類
extern float gDamageDiv;	// ダメージ分割

#define BATTLE_CHECKINDEX( a ) ( ((a)>=BATTLE_battlenum || (a)<0 )?(FALSE):(TRUE) )
#define BATTLE_CHECKSIDE( a ) ( ((a)>=2 || (a)<0)?(FALSE):( TRUE) )
#define BATTLE_CHECKNO( a ) ( ((a)>=20 || (a)<0 )?(FALSE):(TRUE) )
#define BATTLE_CHECKADDRESS( a )  ((&BattleArray[0])<=(a) && (a)<=(&BattleArray[BATTLE_battlenum-1] )?(TRUE):(FALSE) ) 

#define IsBATTLING( a ) (CHAR_getWorkInt((a),CHAR_WORKBATTLEMODE)?(TRUE):(FALSE))

#define STRCPY_TAIL( _pszTop, _pszLast, _szBuffer)	{ int _len = strlen( _szBuffer ); ( _pszTop + _len < (_pszLast)-1 )?( memcpy( _pszTop, _szBuffer, _len ),	_pszTop += _len, _pszTop[0] = 0):(0); }

#define BATTLESTR_ADD( _szBuffer ){ int _len = strlen( _szBuffer ); ( pszBattleTop + _len < ( pszBattleLast)-1 )?( memcpy( pszBattleTop, _szBuffer, _len ),	pszBattleTop += _len, pszBattleTop[0] = 0):(0); }

#define BATTLE_MAP_MAX 219

#define CH_FIX_PLAYERLEVELUP	(+2)	// プレイヤーのレベルがアップ
#define CH_FIX_PLAYERDEAD		(-2)	// プレイヤーが通常死
#define CH_FIX_PLAYEULTIMATE	(-4)	// プレイヤーがアルティメット死
#define CH_FIX_PETESCAPE		(-1)	// ペットが逃げた

#define AI_FIX_PETLEVELUP		(+5*100)	// ペットがレベルアップ
#define AI_FIX_PETWIN			(+1)		// ペットが敵を倒した
#define AI_FIX_PETGOLDWIN		(+2*10)	// ペットがレベルの高い敵を倒した
#define AI_FIX_PETRECOVERY		(+10)		// 戦闘中に回復してもらった
#define AI_FIX_PETRESSURECT		(+3*100)	// 戦闘中に復活してもらった
//#define AI_FIX_PETRECOVERY	(+50)		// 戦闘中に回復してもらった

#define AI_FIX_SEKKAN 			(-2*100)	// 自分のペットを攻撃
#define AI_FIX_PLAYERULTIMATE	(-10*100)	// 自分の主人がアルティメット死
#define AI_FIX_PETULTIMATE		(-10*100)	// ペットがアルティメット死
#define AI_FIX_PLAYERDEAD		(-1*100)	// 自分の主人が気絶
#define AI_FIX_PETDEAD			(-5*100)	// ペットが気絶


#ifdef _Item_ReLifeAct
int BATTLE_getBattleDieIndex( int battleindex, int bid );
#endif

int BATTLE_No2Index( int battleindex, int No);

int BATTLE_Index2No( int battleindex, int charaindex);

BOOL BATTLE_initBattleArray( int battlenum);

int BATTLE_CreateBattle( void );
int BATTLE_DeleteBattle( int battleindex);

int BATTLE_NewEntry( int charaindex, int battleindex, int side);

#define BATTLE_Exit( charaindex, battleindex) _BATTLE_Exit( __FILE__, __LINE__, charaindex, battleindex)
INLINE int _BATTLE_Exit( char *file, int line, int charaindex ,int battleindex);

#define BATTLE_ExitAll( battleindex) _BATTLE_ExitAll( __FILE__, __LINE__, battleindex)
INLINE void _BATTLE_ExitAll( char *file, int line, int battleindex);

int BATTLE_CreateVsPlayer( int charaindex0,	int charaindex1);

int BATTLE_CreateVsEnemy( int charaindex, int mode,	int	npcindex);

int BATTLE_CountEntry( int battleindex,	int side);

int BATTLE_Loop( void );

int BATTLE_FinishSet( int battleindex );
int BATTLE_StopSet( int battleindex );
int BATTLE_RescueEntry( int charaindex, int toindex);

int BATTLE_PetDefaultExit( int charaindex, int battleindex);

int	BATTLE_PetDefaultEntry(
	int charaindex,	// ペットを持っているプレイヤーのＩＮＤＥＸ
	int battleindex,// バトルインデックス
	int side
);

BOOL BATTLE_RescueTry( int charaindex);

BOOL BATTLE_RescueParentTry(
		int charaindex,
		int pindex
	);

int BATTLE_DefaultAttacker(	int battleindex, int side);

BOOL BATTLE_IsThrowWepon( int itemindex);

void BATTLE_BadStatusString( int defNo, int status );
int BATTLE_MultiList( int battleindex, int toNo, int ToList[] );
BOOL BATTLE_IsCharge( int com );
BOOL BATTLE_CanMoveCheck( int charaindex );
int BATTLE_TargetCheck( int battleindex, int defNo);
char *BATTLE_CharTitle( int charaindex );
void BATTLE_EscapeDpSend( int battleindex, int charaindex );
int BATTLE_GetDuelPoint( int battleindex, int side, int num);
int BATTLE_TargetCheckDead(	int battleindex, int defNo);

void BATTLE_MultiListDead( int battleindex, int toNo, int ToList[] );
BOOL BATTLE_WatchTry( int charaindex);
int BATTLE_WatchEntry( int charaindex, int toindex);
void BATTLE_WatchStop( int charaindex );
int BATTLE_WatchUnLink( int battleindex );
void BATTLE_BpSendToWatch( BATTLE *pBattle,	char *pszBcString);

int BATTLE_GetWepon( int charaindex );

#ifdef _ITEM_EQUITSPACE
int BATTLE_GetEqShield( int charaindex );
#endif

int BATTLE_GetAttackCount( int charaindex );
int DoujyouRandomWeponSet( int charaindex );
void BATTLE_AttReverse( int charaindex );
void BATTLE_BadStatusAllClr( int charaindex );
#define CHAR_GETWORKINT_HIGH( index, pos )	( CHAR_getWorkInt( (index), (pos) ) >> 16 )
#define CHAR_SETWORKINT_HIGH( index, pos, set )	{ int iTmp = CHAR_getWorkInt( (index), (pos) ) & 0xFFFF, work = (set);	CHAR_setWorkInt( (index), (pos), (work << 16)|iTmp ); }
#define CHAR_GETWORKINT_LOW( index, pos )	( CHAR_getWorkInt( (index), (pos) ) & 0xFFFF )
#define CHAR_SETWORKINT_LOW( index, pos, set )	{ int iTmp = CHAR_getWorkInt( index, pos ) & 0xFFFF0000, work = (set);	CHAR_setWorkInt( (index), (pos), (work & 0x0000FFFF) | iTmp ); }

int Battle_getTotalBattleNum();

#ifdef _TYPE_TOXICATION
void CHAR_ComToxicationHp( int charaindex);
#endif

#ifdef _PROFESSION_SKILL			// WON ADD 人物職業技能
void BATTLE_ProfessionStatus_init( int battleindex, int charaindex );
void BATTLE_ProfessionStatusSeq( int battleindex, int charaindex);
#endif


#endif
