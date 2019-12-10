#ifndef __NPC_AUCTIONEER_H__
#define __NPC_AUCTIONEER_H__

#ifdef _AUCTIONEER

// 以下這兩個常數要跟 saac/auction.h 裡面的定義一致
#define MAX_AUCTION	100	// 最大的委託數
#define MAX_OVERDUE	500	// 過期的委託單保留數 (最小設定=過期保留天數*最大委託數)
#define AUC_PET     1
#define AUC_ITEM    2

typedef struct tagAuctionTable {
	int  flag;		      // 0=(null) 1=使用中 2=過期
	char cdkey[32];		  // 委託人 cdkey
	char customer[32];	// 委託人人物名稱
	int itemtype;		    // 1=寵物 2=道具 otherwise=error
	char goods[4096];	  // 物品 (寵物 or 道具)
	char description[256];	// 敘述
  char listdata[512]; // 顯示在表單的資料
	int price;		      // 標價
  int onsaletime;
  int overduetime;
} AuctionTable;

extern AuctionTable onsale[MAX_AUCTION];	// 拍賣中的道具、寵物
//extern AuctionTable overdue[MAX_OVERDUE];	// 過期的道具、寵物

BOOL NPC_AuctioneerInit(int meindex);
void NPC_AuctioneerTalked(int meindex, int talkerindex, char *msg, int color);
void NPC_AuctioneerWindowTalked(int meindex, int talkerindex, int seqno, int select, char *data);
void NPC_AuctioneerLoop(int meindex);

#endif /* _AUCTIONEER */

#endif 
