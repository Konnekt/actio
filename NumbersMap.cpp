#include "stdafx.h"

#include "actio.h"
#include "actio_main.h"

using namespace Stamina::PhonoLogic;
using namespace Stamina;

namespace Actio {
	void NumbersMap::updateContact(tCntId cnt) {
		removeContact(cnt);
		addContact(cnt);
	}

	void NumbersMap::removeContact(tCntId cnt) {
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (it->second == cnt) {
				it = this->erase(it);
			}
		}
	}

	void NumbersMap::addContact(tCntId cnt) {
		if (cnt == -1) return;
		cnt = Ctrl->DTgetID(DTCNT, cnt);
		CStdString nr;
		if (GETCNTI(cnt, CNT_NET) == Actio::net) {
			nr = GETCNTC(cnt, CNT_UID);
			if (!nr.empty() && this->getContact(nr) == -1) {
				this->insert(value_type(PhoneUrl::createAuto(nr), cnt));
			}
		}

		nr = GETCNTC(cnt, Actio::CNT::phoneUrl);
		if (!nr.empty() && this->getContact(nr) == -1) {
			this->insert(value_type(PhoneUrl::createAuto(nr), cnt));
		}

		nr = GETCNTC(cnt, CNT_CELLPHONE);
		if (!nr.empty() && this->getContact(nr) == -1) {
			this->insert(value_type(PhoneUrl::createPSTN(nr, phoneMobile), cnt));
		}
		nr = GETCNTC(cnt, CNT_PHONE);
		if (!nr.empty() && this->getContact(nr) == -1) {
			this->insert(value_type(PhoneUrl::createPSTN(nr, phoneHome), cnt));
		}
		nr = GETCNTC(cnt, CNT_WORK_PHONE);
		if (!nr.empty() && this->getContact(nr) == -1) {
			this->insert(value_type(PhoneUrl::createPSTN(nr, phoneWork), cnt));
		}
	}

	void NumbersMap::updateContacts() {
		LockerCS(this->_cs);
		this->clear();
		int c = ICMessage(IMC_CNT_COUNT);
		for (int i=1; i < c; i++) {
			addContact(i);
		}
	}

	tCntId NumbersMap::getContact(const PhoneUrl& url) {
		iterator found = this->find(url);
		if (found == this->end())
			return -1;
		else
			return found->second;
	}
	tCntId NumbersMap::getContact(const char* url) {
		return getContact(PhoneUrl::createAuto(url));
	}
	CStdString NumbersMap::getDisplay(const Stamina::PhonoLogic::PhoneUrl& url, tCntId cnt) {
		CStdString display = url.getName();
		if (display.empty() && cnt != -1) {
			GETCNTC(cnt, CNT_DISPLAY, stringBuffer(display, 101), 100);
			stringRelease(display);
		}
		if (display.empty())
			return url.getDisplayName();
		else
			return display;
	}
	CStdString NumbersMap::getDisplay(const PhoneUrl& url) {
		return getDisplay(url, getContact(url));
	}

	bool NumbersMap::contactExists(tCntId cnt, PhoneType type) {
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (it->second == cnt && (type == phoneNone || type == it->first.getType())) {
				return true;
			}
		}
		return false;
	}

	PhoneUrl NumbersMap::chooseNumber(tCntId cnt, PhoneType type) {
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (it->second == cnt && (type == phoneNone || type == it->first.getType())) {
				return it->first;
			}
		}
		return PhoneUrl();
	}

	int NumbersMap::createContact(const Stamina::PhonoLogic::PhoneUrl& url, bool onList) {
//		cnt = ICMessage(IMC_CNT_ADD, NET_NONE, (int)"");
		int cnt = ICMessage(IMC_CNT_ADD, Actio::net, (int)url.getUrl().c_str());
		SETCNTC(cnt, CNT_DISPLAY, url.getDisplayName());
//			SETCNTC(cnt, Actio::CNT::phoneUrl, call->getPhoneUrl().getUrl());
		if (!onList)
			SETCNTI(cnt, CNT_STATUS, ST_NOTINLIST, ST_NOTINLIST);
		ICMessage(IMC_CNT_CHANGED, cnt);
		ICMessage(IMI_REFRESH_CNT, cnt);
		IMDEBUG(DBG_MISC, "contact added (%d)", cnt);

		return cnt;
	}



}
