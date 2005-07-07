#include "stdafx.h"

#include "actio.h"
#include "actio_main.h"
#include <Stamina\PhonoLogic\CallOutgoing.h>
#include <Stamina\PhonoLogic\CallIncoming.h>
#include <Stamina\Internet.h>
#include <Stamina\MD5.h>
#include <Stamina\RegEx.h>


using namespace Actio;

namespace Stamina { namespace PhonoLogic {

	std::stack <unsigned int> callNotifies;

	uintptr_t konnektBeginThread (const char* name, void * sec, unsigned stack,	cCtrl::fBeginThread cb, void * args, unsigned flag, unsigned * addr) {
		unsigned int id;
		int handle = (uintptr_t)Ctrl->BeginThread(name, sec, stack, cb, args, flag, &id);
		if (addr) *addr = id;
		// Ctrl->BeginThread siê tym zajmie...
/*		if (name && *name) { 
			Thread::setName(name, id);
		}*/
		return handle;
	}


	KAccount::KAccount() {
		this->logFile = Stamina::expandEnvironmentStrings("%KonnektLog%\\actio.log");
		this->_threadRunner = new ThreadRunnerStore(konnektBeginThread);
		//sipxConfigSetBeginThread(konnektBeginThread);

		this->stunHost = "";
		PhoneUrl::defaultHost = serverHost;
		keepOneSession = true;
		_balance = 0;
		_gotBalance = false;
		_isFetchingInfo = false;
		setVisibleStatus(ST_OFFLINE);
	}
	KAccount::~KAccount() {
		this->finish();
	}


	void KAccount::createWindow() {
		IMDEBUG(DBG_FUNC, "createWindow()");
		Account::createWindow();
		SetWindowText(this->_window->getHwnd(), "Actio");
	}

	CallEntry* KAccount::createCallEntry(const oCall& call) {
		return new KCallEntry(call);
	}


	void KAccount::connect(bool byUser) {
		if (this->isConfigured()) {
			CStdString url = GETSTR(CFG::userName);
			url += "@" + serverHost;
			account->setSipUrl(url  /* Ctrl->DTgetStr(DTCFG, 0, "Actio/sipUrl")*/);
			account->setSipUser(GETSTR(CFG::userName));
			account->setSipPassMD5(GETSTR(CFG::userPassMD5));
			Account::publicAddress = GETSTR(CFG::publicAddress);
			if (this->_mainThread.isCurrent()) {
				IMDEBUG(DBG_FUNC, "connect(url=%s, user=%s, pub=%s)",url.c_str() , GETSTR(CFG::userName), GETSTR(CFG::publicAddress));
			}
			Account::connect(byUser);
		} else {
			if (Ctrl->IMessage(&sIMessage_msgBox(IMI_CONFIRM, "Zanim bêdziesz móg³ wykonywaæ po³¹czenia musisz najpierw za³o¿yc lub ustawiæ swoje konto.\r\nCzy chcesz przejœæ do okna ustawieñ?", "Actio"))) {
				this->onButtonPressed(buttonOptions);
			}
		}
	}
	void KAccount::disconnect() {
		Account::disconnect();
	}


	void KAccount::onInitialize() {
		Account::onInitialize();
		sipxFieldWatchAdd(_sxInst, "PortaBilling");
	}

	void KAccount::onTapiLineEvent(SIPX_LINE line, SIPX_LINE_EVENT_TYPE_MAJOR major) {
		Account::onTapiLineEvent(line, major);
		switch(major) {
			case SIPX_LINE_EVENT_HEADER_VALUE_CHANGE: {
				// Wyci¹gamy informacjê o stanie konta 
				// PortaBilling: available-funds:3.45 currency:PLN 
				const int valueSize = 255;
				char value [valueSize];
				sipxFieldWatchGet(_sxInst, "PortaBilling", value, valueSize);
				RegEx regex;
				if (regex.match("/available-funds:([\\d]+(?:[,.][\\d]+)?)/", value)) {
					IMDEBUG(DBG_DEBUG, "WatchPortaBilling - ");
					CStdString v = regex[1];
					v.Replace(".", localeconv()->decimal_point);
					v.Replace(",", localeconv()->decimal_point);
					this->_balance = strtod(v.c_str(), 0);
					this->_gotBalance = true;

					if (this->windowExists()) {
						this->getWindow()->refreshAccountInfo();
					}
				}
				Stamina::log(logDebug, "KAccount", "onTapiLineEvent::HEADER_VALUE_CHANGE", "PortaBilling = %s (%.2f)", value, this->_balance);
				break;}
		};
	}


