#ifndef __MAP_H__
#define __MAP_H__
#include "common.h"
#include "util.h"

typedef struct tagMAP_Objlink
{
    int objindex;
    struct tagMAP_Objlink* next;
}MAP_Objlink, *OBJECT;

#define GET_OBJINDEX(x) ((x)->objindex)
#define NEXT_OBJECT(x) ((x)->next)

typedef struct tagMAP_Map
{
    int     id;             /*  フロアID    */
    int     xsiz,ysiz;      /*  サイズ  */
    char    string[64];     /*  表示    */
    unsigned short*  tile;           /*  タイル  */
    unsigned short*  obj;            /*  オブジェクト    */
    MAP_Objlink** olink;
#ifdef _MAP_NOEXIT
	unsigned int startpoint;
	int MapType;
#endif
}MAP_Map;


typedef enum
{
    MAP_WALKABLE,           /*
                             * パーツの場合
                             * 歩けなくてタイルを見ない 0
                             * 歩けてタイルを見る 1
                             * 歩けてタイルを見ない 2
                             */
    MAP_HAVEHEIGHT,         /*  高さがあったら非0 なかったら0   */
    MAP_DEFENCE,            /*  守れるなら、守備力、0 以下なら守れない  */

    MAP_INTODAMAGE,         /*  そこに入った時にHPに足する  */
    MAP_OUTOFDAMAGE,        /*  そこから出た時にHPに足する  */

    MAP_SETED_BATTLEMAP,    /* バトルマップ設定値 */
	MAP_BATTLEMAP,			/* バトルマップの番号 １*/
	MAP_BATTLEMAP2,			/* バトルマップの番号 ２*/
	MAP_BATTLEMAP3,			/* バトルマップの番号 ３*/

/* 以下はＬＳ２で使っていてＳＡでは使わない(打ち合わせにだす) */

    MAP_INTODARKNESS,       /*  入った時の暗闇    */
    MAP_INTOCONFUSION,      /*  入った時の混乱    */

    MAP_OUTOFPOISON,         /*  入った時の毒    */
    MAP_OUTOFPARALYSIS,      /*  入った時のしびれ    */
    MAP_OUTOFSILENCE,        /*  入った時の沈黙  */
    MAP_OUTOFSTONE,          /*  入った時の石化    */
    MAP_OUTOFDARKNESS,       /*  入った時の暗闇    */
    MAP_OUTOFCONFUSION,      /*  入った時の混乱    */

    MAP_DATAINT_NUM,
}MAP_DATAINT;
typedef enum
{
    MAP_DATACHAR_NUM,
}MAP_DATACHAR;
typedef struct tagMAP_ImageData
{
    int     data[MAP_DATAINT_NUM];
    STRING32    string[MAP_DATACHAR_NUM];
}MAP_ImageData;

typedef enum
{
    MAP_KINDWALKABLE,           /*  歩けたら 1 が入る   */

    MAP_KINDNUM,
}MAP_kind;


INLINE int MAP_getfloorIndex( int floorid );
int MAP_getfloorX( int floorid );
int MAP_getfloorY( int floorid );

BOOL MAP_initReadMap( char* maptilefile , char* mapdir );
BOOL MAP_initMapArray( int   num );
void    MAP_endMapArray( void );

char* MAP_getdataFromRECT( int floor, RECT* seekr, RECT* realr );
char *MAP_getChecksumFromRECT( int floor, RECT* seekr, RECT* realr,
								int *tilesum, int *objsum, int *eventsum );

BOOL MAP_checkCoordinates( int mapid, int x, int y );

BOOL MAP_setTileAndObjData( int ff ,int fx, int fy, int tile, int obj);
BOOL MAP_getTileAndObjData( int ff ,int fx, int fy, int* tile, int* obj);
void MAP_sendAroundMapdata( int fl, int x, int y);
int MAP_getImageInt( int imagenumber, int element );
BOOL MAP_setImageInt( int imagenumber, int element, int value );
BOOL IsValidImagenumber( int imagenumber );

char* MAP_getfloorShowstring( int floorid );
BOOL MAP_makeVariousMap(char* atile, char* aobj, int floor, int startx, int starty, int xsiz, int ysiz, MAP_kind   kind );
BOOL MAP_makeWalkableMap( char* data,  int floor, int startx, int starty,int xsiz, int ysiz );
BOOL MAP_IsThereSpecificFloorid( int floorid );
BOOL MAP_IsValidCoordinate( int floorid, int x, int y );
int MAP_attackSpecificPoint( int floor, int x, int y, int damage ,
                             int charaindex );

BOOL MAP_addNewObj( int floor, int x, int y, int objindex );
BOOL MAP_removeObj( int floor, int x, int y, int objindex );
#define		MAP_getTopObj( fl, x, y)	_MAP_getTopObj( __FILE__, __LINE__, fl, x, y)
MAP_Objlink* _MAP_getTopObj( char *file, int line, int floor, int x, int y );
#define		MAP_objmove( objindex, of, ox, oy, nfl, nx, ny) _MAP_objmove( __FILE__, __LINE__, objindex, of, ox, oy, nfl, nx, ny)
BOOL _MAP_objmove( char *file, int line, int objindex, int ofloor, int ox, int oy, int nfloor,
                  int nx, int ny );
char *MAP_getFloorName( int floor);
BOOL MAP_setObjData( int ff ,int fx, int fy, int obj, int objhp );

#ifdef _STATUS_WATERWORD //水世界狀態
int MAP_getMapFloorType( int floor);
#endif

#ifdef _MAP_NOEXIT
unsigned int MAP_getExFloor_XY( int floor , int *map_type);
BOOL CHECKFLOORID( int id);
#endif

#endif 
/*__MAP_H__*/
