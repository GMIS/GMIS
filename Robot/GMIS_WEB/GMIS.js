
//预定义的信息ID
(function() {
    msgid = {
        LINKER_FEEDBACK: '2',
        MSG_BASE: '127282417797660000',
        MSG_WHO_ARE_YOU: '127282417797660001',
        MSG_I_AM: '127282417797660201',
        MSG_CONNECT_OK: '127282417797660102',
        MSG_BRAIN_TO_GUI: '127283417797660101'
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

//////////////////////////////

var GMIS = {}; //命名空间
GMIS.ws = null;
GMIS.bUserLogin = false;
GMIS.SendedMsgTimeStamp = 0;
GMIS.SendedMsgID = 0;

GMIS.Log = function(Text, MessageType) {
    if (MessageType == "OK") Text = "<span style='color: green;'>" + Text + "</span>";
    if (MessageType == "ERROR") Text = "<span style='color: red;'>" + Text + "</span>";
    document.getElementById("LogView").innerHTML = document.getElementById("LogView").innerHTML + Text + "<br />";
    var LogContainer = document.getElementById("LogView");
    LogContainer.scrollTop = LogContainer.scrollHeight;
};

GMIS.MsgProc = function(s) {
    m = new GMIS_Msg();
    ret = m.FromString(s);
    if (!ret) {
        Pipe = new ePipeline();
        ret = Pipe.FromString(s);
        if (ret) {
            if (Pipe.ID == window.MSG_ID.LINKER_FEEDBACK) {
                //info = "远端收到MSG:"+SendedMsgID;
                //Log(info,"OK");
                m = NULL;
                return;
            };
        }
        GMIS.Log("收到一个格式不符的信息:" + s, "ERROR");
        return;
    } else { //首先发送一个反馈信息以便对方能继续发送下一个信息
        TimeStamp = m.ID;
        SenderID = m.GetSenderID();
        rMsg = new ePipeline();
        rMsg.ID = window.MSG_ID.LINKER_FEEDBACK;
        rMsg.PushInt('1'); //RECEIVE_OK
        rMsg.PushInt(TimeStamp);
        rMsg.PushInt(SenderID);
        s = rMsg.ToString();
        ws.send(s);
    }
    MsgID = m.GetMsgID();
    switch (MsgID) {
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
            s = "收到未定义信息：" + MsgID;
            Log(s, "ERROR");
            break;
    }
    m = NULL;
}

GMIS.OnWhoAreYou = function (m) {
    SenderID = m.GetSenderID();
    rMsg = new GMIS_Msg();
    rMsg.WriteEnvelope(SenderID, window.MSG_ID.MSG_I_AM, 0, 0);

    rMsg.Letter.PushString("zhb1"); //user name
    rMsg.Letter.PushString("zhb1123"); //

    SendToBrain(rMsg);
    $('#LoginTip').show().html("Verifying user ...");
}

GMIS.OnConnectOK = function (m) {
	GMIS.EndConnection(true);
    GMIS.Log("登录成功", "OK");
}

GMIS.OnBrainToGUI = function (m) {
    Log("收到一个Brain to GUI信息", "OK");
}

GMIS.SendToBrain = function (m) {
    m.ID = new Date().getTime();
    s = m.ToString();

    SendedMsgTimeStamp = m.ID;
    SendedMsgID = m.GetMsgID();

    ws.send(s);
}

GMIS.StartConnection = function () {
	$('#LoginForm p').hide();	
    $('#LoginTip').show().html("Connecting...");
}

GMIS.EndConnection = function (success,str) {
    
    if(success){
    	$('#LoginTip').hide().html("");
    	#('#LoginView').hide();
    	$('#LoginForm p').show();;
    }else{
    	$('#LoginTip').html(str);
    	setTimeout(function(){
    		$('#LoginForm p').show();
    		$('#LoginTip').hide();
	    },1000); 
    }
}

GMIS.ClickLoginBnt = function () {
    if (GMIS.isUserloggedout) {
        return false;
    } else {
    	
    	var address = document.getElementById('ip_address').value;
    	if(address==""){
    		address = "127.0.0.1";
    	}
    	
        GMIS.StartConnection();
        try {
            if ("WebSocket" in window) {
                ws = new WebSocket("ws://" + address);
            } else if ("MozWebSocket" in window) {
                ws = new MozWebSocket("ws://" + address);
            }

        } catch (ex) {
        	
            GMIS.EndConnection(false,"Connection fail<br/>The browser don't support this app");
            return false;
        }

        ws.onopen = GMIS.WSonOpen;
        ws.onmessage = GMIS.WSonMessage;
        ws.onclose = GMIS.WSonClose;
        ws.onerror = GMIS.WSonError;
        
    }
    return false;
};


GMIS.WSonOpen = function () {
    GMIS.EndConnection(true);
    Log("连接已经建立。", "OK");
    $("#SendView").show();
    //ws.send("login:" + document.getElementById("UserName").value);
    //TestMsg();
};
GMIS.Layout = function(){
   //alert("test");
};
GMIS.WSonMessage = function (event) {

    MsgProc(event.data);

    //Log(event.data);            
};

GMIS.WSonClose = function () {
    if (GMIS.isUserloggedout)
        Log("【" + document.getElementById("UserName").value + "】离开了聊天室！");
    document.getElementById("ConnecttionBnt").innerHTML = "连接";
    $("#SendView").hide();

};

GMIS.WSonError = function () {
    GMIS.EndConnection(false,"Connection fail<br/>Network is not available");
   
};


GMIS.ClicktSendBnt = function () {
    if (document.getElementById("InputEdit").value.trim() != "") {

        s = document.getElementById("InputEdit").value;
        rMsg = new GMIS_Msg();

        rMsg.WriteEnvelope(0, window.MSG_ID.MSG_I_AM, 0, 0);
        rMsg.Letter.PushString(s);

        SendToBrain(rMsg);
        document.getElementById("InputEdit").value = "";
    }
};
$(document).ready(function() {
    $("#SendView").hide();
    
    //window.onresize=GMIS.Layout();
    
    var WebSocketsExist = true;
    try {
        var dummy = new WebSocket("ws://localhost");
    } catch (ex) {
        try {
            webSocket = new MozWebSocket("ws://localhost");
        } catch (ex) {
            WebSocketsExist = false;
        }
    }

    if (!WebSocketsExist) {
        Log("您的浏览器不支持WebSocket。请选择其他的浏览器再尝试连接服务器。", "ERROR");
        document.getElementById("ConnecttionBnt").disabled = true;
    }

    $("#InputEdit").keypress(function(evt) {
        if (evt.keyCode == 13) {
            $("#SendBnt").click();
            evt.preventDefault();
        }
    })
});