	void KAccount::onConnected() {
		if (this->getStatus() == accountOnline) return;

		IMDEBUG(DBG_FUNC, "onConnected()");
		ICMessage(IMC_SETCONNECT, 0);
		this->setVisibleStatus(ST_ONLINE);
		Account::onConnected();
	}
	void KAccount::onDisconnected() {
		if (this->getStatus() == accountOffline) return;
		IMDEBUG(DBG_FUNC, "onDisconnected()");
		this->setVisibleStatus(ST_OFFLINE);
		Account::onDisconnected();
	}
	void KAccount::onConnectionFailure() {
		if (GETINT(CFG_RETRY)) 
			ICMessage(IMC_SETCONNECT, 1);
		IMDEBUG(DBG_FUNC, "onConnectionFailure()");
		if (this->_connectionByUser) return;
		Account::onConnectionFailure();
	}

	void KAccount::onConnecting() {
		IMDEBUG(DBG_FUNC, "onConnecting()");
		this->setVisibleStatus(ST_CONNECTING);
		Account::onConnecting();
	}


	void KAccount::onNewIncomingCall(const oCall& call) {
		if (!this->getMainThread().isCurrent()) {
			threadInvokeObject(this->getMainThread(), call.get()
				, boost::bind(&KAccount::onNewIncomingCall, this, call), true);
			return;
		}

		IMDEBUG(DBG_FUNC, "onNewIncomingCall(%s)", call->getPhoneUrl().getUrl().c_str());

		int cnt = numbersMap.getContact(call->getPhoneUrl());
		if (cnt == -1) { // dorzucamy kontakt...
			cnt = numbersMap.createContact(call->getPhoneUrl(), false);
		}

		if (account && account->windowExists() && account->getWindow()->isActive())
			return;

		UI::CFG::enMsgPopup popup = (UI::CFG::enMsgPopup) GETINT(Actio::CFG::callPopup);
		if (popup == -1) {
			popup = (UI::CFG::enMsgPopup) GETINT(CFG_UIMSGPOPUP);
		}
		if (popup == UI::CFG::mpFocused) {
			account->getWindow()->show(true, true);
		} else if (popup == UI::CFG::mpBackground) {
			account->getWindow()->show(false, true);
		} else if (popup == UI::CFG::mpMinimized) {
			account->getWindow()->show(false, false);
		}

		if (GETINT(CFG_UIMSGFLASH)) {
			FLASHWINFO fwi;
			memset(&fwi , 0 , sizeof(fwi));
			fwi.cbSize=sizeof(fwi);
			fwi.hwnd=account->getWindow()->getHwnd();
			fwi.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
			fwi.uCount = 1;
			fwi.dwTimeout = 0;
			FlashWindowEx(&fwi);
		}


		cMessage m;
		CStdString body;
		body.Format("%s", numbersMap.getDisplay(call->getPhoneUrl()).c_str());
		m.body = (char*)body.c_str();
//		CStdString ext = SetExtParam("", MEX_TITLE, body);
//		m.ext = (char*)ext.c_str();
		m.flag = MF_NOSAVE;
		m.net = NET_NONE;
		char buff [16];
		itoa(cnt, buff, 10);
		m.fromUid = buff;
		m.type = mtypePhoneCall;
		m.time = _time64(0);
		m.action = sUIAction(Actio::ACT::statusGroup, Actio::ACT::answerCall, cnt);
		m.notify = UIIcon(IT_MESSAGE, Actio::net, MT_SPECIAL, 0);

		sMESSAGESELECT ms;
		ms.id = ICMessage(IMC_NEWMESSAGE, (int)&m);
		if (ms.id >=0)
			ICMessage(IMC_MESSAGEQUEUE, (int)&ms);

		if (IMessage(IM_PLUG_NET, KNotify::net)) { // je¿eli jest KNotify...
			body = body + " dzwoni do Ciebie!";
			KNotify::sIMessage_notify sin (body , m.notify,KNotify::sIMessage_notify::tInform, 0);
			sin._action = m.action;
			sin._clickable = true;
			callNotifies.push(Ctrl->IMessage(&sin));
		}


	}

