#ifndef __VERSION_H__
#define __VERSION_H__
#include "correct_bug.h"  //BUG FIX
#include "version_pk.h"   //PK SERVER

/*Server 版本控制*/
#define _NEW_SERVER_
#define _SERVER_NUMS
#define SERVER_VERSION "v7.01a"

#define UNIQUE_KEYCODEITEM	100
#define UNIQUE_KEYCODEPET	"i"

#define _SA_VERSION_70				// (可開放) WON ADD 石器時代7.0 的版本控制
//#define _DEFAULT_PKEY "Wonwonwon"	// (可開放)( 石器 6.0 )
//#define _RUNNING_KEY  "11111111"	// (可開放)( 石器 6.0 )
#define _DEFAULT_PKEY "ttttttttt"	// (可開放)( 石器 6.0 )
#define _RUNNING_KEY  "20041215"	// (可開放)( 石器 6.0 )

#define _VERSION_20010911	        // (可開放) 9.11 編碼修改 

// -------------------------------------------------------------------
//新功能：測試伺服器專用	人員：彥豪
#define _TEST_PETCREATE             // (不可開) ANDY 測寵的GM指令
//#define _LOGIN_ADDITEM			// (不可開) ANDY 登入加滿物品
//#define _NEW_TESTSERVER			// (不可開) ANDY 玩家創新人物時，給予五轉125等級及三隻寵物
//#define _TEST_PETFUSIONTIME		// (不可開) ANDY 測試寵物融合餵食時間加快
/* -------------------------------------------------------------------
 * 專案：家族莊園數量修改	人員：小瑜、彥勳、志偉、小民
 */

/*
修改裝園數量方式：
Account Server 程式:
	acfamily.h	MAX_FMPOINT要修改
Account Server 遊戲環境:
	saac/family/fmpoint/db_fmpoint檔案中須新增莊園入口坐標等設定
	莊園進入圖層|莊園進入Ｘ座標|莊園進入Ｙ座標|莊園族長圖層|莊園鄰近之村莊|佔領莊園之家族 index|佔領莊園之家族 name|佔領莊園之家族綜合聲望值
	例如：
		100|607|563|1041|1|3|shan|3132238
		200|73|588|2031|2|||
Game Server 程式：
	version.h FAMILY_MAXHOME、MANORNUM要修正
	family.c	 FM_PKFLOOR	須增設戰鬥圖層
				 FAMILY_RidePet() 中須加入族長家圖層
				 FAMILY_LeaderFunc() 中須加入邀請函id
	npcutil.c addNpcFamilyTax function中須加入(莊園圖層%100)此為莊園稅收部份
Game Server 遊戲環境:
	gmsv_v2/data/npc/family/manorsman.arg* id須修正
	gmsv_v2/data/npc/family/scheduleman.arg* id須修正
*/

#define _FAMILY_MANORNUM_CHANGE	// CoolFish 用來修改裝園數量(4.0)
#ifdef _FAMILY_MANORNUM_CHANGE
#define FAMILY_MAXHOME			9	// (4.0) 家族據點
#define MANORNUM                9	// (4.0) 莊園數量
#define FMPOINTNUM				9   // (4.0) 有據點家族的最大數量
#endif
#define _FMVER21		        // (可開放) 家族職掌功能  
#define _PERSONAL_FAME	    	// (可開放) Arminius 8.30 家族個人聲望  
// -------------------------------------------------------------------
//專案：石器活動自動化		人員：曉威、家麟
#define _SERVICE     	    	// (可開放) Terry 2001/10/03 活動功能自動化 

#define _EVERYONE_RIDE	    	// (可開放) Robin 0924 騎寵平民化  
// -------------------------------------------------------------------
//專案：攻擊性咒術的製作	人員：篤志
// #define __ATTACK_MAGIC_MSG   // (不開放)
#define __ATTACK_MAGIC          // (可開放)	提供戰場中的攻擊性咒術
#define _ADD_ACTION             // (可開放) 一般NPC動作狀況的設定

// -------------------------------------------------------------------
//專案：寵物、道具編碼		人員：小瑜
#define _UNIQUE_P_I	        	// (可開放) CoolFish 2001/10/11 

