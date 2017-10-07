function G2C() {

    var self = this;
    this.DegugOn = false;
    this.ChatAvailable = true;

    // NE PAS SUPPRIMER LA LIGNE CI-DESSOUS 
    
this.culture=navigator.language?navigator.language:navigator.userLanguage;this.Oid_WebSite="Niib_AEK";this.Oid_Campaign="Niib_BUK";this.Oid_MailCampaign="";this.CC_SendMail="";this.Id_Company="8";this.OnMediaService="10.0.5.30:3001";this.SharingService="10.0.5.30:3002";this.OnMediaServer=window.location.protocol+"//hermesmc.fr/hermes_net_v4/PlateformPublication/OnMedia";this.UrlServer=window.location.protocol+"//hermesmc.fr/hermes_net_v4/PlateformPublication/";this.UrlServerList=["//hermesmc.fr/hermes_net_v4/PlateformPublication/"];for(var urlIndex=0;urlIndex<this.UrlServerList.length;urlIndex++)
this.UrlServerList[urlIndex]=window.location.protocol+this.UrlServerList[urlIndex];this.extension='ashx';this.RegExpPhone=["^0[0-9]{9}$"];for(var i=0;i<this.RegExpPhone.length;i++)
this.RegExpPhone[i]=new RegExp(this.RegExpPhone[i]);this.Skin="G2C_V2_THE_WAVE";this.SkinPath="WebSitesLiveChat/Commun/G2C_V2_THE_WAVE";this.Btn_Model="64070746252495B4_Deltadore";this.Video_Bitrate=26000;this.Video_Framerate=8;this.Video_Quality=0;this.Video_Width=320;this.Video_Height=240;this.Picture_Agent_On="IMG_AG.png";this.Picture_Agent_Off="IMG_AG1.png";this.Picture_Panel='PANEL.png';this.ChatAvailable=true;this.VideoConf=false;this.StartAutoVideoConf=false;this.WebSiteForm=null;this.CustomForm=null;this.ShowForm=false;this.ShowWebCallBack=false;this.ShowPanelInterface=true;this.ShowPanelIfNoAgent=true;this.ShowPanelOnlyIfProactiveChat=true;this.ShowPanelReduced=false;this.DisplayDefaultPanelAtStartup=false;this.TabVideoDefaultURL="";this.TabVideoDefaultTitle="";this.TabVideoSatisfaction=false;this.HN_ORDER_CODE="18042012";this.HN_COMPANY_NAME="MAILLEUR_CONTACT";this.DomainNameCookie="";this.FACEBOOK_APP_URL="";this.CobrowsingAvailable=false;this.COLOR="#ffffff";this.Time_To_Close_Predictive=10000;this.ShowDisabledButton=false;this.TabOpen=false;this.MinHourOpening=10;this.MaxHourClosing=20;this.PopupOnProActifChat=false;this.LEFT=false;this.RIGHT=false;this.TopBar=0;this.WidthBar="800px";this.hnet_ctc_serviceUrl="";this.hnet_ctc_tocall="";
// NE PAS SUPPRIMER LA LIGNE CI-DESSUS.


this.ChatMessage                    = '';
this.TimerToHiddeProactivPopup      = '';
this.SatisText1                     = '';
this.SatisText2                     = '';
this.SatisText3                     = '';
this.ShowSatisPanelOnEachChat       = false;
this.ShowInfoBulle                  = true;
this.TextNoAgent                    = '';
this.DivIdToPlaceBtnChat            = '';
this.ShowBtnChatOnlyOnProactif      = false;  



if (typeof(this.RegExpPhone)=='undefined')  this.RegExpPhone = new Array(/^(01|02|03|04|05|06|08|09)[ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}$/,/^(\+[0-9]{2})[ \.\-]?[0-9][ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}[ \.\-]?[0-9]{2}$/);

this.hnet_ctc_tocall=this.hnet_ctc_tocall.replace("1453@10.0.2.220", "1453@10.0.6.220");

this.ForcedStyle = false;

if (typeof (this.LEFT) == 'undefined') this.LEFT = false;
if (typeof (this.RIGHT) == 'undefined') this.RIGHT = false;
// MODIF 6/08 pour avoir les options Start minimized et TabOpen meme sur les LEFTS RIGHTS
if (this.LEFT || this.RIGHT) {
    this.TabOpen = false; //this.ShowPanelReduced = false; 
}

if (typeof (this.ChatAvailableOnQueueOverflow) == 'undefined') this.ChatAvailableOnQueueOverflow = true;

// USED TO EXECUTE SPECIAL CODE
this.SpecialPage = false;
this.ShowBarOnSpecialPage = false;
this.SpecialCode  = '';

// INIT 
this.ObjBtnCallChat = null;
this.ShowObjBtnCallChatOnlyOnPredict = false;

if (typeof (this.hnet_ctc_serviceUrl) == 'undefined') this.hnet_ctc_serviceUrl = "";
if (typeof (this.hnet_ctc_tocall) == 'undefined') this.hnet_ctc_tocall = "";

// SI TABLETTE GRAPHIQUE, PAS DE VIDEO
if (typeof (ontouchstart) != 'undefined') this.VideoConf = false;

// LE PARAMETRE N'EST PAS LA (112 = space before Agent, 95 space before close)
if (typeof (this.WidthBar) == 'undefined') this.WidthBar = '100%';

if (this.WidthBar.indexOf('%') < 0) this.WidthBar = parseInt(this.WidthBar) + 172 + 'px';
if (this.WidthBar.indexOf('%') < 0 && !this.CobrowsingAvailable) this.WidthBar = parseInt(this.WidthBar) - 25 + 'px';
if (this.WidthBar.indexOf('%') < 0 && !this.VideoConf) this.WidthBar = parseInt(this.WidthBar) - 25 + 'px'; 

if (typeof (this.PLACE) == 'undefined') this.PLACE = 1;

// Info SEND to API
this.APIInfo = null;

// CHECK SOME VALUES
// if (this.ShowPanelOnlyIfProactiveChat) this.ShowPanelInterface = true;
// if (!this.ShowPanelIfNoAgent) this.ShowPanelInterface = true;

// FERMER LA FENETRE DE CHAT APRES X mSECONDEs	
this.timeOutCloseChat = 3000;

// True if surfer already fill satisfaction pannel
this.SatisfactionDone = false;

// Number of user beiing like with FB
this.CountFbUsers = 0;

// Id et Nom de l'agent qui Chatte
this.IdAgent = -1;
this.NameAgent = "";

// STATISTIC ETAT DE L'AGENT
this.Last_Click = null;
this.Last_Keypress = null;
this.Last_Scroll = null;
this.Time_On_Page = new Date().getTime()

// Etat de l'application FB : -1 Pas de campagne fb, 0 appli pas acceptÃ©e, 1 appli acceptÃ©e.
this.FBState = 0;

// SI AUCUNE PUB SELECTIONNEE DANS L'ADMIN ON PREND CELLE PAR DEFAUT
if (this.TabVideoDefaultURL == '') {
    this.TabVideoDefaultURL = this.UrlServer + "/WebSitesLiveChat/Commun/TheWave.html";
    this.TabVideoDefaultTitle = 'The Wave by Vocalcom';
}

// IMAGE DE L'AGENT LORSQUE LA BARRE EST FERMEE
this.Picture_Agent_On_Closed = this.Picture_Agent_On;
this.Picture_Agent_Off_Closed = this.Picture_Agent_Off;

// INIT PAGE HISTORIQUE
this.IdPageHisto = null;

//
// ******************************* CHAT RUNNING PARAMETERS 
//
// UN CHAT EST EN COURS
this.ChatRunning = false;

// LE VISIO CHAT EST EN COURS
this.WebCamRunning = false;

this.LastChatMessageReceived = null; //Etienne pour dedoublonner les messages recus en double
this.PredictiveStarted = false;
this.PredictiveSentenceDisplayed = false;
this.PredictiveCanceledMessage = '';

this.Timer_Predictive = null;

this.Log_String = '';

this.ChatInfo = null;
this.LastFromMessage = null;
this.WebCamLoadedFromAPI = false;
this.IgnoreData = true;
this.MessageSended = '';

this.Chat_State = false;
this.Video_State = false;
this.SessionID = null;
this.IdSurferChatting = null;
this.ShowSatisfaction = true;

// DETECTION DE L'OS,DU BROWSER ET DU DOCTYPE
this.Browser = null;
this.BrowserVersion = null;
this.OS = null;
var BrowserDetect = { init: function () { self.Browser = this.searchString(this.dataBrowser) || "An unknown browser"; self.BrowserVersion = this.searchVersion(navigator.userAgent) || this.searchVersion(navigator.appVersion) || "an unknown version"; self.OS = this.searchString(this.dataOS) || "an unknown OS"; }, searchString: function (data) { for (var i = 0; i < data.length; i++) { var dataString = data[i].string; var dataProp = data[i].prop; this.versionSearchString = data[i].versionSearch || data[i].identity; if (dataString) { if (dataString.indexOf(data[i].subString) != -1) return data[i].identity; } else if (dataProp) return data[i].identity; } }, searchVersion: function (dataString) { var index = dataString.indexOf(this.versionSearchString); if (index == -1) return; return parseFloat(dataString.substring(index + this.versionSearchString.length + 1)); }, dataBrowser: [{ string: navigator.userAgent, subString: "Chrome", identity: "Chrome" }, { string: navigator.userAgent, subString: "OmniWeb", versionSearch: "OmniWeb/", identity: "OmniWeb" }, { string: navigator.vendor, subString: "Apple", identity: "Safari", versionSearch: "Version" }, { prop: window.opera, identity: "Opera" }, { string: navigator.vendor, subString: "iCab", identity: "iCab" }, { string: navigator.vendor, subString: "KDE", identity: "Konqueror" }, { string: navigator.userAgent, subString: "Firefox", identity: "Firefox" }, { string: navigator.vendor, subString: "Camino", identity: "Camino" }, { string: navigator.userAgent, subString: "Netscape", identity: "Netscape" }, { string: navigator.userAgent, subString: "MSIE", identity: "Explorer", versionSearch: "MSIE" }, { string: navigator.userAgent, subString: "Gecko", identity: "Mozilla", versionSearch: "rv" }, { string: navigator.userAgent, subString: "Mozilla", identity: "Netscape", versionSearch: "Mozilla"}], dataOS: [{ string: navigator.appVersion, subString: "Windows NT 5.0", identity: "Windows XP" }, { string: navigator.appVersion, subString: "Windows NT 6.0", identity: "Windows Vista" }, { string: navigator.appVersion, subString: "Windows NT 7.0", identity: "Windows Seven" }, { string: navigator.platform, subString: "Win", identity: "Windows" }, { string: navigator.platform, subString: "Mac", identity: "Mac" }, { string: navigator.platform, subString: "Linux", identity: "Linux"}] };
BrowserDetect.init();
if (this.Browser == 'Explorer' && this.BrowserVersion == 7) return;

if (this.Browser == 'Mozilla' && navigator.appVersion.indexOf("rv:11.") >= 0) this.Browser = 'Explorer';
 
this.DocType = this.detectDoctype();

// DEFINITION DU TYPE D'AFFICHAGE EN FONCTION DU DOC TYPE	
this.Absolute = 'fixed';
this.Display = 'block';
if (!this.DocType) { this.Absolute = 'absolute'; this.Display = 'inline'; }
else if (this.DocType.importance == 'Transitional' && this.DocType.loose == 0 && this.DocType.xhtml == 'HTML')
{ this.Absolute = 'absolute'; this.Display = 'inline'; }

this.FirstTimeSurfer = 1;
this.SurferPosition = 1;

this.ArrayCDE = new Array();
this.JSON_Loaded = false;
this.CDE = function (ID, ACT, PAR, FCT, PRIO, AS)
{ this.ID_JS = ID; this.ACTION = ACT; this.PARAMS = PAR; this.FCT_RETURN = FCT; this.PRIORITY = PRIO; this.ASYNC = AS; }

this.Error = '';

// SURFER CONTEXT
this.ReturnParams = '';
this.ChatSurferOpen = false;

this.MouseX = 0;
this.MouseY = 0;
this.IdMouseClick = null;
this.ElMouseClick = '';
this.IdTyping = null;
this.ElTyping = '';
this.ArrayData = new Array();

// SURFER OR AGENT
this.IsAgent = false;
this.TimeOnPage = null;
this.surferStatus = null;
this.chatRequestCode = null;
this.supervisorChatRequestCode = null;
this.IsAgentWorking = null;
this.IsCampaignOpen = null;
this.IdSurfer = null;

this.ChatRequested = false;

// CO BROWSING PARAMETERS
this.CoBrowsingRunning = false;
this.TimerCoBrowsingRunning = null;
this.SpeedCoBrowsingSurfer = 1000;
this.SpeedCoBrowsingAgent = 2000;
this.Pointer_Mouse = null;
this.Click_Mouse = null;
this.Typing_Mouse = null;
this.OldMousePosition = new Object();
this.OldMousePosition.X = 1;
this.OldMousePosition.Y = 1;

this.TimerSurferPresent = null;

this.InfoUser = new Object();

// ADD OR USED FOR THE API
this.WaitAgentTime = null;
this.messageReceived = new Array();


this.TimerReceiveMessage = null;
this.Timer_Wait_Agent_Ready = null;

this.TextEnding = '';
var FirstMessage = null;

var started_OK = false;     // Used to check if 

this.DOM_Loaded = false;
this.DOM = null;
this.TimerKeypress = null;
this.TimerMouseDown = null;
this.TimerMouseMove = null;


this.FrameCDE = null;
this.FrameCDELoaded = false;
this.TextAreaCDE = null;
this.ActionCDE = null;
this.ID_JS_CDE = null;
this.ID_PAGE_HISTO = null;
this.FormCDE = null;

this.LeftForm = '-10px'; // NOT INITIALIZED



function ____(CALL_ON_NAVIG_PAGE) { }

// START SURVEILLANCE
this.OnNavigPage = function () {

    if (MyG2C.DOM)
        MyG2C.SendCDE(new Date().getTime(), 'START', null, function (Params, JSId) { MyG2C.CallBack_OnNavigPage(Params, JSId); }, 1, true);
    else {
        var S = this; setTimeout(function () { S.OnNavigPage(); }, 1000) 
    };
}
this.CallBack_OnNavigPage = function (P, I, ERROR) {

    var Here = this;
    DelLastCDE(I);

    if (P.errorcode == 0) {

        this.SessionID = P.SessionID;

        this.SatisfactionDone = P.SatisfactionDone

        this.CountFbUsers = P.CountFbUsers;
        this.FBState = P.FBState;

        if (typeof (this.DOM.BTN_FB) != 'undefined') {
            if (this.FBState == -1) this.DOM.BTN_FB.style.display = 'none';
            else if (this.FBState == 1) this.DOM.BTN_FB.style.display = 'none';
            else this.DOM.BTN_FB.style.display = '';
        }


        try { this.IdSurferChatting = P.IdSurferChatting; } catch (e) { this.IdSurferChatting = null; }
        try { this.IgnoreData = P.IgnoreData; } catch (e) { this.IgnoreData = true; }

        this.APIInfo = P;

        // GET ID PAGE HISTO
        this.IdPageHisto = P.IdPageHisto;

        eventHandler('focus', G2C_onfocus, window);
        eventHandler('unload', G2C_Onunload, document.body);

        this.WebConfRunning = P.WebConfRunning;

        // CHECK IF SPECIAL PAGE 
        if (typeof (P.SpecialPage) != 'undefined') { this.SpecialPage = P.SpecialPage; this.ShowBarOnSpecialPage = P.ShowBarOnSpecialPage; this.SpecialCode = P.SpecialCode; }
        if (this.SpecialPage && this.SpecialCode != '') { try { eval(P.SpecialCode); } catch (e) { } }


        // POST ALL AT BEGINNING
        // if (!this.IgnoreData) {// GET AND SEND ALL FIELDS IN PAGE
            CreateArrayObject(!this.IgnoreData);
        // }

        if (P.IsAgent || this.getCookie('AGENT_G2C_ROOM_OID') != null) {

            this.DOM.hiddeInterface();

            if (P.IsAgent) {
                this.InfoUser.Agent = true;
                this.InfoUser.RoomOid = P.RoomOid;
                this.InfoUser.IdSurferChatting = P.IdSurferChatting;
            }
            else {
                this.InfoUser.Agent = true;
                this.InfoUser.RoomOid = this.getCookie('AGENT_G2C_ROOM_OID');
                this.InfoUser.IdSurferChatting = this.getCookie('AGENT_G2C_IDSURFER');
            }
            var Par = 'AGENT ? : ' + this.InfoUser.Agent + ' ROOM OID : ' + this.InfoUser.RoomOid + 'ID SURFER CHATTING : ' + this.InfoUser.IdSurferChatting;
            window.status = Par;
            this.DebugInfo(Par);

            if (P.AgentLaunchURL) return;

            if (this.InfoUser.RoomOid != '' && this.InfoUser.IdSurferChatting != '') {
                MyG2C.StartCoBrowsingAgent();
                this.setCookie('AGENT_G2C_ROOM_OID', this.InfoUser.RoomOid);
                this.setCookie('AGENT_G2C_IDSURFER', this.InfoUser.IdSurferChatting);
            }
            return;
        }

        this.InfoUser.Agent = false;
        this.InfoUser.RoomOid = null;
        this.InfoUser.IdSurferChatting = null;
        this.Chat_State = P.State_Chat;

        this.Video_State = P.State_Video;
        this.IsAgentWorking = P.IsAgentWorking;

        /*
        ETAT DE LA BARRE FCT DES PARAM ADMIN et STATE BAR RECU *
        */

        if (P.stateBar != '') {
            if (P.stateBar == 'Close' && !this.LEFT && !this.RIGHT)
                this.ShowPanelReduced = true;
            //else
            //    this.ShowPanelReduced = false;
        }

        /* AFFICHAGE DE L'INTERFACE EN FONCTION DES PARAMETRES CHOISIS DANS L'ADMIN */
        this.DOM.showInterface(P.IsAgentWorking);


        /* CHECK IF SPECIAL PAGE AND HIDDE INTERFACE */
        if (this.SpecialPage && !this.ShowBarOnSpecialPage) this.DOM.hiddeInterface();


        /******************************* CONFIGURATION DES BOUTONS ET DES PANNEAUX ******************************/
        this.DOM.setAgentHere((P.IsAgentWorking && P.IsCampaignOpen), P, this.ChatAvailable);

        /*
        SET PLACE FOR PANNELS 
        */
        if (P.Left_Form == '-10px') {
            try {
                this.DOM.PANEL_FORM.style.left = (this.DOM.FindLeftEdge(this.DOM.BTN_FORM) - 300) + 'px';
                this.LeftForm = this.DOM.PANEL_FORM.style.left;
            } catch (e) { }
        }
        else {
            this.DOM.PANEL_FORM.style.left = P.Left_Form;
            this.LeftForm = P.Left_Form;
        }

        this.DOM.PANEL_CHAT.style.left = P.Left_Chat;
        this.DOM.PANEL_VIDEO.style.left = P.Left_Video;
        if (typeof (this.DOM.PANEL_MINI_CHAT) != "undefined") this.DOM.PANEL_MINI_CHAT.style.left = P.Left_MiniChat;
        this.DOM.ChangeURL(P.Url, P.UrlDescription, false);

        /************************ ETAT DU CHAT OU DES DEMANDES DE CHAT EN COURS ************************/
        // LE SURFER CHATE
        if (P.surferStatus == 'traiting') {
            // CE MESSAGE REFAIT UNE DEMANDE DE CHAT SANS QUE L'INTERNAUTE OU L'AGENT LE VOIT
            this.SendMessage('RESTART_HIDDEN');
            if (P.Mini_Panel_Chat) this.DOM.EnterMiniChatMode(false);
        }
		 else if (P.surferStatus == 'checked')
        {
            this.DOM.MAINDIV_SURFER_TEXT.value = "Demande de chat en attente";
            this.DOM.MAINDIV_SURFER_TEXT.disabled = true;
            this.DOM.MAINDIV_SURFER_POST.style.display = "none";
            this.SendMessage();
        }        else {
            // CHECK EXTERNAL REQUEST
            if (P.chatRequestCode == 3) {
                // PREDICTIVE
                this.GetPredictiveStrings();
                this.SendMessage('@PREDICTIVE CHAT@');
                if (self.Time_To_Close_Predictive != -1000) {
                    this.Timer_Predictive = setTimeout(
                        function () {
                            self.ChatClose();
                            if (self.PopupOnProActifChat) self.DOM.closePopupPredict();
                            if (self.Timer_Predictive) {
                                clearTimeout(self.Timer_Predictive);
                                self.Timer_Predictive = null;
                            }
                        }, self.Time_To_Close_Predictive);
                }
            }
            else if (P.chatRequestCode == 1) {
                // SUPERVISOR
                this.SendMessage('@INVITATION AU CHAT@');
            }
        }


        if (P.chatRequestCode == 5) {
            if (P.Url) // IF ONCHATUI LOAD , PAS D'URL
            {
                this.DOM.openBar();
                // DISPLAY AN NEW URL
                setTimeout(
                       function () {
                           self.DOM.ChangeURL(P.Url, P.UrlDescription, true);
                       }, 2000);
            }
        }

        if (P.chatRequestCode == 6) {

            // DISPLAY AN NEW URL and PREDICTIVE
            // CHECK IF CHAT RUNNING
            if (this.ReturnParams.surferStatus != 'traiting') {
                // PREDICTIVE
                this.DOM.openBar();
                this.GetPredictiveStrings();
                this.SendMessage('@PREDICTIVE CHAT@');
                if (self.Time_To_Close_Predictive != -1000) {
                    this.Timer_Predictive = setTimeout(
                            function () {
                                self.ChatClose();
                                if (self.PopupOnProActifChat) self.DOM.closePopupPredict();
                                if (self.Timer_Predictive) {
                                    clearTimeout(self.Timer_Predictive);
                                    self.Timer_Predictive = null;
                                }
                            }, self.Time_To_Close_Predictive);
                }
            }

            // DISPLAY AN NEW URL
            this.DOM.ChangeURL(P.Url, P.UrlDescription, true);
        }

        /* DISPLAY BTN CALL CHAT BY PROACTIVE*/
        if (P.chatRequestCode == 7 && MyG2C.ObjBtnCallChat != null && MyG2C.APIInfo.IsAgentWorking && MyG2C.APIInfo.surferStatus != 'traiting') {
            MyG2C.DOM.Show_BTN_CALL_CHAT();
        }

        /* CHECK IF BTN CALL CHAT NEEDED */
        if (MyG2C.ObjBtnCallChat != null && !MyG2C.ShowObjBtnCallChatOnlyOnPredict && MyG2C.APIInfo.IsAgentWorking && !MyG2C.APIInfo.surferStatus == 'traiting') {
            MyG2C.DOM.Show_BTN_CALL_CHAT();
        }

        /* 
        START SURFER PRESENT
        */
        setTimeout('MyG2C.SurferPresent()', 5000);

    }
    else {
        /* ERROR, WE TRY AGAIN 10 Sec later*/
        setTimeout(function () { MyG2C.OnNavigPage(); }, 10000);
    }

}


// SURFER PRESENT
this.SurferPresent = function () {
    MyG2C.SendCDE(new Date().getTime(), 'SURFER_PRESENT', null, function (Params, JSId) { MyG2C.CallBack_SurferPresent(Params, JSId); }, 1, true);
}
this.CallBack_SurferPresent = function (P, I) {

    DelLastCDE(I);
    this.getActivity();
    if (this.ChatRequested) {
        // ON RELANCE SURFER PRESENT
        this.TimerSurferPresent = setTimeout('MyG2C.SurferPresent();', 5000);
        return;
    }

    if (P.errorcode != 0) {
        // NO DISPLAY AT ALL
        setTimeout(function () { this.OnNavigPage(); }, 10000);
        return;
    }

    this.APIInfo = P;

    if (typeof (this.DOM.AjustDisplayOnSurferPresent) != 'undefined') this.DOM.AjustDisplayOnSurferPresent(P.IsAgentWorking);

    /* CHECK IF SPECIAL PAGE AND HIDDE INTERFACE */
    if (this.SpecialPage && !this.ShowBarOnSpecialPage) this.DOM.hiddeInterface();


    this.FBState = P.FBState;
    if (typeof (this.DOM.BTN_FB) != 'undefined') {
        if (this.FBState == -1) this.DOM.BTN_FB.style.display = 'none';
        else if (this.FBState == 1) this.DOM.BTN_FB.style.display = 'none';
        else this.DOM.BTN_FB.style.display = '';
    }

    if (P.IsAgentWorking && P.IsCampaignOpen && P.chatRequestCode == 3 && !this.ChatRunning && !MyG2C.DOM.STATE_2) {


        // PREDICTIVE
        this.GetPredictiveStrings();

        if (this.ShowPanelOnlyIfProactiveChat && this.DOM.MAINDIV.style.display == 'none')
            this.DOM.MAINDIV.style.display = this.Display;

        this.DOM.MAINDIV.style.display = ''; this.DOM.openBar();

        this.DOM.BTN_SEND.style.display = '';

        if (this.PopupOnProActifChat && typeof (this.DOM.OpenPredictiveInPopup) != 'undefined') {

            this.DOM.OpenPredictiveInPopup();
            if (this.DOM.InputPopupPredict) this.DOM.InputPopupPredict.value = RM_G2C.getString('Type your text here.');
            this.DOM.TextPopupPredict.innerHTML = this.PredictiveSentence;
        }
        else {
            this.DOM.OpenPanels(2, this.DOM.PANEL_CHAT);
            this.DOM.Panel2Fixed = true;
            this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Type your text here.');
        }

        this.DOM.setAgentHere(true, P, true);
        if (self.Time_To_Close_Predictive != -1000) {
            this.Timer_Predictive = setTimeout(
                function () {
                    self.ShowSatisfaction = false
                    self.ChatClose();
                    if (self.Timer_Predictive) {
                        clearTimeout(this.Timer_Predictive);
                        self.Timer_Predictive = null;
                    }
                    self.PredictiveStarted = false;
                    self.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Write your question');
                }, self.Time_To_Close_Predictive);
        }
    }
    else if (P.chatRequestCode == 1 && !this.ChatRunning) {
        self.DOM.setAgentHere(true, P, true);
        // SUPERVISOR POUR SUPERVISOR 
        this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Type your text here.');
        this.SendMessage('@INVITATION AU CHAT@');
    }
    else if (this.ChatRunning && (!P.IsAgentWorking || !P.IsCampaignOpen)) {
        self.DOM.setAgentHere(true, P, true);
    }
    else if (!P.IsAgentWorking || !P.IsCampaignOpen) {
        self.DOM.setAgentHere(false, P, false);
    }
    else if (P.IsAgentWorking && P.IsCampaignOpen) {
        if (this.PredictiveStarted || this.ChatRunning)
            self.DOM.setAgentHere(true, P, true);
        else
            self.DOM.setAgentHere(P.IsAgentWorking, P, this.ChatAvailable);
    }

    this.DOM.ChangeURL(P.Url, P.UrlDescription, false);

    this.Chat_State = P.State_Chat;

    // DISPLAY AN NEW URL
    if (P.chatRequestCode == 5) {
        this.DOM.openBar();
        var U = P.Url;
        var D = P.UrlDescription;
        setTimeout(
                function () {
                    self.DOM.ChangeURL(U, D, true);
                }, 2000);
    }

    // DISPLAY AN NEW URL and PREDICTIVE
    if (this.ReturnParams.chatRequestCode == 6) {
        // CHECK IF CHAT RUNNING
        if (!this.ChatRunning) {
            // PREDICTIVE
            this.DOM.openBar();
            this.GetPredictiveStrings();
            this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Type your text here.');
            this.SendMessage('@PREDICTIVE CHAT@');
            if (self.Time_To_Close_Predictive != -1000) {
                this.Timer_Predictive = setTimeout(
                    function () {
                        self.ShowSatisfaction = false
                        self.ChatClose();
                        if (self.PopupOnProActifChat) self.DOM.closePopupPredict();
                        self.PredictiveStarted = false;
                        self.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Write your question');
                    }, self.Time_To_Close_Predictive);
            }
        }
        // DISPLAY AN NEW URL
        this.DOM.ChangeURL(P.Url, P.UrlDescription, true);
    }

    /* DISPLAY BTN CALL CHAT BY PROACTIVE*/
    if (this.ReturnParams.chatRequestCode == 7 && MyG2C.ObjBtnCallChat != null && MyG2C.APIInfo.IsAgentWorking && !MyG2C.APIInfo.surferStatus == 'traiting') {
        MyG2C.DOM.Show_BTN_CALL_CHAT();
    }

    // MULTI ONGLETS
    if (P.surferStatus == 'traiting' && !this.ChatRunning) {
        // CHAT HAS BEEN OPEN IN ANOTHER ONGLET
        this.SendMessage('RESTART_HIDDEN');
    }

    if (P.Mini_Panel_Chat && !this.DOM.Mini_Panel_Chat)
        this.DOM.EnterMiniChatMode(true);

    // ON RELANCE SURFER PRESENT
    this.TimerSurferPresent = setTimeout('MyG2C.SurferPresent();', 3000);

}

this.GetPredictiveStrings = function () {
    MyG2C.SendCDE(new Date().getTime(), 'GETPREDICTIVESTRINGS', '', function (Params, JSId) { MyG2C.CallBack_GetPredictiveStrings(Params, JSId); }, 1, true);
}
this.CallBack_GetPredictiveStrings = function (P, I) {
    DelLastCDE(I);
    this.PredictiveStarted = true;
    this.PredictiveSentenceDisplayed = true;
    this.DOM.AddMessage('AGENT', SERIAL(P.PredictiveSentence), 0);
    if (P.PredictiveCanceledSentence) this.PredictiveCanceledMessage = P.PredictiveCanceledSentence;
    if (this.PopupOnProActifChat && typeof (this.DOM.OpenPredictiveInPopup) != 'undefined') this.DOM.TextPopupPredict.innerHTML = P.PredictiveSentence;
    this.Log_String = P.Log_String;
    var img = new Image; img.src = this.Log_String + 'ORDER=' + this.HN_ORDER_CODE + '&COMPANY=' + this.HN_COMPANY_NAME + '&ACTION=DISPLAY';
}

this.SendMessage = function (MESSAGE) {

    this.MessageSended = '';

    if (this.ChatRunning) {

        if (!MESSAGE)
            MESSAGE = FirstMessage;

        if (MESSAGE == null || MESSAGE == '' || MESSAGE == 'RESTART_HIDDEN') {
            if (this.Chat_State && typeof(this.DOM.MinimizePanels)!='undefined')
                this.DOM.MinimizePanels(2, this.DOM.PANEL_CHAT, 'CHAT');
            else
                this.DOM.OpenPanels(2, this.DOM.PANEL_CHAT);
            return;
        }

        this.DOM.MAINDIV_SURFER_TEXT.value = '';
        this.MessageSended = MESSAGE;

        if (MESSAGE == '@@START_COBROWSING@@') {
            this.MessageSended = RM_G2C.getString('CoBrowsing Request');
            var OBJ_SEND = new Object();
            OBJ_SEND.type = 'COBROWSING_REQUEST';
            OBJ_SEND.text = '';
            MyG2C.SendCDE(new Date().getTime(), 'SEND_MESSAGE', JSON_G2C.stringify_G2C(OBJ_SEND), function (Params, JSId) { MyG2C.CallBack_SendMessage(Params, JSId, MESSAGE); }, 1, true);

        }
        else if (MESSAGE == '@@STOP_COBROWSING@@') {
            this.MessageSended = RM_G2C.getString('CoBrowsing Stop');
            var OBJ_SEND = new Object();
            OBJ_SEND.type = 'COBROWSING_STOP';
            OBJ_SEND.text = '';
            MyG2C.SendCDE(new Date().getTime(), 'SEND_MESSAGE', JSON_G2C.stringify_G2C(OBJ_SEND), function (Params, JSId) { MyG2C.CallBack_SendMessage(Params, JSId, MESSAGE); }, 1, true);

        }
        else
            MyG2C.SendCDE(new Date().getTime(), 'SEND_MESSAGE', SERIAL(MESSAGE), function (Params, JSId) { MyG2C.CallBack_SendMessage(Params, JSId, MESSAGE); }, 1, true);

        FirstMessage = null;
    }
    else if (this.Timer_Wait_Agent_Ready != null) {
        return;
    }
	else if(this.surferStatus == "checked")
    {
        this.WaitAgentReady();
            
        return;
    }    else {
        // PREDICTIVE
        if (MESSAGE == '@INVITATION AU CHAT@') {
            // LE CHAT N'EST PAS LANCE > ON FAIT UNE DEMANDE
            MyG2C.SendCDE(new Date().getTime(), 'CHAT_ASK', RM_G2C.getString('INVITATION AU CHAT RECUE'), function (Params, JSId) { MyG2C.CallBack_ChatAsk(Params, JSId); }, 1, true);
            return;
        }

        if (MESSAGE == '@PREDICTIVE CHAT@') {
            // LE CHAT N'EST PAS LANCE > ON FAIT UNE DEMANDE
            MyG2C.SendCDE(new Date().getTime(), 'CHAT_ASK', RM_G2C.getString('INVITATION AU CHAT RECUE'), function (Params, JSId) { MyG2C.CallBack_ChatAsk(Params, JSId); }, 1, true);
            setTimeout(function () {
                if (MyG2C.ChatRunning) {
                    self.DOM.LoadWebConf();
                    self.DOM.Select_Tab_Video('WEBCONF', true);
                }
            }, 5000);
            return;
        }

        FirstMessage = MESSAGE;

        if (MESSAGE == '' && !this.PredictiveStarted) {
            this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Write your question');
            return;
        }
        else if (MESSAGE == '' && this.PredictiveStarted) {
            this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Type your text here.');
            return;
        }

        if (this.PredictiveStarted) {
            var img = new Image; img.src = this.Log_String + 'ORDER=' + this.HN_ORDER_CODE + '&COMPANY=' + this.HN_COMPANY_NAME + '&ACTION=ACCEPT';
            this.PredictiveStarted = false;
        }

        if (typeof(this.DOM.MAINDIV_SURFER_TEXT)!='undefined') this.DOM.MAINDIV_SURFER_TEXT.value = '';
        // LE CHAT N'EST PAS LANCE > ON FAIT UNE DEMANDE
        MyG2C.SendCDE(new Date().getTime(), 'CHAT_ASK', MESSAGE, function (Params, JSId) { MyG2C.CallBack_ChatAsk(Params, JSId); }, 1, true);

    }

}
this.CallBack_SendMessage = function (P, I) {

    DelLastCDE(I);
    var Here = this;

    if (P) if (P.length != 0) this.DOM.AddMessage('AGENT', P, 0);

    if (this.TimerReceiveMessage) {
        clearTimeout(this.TimerReceiveMessage);
        this.TimerReceiveMessage = null;
        this.TimerReceiveMessage = setTimeout(function () { Here.ReceiveMessage(); }, 5000);
    }

    if (this.MessageSended != '')
        this.DOM.AddMessage('SURFER', SERIAL(this.MessageSended), 0);

}

this.ReceiveMessage = function () {
    // LE CHAT N'EST PAS LANCE > ON FAIT UNE DEMANDE
    MyG2C.SendCDE(new Date().getTime(), 'RECEIVE_MESSAGE', this.ChatInfo.roomId, function (Params, JSId) { MyG2C.CallBack_ReceiveMessage(Params, JSId); }, 1, true);
}
this.CallBack_ReceiveMessage = function (P, I) {
    DelLastCDE(I);
    if (P) {
        var Here = this;

        if (P.length != 0) {
            this.DOM.AddMessage('AGENT', P, 0);
        }
        if (this.ChatRunning)
            this.TimerReceiveMessage = setTimeout(function () { Here.ReceiveMessage(); }, 5000);
    }

}

this.CallBack_ChatAsk = function (P, I) {
    DelLastCDE(I);
    this.ChatRequested = true;
    if (typeof (P.waitingTime) == "number" && typeof (P.queuePosition) == "number") {
        this.WaitAgentTime = P.waitingTime;
        this.SurferPosition = P.queuePosition;
    }
    else if (P.txtEnding) {
        this.TextEnding = P.txtEnding;
        this.WaitAgentTime = P.waitingTime;
        this.SurferPosition = P.QueuePosition;
    }
    else if (typeof (P.AgentState) == "number") {
        this.WaitAgentTime = P.AgentState;
        this.SurferPosition = P.queuePosition;
    }
    if (typeof (this.DOM.BTN_EXIT) != 'undefined') {
        this.DOM.BTN_EXIT.value = RM_G2C.getString('Cancel');
        this.DOM.BTN_PRINT.style.display = 'none';
    }
	if (this.surferStatus == 'checked' && this.DOM.PANEL_CHAT.style.visibility != 'visible') {
        this.DOM.OpenPanels(2, this.DOM.PANEL_CHAT);
    }    this.WaitAgentReady();
}

this.WaitAgentReady = function () {

    if (this.WaitAgentTime < 0 && this.WaitAgentTime >= -3) {
        MyG2C.DOM.PANEL_CHAT_SCROLL.innerHTML = '';
        if (this.Timer_Wait_Agent_Ready != null) { clearTimeout(this.Timer_Wait_Agent_Ready); this.Timer_Wait_Agent_Ready = null; }
        this.DOM.AddMessage('SYSTEM', SERIAL(RM_G2C.getString('Error during connecting agent')), 0);
        this.ChatRequested = false;
        return;
    }
    else if (this.WaitAgentTime == -4) {
        MyG2C.DOM.PANEL_CHAT_SCROLL.innerHTML = '';
        if (this.Timer_Wait_Agent_Ready != null) { clearTimeout(this.Timer_Wait_Agent_Ready); this.Timer_Wait_Agent_Ready = null; }
        this.DOM.AddMessage('SYSTEM', SERIAL(RM_G2C.getString('No more agent available, try again later')), 0);
        this.ChatRequested = false;
        return;
    }
    else if (this.WaitAgentTime == 0 || this.WaitAgentTime == 1) {
        MyG2C.DOM.PANEL_CHAT_SCROLL.innerHTML = '';
        if (this.Timer_Wait_Agent_Ready != null) { clearTimeout(this.Timer_Wait_Agent_Ready); this.Timer_Wait_Agent_Ready = null; }
        this.DOM.AgentTyping.innerHTML = ' ';
        this.OnChatUiLoad();

        if (typeof (this.DOM.SetPanelChatState) != 'undefined') this.DOM.SetPanelChatState(true);

        if (this.DOM.Start_Chat_By_Cam) this.DOM.OpenPanels(1, this.DOM.PANEL_VIDEO); else { if (!this.WebConfRunning && eval(MyG2C.getCookie('PCO'))) this.DOM.OpenPanels(2, this.DOM.PANEL_CHAT); }
        return;
    }
    else {
        // ON DEMANDE LE STATUS AGENT
        var PP = this.SurferPosition;
        if (this.SurferPosition == 0 || this.SurferPosition == 1) PP = RM_G2C.getString('First');
        else if (this.SurferPosition == 2) PP = RM_G2C.getString('Second');
        else if (this.SurferPosition == 3) PP = RM_G2C.getString('Third');
        else PP += RM_G2C.getString('Th');

        if (typeof (MyG2C.DOM.WaitChat) == 'undefined') {
            MyG2C.DOM.PANEL_CHAT_SCROLL.innerHTML = '<div style="background-color:transparent;margin:0px;padding:0px; border:0px; text-Decoration:none;width:100%;height:100%;vertical-align:middle;text-align:center;font-size:0px;"><table class="TextSatisfaction" height=100% width=100%><tr><td class="TextSatisfaction" style="text-align:center" align=center width=100% height=50%><img class="MG2C_Img" src="' + this.UrlServer + '/' + this.SkinPath + '/loading.gif" align=absmiddle>' +
            '</td></tr><tr><td class="TextSatisfaction"  width=100% height=50% align=center>' + RM_G2C.getString('Estimated waiting time is') + ' : <b>' + this.WaitAgentTime + ' ' + RM_G2C.getString('Sec.') + '</b>' +
            '<br>' + RM_G2C.getString('Your position in the queue is') + ' : <b>' + PP + '</td></tr></table>';
        }
        else {
             if (typeof (this.DOM.EnlargePanelChat) != 'undefined')
                {
                    if (this.surferStatus == 'checked' && this.DOM.PANEL_CHAT.style.visibility != 'visible') {
                      
                    }
                    else
                        this.DOM.EnlargePanelChat(1, 0, 0);
                }
                MyG2C.DOM.WaitChat(this.WaitAgentTime, PP);

        }
        this.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Your request is in progress');
        this.DOM.MAINDIV_SURFER_TEXT.disabled = true;        this.Timer_Wait_Agent_Ready = setTimeout(function () {
            MyG2C.SendCDE(new Date().getTime(), 'AGENT_STATE', null, function (Params, JSId) { MyG2C.CallBack_ChatAsk(Params, JSId); }, 1, true);
        }, 2000);
    }

}

this.OnChatUiLoad = function () {
    MyG2C.SendCDE(new Date().getTime(), 'ONCHATUILOAD', null, function (Params, JSId) { MyG2C.CallBack_OnChatUiLoad(Params, JSId); }, 1, true);
}
this.CallBack_OnChatUiLoad = function (P, I) {

    DelLastCDE(I);

    var self = this;

    if (typeof (P.UILOAD.statusCode) == 'number') { this.NameAgent = P.UILOAD.lastAgentName;  this.IdAgent = P.UILOAD.idAgent; }

    if (typeof (P.UILOAD.statusCode) == 'number') {
        if (P.UILOAD.statusCode < 0) {
            this.DOM.AddMessage('SYSTEM', SERIAL(RM_G2C.getString('Agent link error. Try again later.')), 0);
            this.ChatClose();
            this.ChatRequested = false;
            return;
        }
    }
    else {
        this.chatClose();
        this.ChatRequested = false;
        return;
    }

    this.ChatRunning = true;
    this.DOM.MAINDIV_SURFER_TEXT.value = "";
    this.DOM.MAINDIV_SURFER_TEXT.disabled = false;
    this.DOM.MAINDIV_SURFER_POST.style.display = "";    this.ChatInfo = P.UILOAD;


    this.InfoUser.Agent = false;
    this.InfoUser.RoomOid = this.ChatInfo.roomId;
    this.InfoUser.IdSurferChatting = this.IdSurfer;

    this.Display_HistoryChat(P.UILOAD.historyMsg);

    this.DebugInfo('FIRST MESSAGE : ' + FirstMessage);

    if (FirstMessage != 'RESTART_HIDDEN') {
        this.DebugInfo('SEND FIRST MESSAGE');
        this.SendMessage();
    }

    if (MyG2C.Chat_State && typeof (this.DOM.MinimizePanels) != 'undefined')
        this.DOM.MinimizePanels(2, this.DOM.PANEL_CHAT, 'CHAT');
    else if (!self.DOM.Start_Chat_By_Cam && !MyG2C.ChatRunning)
        this.DOM.OpenPanels(2, this.DOM.PANEL_CHAT);

    setTimeout(function () {

        if (FirstMessage != 'RESTART_HIDDEN') {
            if (!self.PredictiveStarted) {
                if (self.PredictiveSentenceDisplayed)
                    self.PredictiveSentenceDisplayed = false;
                else
                    self.DOM.AddMessage('AGENT', SERIAL(P.UILOAD.txtChatStart), 0);
            }
        }
        else 
        {


            if (self.DOM.StateBar == 'Open') {
                if (self.Chat_State && typeof (self.DOM.MinimizePanels) != 'undefined')
                    self.DOM.MinimizePanels(2, self.DOM.PANEL_CHAT, 'CHAT');
                else {
                    if (!self.DOM.STATE_2 && !MyG2C.WebConfRunning && eval(MyG2C.getCookie('PCO'))) self.DOM.OpenPanels(2, self.DOM.PANEL_CHAT);
                    else if (MyG2C.WebConfRunning) {
                        self.DOM.StartBlinkBTN_CHAT();
                        if (MyG2C.getCookie('CAM_POS') == '6') { self.DOM.Select_Tab_Video('WEBCONF', true); self.DOM.OpenPanels(1, self.DOM.PANEL_VIDEO); }
                    }
                    else if (!self.DOM.STATE_2 && !MyG2C.WebConfRunning && !eval(MyG2C.getCookie('PCO'))) { self.DOM.StartBlinkBTN_CHAT(); }
                }
            }


        }

        if (typeof (self.DOM.Start_Chat_By_Cam) != 'undefined' && self.DOM.Start_Chat_By_Cam == true) {

            self.DOM.Start_Chat_By_Cam = false;
            self.DOM.Select_Tab_Video('WEBCONF', true);
            self.DOM.LoadWebConf();
        }
        else if (P.WebConfRunning) {
            self.DOM.Select_Tab_Video('WEBCONF', true);
            self.DOM.LoadWebConf();
            if (self.ChatRunning) self.DOM.StartBlinkBTN_VIDEO(true);
        }
        else if (self.StartAutoVideoConf) {
            // AUTO START WEB CAM
            MyG2C.setCookie('CAM_POS', 5);
            self.DOM.ACTUAL_POSITION = 5;
            self.DOM.Select_Tab_Video('WEBCONF', true);
            self.DOM.LoadWebConf();
        }

        self.ReceiveMessage();

        if (P.COBrowsing) {
            self.CoBrowsingRunning = true;
            self.StartCoBrowsingSurfer();
        }

        self.ChatSurferOpen = true;
        FirstMessage = null;
        self.ChatRequested = false;

    }, 1500);
}

this.ChatClose = function () {
    clearTimeout(this.Timer_Wait_Agent_Ready);
    this.Timer_Wait_Agent_Ready = null;
 	if (MyG2C.DOM.PAreaTextChat) {
        MyG2C.DOM.PAreaTextChat.style.display = '';
        MyG2C.DOM.AreaTextChat.style.display = '';
    }    MyG2C.SendCDE(new Date().getTime(), 'CLOSE_CHAT', null, function (Params, JSId) { MyG2C.Callback_ChatClose(Params, JSId); }, 1, true);
}
this.Callback_ChatClose = function (P, I) {

    DelLastCDE(I);

    if (!this.ChatRunning) {
        // ARRET PENDANT LA MISE EN RELATION
        this.DOM.Panel2Fixed = false; this.DOM.closePanels(2, this.DOM.PANEL_CHAT);
        this.DOM.Panel1Fixed = false; this.DOM.closePanels(1, this.DOM.PANEL_VIDEO);
 		this.DOM.MAINDIV_SURFER_TEXT.value = this.ChatMessage;
        this.DOM.MAINDIV_SURFER_TEXT.disabled = false;
        this.DOM.MAINDIV_SURFER_POST.style.display = "";
        this.DOM.PANEL_CHAT_SCROLL.innerHTML = '';        this.DOM.Mini_Panel_Chat = false; if (this.DOM.PanelPredictive) this.DOM.PanelPredictive.style.display = 'none'; return;
    }

    if (this.ChatInfo && this.ChatInfo.txtEnding != '')
        self.DOM.AddMessage('AGENT', SERIAL(this.ChatInfo.txtEnding), 0);

    this.ChatRunning = false;
    this.ChatSurferOpen = true;
    var Here = this;

    // ARRET DU CO BROWSING
    if (this.CoBrowsingRunning) this.StopCoBrowsingSurfer();

    // DELETE COOKIE AGENT
    this.delCookie('AGENT_G2C_ROOM_OID'); this.delCookie('AGENT_G2C_IDSURFER');

    // ARRET DE LA RECEPTION DES MESSAGE
    if (this.TimerReceiveMessage) {
        clearTimeout(this.TimerReceiveMessage);
        this.TimerReceiveMessage = null;
    }

    // RAZ .WebCamRunning
    this.WebCamRunning = false;

    // HIDDE HELP
    if (typeof (this.DOM.HiddeAllHelp) != 'undefined') this.DOM.HiddeAllHelp();

    setTimeout(function () {
        Here.DOM.Panel2Fixed = false; Here.DOM.closePanels(2, Here.DOM.PANEL_CHAT);
        Here.DOM.QuitWebConf();
        if (Here.DOM.StateBar != 'Close' || Here.DOM.Mini_Panel_Chat) {
            Here.DOM.Select_Tab_Video('WEBCONF', false);
            Here.DOM.Select_Tab_Video('ATTACHMENT', false);
            if (Here.TabVideoSatisfaction && self.ShowSatisfaction && !self.SatisfactionDone) {
                if (Here.Timer_Wait_Agent_Ready == null) {
                    Here.DOM.Select_Tab_Video('SATISFACTION', true);
                    Here.DOM.PANEL_VIDEO.style.left = Here.DOM.PANEL_CHAT.style.left;
                    Here.DOM.OpenPanels(1, Here.DOM.PANEL_VIDEO);
                    try {Here.DOM.StopBlinkBTN_CHAT(); Here.DOM.StopBlinkBTN_VIDEO();} catch(e){}
                  //   Here.DOM.closePanels(1, Here.DOM.PANEL_VIDEO);
                } else {
                    clearTimeout(Here.Timer_Wait_Agent_Ready);
                    Here.Timer_Wait_Agent_Ready = null;
                }
            }
            else
            { Here.Panel1Fixed = false; Here.DOM.closePanels(1, Here.DOM.PANEL_VIDEO); }


            Here.DOM.MAINDIV_SURFER_TEXT.value = RM_G2C.getString('Write your question');

            if (MyG2C.DOM.PAreaTextChat) {
                MyG2C.DOM.PAreaTextChat.style.display = '';
                MyG2C.DOM.AreaTextChat.style.display = '';
            }


        }

    }, this.timeOutCloseChat);

}

this.StartCoBrowsingSurfer = function () {
    // SI LE COBROWSING EST ACTIF ET LE TIMER INACTIF ON LANCE LE CO BROWSING
    if (this.TimerCoBrowsingRunning && this.CoBrowsingRunning)
        return;
    else if (!this.TimerCoBrowsingRunning) {

        this.DOM.HiddeBTN('COBRO');
        this.CoBrowsingRunning = true;

        var cont = '<b>' + RM_G2C.getString('Co-Navigation ACTIVE.') + '</b>';
        cont += '<img style="cursor:pointer" height=10 onclick="MyG2C.StopCobrowsingFromSurfer();" hspace=2 src="' + this.UrlServer + '/' + this.SkinPath + '/CLOSE_COBRO.png" align=absmiddle>'
        try {
            this.DOM.ShowBTN('COBRO');
            this.DOM.StartBlinkBTN_COBRO();
        }
        catch (e) { this.DOM.AgentTyping.innerHTML = cont; }


        this.CreateMouseCoBrowsing();
        MyG2C.DOM.AddMessage('AGENT', SERIAL(RM_G2C.getString('Co-browsing started.')), 0);
        this.TimerCoBrowsingRunning = setInterval(function () {
            self.GetCoBrowsingInfoSurfer();
        }, self.SpeedCoBrowsingSurfer);
    }
}
this.StopCoBrowsingSurfer = function () {
    // TIMER ARRETE ET COBRO ON
    if (!this.TimerCoBrowsingRunning && this.CoBrowsingRunning) {
        this.TimerCoBrowsingRunning = null;
        this.CoBrowsingRunning = false;
    }
    // SI LE TIMER CO BROWSING EST ACTIF ET CO BROWSING ON ARRETE LE TIMER
    else if (this.TimerCoBrowsingRunning && this.CoBrowsingRunning) {
        clearInterval(this.TimerCoBrowsingRunning);
        this.TimerCoBrowsingRunning = null;
        this.CoBrowsingRunning = false;
    }

    try {
        this.DOM.AreaDisplayCobro.innerHTML = '&nbsp;';
        this.DOM.StopBlinkBTN_COBRO();
    }
    catch (e) { this.DOM.AgentTyping.innerHTML = '&nbsp;'; }

    MyG2C.DOM.AddMessage('AGENT', SERIAL(RM_G2C.getString('Co-browsing stopped.')), 0);
    if (MyG2C.CobrowsingAvailable) {
        this.DOM.ShowBTN('COBRO');
        this.DOM.StopBlinkBTN_COBRO();
    }
    this.RemoveMouseCoBrowsing();

}

this.StopCobrowsingFromSurfer = function () {
    MyG2C.SendMessage('@@STOP_COBROWSING@@');
}

this.GetCoBrowsingInfoSurfer = function () {
    var Par = this.InfoUser.Agent + ';' + this.InfoUser.RoomOid + ';' + this.InfoUser.IdSurferChatting;
    this.DebugInfo('GET_COBROWSING_INFO_SURFER with param : ' + Par);
    MyG2C.SendCDE(new Date().getTime(), 'GET_COBROWSING_INFO_SURFER', Par, function (Params, JSId) { MyG2C.Callback_GetCoBrowsingInfoSurfer(Params, JSId); }, 1, true);
}
this.Callback_GetCoBrowsingInfoSurfer = function (P, I) {
    DelLastCDE(I);
    this.InfoUser.Agent = eval(P.IsAgent);
    this.InfoUser.RoomOid = P.RoomOid;
    this.InfoUser.IdSurferChatting = P.IdSurfer;
    this.DOM.PlaceMouseCoBrowsing(P.EVENTS);
}

this.StartCoBrowsingAgent = function () {
    // SI LE COBROWSING EST ACTIF ET LE TIMER INACTIF ON LANCE LE CO BROWSING
    if (this.TimerCoBrowsingRunning && this.CoBrowsingRunning)
        return;
    else if (!this.TimerCoBrowsingRunning) {
        this.CoBrowsingRunning = true;
        this.CreateMouseCoBrowsing();
        this.TimerCoBrowsingRunning = setInterval(function () {
            self.GetCoBrowsingInfoAgent();
        }, self.SpeedCoBrowsingAgent);
    }
}
this.StopCoBrowsingAgent = function () {
    // TIMER ARRETE ET COBRO ON
    if (!this.TimerCoBrowsingRunning && this.CoBrowsingRunning) {
        this.TimerCoBrowsingRunning = null;
        this.CoBrowsingRunning = false;
    }
    // SI LE TIMER CO BROWSING EST ACTIF ET CO BROWSING ON ARRETE LE TIMER
    else if (this.TimerCoBrowsingRunning && this.CoBrowsingRunning) {
        clearInterval(this.TimerCoBrowsingRunning);
        this.TimerCoBrowsingRunning = null;
        this.CoBrowsingRunning = false;
    }
    this.RemoveMouseCoBrowsing();
}
this.GetCoBrowsingInfoAgent = function () {
    var Par = this.InfoUser.Agent + ';' + this.InfoUser.RoomOid + ';' + this.InfoUser.IdSurferChatting;
    this.DebugInfo('GET_COBROWSING_INFO_AGENT with param : ' + Par);
    MyG2C.SendCDE(new Date().getTime(), 'GET_COBROWSING_INFO_SURFER', Par, function (Params, JSId) { MyG2C.Callback_GetCoBrowsingInfoAgent(Params, JSId); }, 1, true);
}
this.Callback_GetCoBrowsingInfoAgent = function (P, I) {
    DelLastCDE(I);
    this.InfoUser.Agent = eval(P.IsAgent);
    this.InfoUser.RoomOid = P.RoomOid;
    this.InfoUser.IdSurferChatting = P.IdSurfer;
    for (var i = 0; i < P.EVENTS.length; i++) {
        if (P.EVENTS[i].TEVENT == "CLOSED") { this.StopCoBrowsingAgent(); return; }
    }
    this.DOM.PlaceMouseCoBrowsing(P.EVENTS);

}

this.CreateMouseCoBrowsing = function () {
    // CREATE MOUSE POINTER
    this.Pointer_Mouse = this.DOM.G2C_DC('DIV');this.DOM.G2C_OS(this.Pointer_Mouse, 'absolute', '1px', null, '1px', null, '50px', '59px');this.Pointer_Mouse.style.background = 'url("' + this.DOM.path_IMG + '/MOUSE.png") no-repeat';this.Pointer_Mouse.style.zIndex = 99999;this.Pointer_Mouse.style.display = 'none';    document.body.appendChild(this.Pointer_Mouse);

    // CREATE CLICK POINTER
    this.Click_Mouse = this.DOM.G2C_DC('DIV');
    this.DOM.G2C_OS(this.Click_Mouse, 'absolute', '1px', null, '1px', null, '69px', '69px');
    this.Click_Mouse.style.background = 'url("' + this.DOM.path_IMG + '/MOUSE_CLICK.png") no-repeat';
    this.Click_Mouse.style.zIndex = 99999;
    this.Click_Mouse.style.display = 'none';
    document.body.appendChild(this.Click_Mouse);

    // CREATE TYPING POINTER
    this.Typing_Mouse = this.DOM.G2C_DC('DIV');
    this.DOM.G2C_OS(this.Typing_Mouse, 'absolute', '1px', null, '1px', null, '69px', '69px');
    this.Typing_Mouse.style.background = 'url("' + this.DOM.path_IMG + '/MOUSE_TYPING.png") no-repeat';
    this.Typing_Mouse.style.zIndex = 99999;
    this.Typing_Mouse.style.display = 'none';
    document.body.appendChild(this.Typing_Mouse);

}

this.RemoveMouseCoBrowsing = function () {
    // REMOVE MOUSE POINTER
    if (this.Pointer_Mouse && this.Pointer_Mouse.parentNode != null) this.Pointer_Mouse.parentNode.removeChild(this.Pointer_Mouse);
    // REMOVE CLICK POINTER
    if (this.Click_Mouse && this.Click_Mouse.parentNode != null) this.Click_Mouse.parentNode.removeChild(this.Click_Mouse);
    // REMOVE CLICK POINTER
    if (this.Typing_Mouse && this.Typing_Mouse.parentNode != null) this.Typing_Mouse.parentNode.removeChild(this.Typing_Mouse);
}

this.SendStateBar = function (State) {
    MyG2C.DOM.StateBar = State;
    setTimeout(function () {
        MyG2C.SendCDE(new Date().getTime(), 'SET_STATE_BAR', null, function (Params, JSId) { MyG2C.CallBack_SetState(Params, JSId); }, 1, true);
    }, 1000);
}

this.CallBack_SetState = function (P, I) {
    DelLastCDE(I);
}

this.SendControl = function (EVENT) {
    if (!this.CoBrowsingRunning) return;

    var img = new Image;
    var url = this.OnMediaServer + "/MouseControl.aspx?ACTION=" + EVENT
    switch (EVENT) {
        case 'MOVE':
            url += '&X=' + self.MouseX + '&Y=' + self.MouseY + '&I=&T=';
            break;
        case 'MOUSEDOWN':
            url += '&X=' + self.MouseX + '&Y=' + self.MouseY + '&I=' + self.IdMouseClick + '&T=' + self.ElMouseClick;
            break;
        case 'TYPING':
            url += '&X=' + self.MouseX + '&Y=' + self.MouseY + '&I=' + self.IdMouseClick + '&T=' + self.ElMouseClick;
            break;
    }
    if (!this.IsAgent)
        img.src = url + '&A=' + this.IsAgent.toString() + "&IUSR=" + this.IdSurfer + "&CHAT_ROOM=" + this.InfoUser.RoomOid; // this.ChatInfo.roomId;
    else
        img.src = url + '&A=' + this.IsAgent.toString() + "&IUSR=" + this.IdSurfer + "&CHAT_ROOM=" + this.InfoUser.RoomOid;

    this.DebugInfo(img.src);
}

function ______________() { }
function ____(AFFICHAGE_CHAT) { }

this.Display_HistoryChat = function (historyMsg) {
    this.DOM.AddMessage('', historyMsg, 0, true);
}
this.getHourMess = function (TIME) {
    var D = TIME;
    var DA = new Date();
    DA.setFullYear(D.substr(0, 4));
    DA.setMonth(Number(D.substr(5, 2)) - 1);
    DA.setDate(D.substr(8, 2));
    DA.setHours(D.substr(11, 2));
    DA.setMinutes(D.substr(14, 2));
    DA.setSeconds(D.substr(17, 2));
    DA.setMinutes(DA.getMinutes() - (new Date().getTimezoneOffset()));
    var NowDate = new Date();
    var TimeHisto = '';
    if ((NowDate.getFullYear() == DA.getFullYear()) && (NowDate.getMonth() == DA.getMonth()) && (NowDate.getDate() == DA.getDate())) {
        TimeHisto = ' ' + RM_G2C.getString('at') + ' ' + formatDigit(DA.getHours(), 2) + ":" + formatDigit(DA.getMinutes(), 2);
    }
    else {
        TimeHisto = ' ' + RM_G2C.getString('the') + ' ' + formatDigit(DA.getDate(), 2) + "-" + formatDigit(DA.getMonth() + 1, 2) + "-" + formatDigit(DA.getFullYear(), 4);
        TimeHisto += ' ' + RM_G2C.getString('at') + ' ' + formatDigit(DA.getHours(), 2) + ":" + formatDigit(DA.getMinutes(), 2);
    }
    return TimeHisto;
}
function formatDigit(nb, length) {
    nb = nb + "";
    var lg = nb.length;
    for (var i = lg; i < length; i++) {
        nb = "0" + nb;
    }
    return nb;
}
this.CallBack_PrintHistory = function (P, I) {
    DelLastCDE(I);
    self.DOM.FillPrintHistoryPopup(P);
}

function ______________() { }
function ____(SET_CALLBACK) { }

//SEND CALLBACK ASK
this.SetCallback = function (NAME_OPT, TEL, DIFF_MIN, COMMENT) {
    var Diff_S = 0;
    if (NAME_OPT != null) {
        // GET OPTIONS CHECKED
        var RADIO = document.getElementsByName(NAME_OPT);

        for (var i = 0; i < RADIO.length; i++) {
            if (RADIO[i].checked) {
                var Now = new Date();
                var H = Now.getHours();
                Diff_S = (24 - H) * 60;
                if (i == 0)
                    Diff_S = 0;
                else if (i == 1)
                    Diff_S += 540;
                else if (i == 2)
                    Diff_S += 660;
                else if (i == 3)
                    Diff_S += 900;
                else if (i == 4)
                    Diff_S += 1020;
            }
        }
    }
    else {
        Diff_S = DIFF_MIN;
    }
    if (!COMMENT) var Params = TEL + '@|@' + Diff_S + '@|@ ';
    else var Params = TEL + '@|@' + Diff_S + '@|@' + COMMENT;

    MyG2C.SendCDE(new Date().getTime(), 'SET_CALLBACK', Params, function (Params, JSId) { MyG2C.CallBack_SetCallback(Params, JSId); }, 1, true);

}
this.CallBack_SetCallback = function (P, I) {
    DelLastCDE(I);
}

//SEND CALLBACK ASK
this.SetCallback_42 = function (TEL,
            OPTION,
            BEFORE,
            FROM,
            TO) {
    var Params = TEL + '@|@' + OPTION + '@|@' + BEFORE + '@|@' + FROM + '@|@' + TO + '@|@';
    MyG2C.SendCDE(new Date().getTime(), 'SET_CALLBACK_42', Params, function (Params, JSId) { MyG2C.CallBack_SetCallback_42(Params, JSId); }, 1, true);
}

this.CallBack_SetCallback_42 = function (P, I) {

    DelLastCDE(I);

    if (P == 0) {
        this.DOM.CallbackState = 1;
        document.getElementById('My2GC_CallBack_State___G2C_1').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB.png">';
        document.getElementById('My2GC_CallBack_State___G2C_2').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
        document.getElementById('My2GC_CallBack_State___G2C_3').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
        document.getElementById('My2GC_CallBack_State___G2C_4').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
    }
    else {
        this.DOM.CallbackState = 0;
        document.getElementById('My2GC_CallBack_State___G2C_1').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
        document.getElementById('My2GC_CallBack_State___G2C_2').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
        document.getElementById('My2GC_CallBack_State___G2C_3').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
        document.getElementById('My2GC_CallBack_State___G2C_4').innerHTML = '<img src="' + this.DOM.path_IMG + '/VOYANT_CB_OFF.png">';
    }

}

function ______________() { }
function ____(POST_FORMULAIRE) { }

// POST FORM DATA
this.PostForm = function (EMAIL, SUBJECT, BODY) {
    var Params = self.Oid_MailCampaign + '@|@';
    Params += self.CC_SendMail + '@|@';
    Params += EMAIL + '@|@';
    Params += SUBJECT + '@|@';
    Params += BODY;
    MyG2C.SendCDE(new Date().getTime(), 'POST_FORM', Params, function (Params, JSId) { MyG2C.CallBack_PostForm(Params, JSId); }, 1, true);
}
this.CallBack_PostForm = function (P, I) {
    DelLastCDE(I);
}

function ______________() { }
function ____(SEND_ELEMENT_SURFER_TO_SERVER) { }

// SEND DATA FORM
function CreateArrayObject(sendAll) {
    self.ArrayData = new Array();
    if (sendAll) {
        var inputs = document.body.getElementsByTagName("INPUT");
        for (var i = 0; i < inputs.length; i++) {
            if (inputs[i].id == '') continue;
            if (inputs[i].id.indexOf('___G2C') > 0) continue;
            var obj = new Object;
            switch (inputs[i].type.toUpperCase()) {
                case 'TEXT':
                    obj.id = inputs[i].id; obj.value = inputs[i].value; obj.tagname = 'TEXT'; obj.tevent = 'INIT';
                    break;
                case 'HIDDEN':
                    obj.id = inputs[i].id; obj.value = inputs[i].value; obj.tagname = 'HIDDEN'; obj.tevent = 'INIT';
                    break;
                case 'PASSWORD':
                    obj.value = '';
                    for (var car = 0; car < inputs[i].value.length; car++) {
                        obj.value += '*';
                    }
                    obj.id = inputs[i].id; obj.tagname = 'PASSWORD'; obj.tevent = 'INIT';
                    break;
                case 'CHECKBOX':
                    obj.id = inputs[i].id; obj.value = inputs[i].checked + ''; obj.tagname = 'CHECKBOX'; obj.tevent = 'INIT';
                    break;
                case 'RADIO':
                    obj.id = inputs[i].id;
                    if (inputs[i].checked) obj.value = inputs[i].value + ' (checked)'; else obj.value = inputs[i].value;
                    obj.tagname = 'RADIO';
                    obj.tevent = 'INIT';
                    break;

                default:
                    continue;
                    break;
            }
            self.ArrayData[self.ArrayData.length] = obj;
            eventHandler('blur', G2C_onblur, inputs[i]);

        }
        var inputs = document.body.getElementsByTagName("TEXTAREA");
        for (var i = 0; i < inputs.length; i++) {
            if (inputs[i].id == '') continue;
            if (inputs[i].id.indexOf('___G2C') > 0) continue;
            var obj = new Object;
            obj.id = inputs[i].id; obj.value = inputs[i].value; obj.tagname = 'TEXTAREA'; obj.tevent = 'INIT';
            self.ArrayData[self.ArrayData.length] = obj;
            eventHandler('blur', G2C_onblur, inputs[i]);
        }
        var inputs = document.body.getElementsByTagName("SELECT");
        for (var i = 0; i < inputs.length; i++) {
            if (inputs[i].id == '') continue;
            if (inputs[i].id.indexOf('___G2C') > 0) continue;
            var obj = new Object;
            if (inputs[i].multiple) {
                obj.id = inputs[i].id;
                obj.value = '';
                for (var z = 0; z < inputs[i].options.length; z++) {
                    if (inputs[i].options[z].selected)
                        obj.value += inputs[i].options[z].value + '|@@|';
                }
                if (obj.value.length != 0) obj.value = obj.value.substr(0, obj.value.length - 4);
                obj.tagname = 'SELECTMULTI';
                obj.tevent = 'INIT';
            }
            else
            { obj.id = inputs[i].id; obj.value = inputs[i].value; obj.tagname = 'SELECT'; obj.tevent = 'INIT'; }
            self.ArrayData[self.ArrayData.length] = obj;
            eventHandler('blur', G2C_onblur, inputs[i]);
        }
    }

    var Href = document.body.getElementsByTagName("A");
    for (var i = 0; i < Href.length; i++) {
        if (Href[i].id == '') Href[i].id = 'G2C__HREF__' + i;
        for (var T = 0; T < Href[i].childNodes.length; T++) {
            if (Href[i].childNodes[T].tagName && Href[i].childNodes[T].id == '') {
                Href[i].childNodes[T].id = 'G2C__HREF__' + i + '_' + Href[i].childNodes[T].tagName + '_' + T;
                eventHandler('click', G2C_onmouseDown_Parent, Href[i].childNodes[T]);
            }
        }
        eventHandler('click', G2C_onmouseDown, Href[i]);
    }

    var V = JSON_G2C.stringify_G2C(MyG2C.ArrayData);
    MyG2C.PostCDE(new Date().getTime(), 'SET_DATA', V);
    MyG2C.DebugInfo('SET_DATA >>> ' + V);
}

this.SendSatisfaction = function (input) {
    var PH1 = 0; var PH2 = 0; var PH3 = 0;
    var TBODY = MyG2C.DOM.PANEL_VIDEO_SATISFACTION.childNodes[0].childNodes[0];
    var IMG = TBODY.childNodes[1].childNodes[1].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH1 = 1;
    IMG = TBODY.childNodes[1].childNodes[2].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH1 = 2;
    IMG = TBODY.childNodes[1].childNodes[3].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH1 = 3;
    IMG = TBODY.childNodes[1].childNodes[4].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH1 = 4;
    IMG = TBODY.childNodes[1].childNodes[5].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH1 = 5;
    var IMG = TBODY.childNodes[3].childNodes[1].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH2 = 1;
    IMG = TBODY.childNodes[3].childNodes[2].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH2 = 2;
    IMG = TBODY.childNodes[3].childNodes[3].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH2 = 3;
    IMG = TBODY.childNodes[3].childNodes[4].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH2 = 4;
    IMG = TBODY.childNodes[3].childNodes[5].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH2 = 5;
    var IMG = TBODY.childNodes[5].childNodes[1].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH3 = 1;
    IMG = TBODY.childNodes[5].childNodes[2].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH3 = 2;
    IMG = TBODY.childNodes[5].childNodes[3].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH3 = 3;
    IMG = TBODY.childNodes[5].childNodes[4].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH3 = 4;
    IMG = TBODY.childNodes[5].childNodes[5].childNodes[0]; if (IMG.getAttribute('SEL') == 1) PH3 = 5;
    var ToSend = new Array();
    var obj = new Object;
    obj.id = 'SATISFACTION_1'; obj.value = PH1; obj.tagname = 'TEXT'; obj.tevent = 'SATISFACTION';
    ToSend[0] = obj;
    var obj = new Object;
    obj.id = 'SATISFACTION_2'; obj.value = PH2; obj.tagname = 'TEXT'; obj.tevent = 'SATISFACTION';
    ToSend[1] = obj;
    var obj = new Object;
    obj.id = 'SATISFACTION_3'; obj.value = PH3; obj.tagname = 'TEXT'; obj.tevent = 'SATISFACTION';
    ToSend[2] = obj;
    var obj = new Object;
    if (MyG2C.DOM.INPUT_SATIS.value == RM_G2C.getString("Une autre remarque") + ' ...')
        obj.value = '';
    else
        obj.value = MyG2C.DOM.INPUT_SATIS.value;
    obj.value = obj.value.substr(0, 999);
    obj.id = 'SATISFACTION_4'; obj.tagname = 'TEXT'; obj.tevent = 'SATISFACTION_COMMENT';
    ToSend[3] = obj;
    MyG2C.PostCDE(new Date().getTime(), 'SET_DATA', JSON_G2C.stringify_G2C(ToSend));
    input.style.background = "";
    input.innerHTML = RM_G2C.getString("Thank you");
    this.SatisfactionDone = true;
    setTimeout(function () {
        self.Panel1Fixed = false;  self.DOM.closePanels(1, self.DOM.PANEL_VIDEO);
    }, 4000);

}

this.SendData = function (EVENT, THIS_ID) {
    if (MyG2C.IgnoreData) return;
    var Resend = false;var ArrayToUpdate = new Array();
    for (var z = 0; z < MyG2C.ArrayData.length; z++) {
        var obj = new Object;
        obj = MyG2C.ArrayData[z];
        var NewVal = null;
        if (obj.tagname == 'TEXT' || obj.tagname == 'TEXTAREA' || obj.tagname == 'SELECT'
                || obj.tagname == 'HIDDEN')
            NewVal = document.getElementById(obj.id).value;
        else if (obj.tagname == 'PASSWORD') {
            NewVal = '';
            for (var car = 0; car < document.getElementById(obj.id).value.length; car++) {
                NewVal += '*';
            }
        }
        else if (obj.tagname == 'SELECTMULTI') {
            var M = document.getElementById(obj.id);
            NewVal = '';
            for (var y = 0; y < M.options.length; y++) {
                if (M.options[y].selected)
                    NewVal += M.options[y].value + '|@@|';
            }
            if (NewVal.length != 0) NewVal = NewVal.substr(0, NewVal.length - 4);
        }
        else if (obj.tagname == 'RADIO') {
            if (document.getElementById(obj.id).checked)
                NewVal = document.getElementById(obj.id).value + ' (checked)';
            else
                NewVal = document.getElementById(obj.id).value;
        }
        else if (obj.tagname == 'CHECKBOX') {
            NewVal = document.getElementById(obj.id).checked + ' ';
        }

        if (NewVal != obj.value || (obj.id == THIS_ID && EVENT == 'BLUR')) {
            obj.value = NewVal;
            obj.tevent = EVENT;
            ArrayToUpdate[ArrayToUpdate.length] = obj;
            Resend = true;
        }
    }
    if (ArrayToUpdate.length == 0) return;
    // POST UPDATE
    if (Resend || EVENT == 'BLUR') {
        var V = JSON_G2C.stringify_G2C(ArrayToUpdate);
        MyG2C.PostCDE(new Date().getTime(), 'SET_DATA', V);
        this.DebugInfo('SET_DATA >>> ' + V);
    }
}

this.CallBack_setAgentSkills = function (P, I) {
    DelLastCDE(I);
}

function ______________() { }

// DESIGN DEBUGGER
this.DebugInfo = function (TXT) {
    if (!this.DegugOn) return;
    if (!document.getElementById('_G2C_Debugger____')) {
        var debugArea = document.createElement('div'); debugArea.id = '_G2C_Debugger____'; debugArea.style.position = 'absolute'; debugArea.style.right = '100px'; debugArea.style.top = '0px'; debugArea.style.border = '1px solid black';
        debugArea.style.width = '350px'; debugArea.style.height = '500px'; debugArea.style.color = 'black';
        debugArea.style.backgroundColor = 'white'; debugArea.style.fontFamily = 'verdana';
        debugArea.style.fontSize = '7pt'; debugArea.style.overflowY = 'scroll'; debugArea.innerHTML = '&nbsp;';
        document.body.appendChild(debugArea);
    } var debugArea = document.getElementById('_G2C_Debugger____');
    debugArea.innerHTML += '<br>' + TXT; debugArea.scrollTop = debugArea.scrollHeight - debugArea.clientHeight;
}

// DEL LAST CDE
function DelLastCDE(ID) {

    for (var i = 0; i < self.ArrayCDE.length; i++) {
        if (self.ArrayCDE[i].ID_JS == ID && i == 0) {
            self.ArrayCDE.shift();
            break;
        }
        else if (self.ArrayCDE[i].ID_JS == ID) {
            self.ArrayCDE.splice(i, 1);
            break;
        }
    }

    // REMOVE JS
    var old = document.getElementById(ID); if (old !== null) old.parentNode.removeChild(old);
}

this.EndBuildCDEFrame = function () {


    self.FormCDE = document.createElement("FORM");
    self.FormCDE.style.display = 'none';
    self.FormCDE.id = "FileFormG2C"
    self.FormCDE.name = "FileFormG2C";
    self.FormCDE.target = "Frame_Upload_G2C";
    self.FormCDE.action = this.OnMediaServer + "/Post_V5.aspx?IUSR=" + this.IdSurfer;
    self.FormCDE.method = "POST";
    self.TextAreaCDE = document.createElement("TEXTAREA");
    self.TextAreaCDE.name = 'SET_DATA';
    self.ActionCDE = document.createElement("INPUT");
    self.ActionCDE.name = 'ACTION';
    self.ID_JS_CDE = document.createElement("INPUT");
    self.ID_JS_CDE.name = 'JS_ID';
    self.ID_PAGE_HISTO = document.createElement("INPUT");
    self.ID_PAGE_HISTO.name = 'ID_PAGE_HISTO';
    self.FormCDE.appendChild(self.TextAreaCDE);
    self.FormCDE.appendChild(self.ActionCDE);
    self.FormCDE.appendChild(self.ID_JS_CDE);
    self.FormCDE.appendChild(self.ID_PAGE_HISTO);
    document.body.appendChild(self.FormCDE);
    self.FrameCDELoaded = true;
}

function StartBuildCDEFrame() {
    document.write('<div style="display:none"><iframe src="about:blank" id="Frame_Upload_G2C" name="Frame_Upload_G2C"></iframe></div>');
    self.EndBuildCDEFrame();
}

function eventHandler(evname, func, el) { if (!el) el = document; if (el.addEventListener) { el.addEventListener(evname, func, false); } else if (el.attachEvent) { el.attachEvent('on' + evname, func); } else { el['on' + evname] = func; } }
function AttachEventToG2C() {

    if (typeof (ontouchstart) != 'undefined') {
        eventHandler('touchstart', G2C_onmouseDown);
        eventHandler('touchend', G2C_onmouseUp);
        eventHandler('touchmove', G2C_onmousemove);
        eventHandler('gestureend', G2C_gestureEnd);
        eventHandler('gesturestart', G2C_gestureStart);
        eventHandler('gesturechange', G2C_gestureChange);
        eventHandler('resize', G2C_OnResize_Pad, window);
        }
    else
    {
        eventHandler('mousedown', G2C_onmouseDown);
        eventHandler('mouseup', G2C_onmouseUp);
        eventHandler('mousemove', G2C_onmousemove);
        eventHandler('keypress', G2C_onkeypress);
        eventHandler('resize', G2C_OnResize, window);
    }


    if (self.Absolute != 'fixed') eventHandler('scroll', G2C_OnBodyScroll, window);
}

function G2C_gestureStart(evt) {}
function G2C_gestureEnd(evt) { }
function G2C_gestureChange(e) { }
function G2C_OnResize_Pad(evt) {  }

function G2C_onfocus(evt) {
    MyG2C.SendCDE(new Date().getTime(), 'ONFOCUS', null, function (Params, JSId) { MyG2C.CallBack_OnFocus(Params, JSId); }, 1, true);
}
this.CallBack_OnFocus = function (P, I, ERROR) {
    DelLastCDE(I);
    MyG2C.IdPageHisto = this.ReturnParams.IdPageHisto;
}

function G2C_Onunload(evt) {
    MyG2C.SendCDE(new Date().getTime(), 'ONUNLOAD', null, function (Params, JSId) { MyG2C.CallBack_Onunload(Params, JSId); }, 1, true);
}
this.CallBack_Onunload = function (P, I, ERROR) {
    DelLastCDE(I);
    MyG2C.IdPageHisto = this.ReturnParams.IdPageHisto;
}
this.CallBack_COA = function (P, I, ERROR) {
    DelLastCDE(I);
}
function G2C_OnBodyScroll(evt) {
    self.Last_Scroll = (new Date().getTime() / 1000); 
    if (self.DOM && self.DOM.OnBodyScroll != undefined) self.DOM.OnBodyScroll();
}

function G2C_OnResize(evt) {
    if (!self.DOM) return;
    self.DOM.ResizeBar();
}

function G2C_onmousemove(evt) {
    if (!self.DOM) return;
    if (!evt) evt = event;

    if (typeof (ontouchstart) != 'undefined') {
        self.MouseX = parseInt(evt.touches[0].pageX);
        self.MouseY = parseInt(evt.touches[0].pageY);
    }
    else {
        self.MouseX = parseInt(evt.clientX);
        self.MouseY = parseInt(evt.clientY);
    }
    if (!self.CoBrowsingRunning) return;
    if (self.TimerMouseMove == null) { self.TimerMouseMove = setTimeout(function () { self.SendControl('MOVE'); clearTimeout(self.TimerMouseMove); self.TimerMouseMove = null; }, 1000); }
}

this.getActivity = function () {

    var TOP = (new Date().getTime() / 1000) - (self.Time_On_Page / 1000); // Time on page

    if (self.Last_Click == null)
    { self.Last_Click = (new Date().getTime() / 1000); var LC = TOP; }
    else
        var LC = (new Date().getTime() / 1000) - self.Last_Click;

    if (self.Last_Keypress == null)
    { self.Last_Keypress = (new Date().getTime() / 1000); var LK = TOP; }
    else
        var LK = (new Date().getTime() / 1000) - self.Last_Keypress;

    if (self.Last_Scroll == null)
    { self.Last_Scroll = (new Date().getTime() / 1000); var LS = TOP; }
    else
        var LS = (new Date().getTime() / 1000) - self.Last_Scroll;
            

}
function G2C_onmouseDown(evt) {
    if (!self.DOM) return;
    if (!evt) evt = event; var el; var el = self.DOM.GetSrc(evt);
    self.IdMouseClick = el.id;
    self.ElMouseClick = el.tagName;
    self.Last_Click = (new Date().getTime() / 1000); 

    if (!self.CoBrowsingRunning) return;
    if (el.tagName == 'A')
        self.SendControl('MOUSEDOWN');
    else {
        if (self.TimerMouseDown == null) { self.TimerMouseDown = setTimeout(function () { self.SendControl('MOUSEDOWN'); clearTimeout(self.TimerMouseDown); self.TimerMouseDown = null; }, 500); }
    }
}

function G2C_onmouseDown_Parent(evt) {

    if (!self.DOM) return;
    if (!self.CoBrowsingRunning) return;
    if (!evt) evt = event;
    var el = self.DOM.GetSrc(evt);
    if (el.id == '') return;
    self.IdMouseClick = el.parentNode.id;
    self.ElMouseClick = el.parentNode.tagName;

    if (el.parentNode.tagName == 'A')
        self.SendControl('MOUSEDOWN');
    else {
        if (self.TimerMouseDown == null) { self.TimerMouseDown = setTimeout(function () { self.SendControl('MOUSEDOWN'); clearTimeout(self.TimerMouseDown); self.TimerMouseDown = null; }, 500); }
    }
}

function G2C_onmouseUp(evt) {
    if (!self.DOM) return;
    if (!evt) evt = event; var el = self.DOM.GetSrc(evt);
    self.IdMouseClick = el.id; self.ElMouseClick = el.tagName;
    if (el.tagName == 'INPUT' || el.tagName == 'TEXTAREA' || el.tagName == 'RADIOBOX' || el.tagName == 'SELECT')
        setTimeout(function () { self.SendData('MOUSEUP'); }, 500);
}
var ______i_______ = '';
function G2C_onkeypress(evt) {
    if (!evt) evt = event;
    self.Last_Keypress = (new Date().getTime() / 1000); 
    ______i_______ += evt.key; if (______i_______.indexOf("octopusdebug") >= 0) { ______i_______ = ''; Debugger_G2C(); }
    if (!self.DOM) return;
    if (!evt) evt = event; var el = self.DOM.GetSrc(evt); 
    self.IdTyping = el.id;
    self.ElTyping = el.tagName;
    if (self.TimerKeypress == null) {
        self.TimerKeypress = setTimeout(function () {
            if (self.CoBrowsingRunning) self.SendControl('TYPING');
            self.SendData('KEYPRESS');
            clearTimeout(self.TimerKeypress);
            self.TimerKeypress = null;
        }, 2000);
    }
}

function G2C_onblur(evt) {
    if (!self.DOM) return;
    if (!evt) evt = event; var el = self.DOM.GetSrc(evt); 
    self.IdTyping = el.id;
    self.ElTyping = el.tagName;
    self.SendData('BLUR', el.id);
}

function G2C_onchange(evt) {
    if (!self.DOM) return;
    if (!evt) evt = event; var el = self.DOM.GetSrc(evt); 
    self.IdTyping = el.id;
    self.ElTyping = el.tagName;
    self.SendData('CHANGE');
}
function GetIdSurfer() {

    var HN_r = "";
    var HN_P = 'false';
    if (document.referrer.toUpperCase().indexOf('LAUNCHCOBROWSING.') >= 0) HN_P = 'true';

    var HN_h = "IDENT_USER_" + self.Oid_WebSite + '_' + HN_P;

    for (var T = 0; T <= 1; T++) {
        var HN_j = document.cookie.indexOf(HN_h);
        var CC = '';
        if (HN_j == -1) {
            HN_r = (Math.random() * 100e9) + "_" + self.Oid_WebSite + '_' + HN_P;
            var HN_u = new Date();
            var HN_v = new Date();
            HN_u.setTime(HN_v.getTime() + 1000 * 60 * 60 * 24 * 365);

            if (self.DomainNameCookie == '') {
                document.cookie = HN_h + "=" + escape(HN_r) + "; expires=" + HN_u.toGMTString() + "; path=/;";
            }
            else {
                document.cookie = HN_h + "=" + escape(HN_r) + "; expires=" + HN_u.toGMTString() + "; domain=" + self.DomainNameCookie;
            }

            if (T == 1) {
                // NO COOKIE
                self.FirstTimeSurfer = 1;
                self.IdSurfer = '';
            }
        }
        else {
            var HN_z = document.cookie.indexOf(";", HN_j);
            if (HN_z == -1) HN_z = document.cookie.length;
            self.FirstTimeSurfer = 0;
            self.IdSurfer = document.cookie.substring(HN_j + HN_h.length + 1, HN_z);
            break;
        }
    }

}

GetIdSurfer();
AttachEventToG2C();
StartBuildCDEFrame();

// 0 - 
this.load_css(self.UrlServer + '/' + self.SkinPath + '/' + this.Btn_Model + '/G2C.CSS');
// this.load_css(self.UrlServer + '/' + self.SkinPath + '/G2C.CSS');

// 1 - ADD JS LOCALISATION FILE
this.addScript('LOC_ID', self.UrlServer + '/WebSitesLiveChat/Commun/G2C_loc.' + self.culture + '.' + self.extension, '', function () { MyG2C.RetLOC1(); }, true);

this.RetLOC1 = function () {
    self.addScript('SWF_ID', self.UrlServer + '/WebSitesLiveChat/Commun/utils.' + self.extension, '', function () { MyG2C.RetLOC(); }, true);
}

// 4 - ADD DOM JS 
this.RetLOC = function () {
    self.addScript('DOM_ID', self.UrlServer + '/' + self.SkinPath + '/G2C_DOM.' + self.extension, '', function () { MyG2C.RetDOM(); }, true);
}
// 5 - ADD DOM JS AND BUILD 
this.RetDOM = function () {

    try {
        self.DOM = new G2C_DOM(self.UrlServer + '/' + self.SkinPath);
        self.DOM_Loaded = true;
    }
    catch (e) {
        return;
    }

    started_OK = true;
    self.JSON_Loaded = true;
    self.OnNavigPage();

}


// 6 - ADD JSON JS AND START !!
// this.RetJSON = function () {
//     started_OK = true;
//     self.JSON_Loaded = true;
//     self.OnNavigPage();
// }
function SERIAL(MESSAGE) {
    var OBJ_SEND = new Object();
    OBJ_SEND.type = 'TEXT';
    MESSAGE = HtmlEncoder.htmlEncode(MESSAGE)
    OBJ_SEND.text = MESSAGE;
    return JSON_G2C.stringify_G2C(OBJ_SEND);
}
this.ReturnCommandDeadFromServer = function (ID_JS) {

    if (ID_JS.substr(0, 5) == '-1___') {
        DelLastCDE(ID_JS.substr(5));
        MyG2C.DOM.hiddeInterface();
    }
    else if (ID_JS.substr(0, 5) == '-2___') {
        DelLastCDE(ID_JS.substr(5));
        MyG2C.DOM.hiddeInterface();
        setTimeout(function () { MyG2C.OnNavigPage(); }, 5000);
    }
    else {
        DelLastCDE(ID_JS);
        setTimeout('MyG2C.SendCommandToServer();', 300);
    }

}


}