	void KAccount::onCriticalError(const CStdString& title, const CStdString& msg) {
		if (IMessage(IM_PLUG_NET, KNotify::net)) { // je¿eli jest KNotify...
			CStdString txt = "Actio - " + title + "\r\n" + msg;
			Ctrl->IMessage(&KNotify::sIMessage_notify(txt , (HICON)0,KNotify::sIMessage_notify::tError,3));
		} else {
			Account::onCriticalError(title, msg);
		}
	}
	void KAccount::onUnauthorized() {
		if (Ctrl->IMessage(& sIMessage_msgBox(IMI_CONFIRM, "Poda³eœ z³y login i/lub has³o!\r\nUpewnij siê czy wpisa³eœ je poprawnie...", "Konnekt-Actio - autoryzacja", MB_ICONERROR | MB_OKCANCEL)) == IDOK) {
			ICMessage(IMI_CONFIG, Actio::ACT::configGroup);
		}
	}

	void KAccount::onButtonPressed(PhoneButton button) {
		switch (button) {
			case buttonInfoConfigure:
			case buttonOptions:
				ICMessage(IMI_CONFIG, Actio::ACT::configGroup);
				return;
			case buttonBanner:
			case buttonInfo:
				ShellExecute(0, "open", urlInformation, "", "", SW_SHOW);
				return;
			case buttonHelp:
				ShellExecute(0, "open", Stamina::expandEnvironmentStrings(urlHelp).c_str(), "", "", SW_SHOW);
				return;
			case buttonAccount:
				ShellExecute(0, "open", urlSelfCare, "", "", SW_SHOW);
				return;
			case buttonInfoMic:
				ShellExecute(0, "open", urlProblemMic, "", "", SW_SHOW);
				return;
			case buttonInfoQuality:
				ShellExecute(0, "open", urlProblemQuality, "", "", SW_SHOW);
				return;
			case buttonInfoSound:
				ShellExecute(0, "open", urlProblemSound, "", "", SW_SHOW);
				return;

			case buttonBalance:
				//this->refreshAccountInfo();
				ShellExecute(0, "open", urlAccountBalance, "", "", SW_SHOW);
				return;

		}
		Account::onButtonPressed(button);
	}
	void KAccount::onConfigChanged() {
		Account::onConfigChanged();
	}
	void KAccount::onWindowActivated() {
//		while (notifyMessages.empty() == false) {
		sMESSAGESELECT ms;
		ms.type = mtypePhoneCall;
		ICMessage(IMC_MESSAGEREMOVE, (int)&ms);
//		}
		while (callNotifies.empty() == false) {
			IMessage(KNotify::IM::hide, KNotify::net, IMT_ALL, callNotifies.top());
			callNotifies.pop();
		}
	}

	CStdString KAccount::getSoundFile(const oCall& call, const CStdString& file) {
		bool ksound = IMessage(IM_PLUG_NET, NET_SOUND, IMT_CONFIG) != 0;

		if (file.compare(0, 4, "test") == 0) {
			if (file == "testSpeaker") {
				return Stamina::expandEnvironmentStrings("%KonnektData%\\Actio\\test.wav");
			} else if (file == "testRinger") {
				const char* path = SAFECHAR(kSound::GetSoundFile(Ctrl, "phoneAlert", 0));
				return *path ? path : "sounds\\phonealert.wav";
			}
		} else {
			bool mute = ksound && GETINT(kSound::Cfg::mute)!=0;
			if (!mute) {
				if (file == "alert") {
					return ksound ? SAFECHAR(kSound::GetSoundFile(Ctrl, "phoneAlert", call ? numbersMap.getContact(call->getPhoneUrl()) : 0)) : "sounds\\phonealert.wav";
				} else if (file == "failed") {
					return ksound ? SAFECHAR(kSound::GetSoundFile(Ctrl, "phoneFailed")) : "sounds\\phonefailed.wav";
				} else {
					return Stamina::expandEnvironmentStrings("%KonnektData%\\Actio\\" + file + ".wav");
				}
			}
		}
		return "";
	}

