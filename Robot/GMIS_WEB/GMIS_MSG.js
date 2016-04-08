
/*test
function TestPipe() {
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


    if (ret) {
        Log("pipe 测试正确", "OK");
    } else {
        Log("pipe 测试错误", "ERROR");
    }
}

function TestMsg() {
    m = new GMIS_Msg();
    m.ID = 12;
    m.Label = "this is test";
    m.ReceiverInfo.PushInt(127282417797660001);
    m.SenderInfo.PushFloat(465.6);
    m.Letter.PushString("this is letter text");

    s = m.ToString();

    //s = "4@2123105954992588378@0@@92@4@0@0@@5@1@1@04@127282417797660001@0@@0@4@2123105954992588376@0@@24@1@19@2123105954992588377";
    s = "4@2123215871002256258@0@@267@4@0@0@@29@1@19@21232158709445293361@1@04@127283417797660101@0@@148@4@4001@0@@134@1@3@1064@0@0@@116@1@1@01@1@01@2@-13@6@System1@1@11@2@101@1@01@2@-13@10@zhb(Local)1@1@11@19@21232158709445293361@1@01@2@-13@4@zhb11@1@14@2123215871002246254@0@@24@1@19@2123215870944529356"

    ret = m.FromString(s);

    ID = m.GetMsgID();
    if (ret) {
        Log("Msg 测试正确", "OK");
    } else {
        Log("Msg 测试错误", "ERROR");
    }
}

*/

//UTF-16转UTF-8
function utf16ToUtf8(s) {
    if (Object.prototype.toString.call(s) != "[object String]") {
        return "";
    }

    var i, code, ret = [],
        len = s.length;
    for (i = 0; i < len; i++) {
        code = s.charCodeAt(i);
        if (code > 0x0 && code <= 0x7f) {
            //单字节
            //UTF-16 0000 - 007F
            //UTF-8  0xxxxxxx
            ret.push(s.charAt(i));
        } else if (code >= 0x80 && code <= 0x7ff) {
            //双字节
            //UTF-16 0080 - 07FF
            //UTF-8  110xxxxx 10xxxxxx
            ret.push(
                //110xxxxx
                String.fromCharCode(0xc0 | ((code >> 6) & 0x1f)),
                //10xxxxxx
                String.fromCharCode(0x80 | (code & 0x3f))
            );
        } else if (code >= 0x800 && code <= 0xffff) {
            //三字节
            //UTF-16 0800 - FFFF
            //UTF-8  1110xxxx 10xxxxxx 10xxxxxx
            ret.push(
                //1110xxxx
                String.fromCharCode(0xe0 | ((code >> 12) & 0xf)),
                //10xxxxxx
                String.fromCharCode(0x80 | ((code >> 6) & 0x3f)),
                //10xxxxxx
                String.fromCharCode(0x80 | (code & 0x3f))
            );
        }
    }

    return ret.join('');
}
//UTF-8转UTF-16
function utf8ToUtf16(s) {
    if (Object.prototype.toString.call(s) != "[object String]") {
        return "";
    }

    var i, codes, bytes, ret = [],
        len = s.length;
    for (i = 0; i < len; i++) {
        codes = [];
        codes.push(s.charCodeAt(i));
        if (((codes[0] >> 7) & 0xff) == 0x0) {
            //单字节  0xxxxxxx
            ret.push(s.charAt(i));
        } else if (((codes[0] >> 5) & 0xff) == 0x6) {
            //双字节  110xxxxx 10xxxxxx
            codes.push(s.charCodeAt(++i));
            bytes = [];
            bytes.push(codes[0] & 0x1f);
            bytes.push(codes[1] & 0x3f);
            ret.push(String.fromCharCode((bytes[0] << 6) | bytes[1]));
        } else if (((codes[0] >> 4) & 0xff) == 0xe) {
            //三字节  1110xxxx 10xxxxxx 10xxxxxx
            codes.push(s.charCodeAt(++i));
            codes.push(s.charCodeAt(++i));
            bytes = [];
            bytes.push((codes[0] << 4) | ((codes[1] >> 2) & 0xf));
            bytes.push(((codes[1] & 0x3) << 6) | (codes[2] & 0x3f));
            ret.push(String.fromCharCode((bytes[0] << 8) | bytes[1]));
        }
    }
    return ret.join('');
}


// ePipeline class
///////////////////////////////////////////////////////////
function ePipeline() {
    this.ID = 0;
    this.Label = "";
    this.Data = [];
}

//ePipeline methods

