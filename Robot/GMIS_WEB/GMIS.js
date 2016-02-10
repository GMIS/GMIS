  
      var ws;
      var SocketCreated = false;
      var isUserloggedout = false;
      var SendedMsgTimeStamp = 0;
      var SendedMsgID = 0;
      	  
  	  (function(){
  	  	 msgid = {
  	  	 	LINKER_FEEDBACK: '2',
  	     	MSG_BASE: '127282417797660000',
  	     	MSG_WHO_ARE_YOU: '127282417797660001',
  	     	MSG_I_AM: '127282417797660201',
            MSG_CONNECT_OK: '127282417797660102',
            MSG_BRAIN_TO_GUI:'127283417797660101'
  	     } 
  	     　
  	     Object.freeze(msgid); // 冻结内部属性
    　　
    　　 Object.defineProperties(window, {
        　　MSG_ID: {
        　　　　configurable: false,
        　　　　writable: false,
        　　　　value: msgid // 指向上面冻结的对象user
        　　}
    　　}); 
  	  }())

      function  MsgProc(s)
      {
      	m = new GMIS_Msg();
      	ret = m.FromString(s);
      	if(!ret){ 
      		Pipe = new ePipeline();
      		ret = Pipe.FromString(s);
      		if(ret){
      			if(Pipe.ID == window.MSG_ID.LINKER_FEEDBACK){
      				//info = "远端收到MSG:"+SendedMsgID;
      				//Log(info,"OK");
      				return;
      			};
      		}
      	   Log("收到一个格式不符的信息:"+s,"ERROR");
      	   return;
    	}else{ //首先发送一个反馈信息以便对方能继续发送下一个信息
    		TimeStamp = m.ID;
    	    SenderID  = m.GetSenderID();
    		rMsg = new ePipeline();
    	    rMsg.ID = window.MSG_ID.LINKER_FEEDBACK;
			rMsg.PushInt('1'); //RECEIVE_OK
			rMsg.PushInt(TimeStamp);
			rMsg.PushInt(SenderID);
			s = rMsg.ToString();
			ws.send(s);
    	}
      	MsgID = m.GetMsgID();
      	switch(MsgID){
      	case window.MSG_ID.MSG_WHO_ARE_YOU:
      		OnWhoAreYou(m);
            break;
	    case window.MSG_ID.MSG_CONNECT_OK:
            OnConnectOK(m);
	        break;
	    case window.MSG_ID.MSG_BRAIN_TO_GUI:
	    	OnBrainToGUI(m);
	        break;
	    default:
	        s = "收到未定义信息："+MsgID;
	        Log(s,"ERROR");
	        break;
      	}
  	  }
  	  
  	  function OnWhoAreYou(m){
  	  	  SenderID = m.GetSenderID();
  	  	  rMsg = new GMIS_Msg();
  	  	  rMsg.WriteEnvelope(SenderID,window.MSG_ID.MSG_I_AM,0,0);
	
		  rMsg.Letter.PushString("zhb1");   //user name
         	  rMsg.Letter.PushString("zhb1123"); //
	  	  
	  	  SendToBrain(rMsg);
  	  }
  	  	 
      function OnConnectOK(m){
  	  	 Log("登录成功", "OK");
  	  }

      function OnBrainToGUI(m){
  	  	 Log("收到一个Brain to GUI信息", "OK");
  	  }
      function SendToBrain(m){
      	   m.ID = new Date().getTime();
      	   s = m.ToString();
      	   
      	   SendedMsgTimeStamp = m.ID;
      	   SendedMsgID = m.GetMsgID();
      	   
      	   ws.send(s);
      }

      function ViewportLocked(str) 
      { 
         var view = document.getElementById('Viewport'); 
         if (view) 
            view.className = 'ViewportLocked'; 
         view.innerHTML = str; 
      } 

      function ViewportNormal()
      {
         var view = document.getElementById('Viewport'); 
         view.className = 'ViewportNormal'; 
      }

      function ClickConnectionBnt() {
            if (SocketCreated && (ws.readyState == 0 || ws.readyState == 1)) {
                 SocketCreated = false;
       	         isUserloggedout = true;
         	     ws.close();
                 ViewportLocked("离开聊天室...");  
        } else {
            	
                ViewportLocked("进入聊天室...");  
                Log("准备连接到聊天服务器 ...");
                try {
                    if ("WebSocket" in window) {
                    	ws = new WebSocket("ws://" + document.getElementById("IP_address").value);
                    }
                    else if("MozWebSocket" in window) {
                    	ws = new MozWebSocket("ws://" + document.getElementById("IP_address").value);
                    }
                    
                    SocketCreated = true;
                    isUserloggedout = false;
                } catch (ex) {
                    Log(ex, "ERROR");
                    return;
                }
                
                document.getElementById("ConnecttionBnt").innerHTML = "断开";
                ws.onopen = WSonOpen;
                ws.onmessage = WSonMessage;
                ws.onclose = WSonClose;
                ws.onerror = WSonError;
            }
        };


        function WSonOpen() {
            ViewportNormal();
            Log("连接已经建立。", "OK");
            $("#SendView").show();
   			//ws.send("login:" + document.getElementById("UserName").value);
   			//TestMsg();
        };

        function WSonMessage(event) {

            MsgProc(event.data);
            
            //Log(event.data);            
        };

        function WSonClose() {
            ViewportNormal();
            if (isUserloggedout)
                Log("【"+document.getElementById("UserName").value+"】离开了聊天室！");
            document.getElementById("ConnecttionBnt").innerHTML = "连接";
            $("#SendView").hide();
            
        };

		function TestPipe()
		{
	      		childpipe = new ePipeline();
            	childpipe.PushInt(135);
            	childpipe.PushFloat(53.23);
            	childpipe.PushString("hello 你好");
            	s = childpipe.ToString();
            	
            	pipe = new ePipeline();
            	pipe.PushInt(135);
            	pipe.PushFloat(53.23);
            	pipe.PushString("hello 你好");
            	pipe.PushNull();
            	pipe.PushPipe(childpipe);
            	
            	s = pipe.ToString();
            	//s = "4@2@4@3@0@@5@1@1@04@2123103235307301495@4@3@0@@104@4@0@4@3@0@@5@1@1@04@127282417797660001@4@3@0@@0@4@2123103235307301493@4@3@0@@24@1@19@2123103235307301494";
            	ret = pipe.FromString(s);
            	
            	
            	if(ret){
            		Log("pipe 测试正确", "OK");
            	}else{
            		Log("pipe 测试错误", "ERROR");
            	}
        }
        
        function TestMsg(){
        	m = new GMIS_Msg();
        	m.ID = 12;
        	m.Label="this is test";
        	m.ReceiverInfo.PushInt(127282417797660001);
        	m.SenderInfo.PushFloat(465.6);
        	m.Letter.PushString("this is letter text");
        	
        	s = m.ToString();
        	
        	//s = "4@2123105954992588378@0@@92@4@0@0@@5@1@1@04@127282417797660001@0@@0@4@2123105954992588376@0@@24@1@19@2123105954992588377";
        	s = "4@2123215871002256258@0@@267@4@0@0@@29@1@19@21232158709445293361@1@04@127283417797660101@0@@148@4@4001@0@@134@1@3@1064@0@0@@116@1@1@01@1@01@2@-13@6@System1@1@11@2@101@1@01@2@-13@10@zhb(Local)1@1@11@19@21232158709445293361@1@01@2@-13@4@zhb11@1@14@2123215871002246254@0@@24@1@19@2123215870944529356"

        	ret = m.FromString(s);
                 
            ID = m.GetMsgID();
            if(ret){
            	Log("Msg 测试正确", "OK");
            }else{
            	Log("Msg 测试错误", "ERROR");
            }	
        }
        function WSonError() {
            ViewportNormal();
            Log("远程连接中断。", "ERROR");
            
        };


        function ClicktSendBnt() {
            if (document.getElementById("InputEdit").value.trim() != "") {
             	
             	s = document.getElementById("InputEdit").value;
             	rMsg = new GMIS_Msg();
  	  	 	
  	  	 		rMsg.WriteEnvelope(0,window.MSG_ID.MSG_I_AM,0,0);
		  		rMsg.Letter.PushString(s);   
   	  	  
	  	  		SendToBrain(rMsg);
                document.getElementById("InputEdit").value = "";
            }
        };


        function Log(Text, MessageType) {
            if (MessageType == "OK") Text = "<span style='color: green;'>" + Text + "</span>";
            if (MessageType == "ERROR") Text = "<span style='color: red;'>" + Text + "</span>";
            document.getElementById("LogView").innerHTML = document.getElementById("LogView").innerHTML + Text + "<br />";
            var LogContainer = document.getElementById("LogView");
            LogContainer.scrollTop = LogContainer.scrollHeight;
        };


        $(document).ready(function () {
            $("#SendView").hide();
            var WebSocketsExist = true;
            try {
                var dummy = new WebSocket("ws://localhost:8989/test");
            } catch (ex) {
                try
                {
                	webSocket = new MozWebSocket("ws://localhost:8989/test");
                }
                catch(ex)
                {
                	WebSocketsExist = false;
                }
            }

            if (WebSocketsExist) {
                document.getElementById("IP_address").value = "127.0.0.1:80";
            } else {
                Log("您的浏览器不支持WebSocket。请选择其他的浏览器再尝试连接服务器。", "ERROR");
                document.getElementById("ConnecttionBnt").disabled = true;
            }    
            
            $("#InputEdit").keypress(function(evt)
            {
            		if (evt.keyCode == 13)
            		{
            				$("#SendBnt").click();
            				evt.preventDefault();
            		}
            })        
        });