	/*
	void KAccount::playFile(const oCall& call, const CStdString& file, bool loop) {
		IMDEBUG(DBG_FUNC, "KAccount::playFile(%s, %d)", file.c_str(), loop);
		bool ksound = IMessage(IM_PLUG_NET, NET_SOUND, IMT_CONFIG) != 0;
		bool mute = ksound && GETINT(kSound::Cfg::mute)!=0;
		if (file.empty() == false && !mute) {
			if (call && loop) {
				call->_playingFile = true;
				if (file == "alert" && !mute) {
					const char* path = ksound ? kSound::GetSoundFile(Ctrl, "phoneAlert", numbersMap.getContact(call->getPhoneUrl())) : "sounds\\phonealert.wav";
					if (path) {
					//	call->playFile(path, loop);
						playFileOnRinger(path, loop);
					}
				}
			} else {
				if (file == "failed" && ksound && !mute)
					kSound::SoundPlay(Ctrl, "phoneFailed");
			}
		} else // wy³¹czamy...
			Account::playFile(call, "", loop);
	}
	*/

	bool KAccount::isConfigured() {
		return *GETSTR(Actio::CFG::userName) != 0;
	}

	CStdString KAccount::phoneToDisplay(const PhoneUrl& url) {
		return numbersMap.getDisplay(url);
	}

	void KAccount::callToHistory(const oCall& call) {
		Tables::oTable dt;
		if (call->getClass() <= PhonoLogic::CallIncoming::classInfo()) {
			// jest incoming...
			dt = Actio::dtIncoming;
		} else {
			// jest outgoing...
			dt = Actio::dtOutgoing;
		}
		dt->lateSave(false);
		tRowId id = dt->addRow();
		dt->setStr(id, DTArchive::phone, call->getPhoneUrl().getUrl());
		dt->setInt(id, DTArchive::type, call->isUnanswered() ? CallArchiveEntry::typeUnanswered : CallArchiveEntry::typeSuccessfull);
		dt->setInt64(id, DTArchive::start, call->isUnanswered() ? _time64(0) : call->getCallStart());
		dt->setInt(id, DTArchive::duration, call->getCallDuration());
		// Account doda samodzielnie do ew. otwartej listy historii...
		dt->lateSave();
		Account::callToHistory(call);
	}

	void KAccount::rejectedCallToHistory(const PhoneUrl& url, CallArchiveEntry::Type type) {
		Tables::oTable dt;
		dt = Actio::dtIncoming;
		dt->lateSave(false);
		tRowId id = dt->addRow();
		dt->setStr(id, DTArchive::phone, url.getUrl());
		dt->setInt(id, DTArchive::type, type);
		dt->setInt64(id, DTArchive::start, _time64(0));
		dt->setInt(id, DTArchive::duration, 0);
		// Account doda samodzielnie do ew. otwartej listy historii...
		dt->lateSave();

		Account::rejectedCallToHistory(url, type);

	}


	void KAccount::markPhoneAsUsed(const PhoneUrl& url) {
		if (url.isValid() == false)
			return;
		Tables::oTable dt = dtNumbers; // dla wygody...
		ObjLocker lock1(this->_window->getPhoneBook());
		ObjLocker lock2(dt.get());
		dt->lateSave(false);
		// szukamy elementu...
		tRowId id = Tables::rowNotFound;
		tRowId oldId = Tables::rowNotFound;
		for (int i=0; i < dt->getCount(); i++) {
			if (url == dt->getStr(i, DTNumbers::phone)) {
				oldId = dt->getRowId(i);
				break;
			}
		}
		// ¿eby nowe by³y zawsze na koñcu...
//		if (id == Tables::rowNotFound) {
		id = dt->addRow();
		dt->setStr(id, DTNumbers::phone, url.getUrl());
//		}
		dt->setInt64(id, DTNumbers::last, _time64(0));
		if (this->_window->getPhoneBook()->getCollBook()->getExpanded() == PBCollectionEntry::expandedLess) {
			this->_window->getPhoneBook()->addPhoneNumber(new PhoneEntryDT(id), true);
		}
		if (oldId != Tables::rowNotFound) {
			dt->removeRow(oldId);
		}
		dt->lateSave();

	}