// -------------------------------------------------------------------
//專案：新版人物儲存pn 及 當機不回朔 1018	人員：彥勳
#define _NEWSAVE                // (可開放) Robin
#define _STORECHAR              // (可開放) Robin
#define _DROPCHECK              // (可開放) Robin 防止重複丟道具
#define _DROPCHECK2             // (可開放) Robin 防止重複丟道具
// -------------------------------------------------------------------
//專案 : 精靈王	人員 : 彥豪
#define _PET_TRANS           	// (可開放) 寵物轉生功能
#define _NEW_WARPMAN        	// (可開放) NPC多重WARP功能
#define _PET_TALK	            // (可開放) 寵物對話
#define _PET_LIMITLEVEL	        // (可開放) 特殊寵設計(瑪蕾)
#define _MAP_NOEXIT	            // (可開放) 特殊地圖不得原登&原登回指定點
// -------------------------------------------------------------------
// 專案：GM指令、層級Reload		人員：小瑜
#define _GMRELOAD           	// (可開放) CoolFish: 2001/11/12 
// -------------------------------------------------------------------
// 專案：伊甸大陸	人員：彥豪、阿民、小瑜
#define _GAMBLE_ROULETTE	    // (可開放) 彥豪 賭場輪盤	寶寶小豬  
#define _FIX_GAMBLENUM          // (可開放) 輪盤修正 : 下線扣積分 
#define _GAMBLE_BANK	        // (可開放) 彥豪  賭場銀行  
#define _DROPSTAKENEW	        // (可開放) CoolFish:  賭場下注  
#define _PETRACE	            // (可開放) CoolFish: 2001/12/3 寵物競速場  
#define _NEWEVENT               // (可開放) CoolFish: 2001/12/5 新的任務旗標  
//#define _BIGSMALL             // (不可開) Arminius 12.15 骰子比大小 
//#define _AUCTIONEER           // (不可開) Arminius 12.20 拍賣店  
// -------------------------------------------------------------------
//專案：鞭炮製作	人員：曉威
#define _ITEM_FIRECRACKER       // (可開放) 戰鬥中使用鞭炮可以把年獸嚇走
// -------------------------------------------------------------------
//新功能：莊園PK規則(酷哥定的)		人員：志偉、志旺
#define _MANOR_PKRULE           // (可開放)
#define _MANOR_FIXFMPK          // (可開放) WON ADD 家族約戰的 GM 指令，測試用的
// -------------------------------------------------------------------
// 新功能：新功能&寵物技能		人員：彥豪
#define _PSKILL_FALLGROUND		// (可開放) 寵物技能  (落馬術)
#define _ENEMY_FALLGROUND		// (可開放) NPC 落馬
#define _BATTLE_NEWPOWER		// (可開放)
#define _Item_MoonAct	        // (可開放) 月神的守護
#define _Item_DeathAct          // (可開放) 惡魔的首飾
#define _Item_PetAct            // (可開放) 馴獸戒指
#define _Item_ReLifeAct         // (可開放) 需要itemset1.txt 才能開) 替身娃娃
#define _BATTLESTEAL_FIX        // (可開放) 修正偷竊
#define _CFREE_petskill         // (可開放) 需要petskill1.txt 才能開) 新學習寵物技能技能
#define _data_newreadfile       // (可開放) 新的server setup.cf 檔案 包含 itemset1.txt petskill1.txt 兩個新檔
#define _ITEM_MAXUSERNUM        // (可開放) 新 itemset1.txt ITEM 使用次數
// -------------------------------------------------------------------
//新功能：伊甸二期		人員：彥豪
#define _PET_TALKPRO      		// (可開放) 寵物對話加強	& 改 END
#define _PRO_NPCFIX	     		// (可開放) NPC功能加強
#define _PRO_BATTLEENEMYSKILL	// (可開放) 戰鬥加強	提昇NPC的AI技能
#define _NEW_WARPPOINT		    // (可開放) 傳送點多重條件判斷WARP
#define _FIX_CHECKITEM	      	// (可開放) 道具不做非法檢查
// -------------------------------------------------------------------
//新功能：伊甸二	人員：小民,WON
#define _ADD_ENCOUNT             // (可開放) WON 增加敵遭遇觸發修件 
#define _WON_PET_MAIL_LOG		 // (可開放) WON 玩家寄寵郵 LOG 
// -------------------------------------------------------------------
// 新功能：伊甸三期	4.0	人員：彥豪, 志偉 , 又榮 , 志旺
//#define _LOSE_FINCH_				// (不可開) ANDY 朱雀技能
#define _FIX_FAMILY_PK_LIMIT	    // (可開放) WON 將可挑戰莊園的由前十大改為前二十大
#define _ADD_POOL_ITEM				// (可開放) WON 增加可寄放的道具
#define _ITEM_METAMO                // (可開放) shan metamo sprite. code:robin->shan
#define _ENEMY_ATTACK_AI            // (可開放) shan enemy attack mode. code:shan  
#define _ITEM_TIME_LIMIT            // (可開放) shan time limit of item. code:shan
#define _MIND_ICON                  // (可開放) shan show icon from char mind. code:shan
#define _BATTLENPC_WARP_PLAYER      // (可開放) shan npc warp player when n round. code:shan
#define _ITEM_WARP_FIX_BI           // (可開放) shan fix bbi to bi. code:shan
#define _SKILL_DAMAGETOHP			// (可開放) ANDY 嗜血技 
#define _Skill_MPDAMAGE				// (可開放) ANDY MP傷害
#define _WAEI_KICK					// (可開放) ANDY 跨星球踢人
#define _BATTLE_TIMESPEED			// (可開放) ANDY 戰鬥時間 測試(北京不必開，擋加速功能)
#define _NEW_RIDEPETS				// (可開放) ANDY 新騎寵
#define _FIXBUG_ATTACKBOW			// (可開放) ANDY 修正騎寵使用投擲武器bug
#define _NPCCHANGE_PLAYERIMG		// (可開放) ANDY NPC改變玩家圖號, 強制落馬,認玩家圖號 (測完)
#define _ITEM_ATTSKILLMAGIC			// (可開放) ANDY 道具技能  
#define _EQUIT_DEFMAGIC				// (可開放) ANDY 抗魔裝備
#define _FIX_MAGICDAMAGE			// (可開放) ANDY 修正魔法公式 三期開放
#define _ITEM_CONSTITUTION			// (可開放) ANDY 修正個人體質
#define _MAGIC_REHPAI				// (可開放) ANDY 補血AI
#define _TRANSER_MAN				// (可開放) ANDY 傳送師
#define _TAKE_ITEMDAMAGE			// (可開放) ANDY 道具損壞	ITEMSET2
#define _FIX_FIRECRACKER			// (可開放) ANDY 修正 鞭炮
#define _FIX_ITEMPROB				// (可開放) ANDY 修正 掉寶率
#define _ADD_DEAMGEDEFC				// (可開放) ANDY 額外傷害 & 附加攻擊	ITEMSET2
#define _FIX_MAXCHARMP				// (可開放) ANDY MP上限
#define _ITEM_INSLAY				// (可開放) ANDY 鑲寶石 動作
#define _ITEMSET2_ITEM				// (可開放) ANDY ITEMSET2
#define _ITEM_NPCCHANGE				// (可開放) ANDY 工匠 (提煉, 精工)
#define _PETSKILL_FIXITEM			// (可開放) ANDY 修復 && 同道具損壞
#define _FIX_SETWORKINT				// (可開放) ANDY (FIXWORKINT)
#define _ITEM_ADDEXP				// (可開放) vincent  道具:指定時間內增加經驗值上昇幅度
#define _ITEM_ADDEXP2				// 智果時間可以累加 Robin
#define _FIX_METAMORIDE				// (可開放) ANDY
#define _M_SERVER					// (可開放) GM SERVER (北京&韓國不必開)
//#define _FIX_MSERVER				// (可開放) GM SERVER PROC (北京&韓國不必開)
#define _CHECK_ITEMDAMAGE			// (可開放) 道具損壞修正	7/26
#define _SKILL_WILDVIOLENT_ATT		// (可開放) vincent  寵技:狂暴攻擊
#define _MAGIC_WEAKEN				// (可開放) vincent  精靈:虛弱
#define _SKILL_WEAKEN				// (可開放) vincent  寵技:虛弱//需要開#define _MAGIC_WEAKEN
#define _SKILL_SPEEDY_ATT			// (可開放) vincent  寵技:疾速攻擊
#define _SKILL_GUARDBREAK2			// (可開放) vincent  寵技:破除防禦2
#define _SKILL_SACRIFICE			// (可開放) vincent  寵技:救援
#define _PSKILL_MODIFY				// (可開放) ANDY 屬性強化攻擊
#define _PSKILL_MDFYATTACK			// (可開放) ANDY 攻擊轉化屬性
#define _MAGIC_DEFMAGICATT			// (可開放) ANDY 魔法防禦
#define _MAGIC_SUPERWALL			// (可開放) ANDY 鐵壁防禦
#define _OTHER_MAGICSTAUTS			// (可開放) ANDY 
#define _SKILL_TOOTH				// (可開放) ANDY 囓齒術
#define _MAGIC_DEEPPOISON			// (可開放) vincent  精靈:劇毒
#define _MAGIC_BARRIER				// (可開放) vincent  精靈:魔障
#define _MAGIC_NOCAST				// (可開放) vincent  精靈:沉默
#define _ITEM_CRACKER               // (可開放) vincent  道具:拉炮//需要開#define _MIND_ICON
#define _SKILL_DEEPPOISON			// (可開放) vincent  寵技:劇毒//需要開#define _MAGIC_DEEPPOISON
#define _SKILL_BARRIER				// (可開放) vincent  寵技:魔障//需要開#define _MAGIC_BARRIER
#define _SKILL_NOCAST				// (可開放) vincent  寵技:沉默//需要開#define _MAGIC_NOCAST
#define _SKILL_ROAR					// (可開放) vincent  寵技:大吼(剋年獸)
#define _SKILL_REFRESH			    // (可開放) vincent  寵技:解除異常狀態
#define _ITEM_REFRESH               // (可開放) vincent  解除異常狀態道具
#define _MAGIC_TOCALL				// (可開放) kjl		 召喚	02/06/20 kjl

