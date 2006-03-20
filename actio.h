#pragma once

using Tables::tColId;
using Tables::tRowId;

#define ACTIO_CFG_PREFIX "Actio/"

namespace Actio {

	const unsigned int net = 64;
	const unsigned int baseId = net * 1000;

	namespace ACT {

		const unsigned int configGroup = baseId + 0;
		const unsigned int statusGroup = baseId + 1;
		const unsigned int configMoreGroup = baseId + 2;

		const unsigned int statusOnline = baseId + 10;
		const unsigned int statusOffline = baseId + 11;
		const unsigned int makeCall = baseId + 20;
		const unsigned int answerCall = baseId + 21;
		const unsigned int showWindow = baseId + 22;

		const unsigned int linkCreateAccount = baseId + 30;
		const unsigned int linkInfo = baseId + 31;
		const unsigned int linkHelp = baseId + 32;
		const unsigned int calibrate = baseId + 33;
		const unsigned int moreOptions = baseId + 34;
		const unsigned int linkSelfcare = baseId + 35;
		const unsigned int linkLostPassword = baseId + 36;
		const unsigned int haveAccount = baseId + 37;

		const unsigned int banner = baseId + 39;

		const unsigned int makeCallGroup = baseId + 40;
		const unsigned int makeCallSip = makeCallGroup + 1;
		const unsigned int makeCallMobile = makeCallGroup + 2;
		const unsigned int makeCallHome = makeCallGroup + 3;
		const unsigned int makeCallWork = makeCallGroup + 4;

		const unsigned int configBindTip = baseId + 50;

	};
	namespace CFG {
		const unsigned int userName = baseId + 0;
		const unsigned int userPass = baseId + 1;
		const unsigned int userPassMD5 = baseId + 2;
		const unsigned int publicAddress = baseId + 3;
		const unsigned int enableAtStartup = baseId + 4;
		const unsigned int identity = baseId + 5;
		const unsigned int tcpPort = baseId + 6;
		const unsigned int udpPort = baseId + 7;
		const unsigned int rtpPort = baseId + 8;

		const unsigned int callPopup = baseId + 9;

		const unsigned int soundAlert = baseId + 10;
		const unsigned int soundFailed = baseId + 11;

		const unsigned int smsDefaultAction = baseId + 12;

		const unsigned int useSTUN = baseId + 20;

		const unsigned int bindIP = baseId + 21;

		const unsigned int useICE = baseId + 22;

		const unsigned int useTCP = baseId + 23;
	};
	namespace CNT {
		const unsigned int phoneUrl = baseId + 0;
	};
	namespace DTArchive {
		const tColId phone = 1;
		const tColId type = 2;
		const tColId start = 3;
		const tColId duration = 4;
	};
	namespace DTNumbers {
		const tColId phone = 1;
		const tColId last = 2;
	};

};