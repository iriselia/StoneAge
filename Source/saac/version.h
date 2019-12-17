#ifndef __VERSION_H__
#define __VERSION_H__

/* 請將控制專案的 definition 放在這個檔案
 * 然後藉由 remark/unmark 來開關功能
 */

/* -------------------------------------------------------------------
 * 專案：族人職務與家族功能補充
 * 人員：小瑜、彥勳、志偉、小民
 * 說明：
 */
#define _LEADERFLAG_VER		// 嗯、ㄟ、這個.... (可開放)
#define __FAMILY			//  (可開放)
#define _PERSONAL_FAME		// Arminius 8.30 家族個人聲望  (可開放)
#define _FMVER21			// CoolFish: 家族族長職務代理人 2001/8/30 (可開放)
#define MEMBER_100			// CoolFish: 家族人數上限 2001/9/3 (可開放)
#define _EVERYONE_RIDE		// CoolFish: 騎寵平民化家族金額上限 2001/9/24 (可開放)

/* -------------------------------------------------------------------
 * 專案：當機不回朔之人物資料存檔方式修正
 * 人員：小瑜、彥勳
 * 說明：修正人物資料存檔方式
 */
#define _NewSave		// CoolFish: 2001/10/16 (可開放)

/* -------------------------------------------------------------------
 * 專案：星球合併
 * 人員：阿民、彥勳、小瑜
 * 說明：
 */
// #define _RELOGIN		// CoolFish: 重複登入機制 2001/10/29 (不可開放)

/* -------------------------------------------------------------------
 * 專案：伊甸大陸
 * 人員：彥豪、阿民、小瑜
 * 說明：工期 1/15
 */
//#define _AUCTIONEER // Arminius 1.10 拍賣店 (伊甸大陸)

/* -------------------------------------------------------------------
 * 專案：家族ＢＵＧ修正
 * 人員：小瑜
 * 說明：
 */
#define _LEADERFUNCHECK	// CoolFish: 2002/02/06 族長功能錯誤、禪讓錯誤修正(可開放)

/* -------------------------------------------------------------------
 * 專案：跨星球踢人
 * 人員：彥豪
 * 說明：
 */
#define _WAEI_KICK				// (可開放)(北京不必開)
#define _FIX_WORKS				// (可開放)(北京不必開) wk修改

//#define _DELAY_UNLOCK			// (不可開) ANDY
//#define _LOCK_SERVER			// (不可開)
//#define _TEST_LOCKSERVER		// (不可開)
//#define _NO_DOUBLECHECK		// (不可開)
//#define _PAUCTION_MAN			// (不可開) 寄賣
/* -------------------------------------------------------------------
 * 專案：九大莊園(4.0)
 * 人員：志旺	
 * 說明：
 */
#define _FIX_9_FMPOINT			// (可開放) WON ADD 九大莊園
#define _FIX_LEADER_ERR			// (可開放) WON ADD 修正族長問題
#define _REMOVE_FM_LOG          // (可開放) WON ADD 移除不必要的家族 LOG
//#define _ADD_FAMILY_TAX		// (不可開) WON ADD 增加莊園稅收


/* -------------------------------------------------------------------
 * 專案：確認莊園挑戰排程
 * 人員：旭誠	
 * 說明：由AC下family目錄裡fmschedule檔統一廣播各莊園挑戰排程 
*/
//#define _CHECKFMSCHEDULE	    // (不可開) Syu ADD 透過AC確認家族挑戰排程
//#define _FAMILYBANKSTONELOG   // (不可開) Syu ADD 新增家族銀行存取Log
//#define _FIXFMASSIGN		    // (不可開) Syu ADD 修正家族一百人無法指派、移除長老	


/* -------------------------------------------------------------------
 * 專案：石器6.0
 * 人員：
 * 說明：
 */