// -------------------------------------------------------------------
// 新功能：來吉卡功能  人員：彥豪
#define _ITEMSET3_ITEM				// (可開放) ANDY itemset3.txt
#define _SUIT_ITEM					// (可開放) ANDY (套裝) 來吉卡 需用itemset3.txt
#define _PETSKILL_SETDUCK			// (可開放) ANDY 蒙奇技能
#define _VARY_WOLF                  // (可開放) pet skill : vary wolf. code:shan
// -------------------------------------------------------------------
// 新功能：專案5.0		人員：
//#define _ITEMTBL_STAIC			// (不可開) ANDY (靜態ITEMTBL)
#define _USEWARP_FORNUM				// (可開放) ANDY 記憶羽毛 來吉卡
#define _IMPRECATE_ITEM				// (可開放) ANDY 特殊道具 來吉卡
#define _MAGICPET_SKILL				// (可開放) ANDY 魔法寵技能
#define _ITEM_CHECKWARES			// (可開放) ANDY 鎖定傳送不能
#define _NPC_REPLACEMENT			// (可開放) ANDY 置換戰鬥npc
#define _EMENY_CHANCEMAN			// (可開放) ANDY EMENY 選擇
#define _TYPE_TOXICATION			// (可開放) ANDY 中毒無法回血
#define _PET_FUSION					// (可開放) ANDY 寵物融合 enemybase1.txt
#define _NPC_FUSION					// (可開放) ANDY NPC寵物融合
#define _ITEM_EDITBASES				// (可開放) ANDY 營養劑
#define _PET_EVOLUTION				// (可開放) ANDY 進化表

