
if(typeof deconcept=="undefined"){var deconcept=new Object();}
if(typeof deconcept.util=="undefined"){deconcept.util=new Object();}
if(typeof deconcept.SWFObjectUtil=="undefined"){deconcept.SWFObjectUtil=new Object();}
deconcept.SWFObject=function(_1,id,w,h,_5,c,_7,_8,_9,_a,_b){if(!document.getElementById){return;}
this.DETECT_KEY=_b?_b:"detectflash";this.skipDetect=deconcept.util.getRequestParameter(this.DETECT_KEY);this.params=new Object();this.variables=new Object();this.attributes=[];if(_1){this.setAttribute("swf",_1);}
if(id){this.setAttribute("id",id);}
if(w){this.setAttribute("width",w);}
if(h){this.setAttribute("height",h);}
if(_5){this.setAttribute("version",new deconcept.PlayerVersion(_5.toString().split(".")));}
this.installedVer=deconcept.SWFObjectUtil.getPlayerVersion();if(c){this.addParam("bgcolor",c);}
var q=_8?_8:"high";this.addParam("quality",q);this.setAttribute("useExpressInstall",_7);this.setAttribute("doExpressInstall",false);var _d=(_9)?_9:window.location;this.setAttribute("xiRedirectUrl",_d);this.setAttribute("redirectUrl","");if(_a){this.setAttribute("redirectUrl",_a);}};deconcept.SWFObject.prototype={setAttribute:function(_e,_f){this.attributes[_e]=_f;},getAttribute:function(_10){return this.attributes[_10];},addParam:function(_11,_12){this.params[_11]=_12;},getParams:function(){return this.params;},addVariable:function(_13,_14){this.variables[_13]=_14;},getVariable:function(_15){return this.variables[_15];},getVariables:function(){return this.variables;},getVariablePairs:function(){var _16=[];var key;var _18=this.getVariables();for(key in _18){_16.push(key+"="+_18[key]);}
return _16;},getSWFHTML:function(){var _19="";if(navigator.plugins&&navigator.mimeTypes&&navigator.mimeTypes.length){if(this.getAttribute("doExpressInstall")){this.addVariable("MMplayerType","PlugIn");}
_19="<embed type=\"application/x-shockwave-flash\" src=\""+this.getAttribute("swf")+"\" width=\""+this.getAttribute("width")+"\" height=\""+this.getAttribute("height")+"\"";_19+=" id=\""+this.getAttribute("id")+"\" name=\""+this.getAttribute("id")+"\" ";var _1a=this.getParams();for(var key in _1a){_19+=[key]+"=\""+_1a[key]+"\" ";}
var _1c=this.getVariablePairs().join("&");if(_1c.length>0){_19+="flashvars=\""+_1c+"\"";}_19+="/>";}else{if(this.getAttribute("doExpressInstall")){this.addVariable("MMplayerType","ActiveX");}
_19="<object id=\""+this.getAttribute("id")+"\" classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" width=\""+this.getAttribute("width")+"\" height=\""+this.getAttribute("height")+"\">";_19+="<param name=\"movie\" value=\""+this.getAttribute("swf")+"\" />";var _1d=this.getParams();for(var key in _1d){_19+="<param name=\""+key+"\" value=\""+_1d[key]+"\" />";}
var _1f=this.getVariablePairs().join("&");if(_1f.length>0){_19+="<param name=\"flashvars\" value=\""+_1f+"\" />";}_19+="</object>";}
return _19;},write:function(_20){if(this.getAttribute("useExpressInstall")){var _21=new deconcept.PlayerVersion([6,0,65]);if(this.installedVer.versionIsValid(_21)&&!this.installedVer.versionIsValid(this.getAttribute("version"))){this.setAttribute("doExpressInstall",true);this.addVariable("MMredirectURL",escape(this.getAttribute("xiRedirectUrl")));document.title=document.title.slice(0,47)+" - Flash Player Installation";this.addVariable("MMdoctitle",document.title);}}
if(this.skipDetect||this.getAttribute("doExpressInstall")||this.installedVer.versionIsValid(this.getAttribute("version"))){var n=(typeof _20=="string")?document.getElementById(_20):_20;n.innerHTML=this.getSWFHTML();return true;}else{if(this.getAttribute("redirectUrl")!=""){document.location.replace(this.getAttribute("redirectUrl"));}}
return false;}};deconcept.SWFObjectUtil.getPlayerVersion=function(){var _23=new deconcept.PlayerVersion([0,0,0]);if(navigator.plugins&&navigator.mimeTypes.length){var x=navigator.plugins["Shockwave Flash"];if(x&&x.description){_23=new deconcept.PlayerVersion(x.description.replace(/([a-zA-Z]|\s)+/,"").replace(/(\s+r|\s+b[0-9]+)/,".").split("."));}}else{try{var axo=new ActiveXObject("ShockwaveFlash.ShockwaveFlash.7");}
catch(e){try{var axo=new ActiveXObject("ShockwaveFlash.ShockwaveFlash.6");_23=new deconcept.PlayerVersion([6,0,21]);axo.AllowScriptAccess="always";}
catch(e){if(_23.major==6){return _23;}}try{axo=new ActiveXObject("ShockwaveFlash.ShockwaveFlash");}
catch(e){}}if(axo!=null){_23=new deconcept.PlayerVersion(axo.GetVariable("$version").split(" ")[1].split(","));}}
return _23;};deconcept.PlayerVersion=function(_27){this.major=_27[0]!=null?parseInt(_27[0]):0;this.minor=_27[1]!=null?parseInt(_27[1]):0;this.rev=_27[2]!=null?parseInt(_27[2]):0;};deconcept.PlayerVersion.prototype.versionIsValid=function(fv){if(this.major<fv.major){return true;}
if(this.major>fv.major){return true;}
if(this.minor<fv.minor){return true;}
if(this.minor>fv.minor){return true;}
if(this.rev<fv.rev){return false;}return true;};deconcept.util={getRequestParameter:function(_29){var q=document.location.search||document.location.hash;if(q){var _2b=q.substring(1).split("&");for(var i=0;i<_2b.length;i++){if(_2b[i].substring(0,_2b[i].indexOf("="))==_29){return _2b[i].substring((_2b[i].indexOf("=")+1));}}}
return"";}};deconcept.SWFObjectUtil.cleanupSWFs=function(){if(window.opera||!document.all){return;}
var _2d=document.getElementsByTagName("OBJECT");for(var i=0;i<_2d.length;i++){_2d[i].style.display="none";for(var x in _2d[i]){if(typeof _2d[i][x]=="function"){_2d[i][x]=function(){};}}}};deconcept.SWFObjectUtil.prepUnload=function(){__flash_unloadHandler=function(){};__flash_savedUnloadHandler=function(){};if(typeof window.onunload=="function"){var _30=window.onunload;window.onunload=function(){deconcept.SWFObjectUtil.cleanupSWFs();_30();};}else{window.onunload=deconcept.SWFObjectUtil.cleanupSWFs;}};if(typeof window.onbeforeunload=="function"){var oldBeforeUnload=window.onbeforeunload;window.onbeforeunload=function(){deconcept.SWFObjectUtil.prepUnload();oldBeforeUnload();};}else{window.onbeforeunload=deconcept.SWFObjectUtil.prepUnload;}
if(Array.prototype.push==null){Array.prototype.push=function(_31){this[this.length]=_31;return this.length;};}
var getQueryParamValue=deconcept.util.getRequestParameter;var FlashObject=deconcept.SWFObject;var SWFObject=deconcept.SWFObject;var flashAnimArray=[];function FlashWrapper(id){this.id=id;this.fl=null;this.__interval=null;this.queues=[];this.onHoldCallback=null;this.agentInConf=new Object();this.currentStream=null;this.flvHold="";this.flvEnd="";this.service="";var me=this;flashAnimArray.push(me);}
FlashWrapper.prototype.Connect=function(sessionId,service){this.service=service;this.fl=document.getElementById(this.id);try{this.fl.Connect(sessionId,service,this.id);}
catch(e){var me=this;this.fl=null;setTimeout(function(){me.Connect(sessionId,service);},1000);}}
FlashWrapper.prototype.setVolumes=function(mic,hp){if(this.fl!=null){this.fl.setVolumes(mic,hp);}}
FlashWrapper.prototype.doMaximize=function(){if(this.fl!=null){this.fl.doMaximize();}}
FlashWrapper.prototype.startWebcam=function(){if(this.fl!=null){this.fl.startWebcam();}}
FlashWrapper.prototype.stopWebcam=function(){if(this.fl!=null){this.fl.stopWebcam();}}
FlashWrapper.prototype.setMicroMuted=function(muted){if(this.fl!=null){this.fl.setMicroMuted(muted);}}
FlashWrapper.prototype.threadCallback=function(){if(this.fl==null)return;if(this.queues.length!=0){var buffer=this.queues;var fct=buffer.shift();try{fct();this.queues=buffer;}
catch(e){}}
else{clearInterval(this.__interval);}}
FlashWrapper.prototype.flashFctCall=function(fct){this.queues.push(fct);if(this.__interval!=null)clearInterval(this.__interval);var me=this;setInterval(function(){me.threadCallback();},500);}
FlashWrapper.prototype.JoinRoom=function(roomId){if(this.service=="")return;if(this.fl!=null){this.fl.joinRoom(roomId,2);}
else{var me=this;this.flashFctCall(function(){me.JoinRoom(roomId,2);});}}
FlashWrapper.prototype.LeaveRoom=function(roomId){if(this.service=="")return;if(this.fl!=null){this.fl.leaveRoom(roomId);}
else{var me=this;this.flashFctCall(function(){me.LeaveRoom(roomId);});}}
FlashWrapper.prototype.Disconnect=function(){if(this.service=="")return;this.currentStream=null;if(this.fl!=null){this.fl.disconnect();RemoveFlashFromAnimArray(this.id);}
else{var me=this;this.flashFctCall(function(){me.Disconnect();});}}
FlashWrapper.prototype.DisplayStream=function(stream,actionAtEnd){if(this.service=="")return;if(this.fl!=null){this.fl.displayStream(stream,actionAtEnd);this.currentStream=stream;}
else{var me=this;this.flashFctCall(function(){me.DisplayStream(stream,actionAtEnd);});}}
FlashWrapper.prototype.OnUserJoinRoom=function(room,user){var type=2;for(var i=0;i<user.rooms.length;i++){if(user.rooms[i].id==room){type=user.rooms[i].userType;}}
var debug=document.getElementById("DebugFlashOctopus");if(type==0){this.agentInConf=new Object();this.agentInConf.name=user.name;this.agentInConf.isStreaming=(user.activeRoom==room)?user.isStreaming:false;if(debug)debug.innerHTML+="OnUserJoinRoom. IsStreaming : "+this.agentInConf.isStreaming+". Name : "+this.agentInConf.name+"</br>";if(this.agentInConf.isStreaming){this.DisplayStream(this.agentInConf.name,0);}}
else{if(debug)debug.innerHTML+="Type of user mismatch.: "+type+"</br>";}}
FlashWrapper.prototype.OnUserTypeChange=function(room,user){this.OnUserJoinRoom(room,user);}
FlashWrapper.prototype.OnUserLeaveRoom=function(room,user){if((this.agentInConf.name==user.name)&&(this.CanTakeStream())&&(this.currentStream!=this.flvEnd)){this.DisplayStream(this.flvEnd,2);}
if(user.name==this.agentInConf.name){this.agentInConf.name="NOBODY";this.agentInConf.isStreaming=false;this.DisplayStream(null,0);}}
FlashWrapper.prototype.OnUserStartStreaming=function(room,user){if(this.agentInConf.name==user.name){this.DisplayStream(user.name,0);this.agentInConf.isStreaming=true;}}
FlashWrapper.prototype.OnUserStopStreaming=function(room,user){if((this.CanTakeStream())||(this.currentStream==user.name))
{if(this.currentStream!=this.flvEnd){this.DisplayStream(null,0);}}
if(this.agentInConf.name==user.name){this.agentInConf.isStreaming=false;}}
FlashWrapper.prototype.OnUserHold=function(room,user){if(user.name==this.agentInConf.name){var fct=this.onHoldCallback;if((fct!=null)&&(typeof(fct)=="function")){this.onHoldCallback();}
if((this.CanTakeStream())||(this.currentStream==user.name)){if(this.flvHold!=""){this.DisplayStream(this.flvHold,1);}
else{this.DisplayStream(null,0);}}
this.agentInConf.isStreaming=false;}
else{alert("hold => pas same user "+user.name+"/"+this.agentInConf.name);}}
FlashWrapper.prototype.OnUserHoldEnd=function(room,user){if((this.CanTakeStream())&&(user.isStreaming)&&(user.name==this.agentInConf.name))
this.DisplayStream(user.name,0);this.agentInConf.isStreaming=user.isStreaming;}
FlashWrapper.prototype.DisplayVideo=function(video,actionAtEnd){if(chatService!=null){chatService.OnVideoDisplay(SESSION_ID,function(){},false);}
var me=this;setTimeout(function(){me.DisplayStream(video,actionAtEnd);},2000);}
FlashWrapper.prototype.CanTakeStream=function(){if((this.currentStream==null)||(this.currentStream==this.flvHold))return true;return false;}
FlashWrapper.prototype.OnVideoStopped=function(){if(this.agentInConf.isStreaming){this.DisplayStream(this.agentInConf.name,0);return;}
if(this.currentStream!=this.flvEnd){this.DisplayStream(null,0);}}
FlashWrapper.prototype.ConnectAndJoin=function(sessionId,service,roomId){this.service=service;this.fl=document.getElementById(this.id);this.fl.ConnectAndJoin(sessionId,service,this.id,roomId);this.connected=true;}
function OnUserJoinRoom(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserJoinRoom;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserJoinRoom(room,user);break;}}}
function OnUserLeaveRoom(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserLeaveRoom;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserLeaveRoom(room,user);break;}}}
function OnUserStartStreaming(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserStartStreaming;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserStartStreaming(room,user);break;}}}
function OnUserStopStreaming(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserStopStreaming;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserStopStreaming(room,user);break;}}}
function OnUserHold(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserHold;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserHold(room,user);break;}}}
function OnUserHoldEnd(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserHoldEnd;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserHoldEnd(room,user);break;}}}
function DisplayVideo(wpId,video,actionAtEnd){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].DisplayVideo;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].DisplayVideo(video,actionAtEnd);break;}}}
function OnVideoStopped(wpId,mySelf){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnVideoStopped;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnVideoStopped(mySelf);break;}}}
function OnUserTypeChange(wpId,room,user){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==wpId){var fct=flashAnimArray[i].OnUserTypeChange;if((fct!=null)&&(typeof(fct)=="function"))
flashAnimArray[i].OnUserTypeChange(room,user);break;}}}
function OnConnectError(code,description){}
function OnConnectSuccess(){if(typeof(MyG2C.DOM.WebCamConnected)!='undefined')MyG2C.DOM.WebCamConnected=true;}
function RemoveFlashFromAnimArray(id){for(var i=0;i<flashAnimArray.length;i++){if(flashAnimArray[i].id==id){flashAnimArray.splice(i,1);break;}}}
if(!this.JSON_G2C){JSON_G2C=function(){function f(n){return n<10?'0'+n:n;}
Date.prototype.toJSON_G2C=function(){return this.getUTCFullYear()+'-'+
f(this.getUTCMonth()+1)+'-'+
f(this.getUTCDate())+'T'+
f(this.getUTCHours())+':'+
f(this.getUTCMinutes())+':'+
f(this.getUTCSeconds())+'Z';};var m={'\b':'\\b','\t':'\\t','\n':'\\n','\f':'\\f','\r':'\\r','"':'\\"','\\':'\\\\'};function stringify_G2C(value,whitelist){var a,i,k,l,r=/["\\\x00-\x1f\x7f-\x9f]/g,v;switch(typeof value){case'string':return r.test(value)?'"'+value.replace(r,function(a){var c=m[a];if(c){return c;}
c=a.charCodeAt();return'\\u00'+Math.floor(c/16).toString(16)+
(c%16).toString(16);})+'"':'"'+value+'"';case'number':return isFinite(value)?String(value):'null';case'boolean':case'null':return String(value);case'object':if(!value){return'null';}
if(typeof value.toJSON_G2C==='function'){return stringify_G2C(value.toJSON_G2C());}
a=[];if(typeof value.length==='number'&&!(value.propertyIsEnumerable('length'))){l=value.length;for(i=0;i<l;i+=1){a.push(stringify_G2C(value[i],whitelist)||'null');}
return'['+a.join(',')+']';}
if(whitelist){l=whitelist.length;for(i=0;i<l;i+=1){k=whitelist[i];if(typeof k==='string'){v=stringify_G2C(value[k],whitelist);if(v){a.push(stringify_G2C(k)+':'+v);}}}}else{for(k in value){if(typeof k==='string'){v=stringify_G2C(value[k],whitelist);if(v){a.push(stringify_G2C(k)+':'+v);}}}}
return'{'+a.join(',')+'}';}}
return{stringify_G2C:stringify_G2C,parse:function(text,filter){var j;function walk(k,v){var i,n;if(v&&typeof v==='object'){for(i in v){if(Object.prototype.hasOwnProperty.apply(v,[i])){n=walk(i,v[i]);if(n!==undefined){v[i]=n;}}}}
return filter(k,v);}
if(/^[\],:{}\s]*$/.test(text.replace(/\\./g,'@').replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,']').replace(/(?:^|:|,)(?:\s*\[)+/g,''))){j=eval('('+text+')');return typeof filter==='function'?walk('',j):j;}
throw new SyntaxError('parseJSON');}};}();}