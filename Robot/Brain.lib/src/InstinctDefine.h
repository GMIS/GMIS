﻿/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _INSTINCT_DEFINE_
#define _INSTINCT_DEFINE_

#include "MsgList.h"

//语言字符集，第一个字符为ID+1 

#define    CHAR_START        127282417795630000    
#define	   CharToID(ch)      CHAR_START+ch 
#define    IDToChar(ID)      ID - CHAR_START 



/**********************************************
  定义本能ID
***********************************************/

#define INSTINCT_START                      127282417796630000    

//通用命令---（34）
/////////////////////////////////

#define    INSTINCT_DEFINE_BYTE			    127282417796630001 //定义数据；
#define    INSTINCT_DEFINE_INT32		    127282417796630002
#define    INSTINCT_DEFINE_UINT32		    127282417796630003
#define    INSTINCT_DEFINE_INT64		    127282417796630004
#define    INSTINCT_DEFINE_UINT64		    127282417796630005
#define    INSTINCT_DEFINE_FLOAT32		    127282417796630006	
#define    INSTINCT_DEFINE_FLOAT64		    127282417796630007
#define    INSTINCT_DEFINE_STRING			127282417796630008     

#define    INSTINCT_USE_OPERATOR            127282417796630009 // 基本运算

#define    INSTINCT_USE_RESISTOR            127282417796630010 // 基本元件；
#define    INSTINCT_USE_INDUCTOR            127282417796630011
#define    INSTINCT_USE_CAPACITOR           127282417796630012
#define    INSTINCT_USE_DIODE               127282417796630013

#define    INSTINCT_REFERENCE_INDUCTOR      127282417796630014
#define    INSTINCT_REFERENCE_CAPACITOR     127282417796630015

#define    INSTINCT_SET_LABEL               127282417796630101
#define    INSTINCT_GOTO_LABEL              127282417796630102
#define    INSTINCT_VIEW_PIPE               127282417796630103

#define    INSTINCT_INPUT_TEXT              127282417796630201 
#define    INSTINCT_INPUT_NUM               127282417796630202

#define    INSTINCT_WAIT_SECOND             127282417796630203

#define    INSTINCT_TABLE_CREATE			127282417796631001
#define    INSTINCT_TABLE_IMPORT			127282417796631002
#define    INSTINCT_TABLE_EXPORT			127282417796631003
#define	   INSTINCT_TABLE_FOCUS			    127282417796631004
#define    INSTINCT_TABLE_INSERT_DATA		127282417796631005
#define    INSTINCT_TABLE_GET_DATA			127282417796631006
#define    INSTINCT_TABLE_REMOVE_DATA		127282417796631007
#define    INSTINCT_TABLE_GET_SIZE			127282417796631008
#define    INSTINCT_TABLE_MODIFY_DATA       127282417796631009
#define    INSTINCT_TABLE_CLOSE			    127282417796631010

#define    INSTINCT_USE_LOGIC               127282417796632001
#define    INSTINCT_FOCUS_LOGIC				127282417796632002
#define    INSTINCT_NAME_LOGIC				127282417796632003
#define    INSTINCT_INSERT_LOGIC			127282417796632004	
#define    INSTINCT_REMOVE_LOGIC			127282417796632005

#define    INSTINCT_GET_DATE                127282417796632006
#define    INSTINCT_GET_TIME                127282417796632007

#define    INSTINCT_OUTPUT_INFO             127282417796632008

//外部命令——操作大脑以外资源的行为（6）
////////////////////////////
#define    INSTINCT_START_OBJECT            127282417797000001
#define    INSTINCT_FOCUS_OBJECT            127282417797000002
#define    INSTINCT_NAME_OBJECT				127282417797000003
#define    INSTINCT_USE_OBJECT              127282417797000004
#define    INSTINCT_CLOSE_OBJECT            127282417797000005
#define    INSTINCT_GET_OBJECT_DOC          127282417797000006

#define    INSTINCT_ASK_PEOPLE              127282417797001001



//内部命令——仅仅影响大脑内部的行为
/////////////////////////////

// 1 内部独立命令——只能单一执行，不能与其他命令组合为逻辑（9）
#define    INSTINCT_BRAIN_INIT				127282417798000000

#define    INSTINCT_THINK_LOGIC             127282417798000001
#define    INSTINCT_RUN_TASK                127282417798000002
#define    INSTINCT_DEBUG_TASK              127282417798000003
#define    INSTINCT_STOP_TASK               127282417798000004
#define    INSTINCT_PAUSE_TASK              127282417798000005
#define    INSTINCT_STEP_TASK               127282417798000006
#define    INSTINCT_GOTO_TASK               127282417798000007

