#define _SASQL_C_

#include "version.h"

#ifdef _SASQL

#include "main.h"
#include "util.h"
#include "mail.h"
#include "db.h"
#include "saacproto_util.h"
#include "saacproto_serv.h"
#ifdef _UNIVERSE_CHATROOM
#include "chatroom.h"
#endif
// CoolFish: Family 2001/5/9
#include "acfamily.h"

#ifdef _DEATH_CONTEND
#include "deathcontend.h"
#endif

#include <signal.h>
#include <sys/types.h>
#include <time.h>

#if PLATFORM_WINDOWS

#else
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <malloc.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#include "saacproto_work.h"
#ifdef _OACSTRUCT_TCP
#include "saacproto_oac.h"
#endif
#ifdef _PAUCTION_MAN
#include "auction.h"
#endif
#include "lock.h"
#define BACKLOGNUM 5


#ifdef _FIX_WORKS
#include "saacproto_work.h"
int worksockfd;
#endif

#ifdef _LOCK_SERVER
#include "saacproto_lserver.h"
#endif

#ifdef _SEND_EFFECT				  // WON ADD 送下雪、下雨等特效 
#include "recv.h"
#endif

#include "defend.h"

#include "char.h"

#include <mysql.h>

#define BOOL int
#define FALSE 0
#define TRUE  1
MYSQL mysql;
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;

char servername[16][20] =
{
		"仙女", // 0
		"太陽", // 1
		"天神", // 2
		"北斗", // 3
		"紫微", // 4
		"蒼龍", // 5
		"銀河系", // 6
		"香港", // 7
		"星樂園", // 8
		"網路家庭", // 9
		"聖獸", // 10
		"天鷹", // 11
		"新界", // 12
		"仙女1", // 13
		"仙女2", // 14
		"仙女3" // 15
};

int sasql_init( void )
{
    if( mysql_init(&mysql) == NULL )
    {
        log("\nmysql init err");
		return FALSE;
    }
	if( !mysql_real_connect( &mysql,
                             "127.0.0.1",
                             "root",//帳號
                             "password",//密碼
                             "stoneage",//選擇的資料庫
                             MYSQL_PORT,
                             NULL,
                             0 ) )
	{
		log("\nmysql connect err");
		return FALSE;
    }
	log("\nmysql connect ok");
	return TRUE;
}

void sasql_close( void )
{
	mysql_close( &mysql );
}

int sasql_save_nm( int id, char *acc, char *data )
{
	int i;
	char str[256];
	memset(str,0,256);
	for(i=0;i<16;i++){
		if( strcmp( servername[i], saacname) == 0 ){
			break;	
		}
	}
	sprintf(str,"select name from nm where idxnum=%d and acc='%s' and STAR=%d",id,acc,i);
	if( !mysql_query(&mysql, str ) ){
		int num_row=0;
		mysql_result = mysql_store_result( &mysql );
		num_row = mysql_num_rows(mysql_result);//取得的資料筆數(正常最多取得一筆)
		mysql_free_result(mysql_result);
		log("\nsasql nm num_row:%d",num_row);
		if( num_row != 0 ){//修改資料
			sprintf(str,"update nm set name = '%s' where idxnum=%d and acc='%s' and STAR=%d",data,id,acc,i);
			if( !mysql_query(&mysql, str ) ){
				log("\nupdate nm set name OK");
				return TRUE;
			}
			else{
				log("\nupdate nm set name ERR");
				return FALSE;
			}
		}
		else{//增加資料
			sprintf(str,"insert into nm values ( %d, '%s', %d, '%s' )",i,acc,id,data);
			if( !mysql_query(&mysql, str ) ){
				log("\ninsert into nm values OK");
				return TRUE;
			}
			else{
				log("\ninsert into nm values ERR");
				return FALSE;
			}
		}
	}
	else{
		log("\nselect name from nm  ERR");
		return FALSE;
	}

	return TRUE;
}