// -------------------------------------------------------------------
// 新功能：5.0追加部分		人員：
#define _AVID_TRADETRYBUG			// (可開放) ANDY 防止交易中丟寵/道具 寄(寵物)郵件 石幣
#define _ASSESS_SYSEFFICACY			// (可開放) ANDY 評估系統效能 12/04 更新
#define _ASSESS_SYSEFFICACY_SUB		// (可開放) Robin 評估系統效能 增加檢查內迴圈
#define _PREVENT_TEAMATTACK			// (可開放) ANDY 防止同隊互打 12/04 更新
#define _ITEM_ORNAMENTS				// (可開放) ANDY 裝飾道具	12/04 更新 //出版部
#define _CHIKULA_STONE				// (可開放) ANDY 奇克拉之石 12/04 更新 //來吉卡 寵包
#define _SEND_EFFECT				// (可開放) WON  AC送下雪、下雨等特效 12/04 更新
#define _PETMAIL_DEFNUMS			// (可開放) ANDY 寵物郵件限制
#define _TEAM_KICKPARTY				// (可開放) ANDY 隊長踢人(非戰鬥狀態)
#define _PETSKILL_TIMID				// (可開放) ANDY 寵技-怯戰
#define _PETS_SELECTCON				// (可開放) ANDY 寵物不可點選出戰
#define _CHRISTMAS_REDSOCKS			// (可開放) ANDY 聖誕紅襪
#define _FIX_ARRAYBUG				// (可開放) ANDY 修正array 溢位
#define _USER_CHARLOOPS				// (可開放) ANDY 新用物件LOOP
#define _BATTLE_PROPERTY			// (可開放) ANDY 戰鬥屬性特性
#define _PETSKILL_PROPERTY			// (可開放) ANDY 無屬魔獸寵技
#define _ITEM_FIXALLBASE			// (可開放) ANDY 修復之石
#define _ITEM_LOVERPARTY			// (可開放) ANDY 情人節道具
#define _ITEM_FORUSERNAMES			// (可開放) ANDY 道具所有人
#define _BATTLECOMMAND_TIME			// (可開放) ANDY 防止玩家開緩速(回合戰鬥時間)
#define _NPCENEMY_ADDPOWER			// (可開放) ANDY 修改怪物的能力值
//#define _BAD_PLAYER				// (不可開) WON  送壞玩家去關


// -------------------------------------------------------------------
// 新功能：專案6.0		人員：
//
#define _PETSKILL_CANNEDFOOD		// (可開放) ANDY 寵物技能罐頭	for 6.0
#define _TEST_DROPITEMS				// (可開放) ANDY 測試丟寵
#define _MAP_WARPPOINT				// (可開放) ANDY Map WarpPoint
#define _AUTO_ADDADDRESS			// (可開放) ANDY 自動新增名片
#define _DEFEND_TIMECHECK			// (可開放) ANDY 內部測試時間
#define _TREASURE_BOX				// (可開放) ANDY 寶箱
#define _BENEMY_EVENTRUN			// (可開放) ANDY 戰鬥npc給道具
#define _BATTLE_ABDUCTII			// (可開放) ANDY 旅途夥伴2
#define _BATTLE_LIGHTTAKE			// (可開放) ANDY 採光術
#define _BATTLE_ATTCRAZED			// (可開放) ANDY 瘋狂暴走
#define _STATUS_WATERWORD			// (可開放) ANDY 水世界狀態
#define _ITEM_WATERWORDSTATUS		// (可開放) ANDY 水世界狀態道具
#define _KILL_12_STOP_GMSV			// (可開放) WON  下sigusr2後關閉GMSV
//#define _DELBORNPLACE				// (可開放) Syu  6.0 統一出生於新手村
#define _REEDIT_PETBBIS				// (可開放) ANDY 修正寵物波比圖形
#define _CAPTURE_FREES				// (可開放) ANDY 補抓條件
#define _THROWITEM_ITEMS			// (可開放) 丟擲後有獎品
#define _OBJSEND_C					// (可開放) ANDY _OBJSEND_C
#define _ADD_SHOW_ITEMDAMAGE		// (可開放) WON  顯示物品耐久度

//--------------------------------------------------------------------------
//專案 7.0	職業系統	人員：彥豪 志旺 旭誠 奇銓