#define    INSTINCT_CLOSE_DIALOG            127282417798000008
#define    INSTINCT_SET_GLOBLELOGIC         127282417798000009


// 2 内部非独立命令（21）
#define    INSTINCT_LEARN_TOKEN             127282417798010001
#define    INSTINCT_LEARN_PRONOUN           127282417798010002
#define    INSTINCT_LEARN_ADJECTIVE         127282417798010003
#define    INSTINCT_LEARN_NUMERAL           127282417798010004
#define    INSTINCT_LEARN_VERB              127282417798010005
#define    INSTINCT_LEARN_ADVERB            127282417798010006
#define    INSTINCT_LEARN_ARTICLE           127282417798010007
#define    INSTINCT_LEARN_PREPOSITION       127282417798010008
#define    INSTINCT_LEARN_CONJUNCTION       127282417798010009
#define    INSTINCT_LEARN_INTERJECTION      127282417798010010
#define    INSTINCT_LEARN_NOUN              127282417798010011
#define    INSTINCT_LEARN_TEXT              127282417798010012

#define    INSTINCT_LEARN_LOGIC             127282417798010013
#define    INSTINCT_LEARN_OBJECT            127282417798010014
#define    INSTINCT_LEARN_ACTION            127282417798010015

#define    INSTINCT_FIND_SET_STARTTIME      127282417798010016
#define    INSTINCT_FIND_SET_ENDTIME        127282417798010017
#define    INSTINCT_FIND_SET_PRICISION      127282417798010018


#define    INSTINCT_FIND                    127282417798010019
#define    INSTINCT_FIND_LOGIC              127282417798010020
#define    INSTINCT_FIND_OBJECT             127282417798010021

#define    INSTINCT_USE_ARM                 127282417798010022

#define    INSTINCT_CREATE_ACCOUNT          127282417798010023
#define    INSTINCT_DELETE_ACCOUNT			127282417798010024

#define    ID_INSTINCT_END                  127282417798010025

//
//misc
//////////////////////////////////////////////////////////////


#define    IsDefineNum(ID)             (ID>127282417796630000 && ID<127282417796630008)
#define    IsDefineOperator(ID)        (ID>127282417796630000 && ID<127282417796630009)

#define    IsLearnWord(ID)             (ID>127282417798010000 && ID<127282417798010012)
#define    IsLearnText(ID)             (ID>127282417798010000 && ID<127282417798010013)

#define    BelongCommanAction(ID)      (ID>INSTINCT_START && ID<127282417796633000)
#define    BelongExternAction(ID)      (ID>127282417797000000 && ID<127282417797002000)
#define    BelongInterAction(ID)       (ID>127282417798000000 && ID<ID_INSTINCT_END)
#define    BelongIndeInterAction(ID)   (ID>127282417798000000 && ID<127282417798010000) //内部独立命令


#define    BelongInstinct(ID)          (ID>INSTINCT_START && ID<ID_INSTINCT_END)