	bool KAccount::updateCollectionBook(PBCollectionBook* entry, PBCollectionItem* item) {
		// wype³niamy listê zawartoœci¹ ksi¹¿ki adresowej...
		PhoneBook* pb = this->_window->getPhoneBook();
		pb->lockRefresh();
		item->removeAll(pb);
		if (entry->getExpanded() == PBCollectionEntry::expandedLess) {
			// 50 ostatnich
			Tables::oTable dt = dtNumbers;
			if (dt->getCount() > 0) {
				for (int i=dt->getCount() - 1; i > dt->getCount() - 51; i--) {
					this->_window->getPhoneBook()->addPhoneNumber(new PhoneEntryDT(dt->getRowId(i)), false);
				}
			}
		} else if (entry->getExpanded() == PBCollectionEntry::expandedMore) {
			// wszystkie istniej¹ce na "mapie numerów"
			for (NumbersMap::iterator it = numbersMap.begin(); it != numbersMap.end(); ++it) {
				this->_window->getPhoneBook()->addPhoneNumber(new PhoneEntryCnt(it->first, it->second), false);
			}
		}
		pb->unlockRefresh();
		return true;
	}

	bool KAccount::updateCollectionArchive(PBCollectionArchive* entry, PBCollectionItem* item) {
		PhoneBook* pb = this->_window->getPhoneBook();
		bool outgoing = pb->getCollOutgoing() == item;
		Tables::oTable dt = (outgoing) ? dtOutgoing : dtIncoming;
		pb->lockRefresh();
		item->removeAll(pb);
		if (entry->getExpanded() == PBCollectionEntry::expandedLess) {
			// po³¹czenia z ostatniej doby
			__time64_t limit = _time64(0) - 60*60*24;
			if (dt->getCount() > 0) {
				for (int i=dt->getCount() - 1; i >= 0; i--) {
					if (dt->getInt64(i, DTArchive::start) < limit) 
						break;
					this->_window->getPhoneBook()->addCallArchive(new CallArchiveEntryDT(dt->getRowId(i), outgoing));
				}
			}
		} if (entry->getExpanded() == PBCollectionEntry::expandedMore) {
			// wszystkie po³¹czenia
			for (int i=0; i < dt->getCount(); i++) {
				this->_window->getPhoneBook()->addCallArchive(new CallArchiveEntryDT(dt->getRowId(i), outgoing));
			}
		}
		pb->unlockRefresh();
		return true;
	}

	bool KAccount::isCalling(tCntId cnt) {
		for (CallList::iterator it = _calls.begin(); it != _calls.end(); ++it) {
			if ((*it)->isIncoming() && numbersMap.getContact((*it)->getPhoneUrl()) == cnt)
				return true;
		}
		return false;
	}


	void KAccount::setVisibleStatus(int status) {
		if (status == _visibleStatus) return;
		_visibleStatus = status;
		IMessage(&sIMessage_StatusChange(IMC_STATUSCHANGE , 0 , _visibleStatus , 0));

	}



	void KAccount::refreshAccountInfoThread() {
		_isFetchingInfo = true;
		bool success = false;
		try {
			CStdString ua;
			ICMessage(IMC_PLUG_VERSION, Ctrl->ID(), (int)ua.GetBuffer(20));
			ua.ReleaseBuffer();
			ua = "Konnekt-Actio " + ua;
			Stamina::oInternet internet = new Stamina::Internet((HINTERNET) ICMessage(IMC_HINTERNET_OPEN, (int)ua.c_str()));
			CStdString url;
			url.Format("%sversion=%x&user=%s&digest=%s", accountServer, ICMessage(IMC_PLUG_VERSION, Ctrl->ID()), urlEncode(this->_sipUser).c_str(), MD5Hex( "account" + this->_sipPassMD5 ).c_str() );

			Stamina::oRequest request = new Stamina::Request(new Stamina::Connection(internet, url), url, Stamina::Request::typeGet, 0, 0, 0, INTERNET_FLAG_RELOAD);
			request->sendThrowable();
			Stamina::RegEx response;
			response.setSubject( request->getResponse() );
			if (response.match("#<account>.*?<balance>([\\d,.]+)</balance>#")) {
				_gotBalance = true;
				_balance = atof(response[1].c_str());
				success = true;
				Stamina::log(logLog, "Actio", "refreshAccountInfoThread", "Stan konta: %f", _balance);
			} else {
				Stamina::log(logError, "Actio", "refreshAccountInfoThread", "OdpowiedŸ nie zawiera informacji o stanie konta! [%.200s] ", response.getSubjectRef().c_str());

			}
		} catch (Exception& e) {
			Stamina::log(logError, "Actio", "refreshAccountInfoThread", "Wyst¹pi³ b³¹d podczas pobierania stanu konta: " + e.getReason());
		}
		_isFetchingInfo = false;
		if (success && this->windowExists()) {
			//this->getWindow()->enableButton(buttonBalance, true);
			this->getWindow()->refreshAccountInfo();
		}
	}