int sasql_save_opt( int id, char *acc, char *data )
{
	int i,star;
	char str[1024];
	
	memset(str,0,256);
	for(i=0;i<16;i++){
		if( strcmp( servername[i], saacname) == 0 ){
			star = i;
			break;	
		}
	}
	sprintf(str,"select name from opt where idxnum=%d and acc='%s' and STAR=%d",id,acc,star);
	if( !mysql_query(&mysql, str ) ){
		int num_row=0;
		mysql_result = mysql_store_result( &mysql );
		num_row = mysql_num_rows(mysql_result);//取得的資料筆數(正常最多取得一筆)
		mysql_free_result(mysql_result);
		log("\nsasql opt num_row:%d",num_row);

		if( num_row != 0 ){//修改資料
			char buf[1024],buf2[16][64];
			for( i=0;i<16;i++)
				memset(buf2[i],0,64);
			memset(buf,0,1024);
			for( i=0;i<16;i++ )
				easyGetTokenFromBuf( data, '|', i+1, buf2[i], sizeof( buf2[i] ) );

			sprintf(buf," d0=%s and d1=%s and d2=%s and d3=%s and d4=%s and d5=%s and d6=%s and d7=%s and d8=%s and d9=%s and d10=%s and d11=%s and d12=%s and d13=%s and name='%s' and mapname='%s' ",
				          buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[5],buf2[6],buf2[7],buf2[8],buf2[9],buf2[10],buf2[11],buf2[12],buf2[13],buf2[14],buf2[15] );

			sprintf(str,"update opt set %s where idxnum=%d and acc='%s' and STAR=%d",buf,id,acc,star);
			if( !mysql_query(&mysql, str ) ){
				log("\nupdate opt set OK");
				return TRUE;
			}
			else{
				log("\nupdate opt set ERR");
				return FALSE;
			}
		}
		else{//增加資料
			char buf[1024],buf2[64],buf3[64];
			memset(buf,0,1024);
			for( i=0;i<14;i++ ){
				easyGetTokenFromBuf( data, '|', i+1, buf2, sizeof( buf2 ));
				strcat(buf,buf2);
				strcat(buf,",");
			}
			easyGetTokenFromBuf( data, '|', i+1, buf2, sizeof( buf2 ));
			easyGetTokenFromBuf( data, '|', i+2, buf3, sizeof( buf3 ));
			sprintf(str,"insert into opt values ( %d, '%s', %d, %s '%s', '%s' )",star,acc,id,buf,buf2,buf3);
			if( !mysql_query(&mysql, str ) ){
				log("\ninsert into opt values OK");
				return TRUE;
			}
			else{
				log("\ninsert into opt values ERR");
				return FALSE;
			}
		}
	}
	else{
		log("\nselect name from opt ERR");
		return FALSE;
	}

	return TRUE;
}