#define _CHANGETRADERULE		    // (可開放) Syu ADD 交易規則修訂
#define _TRADE_SHOWHP				// (可開放) Syu ADD 交易顯示血量
#define _SHOOTCHESTNUT				// (可開放) Syu ADD 寵技：丟栗子
#define _SKILLLIMIT					// (可開放) Syu ADD 不得攻擊我方限制
#define _TRADESYSTEM2				// (可開放) Syu ADD 新交易系統
#define _NPC_NOSAVEPOINT			// (可開放) ANDY 紀錄點不存人物資料
#define _ACFMPK_LIST				// (可開放) ANDY 家族對戰列表修正
#define _PETSKILL2_TXT				// (可開放) ANDY petskill2.txt
#define _PETSKILL_CHECKTYPE			// (可開放) ANDY 寵物技能使用時機
#define _PETSKILL_TEAR				// (可開放) ANDY 寵技 撕裂傷口
#define _NPC_MAKEPAIR				// (可開放) ANDY NPC 配對
#define _ITEMSET4_TXT				// (可開放) ANDY itemset4.txt
#define _ITEM_PILENUMS				// (可開放) ANDY 道具素材堆疊 需 itemset4
#define _ITEM_PILEFORTRADE			// (可開放) ANDY 交易堆疊
#define _ITEM_EQUITSPACE			// (可開放) ANDY 新裝備欄位
#define _PET_LOSTPET				// (可開放) ANDY 遺失寵物搜尋計劃寵物
#define _ITEMSET5_TXT				// (可開放) ANDY itemset5.txt
#define _ITEMSET6_TXT				// (可開放) ANDY itemset6.txt
#define _EQUIT_ARRANGE				// (可開放) ANDY 擋格 需要 itemset5.txt
#define _EQUIT_SEQUENCE				// (可開放) ANDY 攻擊順序 需要 itemset5.txt
#define _EQUIT_ADDPILE				// (可開放) ANDY 增加可堆疊數 需要 itemset5.txt
#define _EQUIT_HITRIGHT				// (可開放) ANDY 命中 需要 itemset5.txt
#define _EQUIT_NEGLECTGUARD			// (可開放) ANDY 忽視目標防禦力% 需要 itemset6.txt
#define _EQUIT_NEWGLOVE				// (可開放) ANDY 手套欄位
#define _FM_JOINLIMIT				// (可開放) ANDY 加入家族條件限制
#define _FIX_MAX_GOLD				// (可開放) WON ADD 增加人物金錢上限 
#define _CK_ONLINE_PLAYER_COUNT		// (可開放) WON 計算線上人數 (要配合ac，否則會重開)	
#define _PET_SKILL_SARS				// (可開放) WON ADD 毒煞蔓延
#define _SONIC_ATTACK				// (可開放) WON ADD 音波攻擊
#define _NET_REDUCESEND				// (可開放) ANDY 減少DB資料傳送
#define _FEV_ADD_NEW_ITEM			// (可開放) Change 增加復活守精
#define _ALLDOMAN					// (可開放) Syu ADD 排行榜NPC
#define _LOCKHELP_OK				// (可開放) Syu ADD 鎖定不可加入戰鬥
#define _TELLCHANNEL				// (可開放) Syu ADD 密語頻道
// 新功能：職業
#define _NEWREQUESTPROTOCOL			// (可開放) Syu ADD 新增Protocol要求細項
#define _OUTOFBATTLESKILL			// (可開放) Syu ADD 非戰鬥時技能Protocol
#define _CHAR_PROFESSION			// (可開放) WON ADD 人物職業欄位
#define _PROFESSION_SKILL			// (可開放) WON ADD 人物職業技能
#define _NPC_WELFARE				// (可開放) WON ADD 職業NPC
#define _PROFESSION_FIX_LEVEL		// (可開放) WON ADD 修正職業經驗值(北京&韓國不必開)
#define _GM_SIGUSR2					// (可開放) Change 下達sigusr2後執行 gm_sigusr_command檔案內的GM指令,若沒有指令就關閉GMSV
#define _GM_BROADCAST				// (可開放) WON ADD 客服公告系統
#define _NPC_ADDLEVELUP				// (可開放) ANDY NPC增加玩家等級
#define _CHAR_FIXDATADEF			// (可開放) ANDY 修正人物資料初始
//系統改良
#define _SIMPLIFY_ITEMSTRING		// (可開放) ANDY 簡化道具字串
#define _CHAR_NEWLOGOUT				// (可開放) ANDY 登出新機制
#define _CHATROOMPROTOCOL			// (可開放) Syu ADD 聊天室頻道 8/27

#define _CHAR_POOLITEM				// (可開放) ANDY 人物道具倉庫共用
#define _NPC_DEPOTITEM				// (可開放) ANDY 人物道具倉庫共用

#define _SIMPLIFY_PETSTRING			// (可開放) ANDY 簡化寵物存檔字串
#define _SIMPLIFY_ITEMSTRING2		// (可開放) ANDY 簡化道具字串2
#define _WOLF_TAKE_AXE				// (可開放) WON ADD 抓雙頭狼的限制
#define _FIX_UNNECESSARY			// (可開放) ANDY 修正不必要的判斷
#define _ITEM_MAGICRECOVERY			// (可開放) ANDY 新光鏡守
//9/17更新
#define _PETSKILL_GYRATE			// (可開放) Change 寵技:迴旋攻擊
#define _PETSKILL_ACUPUNCTURE		// (可開放) Change 寵技:針刺外皮 (client端的_PETSKILL_ACUPUNCTURE也要開)
#define _PETSKILL_RETRACE			// (可開放) Change 寵技:追跡攻擊
#define _PETSKILL_HECTOR			// (可開放) Change 寵技:威嚇
//10/13更新
#define _PETSKILL_FIREKILL			// (可開放) Change 寵技:火線獵殺
#define _PETSKILL_DAMAGETOHP		// (可開放) Change 寵技:暗月狂狼(嗜血技的變體) 
#define _PETSKILL_BECOMEFOX         // (可開放) Change 寵技:媚惑術
//10/22更新
// 新功能：修改WORKER(新付費機制)
#define _FIX_MESSAGE				// (可開放) WON ADD 修改WORKER封包內容 (北京&韓國不必開)     

//11/12更新
#define _PETSKILL_SHOWMERCY         // (可開) Change 寵技:手下留情
#define _NPC_ActionFreeCmp          // (可開) Change 替NPC參數加入新的比較方式
//--------------------------------------------------------------------------


