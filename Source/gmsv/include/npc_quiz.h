#ifndef __NPC_QUIZ_H__
#define __NPC_QUIZ_H__

typedef struct NPC_Quiz{

	int 	no;				//問題の番号
	int 	type;			//問題のタイプ（ゲーム内容、スポーツ)	
	int		level;  		//問題のレベル
	int 	answertype;		//答えの選びかた(２択、３択、単語マッチ）
	int 	answerNo;		//問題の答え
	char	question[512]; 	//問題
	char	select1[128]; 	//選択枝1
	char	select2[128]; 	//選択枝2
	char	select3[128]; 	//選択枝3

}NPC_QUIZ;


void NPC_QuizTalked( int meindex , int talkerindex , char *msg ,
                     int color );
BOOL NPC_QuizInit( int meindex );
void NPC_QuizWindowTalked( int meindex, int talkerindex, int seqno, int select, char *data);

BOOL QUIZ_initQuiz( char *filename);


#endif 

/*__NPC_QUIZ_H__*/