	void KAccount::refreshAccountInfo() {
		if (_isFetchingInfo || !this->isConfigured()) return;

		//this->_threadRunner->run(boost::bind(KAccount::refreshAccountInfoThread, this), "RefreshAccountInfo");
	}


	bool KAccount::getBalance(double& balance) {
		balance = _balance;
		return _gotBalance;
	}


	// ----------------------------------------------------------------------
	
	PhoneEntryDT::PhoneEntryDT(tRowId row):PhoneEntry(Actio::dtNumbers->getInt64(row, Actio::DTNumbers::last)),_row(row) {
	}
	const PhoneUrl PhoneEntryDT::getPhoneUrl() {
		return PhoneUrl::createAuto(Actio::dtNumbers->getStr(_row, Actio::DTNumbers::phone));
	}
	CStdString PhoneEntryDT::getName() {
		return numbersMap.getDisplay(this->getPhoneUrl());
	}

	bool PhoneEntryDT::onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu) {
		PhoneEntry::onContextMenu(lv, li, level, vkey, pos, menu);
		return phoneEntryContextMenu(this, lv, li, level, vkey, pos, menu);
	}

	void PhoneEntryDT::removePBEntry(PhoneBook* pb) {
		dtNumbers->removeRow(this->_row);
		dtNumbers->lateSave(true);
		PhoneEntry::removePBEntry(pb);
	}


	// ----------------------------------------------------------------------

	PhoneEntryCnt::PhoneEntryCnt(const PhoneUrl& url, tColId cnt):PhoneEntry(0) {
		if (cnt == -1)
			_cnt = numbersMap.getContact(url);
		else
			_cnt = cnt;
		_url = url;
	}
	const PhoneUrl PhoneEntryCnt::getPhoneUrl() {
		return _url;
	}
	CStdString PhoneEntryCnt::getName() {
		return numbersMap.getDisplay(this->getPhoneUrl());
	}

	bool PhoneEntryCnt::onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu) {
		PhoneEntry::onContextMenu(lv, li, level, vkey, pos, menu);
		phoneEntryContextMenu(this, lv, li, level, vkey, pos, menu);
		return true;
	}

	void PhoneEntryCnt::removePBEntry(PhoneBook* pb) {
//		this->getTable()->removeRow(this->_row);
		PhoneEntry::removePBEntry(pb);
	}

	// ----------------------------------------------------------------

	CStdString CallArchiveEntryDT::getName() {
		return numbersMap.getDisplay(this->getPhoneUrl());
	}

	Tables::oTable CallArchiveEntryDT::getTable() {
		return _outgoing ? dtOutgoing : dtIncoming;
	}

	bool CallArchiveEntryDT::onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu) {
		CallArchiveEntry::onContextMenu(lv, li, level, vkey, pos, menu);
		phoneEntryContextMenu(this, lv, li, level, vkey, pos, menu);
		return true;
	}

	void CallArchiveEntryDT::removePBEntry(PhoneBook* pb) {
		this->getTable()->removeRow(this->_row);
		this->getTable()->lateSave(true);
		CallArchiveEntry::removePBEntry(pb);
	}


// ---------------


	bool KCallEntry::onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu) {
		CallEntry::onContextMenu(lv, li, level, vkey, pos, menu);
		phoneEntryContextMenu(this, lv, li, level, vkey, pos, menu);
		return true;
	}

	void KCallEntry::removePBEntry(PhoneBook* pb) {
		this->getCall()->callDrop();
		this->getCall()->callDestroy();
	}

// ---------------

	bool phoneEntryContextMenu(PBEntry* entry, ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu) {
		if (numbersMap.getContact(entry->getPhoneUrl()) == -1) {
			bool onList = true;
			menu->insertItem(new MenuItemW32_label("Dodaj do listy kontaktów", boost::bind(NumbersMap::createContact, &numbersMap, entry->getPhoneUrl(), onList)), 1);
		}
		return true;
	}


}}