//11/26
#define _NPC_ProfessionTrans      // (可開) Change 替職業NPC加入判斷轉生
#define _NPC_NewDelPet            // (可開) Change 替NPC加入刪除寵物(原本就有了,只是用warpman使用時不成功,所以先做一個頂一下)
//#define _MUSEUM // 石器博物館  Robin 2003/11
	#ifdef _MUSEUM
	#define _DELBORNPLACE
	#endif
#define _CHANGEGALAXY	// (可開) 星系移民　Robin 2003/10
#define _ALLDOMAN_DEBUG           // (可開) Change 修正重啟server後排行榜消失bug
//20031217
#define _CHRISTMAS_REDSOCKS_NEW	  // (可開) Change 新聖誕紅襪
#define _PETSKILL_COMBINED      // (可開) Change 寵技:難得糊塗 [綜合魔法(依參數亂數取出魔法)]
//情人節活動
#define _NPC_ADDWARPMAN1			// (可開放) Change npcgen_warpman增加功能(設定組隊人數、設定組隊男女人數)
#define _ITEM_QUITPARTY			// (可開放) Change 隊伍解散時特定道具即會消失
#define _ITEM_ADDEQUIPEXP       // (可開) Change 裝備後經驗值增加
//2004/2/18
#define _ITEM_LVUPUP            // (可開) Change 突破寵物等級限制的藥(玄武專用)
//2004/02/25
#define _PETSKILL_BECOMEPIG     // (可開) Change 寵技:烏力化
#define _ITEM_UNBECOMEPIG       // (可開) Change 烏力化的解藥
#define _PET_TALKBBI            // (可開) Change 增加判斷bbi
#define _ITEM_USEMAGIC			// (可開) Change 由道具使用魔法
//2004/04/07
#define _ITEM_PROPERTY          // (可開) Change 改變四屬性道具
#define _SUIT_ADDENDUM          // (可開) Change 替套裝增加功能 (_SUIT_ITEM要開)
//2004/05/05
#define _ITEM_CHECKDROPATLOGOUT   // (可開) Change 檢查回紀錄點時身上有無登出後消失道具,若有,不可回紀錄點
#define _FM_MODIFY				// 家族功能修改(開放)
#define _CHANNEL_MODIFY		// 頻道功能整合(開放)
#define _STANDBYPET		// Robin 待機寵封包
#define	_PROSKILL_OPTIMUM	// Robin 職業技能處理最佳化
#define	_PETSKILL_OPTIMUM	// Robin 寵物技能處理最佳化
#define _MAGIC_OPTIMUM		// Robin Magic table 最佳化
#define _AC_PIORITY		// Nuke 提高AC的優先權(可開)
//2004/05/12
#define _CHAR_POOLPET				// (可開放) Robin 人物寵物倉庫共用
#define _NPC_DEPOTPET				// (可開放) Robin 人物寵物倉庫共用
#define _TIMEOUTKICK	// (可開) 時段制踢人　Robin 2003/10
#define _MAGIC_RESIST_EQUIT			// (可開) WON ADD 職業抗性裝備
#define _LOG_OTHER // 雜項訊息專用LOG

//--------------------------------------------------------------------------
//機動營測試中


// -------------------------------------------------------------------
//#define _NPC_QUIZSCOPE          // (不可開) Change 替NPC增加題號範圍參數
//#define _ITEM_ADDCOMBO          // (不可開) Change 裝備後合擊率增加(戰場上要有2個人以上帶才有用)未完成...
//#define _NPC_AIRDELITEM         // (不可開) Change 搭加美航空時所扣除的道具(就是飛機票啦)
//#define _NPC_AIRLEVEL           // (不可開) Change 搭加美航空的最大最等級限制(等級過高不給搭)

//#define _ANDYLOG_TEST
//#define _NPC_WELFARE_2			// (不可開) WON ADD 職業NPC-2
//#define _UNIVERSE_CHATROOM			// (不可開) ANDY 全星系聊天
//#define _FIX_ALLDOMAN				// (不可開) ANDY 修正英雄戰場
//製作中
//#define _SEND_PROF_DATA			// (不可開) ANDY ADD 傳送職業更新給cli
//#define _AUCPROTOCOL				// (不可開) Syu ADD 拍賣頻道開關Protocol
//#define _NPC_SELLSTH				// (不可開) ANDY 拍賣系統
//#define _NPC_SEPARATEDATA			// (不可開) ANDY NPC 資料分離

//------------------------------------------------------------------------
// 以下為凍結功能
//------------------------------------------------------------------------
//#define fix_login_event			// (不可開) WON 補回伊甸一期的聲望 (北京不必開)
//#define _FIX_DEL_MAP				// (不可開) WON 玩家抽地圖送監獄 
//#define _PETFIGHT					// (不可開) 寵物競技場 
//#define _EXM_RANDITEM				// (不可開) NPC (Exchangeman) 亂數要求物品 (已取消)
//#define add_amulet_map			// (不可開) WON 增加太陽神首飾的範圍
//#define _ACFMPK_NOFREE			// (不可開) ANDY 家族無條件
//--------------------------------------------------------------------------
// 6.0追加功能 pile

