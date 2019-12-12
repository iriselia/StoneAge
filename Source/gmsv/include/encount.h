#ifndef __ENCOUNT_H__
#define __ENCOUNT_H__

#define		ENCOUNT_GROUPMAXNUM		10

BOOL ENCOUNT_initEncount( char* filename );
BOOL ENCOUNT_reinitEncount( void );
int ENCOUNT_getEncountAreaArray( int floor, int x, int y);
int ENCOUNT_getEncountPercentMin( int charaindex, int floor , int x, int y );
int ENCOUNT_getEncountPercentMax( int charaindex, int floor , int x, int y );
int ENCOUNT_getCreateEnemyMaxNum( int floor , int x, int y );
int ENCOUNT_getEncountIndex( int floor , int x, int y );
int ENCOUNT_getEncountIndexFromArray( int array );
int ENCOUNT_getEncountPercentFromArray( int array );
int ENCOUNT_getCreateEnemyMaxNumFromArray( int array );
int ENCOUNT_getGroupIdFromArray( int array, int grouparray );
int ENCOUNT_getGroupProbFromArray( int array, int grouparray );
int ENCOUNT_getZorderFromArray( int array );

#ifdef _ADD_ENCOUNT           // WON ADD 增加敵遭遇觸發修件
typedef struct tagENCOUNT_Table
{
    int                 index;
    int                 floor;
    int                 encountprob_min;                /* エンカウント確率 */
    int                 encountprob_max;                /* エンカウント確率 */
    int                 enemymaxnum;        /* どれだけ敵を作るか */
    int                 zorder;
    int                 groupid[ENCOUNT_GROUPMAXNUM];       /* グループNo */
    int                 createprob[ENCOUNT_GROUPMAXNUM];    /* そのグループの出現率 */
	int					event_now;			
	int					event_end;
	int					enemy_group;		// 怪物的group 編號
    RECT_SA                rect;
}ENCOUNT_Table;

ENCOUNT_Table           *ENCOUNT_table;
#endif

#endif
