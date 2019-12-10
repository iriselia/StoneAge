#ifndef __LINK_H__
#define __LINK_H__

#include "common.h"

/*
 * リスト構造を定義する物。
 * このリストはvalは内部的には何もしない。つまりポインターのコピーのみ
 * 行う。
 */
typedef struct tagNode
{
    struct tagNode* next;           /*次のノードへのポインター*/
    char* val;                      /*保持する文字列*/
    int size;                       /*valのサイズ*/
}Node;

BOOL Nodeappendhead( Node** top  , Node* add );
BOOL Nodeappendtail( Node** top  , Node* add );
BOOL Noderemovehead( Node** top , Node* ret);
BOOL Noderemovetail( Node** top , Node* ret);
#endif
