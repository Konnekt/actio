/* 

(c)2005 Rafa³ Lindemann | Stamina

*/

#include "stdafx.h"

#include <Konnekt/Lib.h>

#include "actio.h"
#include "actio_main.h"

#include <Stamina\PhonoLogic\Window.h>
#include <Stamina\PhonoLogic\Defines.h>
#include <Stamina\Debug.h>
#include <Stamina\Exception.h>


IMPLEMENT_PURECALL_EXCEPTION("Actio.dll");

int __stdcall DllMain(void * hinstDLL, unsigned long fdwReason, void * lpvReserved)
{
	return true;
}

using namespace Actio;
using namespace Stamina;
using namespace PhonoLogic;

namespace Actio {

int Init() {
	if (ICMessage(IMC_VERSION) < VERSION_TO_NUM(0, 6, 22, 123)) {
		MessageBox(0, "Ta wtyczka dzia³a tylko z Konnektem w wersji co najmniej 0.6.22.123", "Actio", MB_ICONERROR);
		return 0;
	}
	Actio::init();
	return 1;
}

int DeInit() {
	return 1;
}

int IStart() {
	/* Tutaj wtyczkê uruchamiamy */
	Actio::start();
	return 1;
}
int IEnd() {
	/* Tutaj wtyczkê wy³¹czamy */
	Actio::finish();
	return 1;
}

Tables::oTable createArchiveTable(const CStdString& name) {
	using namespace Tables;
	Tables::oTable dt = Tables::registerTable(Ctrl, name);
	dt |= optAutoLoad;
	dt |= optAutoSave;
	dt->setColumn(Ctrl->getPlugin(), DTArchive::phone, ctypeString | cflagXor, "phone");
	dt->setColumn(Ctrl->getPlugin(), DTArchive::type, ctypeInt, "type");
	dt->setColumn(Ctrl->getPlugin(), DTArchive::start, ctypeInt64, "start");
	dt->setColumn(Ctrl->getPlugin(), DTArchive::duration, ctypeInt, "duration");
	return dt;
}

int ISetCols() {
	using namespace Tables;
	SetColumn(tableConfig, Actio::CFG::userName, ctypeString | cflagXor, "", "Actio/userName");
	SetColumn(tableConfig, Actio::CFG::userPass, ctypeString | cflagSecret | cflagXor, "", "Actio/userPass");
	//SetColumn(tableConfig, Actio::CFG::userPassMD5, ctypeString | cflagSecret | cflagXor, "", "Actio/userPassMD5");
	SetColumn(tableConfig, colByName, ctypeString | cflagXor, "", "Actio/sipUrl");
	SetColumn(tableConfig, Actio::CFG::publicAddress, ctypeString | cflagXor, "", "Actio/publicAddress");
	SetColumn(tableConfig, Actio::CFG::enableAtStartup, ctypeInt, 1, "Actio/enableAtStartup");

	CStdString prefix = ACTIO_CFG_PREFIX;
	
	// Phonologic...
	SetColumn(tableConfig, colByName, ctypeString, "", prefix + settingDevSpk);
	SetColumn(tableConfig, colByName, ctypeString, "", prefix + settingDevRinger);
	SetColumn(tableConfig, colByName, ctypeString, "", prefix + settingDevMic);
	SetColumn(tableConfig, colByName, ctypeString | cflagXor, "Konnekt-Actio", prefix + settingIdentity);
	SetColumn(tableConfig, CFG::tcpPort, ctypeInt, 5060, prefix + settingTcpPort);
	SetColumn(tableConfig, CFG::udpPort, ctypeInt, 5060, prefix + settingUdpPort);
	SetColumn(tableConfig, CFG::rtpPort, ctypeInt, 9000, prefix + settingRtpPort);

	SetColumn(tableConfig, CFG::useSTUN, ctypeInt, 1, "Actio/useSTUN");


	SetColumn(tableConfig, CFG::smsDefaultAction, ctypeInt, 1, prefix + "ui/smsdefault");

	SetColumn(tableConfig, colByName, ctypeInt, 0, prefix + settingCallAllowWaiting);
	SetColumn(tableConfig, colByName, ctypeInt, 1, prefix + settingCallStoreRejected);
	SetColumn(tableConfig, CFG::callPopup, ctypeInt, -1, prefix + "call/popup");

	SetColumn(tableConfig, colByName, ctypeInt, 0x80, prefix + settingGainMic);
	SetColumn(tableConfig, colByName, ctypeInt, 0x80, prefix + settingGainSpk);

	SetColumn(tableConfig, colByName, ctypeInt, 0, prefix + settingWasAuthorized);
	SetColumn(tableConfig, colByName, ctypeInt, 0, prefix + settingWasQOS);

	SetColumn(tableConfig, colByName, ctypeInt, 1, prefix + settingQOSshowWarnings);

	SetColumn(tableConfig, colByName, ctypeString | cflagXor, "", prefix + settingLastNumber);





	SetColumn(tableContacts, Actio::CNT::phoneUrl, ctypeString | cflagXor, "", "Actio/phoneUrl");


	dtIncoming = createArchiveTable("Actio/incoming");
	dtIncoming->setFilename("actio_in.dtb");
	dtIncoming->setDirectory(0);
	dtOutgoing = createArchiveTable("Actio/outgoing");
	dtOutgoing->setFilename("actio_out.dtb");
	dtOutgoing->setDirectory(0);

	dtNumbers = Tables::registerTable(Ctrl, "Actio/numbers");	
	dtNumbers->setFilename("actio_num.dtb");
	dtNumbers->setDirectory(0);
	dtNumbers |= optAutoLoad;
	dtNumbers |= optAutoSave;
	dtNumbers->setColumn(*Ctrl, DTNumbers::phone, ctypeString | cflagXor, "phone");
	dtNumbers->setColumn(*Ctrl, DTNumbers::last, ctypeInt64, "last");

	return 1;
}

int IPrepare() {

	if (GETINT(CFG::enableAtStartup) && *GETSTR(CFG::userName) && *GETSTR(CFG::userPass)) {
		ICMessage(IMC_SETCONNECT, 1);
	}

	CStdString txt;

	IconRegister(IML_16, UIIcon(IT_LOGO , Actio::net , 0 , 0), "res://dll/logo.ico#size=16");
	IconRegister(IML_16, UIIcon(IT_STATUS, Actio::net, ST_ONLINE, 0), "res://dll/online.ico#size=16");
	IconRegister(IML_16, UIIcon(IT_STATUS, Actio::net, ST_CONNECTING, 0), "res://dll/online.ico#size=16");
	IconRegister(IML_16, UIIcon(IT_STATUS, Actio::net, ST_OFFLINE, 0), "res://dll/offline.ico#size=16");
	IconRegister(IML_16, Actio::ACT::makeCall, "res://dll/makecall.ico#size=16");

	IconRegister(IML_16, Actio::ACT::makeCallSip, "res://dll/phone_sip.ico#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallHome, "res://dll/makecall.ico#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallMobile, "res://dll/makecall.ico#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallWork, "res://dll/makecall.ico#size=16");

	IconRegister(IML_16, Actio::ACT::makeCallSip, "file://" + PhonoLogic::Window::getImagePath("phone_sip", 16) + "#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallHome, "file://" + PhonoLogic::Window::getImagePath("phone_home", 16) + "#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallWork, "file://" + PhonoLogic::Window::getImagePath("phone_work", 16) + "#size=16");
	IconRegister(IML_16, Actio::ACT::makeCallMobile, "file://" + PhonoLogic::Window::getImagePath("phone_mobile", 16) + "#size=16");


	IconRegister(IML_16, UIIcon(IT_MESSAGE, Actio::net, MT_SPECIAL, 0), "res://dll/logo.ico#size=16");

	Stamina::PhonoLogic::Window::wndIconBig = Stamina::loadIconEx(Ctrl->hDll(), "logo", 32);
	Stamina::PhonoLogic::Window::wndIconSmall = Stamina::loadIconEx(Ctrl->hDll(), "logo", 16);

// status

	UIActionInsert(IMIG_STATUS , Actio::ACT::statusGroup , -1 , ACTS_GROUP , "Actio" , UIIcon(IT_LOGO , Actio::net , 0 , 0));

	UIActionInsert(Actio::ACT::statusGroup , Actio::ACT::makeCall , -1 , ACTSMENU_BOLD , "Zadzwoñ" , Actio::ACT::makeCall);
	UIActionInsert(Actio::ACT::statusGroup , Actio::ACT::answerCall , -1 , ACTS_HIDDEN , "Zobacz po³¹czenie" , Actio::ACT::makeCall);
	UIActionInsert(Actio::ACT::statusGroup , Actio::ACT::showWindow , -1 , 0 , "Poka¿ okno" , UIIcon(IT_STATUS , Actio::net , ST_ONLINE , 0));

	UIActionInsert(Actio::ACT::statusGroup , 0 , -1 , ACTT_SEPARATOR , "" , 0);

	UIActionInsert(Actio::ACT::statusGroup , Actio::ACT::statusOnline , -1 , 0 , "W³¹czony" , UIIcon(IT_STATUS , Actio::net , ST_ONLINE , 0));
	UIActionInsert(Actio::ACT::statusGroup , Actio::ACT::statusOffline , -1 , 0 , "Wy³¹czony" , UIIcon(IT_STATUS , Actio::net , ST_OFFLINE , 0));


// kontakty

	UIActionInsert(IMIG_CNT , Actio::ACT::makeCall , 0 , ACTR_INIT , "Zadzwoñ" , Actio::ACT::makeCall);

	UIGroupInsert(IMIG_CNT , Actio::ACT::makeCallGroup , 1 , ACTR_INIT , "Zadzwoñ na..." , Actio::ACT::makeCall);

	UIActionInsert(Actio::ACT::makeCallGroup , Actio::ACT::makeCallSip , -1 , 0 , "Numer Actio" , Actio::ACT::makeCallSip);
	UIActionInsert(Actio::ACT::makeCallGroup , Actio::ACT::makeCallMobile , -1 , 0 , "Komórka" , Actio::ACT::makeCallMobile);
	UIActionInsert(Actio::ACT::makeCallGroup , Actio::ACT::makeCallHome , -1 , 0 , "Stacjonarny" , Actio::ACT::makeCallHome);
	UIActionInsert(Actio::ACT::makeCallGroup , Actio::ACT::makeCallWork , -1 , 0 , "Praca" , Actio::ACT::makeCallWork);


// Kontakt - wiêcej

//
	int pos = UIActionGetPos(IMIG_NFO_CONTACT, IMIB_CNT | CNT_CELLPHONE) + 1;

	UIActionInsert(IMIG_NFO_CONTACT , 0 , pos , ACTT_COMMENT | ACTSC_INLINE , "Actio / SIP", 0, 75);
	UIActionInsert(IMIG_NFO_CONTACT , IMIB_CNT , pos+1 , ACTT_EDIT  | ACTSC_FULLWIDTH , "" CFGTIP "Numer telefonu w sieci Actio, lub w sieci wspó³pracuj¹cej", Actio::CNT::phoneUrl);


// ustawienia

	CStdString prefix = ACTIO_CFG_PREFIX;

	UIGroupAdd(IMIG_CFG_USER , Actio::ACT::configGroup , ACTR_INIT , "Actio" , UIIcon(IT_LOGO , Actio::net , 0 , 0)); {


		UIActionAdd(ACT::configGroup, ACT::banner, ACTT_HWND,"" AP_MINWIDTH "400" AP_TIP "OdwiedŸ nasz¹ witrynê, poznaj szczegó³y oferty!", 0, 400, 80);


		UIActionAdd(ACT::configGroup, 0, ACTT_GROUP,"");{

			txt = SetActParam("Dowiedz siê wiêcej o us³udze tanich rozmów", AP_ICO, Stamina::inttostr(ICON_INFO));
			//txt = SetActParam("Informacje o us³udze", AP_IMGURL, "file://data\\actio\\button_info.png");
			UIActionAdd(ACT::configGroup , ACT::linkInfo ,ACTT_LINK,txt,0,0, 30);

			txt = SetActParam("Masz pytanie lub problem? Zajrzyj do pomocy!", AP_ICO, Stamina::inttostr(ICON_HELP));
			UIActionAdd(ACT::configGroup , ACT::linkHelp ,ACTT_LINK,txt,0,0, 30);
			txt = SetActParam("Za³ó¿ nowe konto, numer miejski otrzymasz ZA DARMO!", AP_ICO, Stamina::inttostr(ICON_ACCOUNTCREATE));
			UIActionAdd(ACT::configGroup , ACT::linkCreateAccount ,ACTT_LINK|ACTSC_BOLD,txt,0,0, 40);

		}UIActionAdd(ACT::configGroup, 0, ACTT_GROUPEND);
		UIActionAdd(ACT::configGroup, 0, ACTT_GROUP,"");{


			UIActionAdd(ACT::configGroup , ACT::haveAccount ,ACTT_COMMENT, "Je¿eli posiadasz ju¿ konto podaj jego numer i has³o:", 0, 0);

			UIActionCfgAdd(ACT::configGroup , 0 ,ACTT_COMMENT | ACTSC_INLINE,"Numer miejski", 0, 0, 5);
			UIActionAdd(ACT::configGroup , IMIB_CFG ,ACTT_EDIT | ACTSC_INLINE | ACTR_CONVERT | ACTSC_INT ,"" AP_TIP "Login jest jednoczeœnie Twoim numerem miejskim. Wpisz go razem z '48' na pocz¹tku.\r\nnp.: 48221234567",CFG::userName,80);
			UIActionAdd(ACT::configGroup , 0 ,ACTT_COMMENT | ACTSC_INLINE ,"Has³o");
			UIActionAdd(ACT::configGroup , IMIB_CFG ,ACTT_PASSWORD | ACTSC_INLINE | ACTR_CONVERT,"" ,CFG::userPass,80);

			txt = SetActParam("Przypomnienie has³a", AP_ICO, Stamina::inttostr(ICON_REMINDPASSWORD));
			UIActionAdd(ACT::configGroup , ACT::linkLostPassword ,ACTT_LINK,txt);


//			UIActionAdd(ACT::configGroup , 0 ,ACTT_SEPARATOR);
		}UIActionAdd(ACT::configGroup, 0, ACTT_GROUPEND);
		UIActionAdd(ACT::configGroup, 0, ACTT_GROUP,"");{


			txt = SetActParam("Panel konta - do³adowywanie, rachunki, zmiana has³a...", AP_ICO, Stamina::inttostr(ICON_USER));
			UIActionAdd(ACT::configGroup , ACT::linkSelfcare ,ACTT_LINK|ACTSC_BOLD,txt,0,0, 40);



	/*
				txt = SetActParam("Importuj listê kontaktów", AP_ICO, inttoch(ICON_IMPORT));
				UIActionAdd(IMIG_GGCFG_USER , IMIA_LIST_GG_IMPORT ,ACTT_BUTTON|ACTSC_BOLD | ACTSC_FULLWIDTH,txt , 0 , 180, 30);

				txt = SetActParam("Zmieñ has³o", AP_ICO, inttoch(ICON_CHANGEPASSWORD));
				UIActionAdd(IMIG_GGCFG_USER , IMIC_GG_NEWPASS ,ACTT_BUTTON|ACTSC_INLINE,txt,0,155, 30);
				txt = SetActParam("Przypomnij has³o", AP_ICO, inttoch(ICON_REMINDPASSWORD));
				UIActionAdd(IMIG_GGCFG_USER , IMIC_GG_REMINDPASS ,ACTT_BUTTON | ACTSC_FULLWIDTH,txt,0,180, 30);
	*/

			txt = SetActParam("Zanim zaczniesz rozmawiaæ skalibruj kartê dŸwiêkow¹!", AP_ICO, Stamina::inttostr(ICON_OPTIONS));
			UIActionAdd(ACT::configGroup , ACT::calibrate ,ACTT_LINK,txt,0,0, 30);
			//if (ShowBits::checkLevel(ShowBits::levelIntermediate)) {
				txt = SetActParam("Dodatkowe ustawienia...", AP_ICO, Stamina::inttostr(ICON_OPTIONS));
				UIActionAdd(ACT::configGroup , ACT::moreOptions ,ACTT_LINK,txt,0,0, 30);
			//}


		}UIActionAdd(ACT::configGroup, 0, ACTT_GROUPEND,"");
		//UIActionAdd(ACT::configGroup, 0, ACTT_GROUP,"Ustawienia");{


		//}UIActionAdd(ACT::configGroup, 0, ACTT_GROUPEND,"");

		//if (ShowBits::checkLevel(ShowBits::levelIntermediate)) {

			UIGroupAdd(ACT::configGroup, ACT::configMoreGroup, 0, "Ustawienia", ICON_OPTIONS);

			UIActionCfgAddPluginInfoBox2(Actio::ACT::configMoreGroup, 
				"KONNEKT Actio umo¿liwia rozmowy telefonicznie za poœrednictwem sieci actio.pl.\r\nProsimy mieæ na uwadze, ¿e wtyczka jest jeszcze w stadium testów!"
				, "Za obs³ugê protoko³u odpowiada biblioteka <b>SipX</b> (http://www.sipfoundry.org/)."
				"<br/>"
				"<br/>Copyright ©2005-2006 <b>Stamina</b>"
				"<br/>Copyright ©2004-2006 <b>Pingtel Corp.</b> (SipX)"
				, "res://dll/logo.ico", -3);


			UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUP,"");{
				UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK,"Po³¹cz od razu po uruchomieniu" CFGTIP "Musisz równie¿ w³¹czyæ opcjê \"³¹cz automatycznie\" w zak³adce po³¹czenia!", CFG::enableAtStartup);

				if (ShowBits::checkLevel(ShowBits::levelNormal)) {

					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK, "Je¿eli rozmawiam, pokazuj nowe po³¹czenia jako oczekuj¹ce" AP_TIP "W przeciwnym razie zostan¹ odrzucone", Ctrl->DTgetNameID(DTCFG, prefix + settingCallAllowWaiting));
					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK, "Zapisuj odrzucone po³¹czenia w historii przychodz¹cych", Ctrl->DTgetNameID(DTCFG, prefix + settingCallStoreRejected));

					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK, "Dla kontaktów bez sieci domyœln¹ akcj¹ jest wys³anie SMSa", CFG::smsDefaultAction);

				}

				UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK, "Wyœwietlaj informacje o problemach z jakoœci¹ po³¹czeñ", Ctrl->DTgetNameID(DTCFG, prefix + settingQOSshowWarnings));


			}UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUPEND,"");


			UIActionAdd(ACT::configMoreGroup, 0 , ACTT_GROUP, "Automatyczne otwieranie okna przy po³¹czeniu przychodz¹cym");{
				UIActionAdd(ACT::configMoreGroup, 0,  ACTT_RADIO |  ACTR_NODATASTORE, "Domyœlnie (jak w ustawieniach wiadomoœci)" AP_VALUE "-1" , Actio::CFG::callPopup);
				UIActionAdd(ACT::configMoreGroup, 0,  ACTT_RADIO | ACTSC_INLINE | ACTR_NODATASTORE, "Wy³¹czone" AP_VALUE "0" , Actio::CFG::callPopup);
				UIActionAdd(ACT::configMoreGroup, 0, ACTT_RADIO | ACTSC_INLINE | ACTR_NODATASTORE,	"Zminimalizowane" AP_TIP "Okno wiadomoœci zostanie otwarte zminimalizowane w tle." AP_VALUE "1" , Actio::CFG::callPopup);
				UIActionAdd(ACT::configMoreGroup, 0, ACTT_RADIO | ACTSC_INLINE | ACTR_NODATASTORE, 
					"W tle" AP_TIP "Okno wiadomoœci zostanie otwarte w tle (bez prze³¹czania aktywnego okna)." AP_VALUE "2" , Actio::CFG::callPopup);
				UIActionAdd(ACT::configMoreGroup, 0, ACTT_RADIO | ACTSRADIO_LAST, 
					"Na wierzchu" AP_TIP "Okno wiadomoœci \"wyskoczy\" na wierzch i automatycznie ustawi siê jako aktywne." AP_VALUE "3" , Actio::CFG::callPopup);
			}UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUPEND,"");
		

			//if (ShowBits::checkLevel(ShowBits::levelNormal)) {
