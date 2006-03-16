#include "stdafx.h"

#include "actio.h"
#include "actio_main.h"
#include <Stamina\PhonoLogic\CalibrationWnd.h>
#include <include/dlghtml.h>
#include "ActioBrowser.h"
// to powinno byæ zrobione inaczej!
#include "KLogger.h"

using namespace Stamina::PhonoLogic;
using namespace Stamina;

namespace Actio {
	Tables::oTable dtIncoming;
	Tables::oTable dtOutgoing;
	Tables::oTable dtNumbers;
	KAccount* account = 0;
	int mtypePhoneCall = 0;
	NumbersMap numbersMap;

//	const CStdString serverHost =  "sip.actio.pl";
	const CStdString serverHost =  "81.15.209.199";
//	const CStdString serverHost =  "fwd.pulver.com";
	const CStdString stunHost =  "stun.fwdnet.net";

	void init() {

// to powinno byæ zrobione inaczej!
		Stamina::mainLogger = new Konnekt::KLogger(Stamina::logAll);
		

		Stamina::setInstance(Ctrl->hDll());
		PhonoLogic::Window::imgPath = expandEnvironmentStrings("%KonnektData%\\actio\\").c_str();
	}
	void start() {
		IMessage(&sIMessage_StatusChange(IMC_STATUSCHANGE , 0 , ST_OFFLINE , 0));
		account = new Stamina::PhonoLogic::KAccount();
		numbersMap.updateContacts();
		mtypePhoneCall = Unique::registerName(Unique::domainMessageType, "Actio/Call", Unique::rangeDefault);
	}
	void finish() {
		if (!account) return;
		delete account;
        account = 0;
	}


	void connect(bool byUser) {
		if (account->isConnected())
			return;
		if (account->isConfigured()) {
			account->setVisibleStatus(ST_CONNECTING);
		}
 		if (account->isConnecting())
			return;
		account->connect(byUser);
		if (account->isInitialized() == false) {
			account->setVisibleStatus(ST_OFFLINE);
		}
	}
	void disconnect() {
		ICMessage(IMC_SETCONNECT, 0);
		account->setVisibleStatus(ST_OFFLINE);
		if (account->isConnected() == false && !account->isConnecting())
			return;
		account->disconnect();
	}
	void showWindow() {
		account->getWindow()->show();
	}

	bool isCalling(tCntId cnt) {
		return account->isCalling(cnt);
	}

	void makeCall(tCntId cnt, PhonoLogic::PhoneType type) {
		// trzeba wybraæ numer telefonu...
		PhoneUrl url = numbersMap.chooseNumber(cnt, type);
		showWindow();
		account->makeCall(url, false);
	}
	void newCall() {
		showWindow();
		account->makeCall("", false);
	}

	void calibrateSound() {
		PhonoLogic::showCalibrationWindow((HWND)UIGroupHandle(sUIAction(0, IMIG_CFGWND)));
//		account->PhonoLogic::Account::onButtonPressed(PhonoLogic::buttonOptions);
	}


	void createAccount(HWND parent) {
		if (*GETSTR(CFG::userName) || *UIActionCfgGetValue(sUIAction(ACT::configGroup, CFG::userName | IMIB_CFG), 0, 0)) {
			if (MessageBox(0, "Masz ju¿ ustawione konto Actio. Czy na pewno chcesz za³o¿yæ kolejne?", "Zak³adanie konta Actio", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDNO) return;
		}
		ActioBrowser* ab = new ActioBrowser();
		ab->setUrl("http://www.konnekt.info/actio/zaloz_konto?byKonnekt", "");
		ab->setOriginalUrl("http://www.konnekt.info/actio/zaloz_konto");
		ab->run(false, 0, "Zak³adanie konta");
	}


	void updateVersion(Version old) {
		if (old.empty()) return;
		if (old < Version(0,7,4,0)) {
			if (*GETSTR(Actio::CFG::userPassMD5)) {
				MessageBox(0, "W wersji 0.7.4 wtyczki KONNEKT-ACTIO zmieni³ siê sposób przechowywania has³a do konta.\r\n\r\nW nastêpnym oknie (lub póŸniej w ustawieniach) bêdziesz musia³ wprowadziæ je ponownie.", "Has³o do konta ACTIO", MB_OK | MB_ICONWARNING);
				sDIALOG_access sd;
				String info = "Podaj has³o do konta Actio nr:\r\n";
				info += GETSTR(Actio::CFG::userName);
				info += ".";
				sd.title = "Podaj has³o do konta Actio";
				sd.info = info.c_str();
				if (ICMessage(IMI_DLGPASS, (int)&sd)) {
					SETSTR(Actio::CFG::userPass, sd.pass);
				}
				SETSTR(Actio::CFG::userPassMD5, "");
			}
		}
	}


};