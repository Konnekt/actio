#pragma once
#include <Stamina\PhonoLogic\Account.h>
#include <Stamina\PhonoLogic\PBCollectionEntry.h>
#include <Stamina\PhonoLogic\PhoneEntry.h>
#include <Stamina\PhonoLogic\CallArchiveEntry.h>
#include <Stamina\PhonoLogic\PhoneUrl.h>

using namespace Actio;

namespace Stamina {namespace PhonoLogic {

	class KAccount:public Account {
	public:

		KAccount();
		~KAccount();

		std::string getUserAgent();

		void connect(bool byUser);
		void disconnect();

		void onConnected();
		void onDisconnected();
		void onConnectionFailure();
		void onConnecting();
		virtual void onNewIncomingCall(const oCall& call);
		virtual void onCriticalError(const CStdString& title, const CStdString& msg);
		virtual void onUnauthorized();
		virtual void onButtonPressed(PhoneButton button);
		virtual void onConfigChanged();
		virtual void onWindowActivated();
	
		void onTapiHeaderWatchEvent(SIPX_HEADERWATCH_INFO* info);
		void onInitialize();

		virtual void createWindow();

		virtual bool isConfigured();

		virtual CStdString getSetting(const CStdString& name, const CStdString& def="") {
			int id = Ctrl->DTgetNameID(DTCFG, ACTIO_CFG_PREFIX + name);
			if (id == -1) return def;
			return Ctrl->DTgetStr(DTCFG, 0, id);
		}
		virtual int getIntSetting(const CStdString& name, int def=0) {
			int id = Ctrl->DTgetNameID(DTCFG, ACTIO_CFG_PREFIX + name);
			if (id == -1) return def;
			return Ctrl->DTgetInt(DTCFG, 0, id);
		}
		virtual void setSetting(const CStdString& name, const CStdString& value) {
			Ctrl->DTsetStr(DTCFG, 0, ACTIO_CFG_PREFIX + name, value);
		}
		virtual void setSetting(const CStdString& name, int value) {
			Ctrl->DTsetInt(DTCFG, 0, ACTIO_CFG_PREFIX + name, value);
		}

		virtual CStdString getSoundFile(const oCall& call, const CStdString& file);
		//virtual void playFile(const oCall& call, const CStdString& file, bool loop = false);


		CStdString phoneToDisplay(const PhoneUrl& url);

		void callToHistory(const oCall& call);

		void rejectedCallToHistory(const PhoneUrl& url, CallArchiveEntry::Type type);

		void markPhoneAsUsed(const PhoneUrl& url);

		bool updateCollectionBook(PBCollectionBook* entry, PBCollectionItem* item);

		bool updateCollectionArchive(PBCollectionArchive* entry, PBCollectionItem* item);


		bool isCalling(tCntId cnt);

		void setVisibleStatus(int status);
		int getVisibleStatus() {
			return _visibleStatus;
		}

		
		void refreshAccountInfo();

		bool getBalance(double& balance);

		void refreshAccountInfoThread();

		virtual CallEntry* createCallEntry(const oCall& call);


	private:

		int _visibleStatus;
		double _balance;
		bool _gotBalance;
		bool _isFetchingInfo;


	};

	// ------------------

    bool phoneEntryContextMenu(PBEntry* entry, ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu);

	class KCallEntry: public CallEntry {
	public:
		KCallEntry(const oCall& call):CallEntry(call) {
		}

		bool onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu);
		void removePBEntry(PhoneBook* pb);

	};


	// ------------------

	class PhoneEntryDT : public PhoneEntry 
	{
	public:
		PhoneEntryDT(tRowId);
	public:
		const PhoneUrl getPhoneUrl();
		CStdString getName();

		bool onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu);
		void removePBEntry(PhoneBook* pb);

	private:
		tRowId _row;
	};

	class PhoneEntryCnt : public PhoneEntry 
	{
	public:
		PhoneEntryCnt(const PhoneUrl& url, tColId cnt = -1);
	public:
		const PhoneUrl getPhoneUrl();
		CStdString getName();

		bool onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu);
		void removePBEntry(PhoneBook* pb);

	private:
		PhoneUrl _url;
		tCntId _cnt;
	};


	class CallArchiveEntryDT:public CallArchiveEntry {
	public:
		CallArchiveEntryDT(tRowId row, bool outgoing)
			:_row(row), _outgoing(outgoing) {}

		Time64 getCallTime() {
			return this->getTable()->getInt64(_row, DTArchive::start);
		}
		Time64 getCallDuration() {
			return this->getTable()->getInt64(_row, DTArchive::duration);
		}
		bool isOutgoing() {
			return _outgoing;
		}

		Type getType() {
			return (Type) this->getTable()->getInt(_row, DTArchive::type);
		}

		const PhoneUrl getPhoneUrl() {
			return PhoneUrl::createAuto(this->getTable()->getStr(_row, DTArchive::phone));
		}
		CStdString getName();

		bool onContextMenu(ListWnd::ListView* lv, const ListWnd::oItem& li, int level, int vkey, const Point& pos, const oMenu& menu);
		void removePBEntry(PhoneBook* pb);


	protected:

		Tables::oTable getTable();

		tRowId _row;
		bool _outgoing;

	};



};};