/*
				UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUP,"");{

//					UIActionCfgAdd(ACT::configMoreGroup, 0, ACTT_COMMENT|ACTSC_INLINE,"Zewnêtrzny adres IP");

					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_EDIT | ACTSC_INLINE,"", CFG::publicAddress);
					UIActionCfgAdd(ACT::configMoreGroup, 0, ACTT_TIPBUTTON, AP_TIP "Nie musisz wype³niaæ tego pola! Program posiada obs³ugê STUN i w wiêkszoœci przypadków rozpozna ten adres samodzielnie.", 0, 0, -2);

				}UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUPEND,"");
*/
				UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUP,"Ustawienia sieci");{

					UIActionAdd(ACT::configMoreGroup, 0, ACTT_COMMENT|ACTSC_INLINE,"Porty: UDP");
					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_EDIT|ACTSC_INT|ACTSC_INLINE|ACTSC_NEEDRESTART, AP_TIP "Komunikacja z serwerem odbywa siê przez UDP.\n\nDomyœlnie - 5060", Actio::CFG::udpPort, 50);

					UIActionAdd(ACT::configMoreGroup, 0, ACTT_COMMENT|ACTSC_INLINE,"TCP");
					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_EDIT|ACTSC_INT|ACTSC_INLINE|ACTSC_NEEDRESTART, AP_TIP "Je¿eli komunikacja poprzez UDP nie jest mo¿liwa - u¿ywany jest protokó³ TCP.\n\nDomyœlnie - 5060", Actio::CFG::tcpPort, 50);

					UIActionAdd(ACT::configMoreGroup, 0, ACTT_COMMENT|ACTSC_INLINE,"RTP");
					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_EDIT|ACTSC_INT|ACTSC_NEEDRESTART | ACTSC_INLINE, AP_TIP "G³os przesy³any jest przy pomocy protoko³u RTP.\nKa¿da rozmowa umieszczana jest na osobnych dwóch kana³ach. Wolnych musi byæ wiêc conajmniej kilkanaœcie portów wzwy¿.\n\nDomyœlnie - 9000", Actio::CFG::rtpPort, 50);

					UIActionCfgAdd(ACT::configMoreGroup, 0, ACTT_TIPBUTTON, AP_TIP "Actio dzia³a na protokole SIP. Domyœlnie protokó³ ten wykorzystuje porty UDP 5060 i 9000-9010, oraz TCP 5060. Je¿eli posiadasz firewall, musisz odblokowaæ te porty do komunikacji obustronnej, lub (najlepiej) zezwoliæ aplikacji Konnekt na pe³n¹ komunikacjê z sieci¹.", 0, 0, -2);

					UIActionAdd(ACT::configMoreGroup, IMIB_CFG, ACTT_CHECK|ACTSC_NEEDRESTART, "U¿yj technologii STUN do ominiêcia NAT" AP_TIP "Us³uga STUN umo¿liwia dzia³anie w nietypowych konfiguracjach sieciowych (np. ³¹czenie zza NAT - tzw. IP Prywatne).", Actio::CFG::useSTUN);

				}UIActionAdd(ACT::configMoreGroup, 0, ACTT_GROUPEND,"");
			//}

		//}


	}
	return 1;
}