static _MSG_TEXT _InstinctTextList[]={                
	    _MSG2TEXT(INSTINCT_DEFINE_BYTE),
		_MSG2TEXT(INSTINCT_DEFINE_INT32),
        _MSG2TEXT(INSTINCT_DEFINE_UINT32),
        _MSG2TEXT(INSTINCT_DEFINE_INT64),
		_MSG2TEXT(INSTINCT_DEFINE_UINT64),
		_MSG2TEXT(INSTINCT_DEFINE_FLOAT32),
		_MSG2TEXT(INSTINCT_DEFINE_FLOAT64),
		_MSG2TEXT(INSTINCT_DEFINE_STRING),

		_MSG2TEXT(INSTINCT_USE_OPERATOR),

		_MSG2TEXT(INSTINCT_USE_RESISTOR),
		_MSG2TEXT(INSTINCT_USE_INDUCTOR),
		_MSG2TEXT(INSTINCT_USE_CAPACITOR),
		_MSG2TEXT(INSTINCT_USE_DIODE),

        _MSG2TEXT(INSTINCT_REFERENCE_INDUCTOR),
        _MSG2TEXT(INSTINCT_REFERENCE_CAPACITOR),
		
        _MSG2TEXT(INSTINCT_SET_LABEL),
        _MSG2TEXT(INSTINCT_GOTO_LABEL),
        _MSG2TEXT(INSTINCT_VIEW_PIPE),
		
        _MSG2TEXT(INSTINCT_INPUT_TEXT),
        _MSG2TEXT(INSTINCT_INPUT_NUM),
		_MSG2TEXT(INSTINCT_WAIT_SECOND),

        _MSG2TEXT(INSTINCT_TABLE_CREATE),
		_MSG2TEXT(INSTINCT_TABLE_IMPORT),
		_MSG2TEXT(INSTINCT_TABLE_EXPORT),
		_MSG2TEXT(INSTINCT_TABLE_FOCUS),
        _MSG2TEXT(INSTINCT_TABLE_INSERT_DATA),
        _MSG2TEXT(INSTINCT_TABLE_GET_DATA),
        _MSG2TEXT(INSTINCT_TABLE_REMOVE_DATA),
        _MSG2TEXT(INSTINCT_TABLE_GET_SIZE),
        _MSG2TEXT(INSTINCT_TABLE_CLOSE),

		_MSG2TEXT(INSTINCT_USE_LOGIC),
		_MSG2TEXT(INSTINCT_FOCUS_LOGIC),
        _MSG2TEXT(INSTINCT_NAME_LOGIC),
        _MSG2TEXT(INSTINCT_INSERT_LOGIC),
        _MSG2TEXT(INSTINCT_REMOVE_LOGIC),

		_MSG2TEXT(INSTINCT_GET_DATE),
		_MSG2TEXT(INSTINCT_GET_TIME),

		_MSG2TEXT(INSTINCT_OUTPUT_INFO),

        _MSG2TEXT(INSTINCT_START_OBJECT),
		_MSG2TEXT(INSTINCT_FOCUS_OBJECT),
		_MSG2TEXT(INSTINCT_NAME_OBJECT),
		_MSG2TEXT(INSTINCT_USE_OBJECT),
        _MSG2TEXT(INSTINCT_CLOSE_OBJECT),

        _MSG2TEXT(INSTINCT_ASK_PEOPLE),

		_MSG2TEXT(INSTINCT_THINK_LOGIC),
		_MSG2TEXT(INSTINCT_RUN_TASK),
		_MSG2TEXT(INSTINCT_DEBUG_TASK),
		_MSG2TEXT(INSTINCT_STOP_TASK),
		_MSG2TEXT(INSTINCT_PAUSE_TASK),
		_MSG2TEXT(INSTINCT_STEP_TASK),
		_MSG2TEXT(INSTINCT_GOTO_TASK),

		_MSG2TEXT(INSTINCT_SET_GLOBLELOGIC),
		_MSG2TEXT(INSTINCT_CLOSE_DIALOG),

		_MSG2TEXT(INSTINCT_LEARN_TOKEN),
		_MSG2TEXT(INSTINCT_LEARN_PRONOUN),
		_MSG2TEXT(INSTINCT_LEARN_ADJECTIVE),
		_MSG2TEXT(INSTINCT_LEARN_NUMERAL),
		_MSG2TEXT(INSTINCT_LEARN_VERB),
		_MSG2TEXT(INSTINCT_LEARN_ADVERB),
		_MSG2TEXT(INSTINCT_LEARN_ARTICLE),
		_MSG2TEXT(INSTINCT_LEARN_PREPOSITION),
		_MSG2TEXT(INSTINCT_LEARN_CONJUNCTION),
		_MSG2TEXT(INSTINCT_LEARN_INTERJECTION),
		_MSG2TEXT(INSTINCT_LEARN_NOUN),
		_MSG2TEXT(INSTINCT_LEARN_TEXT),

		_MSG2TEXT(INSTINCT_LEARN_LOGIC),
		_MSG2TEXT(INSTINCT_LEARN_OBJECT),
		_MSG2TEXT(INSTINCT_LEARN_ACTION),

		_MSG2TEXT(INSTINCT_FIND_SET_STARTTIME),
		_MSG2TEXT(INSTINCT_FIND_SET_ENDTIME),
		_MSG2TEXT(INSTINCT_FIND_SET_PRICISION),

		_MSG2TEXT(INSTINCT_FIND),
		_MSG2TEXT(INSTINCT_FIND_LOGIC),
		_MSG2TEXT(INSTINCT_FIND_OBJECT),
		_MSG2TEXT(INSTINCT_USE_ARM),
};

 



#endif  //_INSTINCT_DEFINE_