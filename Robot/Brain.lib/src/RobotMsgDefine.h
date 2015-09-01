//******* BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is ZhangHongBing (HongBing75@gmail.com)
//
// The Initial Developer of the Original Code is 
//        ZhangHongBing (HongBing75@gmail.com)
//
// Portions created by the Initial Developer are Copyright (C) 2007
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//
// ***** END LICENSE BLOCK *****/


#ifndef _ROBOTMSGDEFINE_H
#define _ROBOTMSGDEFINE_H

//////////////////
//今后所有定义都应该统一整理一遍
//////////////////


//定义标准信息源
#define  FROM_BRAIN   0      //大脑内部组织的信息源ID

//内部使用，不公开
#define    MSG_SAYTEXT               127282417796650033
#define    MSG_SAYCONTINUE           127282417796650034
#define    MSG_SAYISEE               127282417796650035     
#define    MSG_SAY_END               127282417796650036

#define    MSG_CONV_OUTPUT           127282417796650037
#define    MSG_CONV_CTRL             127282417796650038
/*
#define    MSG_ANALYSIS_OK           127282417796650037
#define    MSG_ANALYSIS_ERROR        127282417796650038
#define    MSG_ANALYSIS_WARNING      127282417796650039
#define    MSG_EXECUTE_MSG           127282417796650040
#define    MSG_FORECAST_WORD         127282417796650041
#define    MSG_FORECAST_CLAUSE       127282417796650042
#define    MSG_FORECAST_SENTENCE     127282417796650043
#define    MSG_FORECAST_TEXT         127282417796650044
#define    MSG_RECEIVE_MSG           127282417796650045
#define    MSG_EXECUTE_WAIT          127282417796650046
#define    MSG_FORECAST_BEGIN        127282417796650047
*/

#define    MSG_TASK_END              127282417796650048

 
#define    MSG_FIND_SHOW             127282417796650051
#define    MSG_SHOW_THINKRESULT      127282417796650052
#define    MSG_SHOW_ANALYSERESULT    127282417796650053
           
			
//MSG_INTER_EXECUTE or MSG_COMMON_EXECUTE or MSG_URGENT_EXECUTE(还没实现)

//以下执行是逻辑组织分析完毕后呼叫执行器官执行 
#define    MSG_INTER_EXECUTE         127282417796660039 //内部执行
#define    MSG_OUTER_EXECUTE         127282417796660040 //外部执行
#define    MSG_COMMON_EXECUTE        127282417796660041
#define    MSG_URGENT_EXECUTE        127282417796660042 //紧急执行
#define    MSG_CONSIGN_EXECUTE       127282417796660043
/*
#define    MSG_COMPILE_TASK          127282417796660050 
      #define ERROR_COMPILE          0
      #define BEGIN_COMPILE          1
      #define END_COMPILE            2
//#define    MSG_EXECUTE_TASK          127282417796660051 //通知系统开始执行一个内部任务
      #define ERROR_EXECUTE          0
      #define BEGIN_EXECUTE          1
      #define END_EXECUTE            2
*/
#define    MSG_DEBUG_STEP            127282417796660052
//#define    MSG_CLOSE_DIALOG          127282417796660053

#define    MSG_EXECUTE_CHAT          127282417796660054
#define    MSG_TASK_CONTROL          127282417796660055
#define    MSG_DEBUG_INFO            127282417796660056
#define    MSG_CREATE_NEWTHREAD      127282417796660057 
#define    MSG_SHOW_THREADVIEW       127282417796660058
#define    MSG_DELETE_THREAD         127282417796660059 
#define    MSG_CLOSE_THREADVIEW      127282417796660060
#define    MSG_RUN_TASK              127282417796660061 
#define    MSG_RUN_TASKRESULT        127282417796660062 
#define    MSG_DELETE_LOGIC          127282417796660063
 
#define    MSG_SET_GLOBLELOGIC       127282417796660064



#define CM_OUTPUT_TIP      10000      //在状态栏显示信息  
#define CM_OUTPUT_RESULT   10001      //输出各种中间结果
#define  THINK_FORECAST  100   //在输出窗口显示预测信息
#define  THINK_ERROR     101   //在输出窗口显示错误信息，并且在状态栏显示错误
#define  ANALYSE_RESULT  102   //在输出窗口显示思绪对当前输入的分析或思考结果
#define  RUNTIME_INFO    103   //在输出窗口显示任何运行时信息
#define  INPUT_TEXT      104   //在输出窗口显示输入的信息
#define  FEEDBACK_TEXT   105  
#define CM_CONV_TITLE      10002      //在Conv窗口Title显示任务名
#define CM_SET_PROGRESS    10004      //设置状态栏的进度 
#define  IN_PROGRESS   1      //处理接收到的信息进度 
#define  OUT_PROGRESS  2      //发送消息的进度
#define  WORK_PROGRESS 3      //内部工作进度

#define CM_FIND            10005
#define FIND_START     100
#define FIND_UPDATE    101
#define FINDED_LOGIC   102
#define FINDED_OBJECT  103
#define FINDED_TEXT    104
#define FIND_STOP      105 
#define CM_LOGICVIEW       10006
#define ADD_LOGIC      100
#define ADD_CAPA       101 
#define ADD_INDU       102
#define REF_LOGIC      103
#define REF_CAPA       104 
#define REF_INDU       105 

//Child Cmd of MSG_CONV_CTRL
#define CM_CONTINUEINPUT     20000  //输入窗口有效并可能被预置信息,Send按钮有效
#define CM_TASKBEGIN         20001  //使输入窗口无效，输入文本转入历史窗口，如果是嵌套任务则转入输出窗口
#define CM_TASKRUN           20002  //针对可调试的任务，Pause|Debug|Stop按钮有效
#define CM_NTASKRUN          20003  //针对不能调式的任务,Pause|Stop按钮有效
#define CM_NTASKPAUSE        20004  //针对不能调试的任务，RUN|STOP按钮有效   
#define CM_TASKDEBUG         20005  //Conv设置为Debug模式，通知调试窗口，Run|Step|Goto|Stop按钮有效，状态棒显示调试
#define CM_TASKPAUSE         20006  //Run|Debug|Stop按钮有效，状态棒显示暂停
#define CM_TASKEND           20007  //在历史窗口显示反馈信息，Conv重置为NORMAL_MODE模式，清空输入窗口并使其有效，在状态栏显示反馈信息，Send按钮有效
#define SUCCESS     1
#define FAIL        0
#define CM_TASKFEEDBACK      20008  //输入窗口有效，输入窗口预置提示信息，Stop|Send按钮有效
#define CM_WAITEND           20009  //等待执行结束，此时所有按钮无效，输入窗口无效，提示等待原因

    #define CM_BRAINTASKBEGIN    20010  //输入窗口无效，模拟命令文本至对话窗口，STOP按钮有效

#endif  //_ROBOTMSGDEFINE_H