#define _SEND_EFFECT		    // (可開放) WON ADD 送下雪、下雨等特效
//#define _NEW_PLAYERGOLD		// (不可開) ANDY 新帳號給錢
//#define _BAD_PLAYER           // (不可開) WON ADD 送壞玩家去關

// -------------------------------------------------------------------
#define _AC_SEND_FM_PK			// (可開放) WON ADD 莊園對戰列表儲存在AC
#define _ACFMPK_LIST			// (可開放) ANDY 莊園對戰列表
#define _CK_ONLINE_PLAYER_COUNT	// (可開放) WON ADD 計算線上人數

/* -------------------------------------------------------------------
 * 專案：石器7.0
 * 人員：
 * 說明：
 */
#define _ALLDOMAN				// (可開放) Syu ADD 排行榜NPC
#define _CHAR_POOLITEM			// (可開放) ANDY 共用倉庫
#define _CHAR_POOLPET			// (可開放) Robin 共用寵物倉庫
//#define _FIX_ALLDOMAN			// (不可開) ANDY
// -------------------------------------------------------------------
// 新功能：客服公告系統
#define _GM_BROADCAST				// (可開放) WON ADD 客服公告系統

// -------------------------------------------------------------------
// 新功能：GM密頻功能擴充
//#define _RECAL_BACKUP_PLAYER		// (可開放 (暫時先關起來) ) WON 回溯功能 (北京&韓國不必開)
//#define _UNIVERSE_CHATROOM			// (不可開) ANDY 全星系聊天

// -------------------------------------------------------------------
// 新功能：修改WORKER(新付費機制)
#define _FIX_MESSAGE				// (可開) WON ADD 修改WORKER封包內容 (北京&韓國不必開)
#define _WORKER_AUTO_LOGOUT		// (可開) WON ADD GS斷線自動送LOGOUT給WORKER

//#define _DEFEND_NEWUSERS			// (不可開) ANDY 老手帶新手
//#define _CHECK_BADFILES			// (不可開) ANDY 判斷壞擋
//#define _AC_UNLOCK_GS_PLAYER		// (不可開) WON ADD 自動UNLOCK
//#define _ACFMPK_NOFREE			// (不可開) ANDY 家族無條件

// -------------------------------------------------------------------
// 新功能：跨星系pk伺服器
//#define _OACSTRUCT_TCP			// (不可開) ANDY 跨星系取得人物列表
//#define _TCPIP_UTIL				// (不可開) ANDY TCPIP
//#define _DEATH_CONTEND			// (不可開) ANDY 死鬥
//#define _DEATH_FAMILY_LOGIN_CHECK	// (不可開) WON ADD 家族戰登入檢查
//#define _DEATH_FAMILY_GM_COMMAND	// (不可開) WON ADD 家族戰GM指令
//#define _DEATH_FAMILY_STRUCT		// (不可開) WON ADD 家族戰存放勝負資料

#define _CHANGEGALAXY	// Robin 2003/10/27 星系移民

#define _TIMEOUTKICK // Robin 2003/11/07 時段制自動踢人

//#define _UNLOCKPOOL		// Robin 2003/12/12  Logout暫存區

#define _KILLDEADWORKER // Robin 2004/02/ 用sigusr1清除所有Worker 在unlock.arg加'K'

#define _SLEEP_CHAR		// Robin 2004/02/12	活躍人物與非活躍人物分開儲存

#endif

// Terry define start -------------------------------------------------------------------
#define _FM_MODIFY				// 家族功能修改
#define _CHANNEL_MODIFY		// 頻道功能整合
#define _NEW_MANOR_LAW		// 新莊園規則
#define _LOCK_ADD_NAME		// LockNode 增加名字欄位(未開放)
//#define _TEACHER_SYSTEM   // 導師系統(未開放)
// Terry define end   -------------------------------------------------------------------

#define _ANGEL_SUMMON	// Robin 精靈的召喚
//#define _SASQL //Change 石器資料庫 (不可開)
#define _RACEMAN		// cyg 獵寵大會