//#define _IMPOROVE_ITEMTABLE		// (不可開) ANDY 改善ITEM TABLE
//#define _NPC_SHOPALTER01			// (不可開) Change 買賣系統修改(配合Client改為圖形介面所做的修改)
//#define _GM_IDENTIFY				// (不可開) Change 是否秀出GM頭頂上識別字串的GM指令
//#define _GSERVER_RUNTIME          // (不可開) Change 傳送GSERVER執行多少時間給Mserver (Mserver要配合)
//#define _NPC_VERYWELFARE			// (不可開) Change 非常好康A的NPC
//--------------------------------------------------------------------------
//#define _ADD_NEWEVENT             // (不可開) WON 多增任務旗標
//#define _BLACK_MARKET				// (不可開) shan a black market. code:shan 
//#define _DYNAMIC_BUILDER_MAP		// (不可開) shan (test)dynamic builder on the upper map. code:shan
//#define _TEST_ITEMMEMORY			// (不可開) ANDY (測試ITEMTABL)
//#define _LASTERR_FUNCTION			// (不可開) ANDY (lastFunction)
//#define _MARKET_TRADE				// (不可開) ANDY 交易市集
//#define _PAUCTION_MAN				// (不可開) ANDY 
//#define _SKILL_SELFEXPLODE		// (不可開) vincent  寵技:自爆(缺圖)
//#define _ADD_FAMILY_TAX			// (不可開) WON 增加莊園稅收 (要配合ac，否則會重開)
//#define _SYUTRY 
/* -------------------------------------------------------------------
 * 新功能：PK星系 1121 人員：彥勳,小金
 */
//#define _PKPETSHOP				// (不可開)

// -------------------------------------------------------------------
//特殊活動功能
//#define _CHECK_GAMESPEED			// (不可開) ANDY 加速檢測
//#define _NPCSERVER_NEW			// (不可開) ANDY NPC 伺服器
//#define _OBJS_TALKFUN				// (不可開) ANDY TALK 搜尋法
//#define _NEWOPEN_MAXEXP			// (不可開) ANDY MAX經驗值
//#define _MERGE_LOG				// (不可開) ANDY 合成 LOG
//#define _CHECK_UCODE				// (不可開) ANDY 檢查

//#define _DEATH_FAMILYCONTEND		// (不可開) ANDY 家族戰報名

//#define _NEW_INSERVERPOINT		// (不可開) ANDY 登入座標
//#define _DEATH_CONTENDWATCH		// (不可開) ANDY 決鬥觀看
//#define _DEATH_CONTENDTEST		// (不可開) ANDY 死鬥賽程加快

//#define _DEATH_FAMILY_GM_COMMAND	// (可開) WON ADD 家族戰GM指令
//#define _DEATH_FAMILY_LOGIN_CHECK	// (可開) WON ADD 家族戰登入檢查
//#define _DEATH_FAMILY_STRUCT		// (可開) WON ADD 家族戰存放勝負資料
//#define _DEATH_FAMILY_CANT_SCHEDULE // (可開) WON ADD 家族戰玩家不能自行約戰

//#define _ACTION_BULLSCR			// (不可開) ANDY 蠻牛&朱雀 活動功能 (7/30以後才能關閉)
//#define _BULL_FIXPLAYER			// (不可開) ANDY 修正人物蠻牛活動功能
//#define _BULL_CUTBULLSCR			// (不可開) ANDY 終止蠻牛活動積分
//#define _ACTION_GMQUE				// (不可開) ANDY GM活動功能 永保留
//#define _PET_FUSIONSHOWTIME		// (不可開) ANDY 寵物蛋秀時間

//#define _NEW_PLAYERGOLD			// (可開放) ANDY 新帳號給錢
// -------------------------------------------------------------------
//登入檢查動作

//#define _FIX_TSKILLCAN			// (可開放) ANDY 修正寵物T技罐頭 以及 清除T字技(非機暴)
//#define _CHANGEGOATMETAMO			// (可開放) Syu  羊年獸更換新圖

// -------------------------------------------------------------------
// 新功能：GM密頻功能擴充
//#define _GM_WARP_PLAYER			// (不可開)	WON 傳送玩家 (北京&韓國不必開)
//#define _RECAL_ASK_PLAYER			// (不可開) WON 要求人物資料 (北京&韓國不必開)
//#define _RECAL_SEND_COUNT			// (不可開) WON 傳送GS資訊 (北京&韓國不必開)
//#define _RECAL_SERVER_OFF			// (不可開) WON MSERVER 關閉 GS	(北京&韓國不必開)


//#define	_ENEMYTEMP_OPTIMUM	// Robin EnemyBase table 最佳化
//#define	_ENEMY_OPTIMUM		// Robin Enemy table 最佳化
//#define	_CHECK_BATTLETIME	// Robin 紀錄每一戰鬥指令時間(測試用)
//#define	_CHECK_BATTLE_IO	// Robin 紀錄戰鬥封包傳送(測試用)


//復刻版
//#define _BACK_VERSION	//復刻版
//#define _REDHAT_V9 //(可開) Change 在REDHAT9.0的版本下編譯時打開
//#define _UN_FMPOINT // (可開) Change 關掉申請家族據點
//#define _UN_FMMEMO //(可開)Change  關掉家族之間留言板
//--------------------------------------------------------------------------
//專案 7.5	精靈的招喚	人員：彥勳 奇銓 小威
#define _NPC_EXCHANGEMANTRANS   // (可開) Change 替exchangeman加入判斷轉生
#define	_ANGEL_SUMMON	// Robin 天使召喚
	#ifdef _ANGEL_SUMMON
	#define _ADD_NEWEVENT
	#endif
