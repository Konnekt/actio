#include "stdafx.h"

#include "actio.h"
#include "actio_main.h"
#include <Stamina\PhonoLogic\CalibrationWnd.h>
// to powinno byæ zrobione inaczej!
#include "..\core\KLogger.h"

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

	void init() {

// to powinno byæ zrobione inaczej!
		Stamina::mainLogger = new Konnekt::KLogger(Stamina::logAll);
		

		Stamina::setInstance(Ctrl->hDll());
		PhonoLogic::Window::imgPath = expandEnvironmentStrings("%KonnektData%\\actio\\");
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


};