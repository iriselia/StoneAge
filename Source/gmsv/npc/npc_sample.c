#include "version.h"
#include "char.h"
#include "object.h"
#include "npcutil.h"
#include "readmap.h"
 
/*
 * サンプルのNPCコード
 *
 * タイプ名： Sample
 * 定義する関数名： 全種類。
 * NPC_SamplePreWalk
 * NPC_SamplePostWalk
 * NPC_SamplePreOver
 * NPC_SamplePostOver
 * NPC_SampleWatch
 * NPC_SampleLoop
 * NPC_SampleDying
 * NPC_SampleTalked
 * NPC_SamplePreAttacked
 * NPC_SamplePostAttacked
 * NPC_SampleOff
 * NPC_SampleLooked
 * NPC_SampleItemPut
 */
/*------------------------------------------------------------
 * NPCのサンプルルーチン
 * 引数
 *  index       int     インデックス
 ------------------------------------------------------------*/
void NPC_SampleLoop( int index )
{
    int i,j;
    BOOL    found=FALSE;
    int     fl,x,y;
    BOOL    nearby = FALSE;
    
    fl = CHAR_getInt(index,CHAR_FLOOR);
    x  = CHAR_getInt(index,CHAR_X);
    y  = CHAR_getInt(index,CHAR_Y);

    /*
     * 近くにプレイヤーがいるかどうかを調べて、いたらそいつに近づく
     * 現在、最初にみつかったやつだけ。
     * いなかったらランダムウォーク
     *  一歩歩く毎に攻撃する
     */
    for( i=x-2 ; i<=x+2 ; i ++ ){
        for( j=y-2 ; j<=y+2 ; j++ ){
            OBJECT  object;
            for( object=MAP_getTopObj(fl,i,j) ; object ;
                 object = NEXT_OBJECT(object) ){
                int     objindex = GET_OBJINDEX(object);
                if( OBJECT_getType(objindex) == OBJTYPE_CHARA
                    && CHAR_getInt(OBJECT_getIndex(objindex),
                                   CHAR_WHICHTYPE) == CHAR_TYPEPLAYER ){

                    int     dir;
                    POINT_SA   start,end;
                    end.x = CHAR_getInt(OBJECT_getIndex(objindex),CHAR_X);
                    end.y = CHAR_getInt(OBJECT_getIndex(objindex),CHAR_Y);
                    start.x = x;
                    start.y = y;
                    dir = NPC_Util_getDirFromTwoPoint( &start,&end );
                    if( dir != -1 )
                        CHAR_walk(index,dir,0);
                    else
                        /*  同じ座標や  */
                        print( "????????same coordinates\n" );
                    found = TRUE;

                    if( ABS( x-OBJECT_getX(objindex) ) <= 1 &&
                        ABS( y-OBJECT_getY(objindex) ) <= 1 ){
                        nearby = TRUE;
                    }
                    
                    break;
                }
            }
        }
    }
    
    if( !found )
        CHAR_walk( index,RAND(0,7),0);

/*    print( "e(%d,%d)[%d]" ,CHAR_getInt(index,CHAR_X),
           CHAR_getInt(index,CHAR_Y),found );*/
    

    if( found && nearby ){
/*        print( "found" );*/
        /*CHAR_Attack( index, CHAR_getInt(index,CHAR_DIR));*/
    }
/*    print("\n");*/

//    print( "%d %d\n",CHAR_getInt( index,CHAR_X),
//           CHAR_getInt( index,CHAR_Y) );
    
}
