#pragma once

#include "actio.h"

#include "KAccount.h"

#include <Stamina/Version.h>

namespace Actio {
	extern Tables::oTable dtIncoming;
	extern Tables::oTable dtOutgoing;
	extern Tables::oTable dtNumbers;
	extern Stamina::PhonoLogic::KAccount* account;
	extern int mtypePhoneCall;

	const char * const accountServer = "http://www.konnekt.info/actio_account.php?";

	const char * const urlInformation = "http://www.konnekt.info/actio";
	const char * const urlHelp = "http://www.konnekt.info/actio/pytania.html";
	const char * const urlCreateAccount = "http://www.konnekt.info/actio/zaloz_konto.html";
	const char * const urlLostPassword = "http://www.konnekt.info/actio/przypomnij_haslo.html";
	const char * const urlSelfCare = "http://www.konnekt.info/actio/strefa_klienta.html";
	const char * const urlProblemQuality = "http://www.konnekt.info/actio/porady/jakosc.html";
	const char * const urlProblemMic = "http://www.konnekt.info/actio/porady/mikrofon.html";
	const char * const urlProblemSound = "http://www.konnekt.info/actio/porady/dzwiek.html";
	const char * const urlAccountBalance = "http://www.konnekt.info/actio/stan_konta.html";

	extern const CStdString serverHost;
	extern const CStdString stunHost;

	class NumbersMap: public std::map<Stamina::PhonoLogic::PhoneUrl, tCntId> {
	public:
		void updateContact(tCntId cnt);
		void removeContact(tCntId cnt);
		void addContact(tCntId cnt);
		int createContact(const Stamina::PhonoLogic::PhoneUrl& url, bool onList);
		/// Przebudowuje ca³¹ listê od nowa
		void updateContacts();
		tCntId getContact(const Stamina::PhonoLogic::PhoneUrl& url);
		tCntId getContact(const char* url);
		CStdString getDisplay(const Stamina::PhonoLogic::PhoneUrl& url);
		static CStdString getDisplay(const Stamina::PhonoLogic::PhoneUrl& url, tCntId cnt);
		bool contactExists(tCntId cnt, Stamina::PhonoLogic::PhoneType type = Stamina::PhonoLogic::phoneNone);
		Stamina::PhonoLogic::PhoneUrl chooseNumber(tCntId cnt, Stamina::PhonoLogic::PhoneType type = Stamina::PhonoLogic::phoneNone);
	private:
		Stamina::CriticalSection _cs;

	};

	extern NumbersMap numbersMap;

	void init();
	void start();
	void finish();
	void connect(bool byUser);
	void disconnect();
	void showWindow();
	bool isCalling(tCntId cnt);
	void makeCall(tCntId cnt, Stamina::PhonoLogic::PhoneType type = Stamina::PhonoLogic::phoneNone);
	void newCall();
	void calibrateSound();

	void createAccount(HWND parent);

	void updateVersion(Stamina::Version old);

};