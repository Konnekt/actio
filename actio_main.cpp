#include "stdafx.h"

#include "actio.h"
#include "actio_main.h"
#include <Stamina\PhonoLogic\CalibrationWnd.h>
#include <Stamina\Debug.h>
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

	void initPSTNPatterns() {
		PhoneUrl::patternPSTNSIP.reset(new Stamina::RegEx::Compiled (
			"/^00("
			"9266|9201|9261|9208|9366|9393|9211|9478|9220|9211|9777|9747|9471|9353|9258|9027"
			")/"
			));
		PhoneUrl::patternPSTNSIPNetwork.reset(new Stamina::RegEx::Compiled (
			"/^0("
			"123833|1239400|1239401|1239402|1239403|1239404|"
            "134932|146922|1771742|1771743|1771744|185422|"
			"2221322|2221323|2221324|2221325|2221326|223821|224861|"
			"243624|2438710|2438711|2438712|296430|"
			"327444|3275034|3275035|3275036|3275037|"
			"3348610|3348611|3348612|3439050|3439051|3439052|"
            "4131036|4131037|4131038|422784|4229935|4229936|4229937|"
			"447392|4838076|5255289|5255300|5255301|5255302|544449|"
			"556202|564772|5874142|5874143|5874144|5874145|5874146|587420|"
            "616489|625794|632194|673499|684146|717199|746612|756115|767445|"
			"775441|7754506|7754507|8146363|8146364|8146365|814705|"
			"845384|858743|896748|8967950|8967951|8967952|"
			"918828|9188665|9188666|9188667|9188668|9188669|947172|"
            "957829"
			")/"
			));
	}

	void init() {

// to powinno byæ zrobione inaczej!
		Stamina::mainLogger = new Konnekt::KLogger(Stamina::logAll);
		

		Stamina::setInstance(Ctrl->hDll());
		PhonoLogic::Window::imgPath = expandEnvironmentStrings("%KonnektData%\\actio\\").c_str();
		initPSTNPatterns();
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

//		Stamina::debugDumpObjects(Stamina::mainLogger, logDebug);
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