int sasql_save_int_info( int id, char *acc, char *data )
{
	int i,star;
	char str[10042];
	log("\n%s",data);

	memset(str,0,10042);
	for(i=0;i<16;i++){
		if( strcmp( servername[i], saacname) == 0 ){
			star = i;
			break;	
		}
	}


	sprintf(str,"select pn from intdata where idxnum=%d and acc='%s' and STAR=%d",id,acc,star);
	if( !mysql_query(&mysql, str ) ){
		int num_row=0;
		mysql_result = mysql_store_result( &mysql );
		num_row = mysql_num_rows(mysql_result);//取得的資料筆數(正常最多取得一筆)
		mysql_free_result(mysql_result);
		log("\nsasql intdata num_row:%d",num_row);
		if( num_row != 0 ){//修改資料
			char buf[5012],buf2[64];
			char *p=NULL;
			memset(buf,0,5012);
			for( i=0;i<153;i++ ){//int資料欄位有增加的話,迴圈也要增加
				easyGetTokenFromBuf( data, '\n', i+1, buf2, sizeof( buf2 ));
				sprintf(str,"update intdata set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
				if( !mysql_query(&mysql, str ) ){
				//	log("\ninsert into intdata values OK");
				}
				else{
				//	log("\ninsert into intdata values ERR");
				}
			}
		}
		else{//增加空的資料
			char buf[5012],buf2[64];
			char *p=NULL;
			memset(buf,0,5012);
			for( i=0;i<153;i++ ){//int資料欄位有增加的話,迴圈也要增加
				strcat( buf, "0" );
				if( i != 153-1 )
					strcat(buf,",");
			}
			sprintf(str,"insert into intdata values ( %d, '%s', %d, %s )",star,acc,id,buf);
			if( !mysql_query(&mysql, str ) ){
				log("\ninsert into intdata values OK");
				//return TRUE;
			}
			else{
				log("\ninsert into intdata values ERR");
				return FALSE;
			}
			//填入正確的資料
			memset(buf,0,5012);
			for( i=0;i<153;i++ ){//int資料欄位有增加的話,迴圈也要增加
				easyGetTokenFromBuf( data, '\n', i+1, buf2, sizeof( buf2 ));
				sprintf(str,"update intdata set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
				if( !mysql_query(&mysql, str ) ){
				//	log("\ninsert into intdata values OK");
				}
				else{
				//	log("\ninsert into intdata values ERR");
				}
			}
		}
	}
	else{
		log("\nselect pn from intdata ERR");
		return FALSE;
	}


	return TRUE;
}

int sasql_save_char_info( int id, char *acc, char *data )
{
	int i,star;
	char str[10042],*p=NULL;
	memset(str,0,10042);

	for(i=0;i<16;i++){
		if( strcmp( servername[i], saacname) == 0 ){
			star = i;
			break;	
		}
	}

	if( p = strstr(data, "name=") ){
		char data2[CHARDATASIZE];
		memset(data2,0,CHARDATASIZE);
		sprintf(data2,"%s",p);
	
		sprintf(str,"select name from chardata where idxnum=%d and acc='%s' and STAR=%d",id,acc,star);
		if( !mysql_query(&mysql, str ) ){
			int num_row=0;
			mysql_result = mysql_store_result( &mysql );
			num_row = mysql_num_rows(mysql_result);//取得的資料筆數(正常最多取得一筆)
			mysql_free_result(mysql_result);
			log("\nsasql chardata num_row:%d",num_row);
			if( num_row != 0 ){//修改資料
				char buf[5012],buf2[64];
				memset(buf,0,5012);
				for( i=0;i<11;i++ ){//資料欄位有增加的話,迴圈也要增加
					char buf3[64],*p=NULL;
					memset(buf3,0,64);
					easyGetTokenFromBuf( data2, '\n', i+1, buf2, sizeof( buf2 ));
					if( p = strtok(buf2,"=") ){
						strncpy(buf3,p,32);
						if( p = strtok(NULL,"=") ){
							log("\n%s",p);
							strncpy(buf2,p,32);
							strcat(buf3,"='");
							strcat(buf3,buf2);
							strcat(buf3,"'");
							strncpy(buf2,buf3,64);
							log("\n%s",buf3);
						}
						else{
							sprintf(buf2,"%s=NULL",buf3);
						}
					}
					sprintf(str,"update chardata set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
					if( !mysql_query(&mysql, str ) ){
					//	log("\ninsert into intdata values OK");
					}
					else{
					//	log("\ninsert into intdata values ERR");
					}
				}
			}
			else{//增加空的資料
				char buf[5012],buf2[64];
				char *p=NULL;
				memset(buf,0,5012);
				sprintf(buf,"%s","NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL");//char資料欄位有增加的話,空字串也要增加
				sprintf(str,"insert into chardata values ( %d, '%s', %d, %s )",star,acc,id,buf);
				if( !mysql_query(&mysql, str ) ){
					log("\ninsert into chardata values OK");
					//return TRUE;
				}
				else{
					log("\ninsert into chardata values ERR");
					return FALSE;
				}
				//填入正確的資料
				memset(buf,0,5012);
				for( i=0;i<11;i++ ){//int資料欄位有增加的話,迴圈也要增加
					char buf3[64],*p=NULL;
					memset(buf3,0,64);
					easyGetTokenFromBuf( data2, '\n', i+1, buf2, sizeof( buf2 ));
					if( p = strtok(buf2,"=") ){
						strncpy(buf3,p,32);
						if( p = strtok(NULL,"=") ){
							log("\n%s",p);
							strncpy(buf2,p,32);
							strcat(buf3,"='");
							strcat(buf3,buf2);
							strcat(buf3,"'");
							strncpy(buf2,buf3,64);
							log("\n%s",buf3);
						}
						else{
							sprintf(buf2,"%s=NULL",buf3);
						}
					}
					sprintf(str,"update chardata set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
					if( !mysql_query(&mysql, str ) ){
					//	log("\ninsert into intdata values OK");
					}
					else{
					//	log("\ninsert into intdata values ERR");
					}
				}
			}
		}
		else{
			log("\nselect name from chardata ERR");
			return FALSE;
		}
		
	}

	if( p = strstr(data, "flg0=") ){
		char data2[CHARDATASIZE];
		memset(data2,0,CHARDATASIZE);
		sprintf(data2,"%s",p);
		sprintf(str,"select idxnum from flg where idxnum=%d and acc='%s' and STAR=%d",id,acc,star);
		if( !mysql_query(&mysql, str ) ){
			int num_row=0;
			mysql_result = mysql_store_result( &mysql );
			num_row = mysql_num_rows(mysql_result);//取得的資料筆數(正常最多取得一筆)
			mysql_free_result(mysql_result);
			log("\nsasql flg num_row:%d",num_row);
			if( num_row != 0 ){//修改資料
				char buf[5012],buf2[64];
				char *p=NULL;
				memset(buf,0,5012);
				for( i=0;i<3;i++ ){//資料欄位有增加的話,迴圈也要增加
					easyGetTokenFromBuf( data2, '\n', i+1, buf2, sizeof( buf2 ));
					sprintf(str,"update flg set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
					if( !mysql_query(&mysql, str ) ){
					//	log("\ninsert into intdata values OK");
					}
					else{
					//	log("\ninsert into intdata values ERR");
					}
				}
			}
			else{//增加空的資料
				char buf[5012],buf2[64];
				char *p=NULL;
				memset(buf,0,5012);
				for( i=0;i<3;i++ ){//資料欄位有增加的話,迴圈也要增加
					strcat( buf, "0" );
					if( i != 3-1 )
						strcat(buf,",");
				}
				sprintf(str,"insert into flg values ( %d, '%s', %d, %s )",star,acc,id,buf);
				if( !mysql_query(&mysql, str ) ){
					log("\ninsert into flg values OK");
					//return TRUE;
				}
				else{
					log("\ninsert into flg values ERR");
					return FALSE;
				}
				memset(buf,0,5012);
				//填入正確的資料
				for( i=0;i<3;i++ ){//資料欄位有增加的話,迴圈也要增加
					easyGetTokenFromBuf( data2, '\n', i+1, buf2, sizeof( buf2 ));
					sprintf(str,"update flg set %s where idxnum=%d and acc='%s' and STAR=%d",buf2,id,acc,star);
					if( !mysql_query(&mysql, str ) ){
					//	log("\nupdate into flg values OK");
					}
					else{
					//	log("\nupdate into flg values ERR");
					}
				}
			}
		}
		else{
			log("\nselect name from flg ERR");
			return FALSE;
		}
	}

	return TRUE;
}

#endif