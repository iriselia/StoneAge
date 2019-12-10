#include <stdlib.h>
#include "version.h"
#include "link.h"
#include "buf.h"

/*
 *  リンクの後につける
 * 引数
 *      top         トップノード
 *      add         付け加えるノードの内容を保持するノードへのポインター
 * 返り値
 *      TRUE(1)     成功
 *      FALSE(0)    失敗
 */
BOOL Nodeappendtail( Node** top  , Node* add )
{
    Node* c;        /*ループ用*/
    Node* next;     /*新しく作るノード用*/

    /*トップがNULLかどうかのチェック*/
    if( *top == NULL ){
        *top = allocateMemory( sizeof( Node  ) );
        if( *top== NULL ) return FALSE;
        (*top)->next    = NULL;            /*続きは無い*/
        (*top)->size    = add->size;   /*長さの保管*/
        (*top)->val     = add->val;         /*ポインターのコピー*/
        return TRUE;
    }

    for( c = *top ; c->next ; c = c->next ); /* cの位置をずらす */
    next = allocateMemory( sizeof(Node) );
    if( next == NULL )return FALSE;
    c->next = next;                         /* nextに値を設定する */
    next->next  = NULL;                      /*続きは無い*/
    next->val   = add->val;                   /*ポインターのコピー*/
    next->size  = add->size;             /*長さの保管*/
    return TRUE;
}




/*
 *  リンクの前につける
 * 引数
 *      top         トップノード
 *      add         付け加えるノードの内容を保持するノードへのポインター
 * 返り値
 *      TRUE(1)     成功
 *      FALSE(0)    失敗
 */
BOOL Nodeappendhead( Node** nowtop  , Node* add )
{
    Node* newtop;       /*新しい先頭用*/

    /*トップがNULLかどうかのチェック*/
    if( *nowtop == NULL ){
        *nowtop = allocateMemory( sizeof( Node  ) );
        if( *nowtop == NULL ) return FALSE;
        (*nowtop)->next = NULL;             /*続きは無い*/
        (*nowtop)->size = add->size;        /*長さの保管*/
        (*nowtop)->val  = add->val;         /*ポインターのコピー*/
        return TRUE;
    }

    /*
     * 新しいノードを一つ作る
     * next が差すものを nowtop にする。
     * nowtop には 確保した newtop を代入する。
     */
    newtop = allocateMemory( sizeof(Node) );
    newtop->next    = *nowtop;
    newtop->val     = add->val;
    newtop->size    = add->size;
    *nowtop = newtop;
    return TRUE;
}




/*
 * 一番最初のノードを引数にコピーして取り除く
 * 引数
 *          top         ノードの先頭
 *          ret         取り除いたノードのコピー
 * 返り値
 *      TRUE(1)         成功
 *      FALSE(0)        失敗    ノードの先頭がNULL
 */
BOOL  Noderemovehead( Node** top , Node* ret)
{
    Node* newtop;         /*新しく先頭になるノード*/

    if( *top == NULL )return FALSE;

    ret->val    = (*top)->val;
    ret->size   = (*top)->size;
    newtop = (*top)->next;
    freeMemory( *top );
    *top = newtop;

    return TRUE;
}





/*
 * 一番最後のノードを引数にコピーして取り除く
 * 引数
 *          top         ノードの先頭
 *          ret         取り除いたノードのコピー
 * 返り値
 *      TRUE(1)         成功
 *      FALSE(0)        失敗    ノードの先頭がNULL
 */
BOOL  Noderemovetail( Node** top , Node* ret)
{
    Node* c;             /*ループ用*/
    Node* c1;            /*ループ用 いつでも c->nextを指す*/

    if( *top == NULL )return FALSE;

    c = *top;           /*初期値設定*/
    c1 = c->next;       /*初期値設定*/
    while(1){
        if( c1->next == NULL )
            /*
             * この時点で          c1
             *                      |
             *  c ---> +------+     ↓
             *         | next |---->+------+
             *         |------|     | next |---->NULL
             *         |      |     +------+
             *         +------+     |      |
             *                      +------+
             *となっている
             */
            break;
        c=c->next;
        c1=c->next;
    }
    c->next = NULL;     /*c1にあたるものを削除するのでnextはNULLとする*/
    /*コピー*/
    ret->val    = c1->val;
    ret->size   = c1->size;

    freeMemory( c1 );

    return TRUE;
}