int ActionCfgProc(sUIActionNotify_base * anBase) {
	static bool nameChanged = false;
	/* Tutaj obs³ugujemy akcje dla okna konfiguracji */ 
	/* Sytuacja taka sama jak przy ActionProc  */
	sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
	switch (anBase->act.id & ~IMIB_CFG) {
		case Actio::CFG::userName: 
			if (anBase->code == ACTN_SET) { // na pewno bêdzie wywo³ane zaraz przed userPass...
				sUIActionNotify_buff* anb = (sUIActionNotify_buff*)(anBase);
				nameChanged = strcmp(anb->buff , GETSTR(CFG::userName));
			}
			break;
		case Actio::CFG::userPass: 
			if (anBase->code == ACTN_SET) {
				sUIActionNotify_buff* anb = (sUIActionNotify_buff*)(anBase);
				bool passChanged = (strcmp(anb->buff, GETSTR(CFG::userPass)));
				if ( passChanged || nameChanged ) {
					account->setSetting(settingWasAuthorized, 0);
				}

			}
			return 0;
	}
	return 0;
}

ActionProc(sUIActionNotify_base * anBase) {
	sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;

	if ((anBase->act.id & IMIB_) == IMIB_CFG) return ActionCfgProc(anBase); 
	switch (anBase->act.id) {
		case ACT::banner:
			if (anBase->code == ACTN_CREATEWINDOW) {
				sUIActionNotify_createWindow * cw = (sUIActionNotify_createWindow *) anBase;
				oImage img = PhonoLogic::Window::getImage("banner_options");
				Size size(400, 40);
				if (img) {
					size = img->getSize();
				}

				Stamina::UI::ButtonX* b = new Stamina::UI::ButtonX("", BS_OWNERDRAW | WS_VISIBLE | WS_TABSTOP | WS_CHILD, cw->x, cw->y, size.w, size.h, cw->hwndParent, 0, 0);
				if (img) {
					b->setImage(img);
				} else {
					b->setText("SprawdŸ ofertê Konnekt - Actio!");
				}

				cw->hwnd = b->getHWND();
				cw->y += size.h;
			} else if (anBase->code == ACTN_DESTROYWINDOW) {
				sUIActionNotify_destroyWindow * dw = (sUIActionNotify_destroyWindow *) anBase;
				DestroyWindow(dw->hwnd);
			} else if (anBase->code == ACTN_ACTION) {
				account->onButtonPressed(buttonBanner);
			}
			return 0;
		case ACT::statusOffline:
			ACTIONONLY(anBase);
			Actio::disconnect();
			return 0;
		case ACT::moreOptions:
			ACTIONONLY(anBase);
			ICMessage(IMI_CONFIG, Actio::ACT::configMoreGroup);
			return 0;
		case ACT::statusOnline:
			ACTIONONLY(anBase);
			Actio::connect(true);
			return 0;
		case ACT::statusGroup:
		case ACT::showWindow:
		case ACT::answerCall:
			ACTIONONLY(anBase);
			Actio::showWindow();
			return 0;
		case ACT::makeCall:
			if (anBase->act.cnt == 0) {
				if (anBase->code == ACTN_ACTION)
					//Actio::showWindow();
					Actio::newCall();
			} else if (anBase->act.cnt != -1) { // menu kontaktu...
				bool indb = numbersMap.contactExists(anBase->act.cnt);
				bool calling = indb ? Actio::isCalling(anBase->act.cnt) : false;
				int net = GETCNTI(anBase->act.cnt, CNT_NET);
				if (anBase->code == ACTN_CREATE) {
					UIActionSetStatus(anBase->act, indb ? 0 : -1, ACTS_HIDDEN);
					if (indb)
						UIActionSetText(anBase->act, calling ? "Zobacz po³¹czenie" : "Zadzwoñ");
				} else if (anBase->code == ACTN_DEFAULT) {
					static bool smsAvailable = IMessage(IM_PLUG_NET, NET_SMS) != NET_NONE;
					if (smsAvailable && GETINT(CFG::smsDefaultAction) && GETCNTI(anBase->act.cnt, CNT_NET) == NET_NONE) {
						return false;
					}
					return (indb && (calling || net==NET_NONE || net==Actio::net));
				} else if (anBase->code == ACTN_ACTION) {
					if (calling) {
						Actio::showWindow();
					} else {
						Actio::makeCall(anBase->act.cnt);
					}
				}
			} else {
				if (anBase->code == ACTN_CREATE)
					UIActionSetStatus(anBase->act, -1, ACTS_HIDDEN);
			}
			return 0;
		case ACT::makeCallGroup: {
			if (anBase->code == ACTN_CREATE) {
				bool sip = numbersMap.contactExists(anBase->act.cnt, PhonoLogic::phoneSip);
				bool home = numbersMap.contactExists(anBase->act.cnt, PhonoLogic::phoneHome);
				bool work = numbersMap.contactExists(anBase->act.cnt, PhonoLogic::phoneWork);
				bool mobile = numbersMap.contactExists(anBase->act.cnt, PhonoLogic::phoneMobile);
				UIActionSetStatus(anBase->act, (sip + home + work + mobile > 1) ? 0 : -1, ACTS_HIDDEN);
				sUIAction act = anBase->act;
				act.parent = anBase->act.id;
				act.id = Actio::ACT::makeCallSip;
				UIActionSetStatus(act, sip ? 0 : -1, ACTS_HIDDEN);
				act.id = Actio::ACT::makeCallHome;
				UIActionSetStatus(act, home ? 0 : -1, ACTS_HIDDEN);
				act.id = Actio::ACT::makeCallWork;
				UIActionSetStatus(act, work ? 0 : -1, ACTS_HIDDEN);
				act.id = Actio::ACT::makeCallMobile;
				UIActionSetStatus(act, mobile ? 0 : -1, ACTS_HIDDEN);
			}
			return 0;
			}
		case ACT::makeCallSip:
			ACTIONONLY(anBase);
			Actio::makeCall(anBase->act.cnt, PhonoLogic::phoneSip);
			return 0;
		case ACT::makeCallHome:
			ACTIONONLY(anBase);
			Actio::makeCall(anBase->act.cnt, PhonoLogic::phoneHome);
			return 0;
		case ACT::makeCallWork:
			ACTIONONLY(anBase);
			Actio::makeCall(anBase->act.cnt, PhonoLogic::phoneWork);
			return 0;
		case ACT::makeCallMobile:
			ACTIONONLY(anBase);
			Actio::makeCall(anBase->act.cnt, PhonoLogic::phoneMobile);
			return 0;

		case ACT::calibrate: {
			ACTIONONLY(anBase);
			Actio::calibrateSound();
			return 0;}

		case Actio::ACT::configGroup:
			if (anBase->code == ACTN_CREATE) {
				int unauthorized = (account->getIntSetting(settingWasAuthorized) == -1) ? -1 : 0;
				int lowQos = (account->getIntSetting(settingWasQOS) == -1) ? -1 : 0;
				int newAccount = (account->isConfigured()) ? 0 : -1;
				int needCalib = (!newAccount && account->getIntSetting(settingWasQOS) < 1) ? -1 : 0;
				int problems = ( unauthorized || lowQos ) ? -1 : 0;
				double balance;
				int needRefill = (!newAccount && account->getIntSetting(settingWasAuthorized) == 1 && account->getBalance(balance) && balance < 5) ? -1 : 0;
				if (!problems && !needCalib && !newAccount)
					needRefill = -1; // skoro nie ma co pogrubiaæ, pogrubmy konto ;)

				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::linkInfo), newAccount, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::linkHelp), problems, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::linkCreateAccount), newAccount, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::linkSelfcare), needRefill, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::calibrate), needCalib, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::linkLostPassword), unauthorized, ACTSC_BOLD);
				UIActionSetStatus(sUIAction(ACT::configGroup, ACT::haveAccount), newAccount, ACTSC_BOLD);
				

			}
			return 0;

		case Actio::ACT::linkCreateAccount:
			ACTIONONLY(anBase);
			//ShellExecute(0, "open", Actio::urlCreateAccount, 0, 0, SW_SHOW);
			Actio::createAccount( (HWND)UIActionHandle(sUIAction(0, IMIG_CFGWND)) );
			return 0;
		case Actio::ACT::linkLostPassword:
			ACTIONONLY(anBase);
			ShellExecute(0, "open", Actio::urlLostPassword, 0, 0, SW_SHOW);
			return 0;
		case Actio::ACT::linkHelp:
			ACTIONONLY(anBase);
			account->onButtonPressed(buttonHelp);
			return 0;
		case Actio::ACT::linkInfo:
			ACTIONONLY(anBase);
			account->onButtonPressed(buttonInfo);
			return 0;
		case Actio::ACT::linkSelfcare:
			ACTIONONLY(anBase);
			account->onButtonPressed(buttonAccount);
			return 0;


	}
	return 0;
}

}//namespace