// Terry define start -------------------------------------------------------------------
#define _STREET_VENDOR		// 擺攤功能(開放)
#define _WATCH_EVENT			// 查詢任務功能(開放)
#define _NEW_MANOR_LAW		// 新莊園規則(開放)
#define _MAP_TIME					// 特殊地圖,會倒數(開放)
#define _PETSKILL_LER			// 雷爾技能(開放)
//#define _TEACHER_SYSTEM		// 導師系統(不開放)
// Terry define end   -------------------------------------------------------------------
#define _FIX_MAGIC_RESIST				// (可開) Change 職業魔法抗性
#define _ADD_DUNGEON					// (可開) Change 追加地牢
#define _PROFESSION_ADDSKILL			// (可開) Change 追加職業技能
#define _ITEM_ADDPETEXP				// (可開) Change 增加寵物經驗值道具(可吃此道具寵物,平常不可獲得經驗值)
#define _TRANS_6						// (可開) Change 人物6轉
#define _PET_2TRANS					// (可開) Change 寵物2轉
#define _PET_2LIMITLEVEL				// (可開) Change 幫助寵物2轉的瑪蕾
#define _MAGICSTAUTS_RESIST			// (可開) Change (火電冰)抗性精靈
#define _EQUIT_RESIST					// (可開) Change 單一裝備抗性(可抗單一功能異常狀態)有抗火 雷 冰 虛弱 魔障 沉默 落馬
#define _SUIT_TWFWENDUM				// (可開) Change 四屬性套裝功能 (_SUIT_ITEM要開)
#define _EXCHANGEMAN_REQUEST_DELPET	// (可開) Change 替exchangeman的TYPE:REQUEST增加刪除寵物
//#define _75_TEST						// (不可開) 7.5測試server
//#define _PK_SERVER //百萬pk活動
//#define _DEATH_CONTEND			// (可開) 百萬pk活動 ANDY 死鬥
//#define _DEATH_CONTENDAB			// (可開) 百萬pk活動 ANDY 正式死鬥
//--------------------------------------------------------------------------

#define _ESCAPE_RESET			// 惡寶逃跑後暫時不可組隊  Robin 2004/07/22
#define _DEL_DROP_GOLD			// 刪除地上過時的石幣	Robin
#define _ITEM_STONE				// cyg 奇怪的石頭
#define _HELP_NEWHAND			// cyg 新創角色給予新手道具)
#define _DEF_FMFREETIME			// cyg GM指令 退出家族時間限制設為0
#define _DEF_GETYOU				// cyg GM指令 查詢自己兩格範圍內的玩家帳號
#define _DEF_NEWSEND			// cyg GM指令 傳送玩家時可加句子也可不加
#define _DEF_SUPERSEND			// cyg GM指令 可傳送已某帳號為中心附近3格內所有玩家
#define _SUIT_ADDPART3			// (可開) Change 套裝功能第三彈 迴避提高% 沉默抗性提高% 敏提高% 防提高% 攻提高%
#define _PETSKILL_2TIMID		// (可開) Change 怯戰第二版(狂獅怒吼)
#define CAVALRY_DEBUG			// (暫時開) Change 座騎攻擊除錯中
#define _SHOW_FUSION			// (可開) Change 融合寵物在CLIENT端只顯示轉生數,修改為融合寵顯示為融合(client端也要開)
#define _CONTRACT				// (可開) Robin 契約功能 
#define _FONT_SIZE				// (可開) Robin 字型大小功能
#define _PROSK99				// (可開) Change 檢查職技等級送白狼
#define _TIME_TICKET			// Robin 限時入場 森林捉寵用
#define _HALLOWEEN_EFFECT		// 萬聖節特效
#define _ADD_STATUS_2			// 新增傳送人物狀態第二類
#define _ADD_reITEM				// (可開) Change 檢查道具欄位空格
#define _ADD_NOITEM_BATTLE		// 新增NPC檢查人物身上沒道具會進入戰鬥
#define _PETSKILL_BATTLE_MODEL	// 寵物技能戰鬥模組
#define _RACEMAN				// cyg 獵寵大會新npc
#define _PETSKILL_ANTINTER		// (可開) Change 寵技:蟻之葬 參考_PETSKILL_2TIMID
#define _PETSKILL_REGRET		// (可開) Change 寵技:憾甲一擊 參考_SONIC_ATTACK
#define _SUIT_ADDPART4			// (可開) Change 套裝功能第四彈 中毒攻擊% 加強法師魔法(機率30%) 抵抗法師的魔法

//#define _CHECK_ENEMY_PET		// (不可開) Change 檢查玩家是否持有不可當寵物的角色(enemy.txt的petflg欄位為0代表不可拿來當寵物)
//#define _ITEM_TYPETABLE		// (不可開) Change 追加typetable
//#define _MERGE_NEW_8			// Robin 8.0版 新合成規則
//#define _RIGHTCLICK			// Robin 滑鼠右鍵功能
//#define _JOBDAILY				// cyg 任務日誌功能
//#define	_ALCHEMIST			// Robin 精鍊功能
//#define _CHECK_ITEM_MODIFY	// 檢查異常加強的道具
//#define _PETSKILL_EXPLODE		// (不可開) Change 寵技:爆裂攻擊


#endif