ePipeline.prototype = {
    ToString: function () { //TYPE_PIPELINE@ID@len@Label@LEN@ [type@len@data1..]

        //type
        var s = "";
        s += '4@';

        //ID
        s += this.ID;
        s += '@';

        //len
        var temp = '';
        temp = utf16ToUtf8(this.Label);
        s += temp.length;
        s += '@';

        //Label
        s += temp;
        s += '@';

        var datastring = "";
        var len = this.Data.length;
        for (var i = 0; i < len; i++) {
            var s1 = "";
            var type = this.Data[i].type;
            if (type == "int") {
                s1 = String(this.Data[i].value);
                datastring += "1@" + s1.length + "@" + s1;
            } else if (type == "float") {
                s1 = String(this.Data[i].value);
                datastring += "2@" + s1.length + "@" + s1;
            } else if (type == "string") {
                s1 = utf16ToUtf8(this.Data[i].value);
                datastring += "3@" + s1.length + "@" + s1;
            } else if (type == "pipe") {
                s1 = this.Data[i].value.ToString();
                datastring += s1;
            } else { //NULL
                datastring += "0@0@"
            }
        }
        //data len
        s += datastring.length;
        s += '@';
        //data
        s += datastring;
        return s;
    },
    FromString: function (s) {
        this.Data = [];

        var slen = s.length;
        var pos = 0;
        if (slen < 9 || s[pos] != '4' || s[pos + 1] != '@') return false;

        pos += 2;
        //get ID
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        this.ID = s.substring(pos, i);

        //get label
        pos = i + 1;
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        s1 = s.substring(pos, i);
        len = Number(s1); //label's len
        pos = i + 1;
        i = pos + len;
        this.Label = s.substring(pos, i);

        //get data length
        pos = i + 1;
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        s1 = s.substring(pos, i);
        len = Number(s1);
        pos = i + 1;

        while (pos < slen) {
            ch = s[pos];
            switch (ch) {
                case '0':
                    {
                        if (s.substr(pos, 4) != "0@0@") return false;
                        this.Data.push({
                            type: 'null',
                            value: '0'
                        });
                        pos += 4;
                    }
                    break;
                case '1':
                    {
                        pos += 2;
                        i = s.indexOf('@', pos);
                        if (i == -1) return false;
                        s1 = s.substring(pos, i);
                        len = Number(s1);

                        pos = i + 1;
                        i = pos + len;
                        s1 = s.substring(pos, i);
                        //t = Number(s1);

                        this.Data.push({
                            type: 'int',
                            value: s1
                        });
                        pos = i;
                    }
                    break;
                case '2':
                    {
                        pos += 2;
                        i = s.indexOf('@', pos);
                        if (i == -1) return false;
                        s1 = s.substring(pos, i);
                        len = Number(s1);

                        pos = i + 1;
                        i = pos + len;
                        s1 = s.substring(pos, i);
                        //t = Number(s1);
                        this.Data.push({
                            type: 'float',
                            value: s1
                        });

                        pos = i;
                    }
                    break;
                case '3':
                    {
                        pos += 2;
                        i = s.indexOf('@', pos);
                        if (i == -1) return false;
                        s1 = s.substring(pos, i);
                        len = Number(s1);

                        pos = i + 1;
                        i = pos + len;
                        s1 = s.substring(pos, i);

                        s2 = utf8ToUtf16(s1);
                        this.Data.push({
                            type: 'string',
                            value: s2
                        });
                        pos = i;
                    }
                    break;
                case '4':
                    {
                        start = pos;
                        pos += 2;
                        i = s.indexOf('@', pos); //ID
                        if (i == -1) return false;
                        pos = i + 1;
                        i = s.indexOf('@', pos); //Label len
                        if (i == -1) return false;
                        s1 = s.substring(pos, i);
                        len = Number(s1);

                        pos = i + 2 + len;
                        i = s.indexOf('@', pos); //Data len
                        if (i == -1) return false;

                        s1 = s.substring(pos, i);
                        len = Number(s1);

                        end = i + 1 + len; //end pos of this pipe
                        s1 = s.substring(start, end); //get this pipe's string
                        pos = end;

                        child = new ePipeline();
                        ret = child.FromString(s1);
                        if (!ret) return false;
                        this.Data.push({
                            type: 'pipe',
                            value: child
                        });

                    }
                    break;
                default:
                    return false;
            } //switch
        } //while
        return true;
    },
    PushNull: function () {
        this.Data.push({
            type: 'null',
            value: 0
        });
    },
    PushInt: function (i) {
        this.Data.push({
            type: 'int',
            value: i
        });
    },
    PushFloat: function (f) {
        this.Data.push({
            type: 'float',
            value: f
        });
    },
    PushString: function (s) {
        this.Data.push({
            type: 'string',
            value: s
        });
    },
    PushPipe: function (p) {
        this.Data.push({
            type: 'pipe',
            value: p
        });
    }
}