int __stdcall IMessageProc(sIMessage_base * msgBase) {
	sIMessage_2params * msg = (msgBase->s_size>=sizeof(sIMessage_2params))?static_cast<sIMessage_2params*>(msgBase):0;
	switch (msgBase->id) {
		/* Wiadomoœci na które TRZEBA odpowiedzieæ */
	case IM_PLUG_NET:        return Actio::net; 
	case IM_PLUG_TYPE:       return IMT_CONFIG | IMT_CONTACT | IMT_NET | IMT_NETUID | IMT_PROTOCOL | IMT_UI | IMT_MESSAGE; 
	case IM_PLUG_VERSION:    return 0;
	case IM_PLUG_SDKVERSION: return KONNEKT_SDK_V;
	case IM_PLUG_SIG:        return (int)"ACTIO"; // Sygnaturka wtyczki (krótka, kilkuliterowa nazwa)
	case IM_PLUG_NAME:       return (int)"Actio";
	case IM_PLUG_NETNAME:    return (int)"Actio"; 
	case IM_PLUG_NETSHORTNAME: return (int)"actio";
	case IM_PLUG_INIT:       Plug_Init(msg->p1,msg->p2);return Init();
	case IM_PLUG_DEINIT:     Plug_Deinit(msg->p1,msg->p2);return DeInit();
	case IM_PLUG_DONTFREELIBRARY: return 1;
    case IM_PLUG_PRIORITY:   return PLUGP_LOW;


	case IM_SETCOLS:		 return ISetCols();

	case IM_UI_PREPARE:      return IPrepare();
	case IM_START:           return IStart();
	case IM_END:             return IEnd();

	case IM_UIACTION:        return ActionProc((sUIActionNotify_base*)msg->p1);

	case IM_PLUG_UPDATE:	 Actio::updateVersion(Version(msg->p1));
		return 0;

	case IM_CONNECT:
		if (account && account->isConfigured())
			Actio::connect(false);
		return 0;
	case IM_DISCONNECT:
		Actio::disconnect();
		return 0;

	case IM_GET_STATUS:
		return account && account->isConnected() ? ST_ONLINE : ST_OFFLINE;
	case IM_GET_STATUSINFO:
		return (int)"";

	case IM_CHANGESTATUS:
		if (msg->p1 != ST_OFFLINE) {
			if (account && account->isConfigured()) {
				Actio::connect(true);
			}
		} else {
			Actio::disconnect();
		}
		return 0;

	case IM_CNT_CHANGED: case IM_CNT_ADD:
		numbersMap.updateContact(msg->p1);
		return 0;
	case IM_CNT_REMOVE:
		numbersMap.removeContact(msg->p1);
		return 0;
	
	case IM_CFG_CHANGED:
		if (account)
			account->onConfigChanged();
		return 0;

	case IM_MSG_RCV: {
		cMessage* m = (cMessage*) msg->p1;
		if (m->type == mtypePhoneCall) {
			return IM_MSG_ok;
		}
		return 0;}

	case IM_DEBUG_COMMAND: {
		sIMessage_debugCommand* arg = (sIMessage_debugCommand*) msg;
		if (arg->argEq(0, "debug")) {
			if (arg->argEq(1, "objects")) {
				Stamina::debugDumpObjects(Stamina::mainLogger);
			}
		}
		return 0;
		}

	case kSound::DOREGISTER:
		kSound::SoundRegister(Ctrl, "phoneAlert", "Dzwonek telefonu", kSound::flags::contacts, Actio::CFG::soundAlert, "phonealert.wav");
		kSound::SoundRegister(Ctrl, "phoneFailed", "B³¹d podczas dzwonienia", 0 , Actio::CFG::soundFailed, "phonefailed.wav");
		return 0;

	}
	if (Ctrl) Ctrl->setError(IMERROR_NORESULT);
	return 0;
}

