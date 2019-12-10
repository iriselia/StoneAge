#include "version.h"
#include "char.h"

/*


  かたりべ。

  リリース版では2種類のことだま。

 NPCARGUMENT:  0だったらことだま0、1だったらことだま1の語り部。  

 状態は3つあり、順に進行する。

 状態0： アイドリング。この状態でtalkされると、
 「よくきた、ほげほげよ。いかにも、わしが伝説のかたりべじゃ。」で
 状態1に移行。
 状態1：talkされたら、
 「そなたがのぞむならば、魂とひきかえに、強力な言霊をひとつ、 さずけよう。
        その言霊をのぞむか? yes/no」yesで状態2へ。noで
        「では、さらばじゃ。」で状態0へ
        
 状態2： talkされたら、
 「そなたに伝説の言霊「ほげほげ」を伝承した。命のつぎに大切なものとして
        一生大切にするべし。さらばじゃ。」
        といって魂を減らし、言霊フラグを追加する。
        で状態0にもどる。



 
 Talked:

 if( 最後にはなしてから1分たっているか？){
   状態0にもどる。
 }
   
 switch(状態){
 case 0:  処理。状態1へ。break;
 case 1:  処理。状態2へ。break;
 case 2:  処理。状態0へ。break;
 } 

 }

 
 
 

 



 */
BOOL NPC_StoryTellerInit( int meindex )
{
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPESTORYTELLER );
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );
    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );

    return TRUE;
}


void NPC_StoryTellerTalked( int meindex , int talker , char *msg , int col )
{
    
}