//GMIS_Msg class
////////////////////////////////////////
function GMIS_Msg() {
    this.ID = 0;
    this.Label = "";
    this.ReceiverInfo = new ePipeline();
    this.Letter = new ePipeline();
    this.SenderInfo = new ePipeline();

}

//GMIS_Msg methods
GMIS_Msg.prototype = {
    FromString: function (s) {

        this.ID = 0;
        this.Label = "";
        this.ReceiverInfo = new ePipeline();
        this.Letter = new ePipeline();
        this.SenderInfo = new ePipeline();

        var slen = s.length;
        var pos = 0;
        if (slen < 9 || s[pos] != '4' || s[pos + 1] != '@') return false;

        pos += 2;
        //get ID
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        this.ID = s.substring(pos, i);

        //get label
        pos = i + 1;
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        s1 = s.substring(pos, i);
        len = Number(s1); //label's len
        pos = i + 1;
        i = pos + len;
        this.Label = s.substring(pos, i);

        //get data length
        pos = i + 1;
        i = s.indexOf('@', pos);
        if (i == -1) return false;
        s1 = s.substring(pos, i);
        Datalen = Number(s1);
        pos = i + 1;

        if (pos >= slen) return false;

        ch = s[pos];

        if (ch != '4') return false;
        else {
            start = pos;
            pos += 2;
            i = s.indexOf('@', pos); //ID
            if (i == -1) return false;

            pos = i + 1;
            i = s.indexOf('@', pos); //Label len
            if (i == -1) return false;
            s1 = s.substring(pos, i);
            len = Number(s1);

            pos = i + 2 + len;
            i = s.indexOf('@', pos); //Data len
            if (i == -1) return false;

            s1 = s.substring(pos, i);
            len = Number(s1);

            end = i + 1 + len; //end pos of this pipe
            pos = end;
            s1 = s.substring(start, end); //get this pipe's string
            ret = this.ReceiverInfo.FromString(s1);
            if (!ret) return false;

        }

        if (pos >= slen) return false;
        ch = s[pos];

        if (ch != '4') return false;
        else {
            start = pos;
            pos += 2;
            i = s.indexOf('@', pos); //ID
            if (i == -1) return false;
            pos = i + 1;
            i = s.indexOf('@', pos); //Label len
            if (i == -1) return false;
            s1 = s.substring(pos, i);
            len = Number(s1);

            pos = i + 2 + len;
            i = s.indexOf('@', pos); //Data len
            if (i == -1) return false;

            s1 = s.substring(pos, i);
            len = Number(s1);

            end = i + 1 + len; //end pos of this pipe
            pos = end;

            s1 = s.substring(start, end); //get this pipe's string
            ret = this.Letter.FromString(s1);
            if (!ret) return false;

        }
        if (pos >= slen) return false;
        ch = s[pos];

        if (ch != '4') return false;
        else {
            start = pos;
            pos += 2;
            i = s.indexOf('@', pos); //ID
            if (i == -1) return false;
            pos = i + 1;
            i = s.indexOf('@', pos); //Label len
            if (i == -1) return false;
            s1 = s.substring(pos, i);
            len = Number(s1);

            pos = i + 2 + len;
            i = s.indexOf('@', pos); //Data len
            if (i == -1) return false;

            s1 = s.substring(pos, i);
            len = Number(s1);

            end = i + 1 + len; //end pos of this pipe
            pos = end;
            s1 = s.substring(start, end); //get this pipe's string
            ret = this.SenderInfo.FromString(s1);
            if (!ret) return false;

        }
        return true;
    },
    ToString: function () {
        sReceiver = this.ReceiverInfo.ToString();
        sLetter = this.Letter.ToString();
        sSender = this.SenderInfo.ToString();

        var s = "";

        //type
        s += "4@";

        //ID
        s += this.ID;
        s += '@';

        //Label len
        var temp = '';
        temp = utf16ToUtf8(this.Label);
        s += temp.length;
        s += '@';

        //Label
        s += temp;
        s += '@';

        datalen = sReceiver.length + sLetter.length + sSender.length;
        s += datalen;
        s += '@';
        s += sReceiver + sLetter + sSender;

        return s;
    },
    GetSourceID: function () {
        return this.ID;
    },
    GetEventID: function () {
        return this.ReceiverInfo.ID;
    },
    GetMsgID: function () {
        return this.Letter.ID;
    },
    GetSenderID: function () {
        return this.SenderInfo.Data[0].value;
    },
    WriteEnvelope: function (ReceiverID, MsgID, SenderID, EventID) {
        this.ReceiverInfo.PushInt(ReceiverID);
        this.ReceiverInfo.ID = EventID;
        this.Letter.ID = MsgID;
        this.SenderInfo.PushInt(SenderID);
    }
}