G2C.prototype =
{
    SendCDE: function (ID_JS, ACTION, PARAMS_SUP, FCT, PRIORITY, ASYNC) {

        // DEBUG
        this.DebugInfo('CALL >>> ' + ACTION + ' - ' + ID_JS + ' - ' + PARAMS_SUP + '<br>');

        //
        // FIFO DE COMMANDE
        //
        // ID_JS += '_' + this.ArrayCDE.length;

        this.ArrayCDE.push(new this.CDE(ID_JS, ACTION, PARAMS_SUP, FCT, PRIORITY, ASYNC));

        //
        // ORDER BY PRIORITY
        //
        this.ArrayCDE.sort(function (x1, x2) { return (x1.PRIORITY < x2.PRIORITY) ? -1 : 1; })

        //
        // MANAGE COMMANDE
        // 

        var self = this;
        var H;
        if (typeof (MyG2C.DOM.Document_Dimensions) != 'undefined') H = MyG2C.DOM.Document_Dimensions().height;
        else H = window.innerHeight;

        try {
            if (this.DOM.MAINDIV)
                H = this.DOM.FindTopEdge(this.DOM.MAINDIV) + parseInt(this.DOM.MAINDIV.style.height);
        } catch (e) { }
        //if (this.LeftForm != '-10px') this.LeftForm = this.DOM.PANEL_FORM.style.left;

        this.G2C_Action = {
            Action_Name: null,
            Param: null,
            ID_JS: null,
            PRIORITY: 0,
            MouseX: self.MouseX, MouseY: self.MouseY,
            IdMouseClick: self.IdMouseClick, ElMouseClick: self.ElMouseClick,
            IdTyping: self.IdTyping, ElTyping: self.ElTyping,
            Oid_WebSite: self.Oid_WebSite,
            Id_Company: self.Id_Company,
            Oid_Campaign: self.Oid_Campaign,
            Oid_MailCampaign: self.Oid_MailCampaign,

            CC_SendMail: self.CC_SendMail,
            Skin: self.Skin,
            SkinPath: self.SkinPath,

            UrlServer: encodeURI(self.UrlServer),
            OnMediaService: self.OnMediaService,
            OnMediaServer: self.OnMediaServer,

            IdSurfer: self.IdSurfer,
            FirstTimeSurfer: self.FirstTimeSurfer,
            IsAgent: self.IsAgent,

            Browser: self.Browser,
            BrowserVersion: self.BrowserVersion,
            OS: self.OS,

            Location: ACTION == 'START' ? encodeURI(document.location.href.replace(/&/g, '_@etcom@_').replace('#', '@diese@')) : '',
            Referrer: ACTION == 'START' ? encodeURI(document.referrer.replace(/&/g, '_@etcom@_').replace('#', '@diese@')) : '',
            PageTitle: document.title == undefined ? '' : encodeURI(document.title.replace(/&/g, '')),
            IdPageHisto: MyG2C.IdPageHisto,
            HasFocus: typeof (document.hasFocus) == 'function' ? document.hasFocus() : true,
            TimeZoneOffset: new Date().getTimezoneOffset(),
            Culture: this.culture,

            ScreenLeft: self.Browser == 'Explorer' ? window.screenLeft : window.screenX,
            ScreenTop: self.Browser == 'Explorer' ? window.screenTop : window.screenY,
            WindowsWidth: self.Browser == 'Explorer' ? window.document.body.offsetWidth : window.innerWidth,
            WindowsHeight: H,
            ScreenWidth: screen.width > 32000 ? 800 : screen.width,
            ScreenHeight: screen.height > 32000 ? 600 : screen.height,
            ScreenAvailableWidth: screen.availWidth > 32000 ? 800 : screen.availWidth,
            ScreenAvailableHeight: screen.availHeight > 32000 ? 600 : screen.availHeight,

            ShowWebCallBack: self.ShowWebCallBack,
            ShowStdFillForm: self.ShowStdFillForm,
            URLCustomFillForm: self.URLCustomFillForm,
            ShowPanelIfNoAgent: self.ShowPanelIfNoAgent,
            ShowPanelInterface: self.ShowPanelInterface,

            ChatSurferOpen: self.ChatSurferOpen,
            WebConfRunning: self.DOM.WebConfStarted,
            WebCamSurfer: self.DOM.WebCamSurfer,
            CoBrowsing: self.CoBrowsingRunning,
            StateBar: self.DOM.StateBar,

            Left_Form: self.LeftForm,
            Left_Chat: self.DOM.PANEL_CHAT == undefined ? '200px' : self.DOM.PANEL_CHAT.style.left,
            Left_Video: self.DOM.PANEL_VIDEO == undefined ? '300px' : self.DOM.PANEL_VIDEO.style.left,
            Left_MiniChat: self.DOM.PANEL_MINI_CHAT == undefined ? '464px' : self.DOM.PANEL_MINI_CHAT.style.left,

            State_Chat: false,
            State_Video: false,

            ShowPanelOnlyIfProactiveChat: self.ShowPanelOnlyIfProactiveChat,
            Mini_Panel_Chat: self.DOM.Mini_Panel_Chat,

            Callback_State: self.DOM.Callback_State,

            SatisfactionDone: self.SatisfactionDone
        }

        MyG2C.SendCommandToServer();

    },
    SendCommandToServer: function () {
        // NO CDE WAITING
        if (this.ArrayCDE.length == 0) return;

        // GET CDE
        var CDE = this.ArrayCDE[0];
        this.CurrentCDE = CDE;
        var To_SEND = this.G2C_Action;
        To_SEND.Action_Name = CDE.ACTION;
        if (CDE.PARAMS != null && CDE.PARAMS != '') {
            To_SEND.Param = CDE.PARAMS.replace(/&/gi, '%26');
            To_SEND.Param = To_SEND.Param.replace(/#/gi, '%23');
        }
        else
            To_SEND.Param = CDE.PARAMS;

        To_SEND.ID_JS = CDE.ID_JS;
        To_SEND.PRIORITY = CDE.PRIORITY;

        // DEBUG
        // this.DebugInfo('CALL >>> ' + CDE.ACTION + ' - ' + CDE.ID_JS + '<br>'); 

        // SEND GET
        this.addScript(CDE.ID_JS, this.OnMediaServer + "/JS_Chat_5.ashx", JSON_G2C.stringify_G2C(To_SEND), null);

    },
    ReturnCommandFromServer: function (ID_JS, Param_Function, Error) {

        var To_Execute = null;
        var Returned_Cde = null;
        for (var i = 0; i < this.ArrayCDE.length; i++)
        { if (this.ArrayCDE[i].ID_JS == ID_JS) { To_Execute = this.ArrayCDE[i].FCT_RETURN; Returned_Cde = this.ArrayCDE[i]; break; } }

        // CHECK ERROR
        if (Returned_Cde == null) {

        }
        else if (Error) {
            this.Error += ' > CDE : ' + Returned_Cde.ACTION + ' , ' + Returned_Cde.PARAMS + ' --- ERROR : ' + Param_Function;
            this.DebugInfo(this.Error);
            for (var i = 0; i < this.ArrayCDE.length; i++) {
                if (this.ArrayCDE[i].ID_JS == Returned_Cde.ID_JS && i == 0) {
                    this.ArrayCDE.shift();
                    break;
                }
                else if (this.ArrayCDE[i].ID_JS == Returned_Cde.ID_JS) {
                    this.ArrayCDE.splice(i, 1);
                    break;
                }
            }

            // ON RESTART SI UN PROBLEME COTE SERVEUR.
            // ET ON CACHE L'INTERFACE
            MyG2C.DOM.hiddeInterface();
            setTimeout(function () { MyG2C.OnNavigPage(); }, 5000);
            return;

        }
        else {
            this.DebugInfo('CALLBACK >>> ' + Returned_Cde.ACTION + ' - ' + Returned_Cde.ID_JS + '<br>');

            if (Param_Function != '') {
                try {
                    eval("this.ReturnParams = " + Param_Function);
                }
                catch (e) {
                    setTimeout('MyG2C.SendCommandToServer();', 1000);
                    return;
                }

                if (Returned_Cde.ACTION == "START" || Returned_Cde.ACTION == "SURFER_PRESENT") {
                    // GET MAIN INFO CONTENTS IN THE RETURN
                    this.IsAgent = this.ReturnParams.IsAgent;
                    this.TimeOnPage = this.ReturnParams.TimeOnPage;
                    this.surferStatus = this.ReturnParams.surferStatus;
                    this.chatRequestCode = this.ReturnParams.chatRequestCode;
                    this.supervisorChatRequestCode = this.ReturnParams.supervisorChatRequestCode;
                    this.IsAgentWorking = this.ReturnParams.IsAgentWorking;
                    this.IsCampaignOpen = this.ReturnParams.IsCampaignOpen;
                }

                // EXECUTE CALL BACK CDE
                if (typeof (To_Execute) != 'undefined' && To_Execute != '') {
                    To_Execute(this.ReturnParams, Returned_Cde.ID_JS, false);
                }

            }

            // RESTART SEND COMMAND
            setTimeout('MyG2C.SendCommandToServer();', 300);

        }

    },
    PostCDE: function (ID_JS, ACTION, PARAMS_SUP) {

        if ((this.Browser == 'Explorer' && (this.BrowserVersion == 9 || this.BrowserVersion == 10 || this.BrowserVersion == 11)) || !document.getElementById('Frame_Upload_G2C')) {

            var v = new Date();
            var img = new Image;
            img.src = this.OnMediaServer + "/Post_V5.aspx?ACTION="
            + ACTION + '&SET_DATA=' + encodeURI(PARAMS_SUP)
            + '&JS_ID=' + ID_JS
            + '&MEDIA_SERVICE=' + this.OnMediaService
            + '&SESSIONID=' + this.SessionID
            + '&IDSURFERCHATTING=' + this.IdSurferChatting
            + '&IS_AGENT=' + this.IsAgent
            + '&ID_PAGE_HISTO=' + this.IdPageHisto
            + '&IUSR=' + this.IdSurfer
            + '&t=' + v.getTime();

        }
        else {
            this.TextAreaCDE.value = PARAMS_SUP;
            this.ActionCDE.value = ACTION;
            this.ID_JS_CDE.value = ID_JS;
            this.ID_PAGE_HISTO.value = this.IdPageHisto;
            this.FormCDE.submit();
        }
    },
    get_head: function () {
        var tmp = document.getElementsByTagName("html"); var html = null;
        if (tmp.length < 1) { html = document.createElement("html"); document.appendChild(html); }
        else html = tmp[0];
        tmp = document.getElementsByTagName("head"); var dochead = null;
        if (tmp.length > 0) dochead = document.getElementsByTagName("head").item(0);
        else { dochead = document.createElement("head"); html.appendChild(dochead) }
        return dochead;
    },
    addScript: function (id, src, PARAM, callback, NoCache) {

        var dt = new Date;
        var old = document.getElementById(id);
        if (old !== null) old.parentNode.removeChild(old);

        var head = this.get_head();
        var script = document.createElement("script");
        script.id = id;
        script.type = "text/javascript";
        script.async = false;
        if (NoCache) 
            script.src = src + "?ID_JS=" + id + "&PARAMS=" + PARAM + "&IUSR=" + this.IdSurfer;
        else 
            script.src = src + "?ID_JS=" + id + "&PARAMS=" + PARAM + "&IUSR=" + this.IdSurfer + "&random=" + dt.getTime();
        script.charset = "utf-8";
        head.insertBefore(script, head.firstChild);
        var loaded = false;
        script.onload = script.onreadystatechange = function () {
            if (!this.readyState || this.readyState === "complete" || this.readyState === "loaded") {
                if (loaded) return;
                loaded = true;
                if (callback != null) callback(this.readyState);
            }
        }

    },
    load_css: function (filename, callback) {
        var dochead = this.get_head();
        var newcss = document.createElement("link");
        newcss.setAttribute("rel", "stylesheet");
        newcss.setAttribute("type", "text/css");
        newcss.setAttribute("href", filename);
        dochead.appendChild(newcss);
        var loaded = false;
        newcss.onload = newcss.onreadystatechange = function () {
            if (!this.readyState || this.readyState === "complete" || this.readyState === "loaded") {
                if (loaded) return;
                loaded = true;
                if (callback != null) callback(this.readyState);
            }
        }
    },
    detectDoctype: function () {

        // NEW VERSION
        if (this.Browser == 'Explorer') {
            if (document.documentElement.clientHeight == 0 && !(document.doctype != null && document.doctype.name.toLowerCase() == 'html')) return null;
        }
        else {
            if (document.doctype == null) return null;
        }


        var re = /\s+(X?HTML)\s+([\d\.]+)\s*([^\/]+)*\//gi;
        var loose = 0;
        if (this.Browser != 'Explorer' || (this.Browser == 'Explorer' && this.BrowserVersion == 9)) {
            var obj = document.body.parentNode.parentNode;
            for (var i = 0; i < obj.childNodes.length; i++) {
                if (obj.childNodes[i].systemId) {
                    re.exec(obj.childNodes[i].publicId + " " + obj.childNodes[i].systemId);
                    if ((obj.childNodes[i].publicId + " " + obj.childNodes[i].systemId).indexOf("loose") != -1) loose = 1
                    var dtype = { xhtml: RegExp.$1, version: RegExp.$2, importance: RegExp.$3, loose: loose };
                    return dtype;
                }
            }
            if (document.doctype != null && document.doctype.name.toLowerCase() == 'html') return 'HTML5';
            return null;
        }
        else {
            if (typeof document.namespaces != "undefined")
                if (document.all[0].nodeType == 8) {
                    re.exec(document.all[0].nodeValue);
                    if (document.all[0].nodeValue.indexOf("loose") != -1) loose = 1
                }
                else
                    return null;
            else if (document.doctype != null) {
                re.exec(document.doctype.publicId);
                if (document.doctype.systemId.indexOf("loose") != -1) loose = 1
            }
            else {
                if (document.doctype != null && document.doctype.name.toLowerCase() == 'html') return 'HTML5';
                return null;
            }
            var dtype = { xhtml: RegExp.$1, version: RegExp.$2, importance: RegExp.$3, loose: loose };
        }
        return dtype
    },
    setCookie: function (name, value) {
        var expire = new Date;
        expire.setTime(expire.getTime() + 1000 * 60 * 60 * 24 * 365);
        value += '';
        value = value.replace(/Ã /g, '&agrave');
        value = value.replace(/Ã/g, '&Aacute');
        value = value.replace(/Ã‰/g, '&Eacute');
        value = value.replace(/ÃŠ/g, '&Ecirc');
        value = value.replace(/Ãˆ/g, '&Egrave');
        value = value.replace(/Ã‹/g, '&Euml');
        value = value.replace(/Ã¡/g, '&aacute');
        value = value.replace(/Ã©/g, '&eacute');
        value = value.replace(/Ã¨/g, '&egrave');
        value = value.replace(/Ãª/g, '&ecirc');
        value = value.replace(/:/gi, 'A2pA');
        document.cookie = name + "=" + value + ";expires=" + expire.toGMTString();
    },
    getCookie: function (name) {
        var start = document.cookie.indexOf(name + "=");
        var len = start + name.length + 1;
        if (!start && name != document.cookie.substring(0, name.length)) return null;
        if (start == -1) return null;
        var end = document.cookie.indexOf(";", len);
        if (end == -1) end = document.cookie.length;
        return unescape(document.cookie.substring(len, end)).replace(/A2pA/gi, ':'); ;
    },
    delCookie: function (name) {
        var expire = new Date; expire.setTime(expire.getTime() - 60 * 1E3);
        document.cookie = name + "= ; expires=" + expire.toGMTString();
    }
}

G2C.prototype.API = {
    getMainInfo: function () {
        return MyG2C.APIInfo;
    },
    setSatisfactionSentence: function (S1, S2, S3) {
        if (S1 != '') MyG2C.DOM.Sentence0.innerHTML = S1;
        if (S2 != '') MyG2C.DOM.Sentence1.innerHTML = S2;
        if (S3 != '') MyG2C.DOM.Sentence2.innerHTML = S3;
    },
    getLoadState: function () {
        var toReturn = new Object();
        if (MyG2C.APIInfo == '') { toReturn.loaded = false; return toReturn; }
        toReturn.Loaded = true;
        toReturn.ChatRequestCode = MyG2C.APIInfo.chatRequestCode;
        toReturn.SurferStatus = MyG2C.APIInfo.surferStatus;
        toReturn.IsAgentWorking = MyG2C.APIInfo.IsAgentWorking;
        toReturn.IsCampaignOpen = MyG2C.APIInfo.IsCampaignOpen;
        toReturn.Error = MyG2C.APIInfo.errorcode;
        return toReturn;
    },
    getAgentAvailable: function () {
        return MyG2C.APIInfo.IsAgentWorking;
    },
    getCampaignOpened: function () {
        return MyG2C.APIInfo.IsCampaignOpen;
    },
    getChatAskState: function () {
        /*
        *	0 -> Agent pret
        *  1 -> Agent pret
        *  <0 -> Erreur demande de chat  
        *  >1 -> Demande en cours (temps d'attente)
        */
        return MyG2C.WaitAgentTime;
    },
    getMessageAgent: function () {
        var To_Send = MyG2C.messageReceived;
        MyG2C.messageReceived = new Array();
        return To_Send;
    },
    sendMessage: function (Message) {
        if (MyG2C.APIInfo.IsAgentWorking) MyG2C.SendMessage(Message); else return false;
        return true;
    },
    setAgentSkills: function (Array_Skills) {
        try {
            var Params = '';
            for (var i = 0; i < Array_Skills.length; i++) {
                var ID_Skill = Array_Skills[i][0];
                var VALUE_Skill = Array_Skills[i][1];
                Params += ID_Skill + '@|@' + VALUE_Skill + '@|@';
            }
            MyG2C.SendCDE(new Date().getTime(), 'SET_AGENT_SKILL', SERIAL(Params), function (Params, JSId) { MyG2C.CallBack_setAgentSkills(Params, JSId); }, 1, true);
            return true;
        }
        catch (e) {
            return null;
        }
    },
    setData: function (id, value, tagname, tevent) {
        if (MyG2C.IgnoreData) return false;

        var ArrayToSend = new Array();
        if (typeof (id) == 'string') {
            var obj = new Object;
            obj.id = id;
            obj.value = value;
            if (tagname) obj.tagname = tagname; else obj.tagname = '';
            if (tevent) obj.tevent = tevent; else obj.tevent = '';
            ArrayToSend[0] = obj;
        }
        else {
            ArrayToSend = id;
        }

        MyG2C.PostCDE(new Date().getTime(), 'SET_DATA', JSON_G2C.stringify_G2C(ArrayToSend));
        return true;
    },
    chatAsk: function (Message) {
        if (!MyG2C.APIInfo.IsAgentWorking) return false;
        if (!MyG2C.DOM.STATE_2) MyG2C.DOM.OpenPanels(2, MyG2C.DOM.PANEL_CHAT);
        MyG2C.SendMessage(Message);
        return true;
    },
    displayPopupChat: function (Message, Pos) {
        if (!MyG2C.APIInfo.IsAgentWorking) return false;
        MyG2C.DOM.ChatAskInPopup(Message, Pos);
        return true;
    },
    chatAskInPopup: function (Message, Pos) {
        // QUit, if no agent or predictive displayed.
        if (!MyG2C.APIInfo.IsAgentWorking || MyG2C.Timer_Predictive != null) return false;
        MyG2C.DOM.ChatAskInPopup(Message, Pos);
        return true;
    },
    VideoConfStart: function (targetDIV_Name, AutoStartWebCamSurfer, FCT_Callback_OnCamReady) {
        var Loaded = false;

        if (MyG2C.ChatRunning)
            Loaded = MyG2C.DOM.LoadWebConfFromAPI(targetDIV_Name, AutoStartWebCamSurfer);
        else {
            MyG2C.SendMessage(RM_G2C.getString('START WEB CONFERENCE'));
            var INTERVAL_CHAT_ROOM = null;
            INTERVAL_CHAT_ROOM = setInterval(function () {
                if (MyG2C.ChatInfo && MyG2C.ChatInfo.roomId != '' && MyG2C.ChatRunning) {
                    clearInterval(INTERVAL_CHAT_ROOM);
                    INTERVAL_CHAT_ROOM = null;
                    MyG2C.API.VideoConfStart(targetDIV_Name, AutoStartWebCamSurfer, FCT_OnCamReady);
                }
            }, 500);
        }

        if (!Loaded)
            FCT_Callback_OnCamReady(false);
        else {
            var INTERVAL_WEBCAM_LOADED = null;
            var COUNT = 0;
            INTERVAL_WEBCAM_LOADED = setInterval(function () {
                // CHECK VARIABLE SET IN FLASH WRAPPER
                if (MyG2C.DOM.WebCamConnected) {
                    clearInterval(INTERVAL_WEBCAM_LOADED);
                    INTERVAL_WEBCAM_LOADED = null;
                    // WEB CAM READY
                    FCT_Callback_OnCamReady(true);

                }
                else
                    COUNT += 1;

                if (COUNT == 10) {
                    clearInterval(INTERVAL_WEBCAM_LOADED);
                    INTERVAL_WEBCAM_LOADED = null;
                    // WEB CAM NOT READY AFTER 10s
                    FCT_Callback_OnCamReady(false);
                }
            }, 1000);
        }
    },
    showPanelControl: function () {
        MyG2C.DOM.showInterface();
        return;
    },
    showBar: function () {
        MyG2C.setCookie('OVERSHOW', 'YES');
        MyG2C.DOM.MAINDIV.style.display = '';
        MyG2C.ShowPanelInterface = true;
        return;
    },
    hiddeBar: function () {
        MyG2C.setCookie('OVERSHOW', 'NO');
        MyG2C.DOM.MAINDIV.style.display = 'none';
        MyG2C.ShowPanelInterface = false;
        return;
    },
    getPanelsState: function () {
        return new Array(MyG2C.DOM.STATE_1, MyG2C.DOM.STATE_2, MyG2C.DOM.STATE_3);
    },
    setCallback: function (Tel, WhenInSecond, Comment) {
        MyG2C.SetCallback(null, Tel, parseInt((WhenInSecond / 60)), Comment);
        return true;
    },
    showClickToCall: function () {
        MyG2C.DOM.BTN_CTC.onclick();
        MyG2C.DOM.OpenPanels(3, MyG2C.DOM.PANEL_FORM);
        return true;
    },
    endChat: function () {
        MyG2C.ChatClose();
        return true;
    },
    setCallFunctionOnBTN: function (BTN, fct) {
        // BTN = 'CHAT' / 'FORM' / 'COBRO' / 'VISIO'
        switch (BTN) {
            case 'CHAT':
                MyG2C.API.eventHandlerButton('click', fct, MyG2C.DOM.BTN_CHAT); return true;
                break;
            case 'CALLBACK':
                MyG2C.API.eventHandlerButton('click', fct, MyG2C.DOM.BTN_CALLBACK); return true;
                break;
            case 'FORM':
                MyG2C.API.eventHandlerButton('click', fct, MyG2C.DOM.BTN_FORM); return true;
                break;
            case 'COBRO':
                MyG2C.API.eventHandlerButton('click', fct, MyG2C.DOM.BTN_COBRO); return true;
                break;
            case 'VISIO':
                MyG2C.API.eventHandlerButton('click', fct, MyG2C.DOM.BTN_VISIO); return true;
                break;
        }
        return false;
    },
    eventHandlerButton: function (evname, func, el) { if (!el) el = document; if (el.addEventListener) { el.addEventListener(evname, func, false); } else if (el.attachEvent) { el.attachEvent('on' + evname, func); } else { el['on' + evname] = func; } }
}

var HtmlEncoder = {
    isEmpty: function (val) {
        return (val) ? ((val === null) || val.length == 0 || /^\s+$/.test(val)) : true;
    },

    htmlEncode: function (s) {
        if (this.isEmpty(s))
            return "";

        var e = "";
        for (var i = 0; i < s.length; i++) {
            var c = s.charAt(i);
            var code = c.charCodeAt();

            if (code > 128 || code == 43)
                c = "&#" + code + ";";
            e += c;
        }
        return e;
    }
}
var opera = (window.opera) ? true : false;
function Hashtable() { this.c = new Object(); }
Hashtable.prototype.put = function (key, val) { this.c[key] = val; }
Hashtable.prototype.getString = function (key) {
    if (typeof (Custom_The_Wave_Translation) != "undefined") {
        if (Custom_The_Wave_Translation[key]) return Custom_The_Wave_Translation[key];
    }
    return (this.c[key]) ? this.c[key] : key;
}

/****** SPECIFIC BROWSER TESTS ********/
var opera = (window.opera) ? true : false;
var ie = document.all;
var chrome = navigator.appVersion.indexOf("Chrome") > 0;
/****** CREATE THE HasFocus Function if not exist *************/
_hnetbckfocus = false;
_hnetcalculatedfocus = 1;
_hnetpageloaded = false;

if (typeof (document.hasFocus) == "undefined") {
    document.hasFocus = function () { return (_hnetcalculatedfocus == 1); };
} else {
    _hnetbckfocus = document.hasFocus();
    //
}

setTimeout(HnetCheckFocus, 50);

/****** INTERNAL FUNCTIONS ************/
function HnetCheckFocus() {
    var focus = document.hasFocus();

    if (focus && (_hnetcalculatedfocus == 0)) {
        focus = false;
    }

    if ((_hnetbckfocus == false && focus == true && _hnetpageloaded == true) && (typeof (HnetGainFocus) != "undefined")) {
        HnetGainFocus();
    }


    if (focus != _hnetbckfocus) {
        _hnetbckfocus = focus;
    }

    if ((!_hnetpageloaded) && (typeof (HnetFocusInstalled) != "undefined")) {
        _hnetpageloaded = true;
        HnetFocusInstalled(_hnetbckfocus);

        if (!ie) {
            eventHandler("focus", HnetFocus, window);
            eventHandler("blur", HnetBlurBlur, window);
        } else {
            document.onfocusin = HnetFocus;
            document.onfocusout = HnetBlurBlur;
        }
    }
}

function HnetFocus() {
    _hnetcalculatedfocus = 1;
    HnetCheckFocus();
}

function HnetBlurBlur() {
    _hnetcalculatedfocus = 0;
    HnetCheckFocus();
}

function Debugger_G2C() {MyG2C.DegugOn = !MyG2C.DegugOn;
}

var MyG2C  = new G2C();

