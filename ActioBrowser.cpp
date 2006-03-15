#include "stdafx.h"
#include "ActioBrowser.h"
#include <Stamina\FileResource.h>
#include <Stamina\RegEx.h>

#include "actio.h"
#include "actio_main.h"

#include "resource.h"


namespace Actio {

	void __stdcall ActioBrowserSink::ProgressChange(long pos, long max) {
		if (pos != -1 && max > 0) {
			_owner->_wndProgress->showWindow(SW_SHOW);
			_owner->_wndProgress->sendMessage(PBM_SETRANGE32, 0, max);
			_owner->_wndProgress->sendMessage(PBM_SETPOS, pos, 0);
		} else {
			_owner->_wndProgress->showWindow(SW_HIDE);
			_owner->_wndProgress->sendMessage(PBM_SETPOS, 0, 0);
		}
		
	}

	class RegExParams: public Stamina::RegEx {
	public:
		RegExParams(const StringRef& val) {
			this->setSubject(val.c_str());
		}
		String get(const StringRef& name) {
			this->match(("#&" + name + "=([^&]+)#").c_str());
			return this->getSub(1);
		}
	};

	void __stdcall ActioBrowserSink::DocumentComplete(IDispatch* disp, VARIANT* vUrl) {
		__super::DocumentComplete(disp, vUrl);
	}

	void __stdcall ActioBrowserSink::DownloadComplete() {
		__super::DownloadComplete();

		_owner->_wndInfo->showWindow(SW_HIDE);

		String url = _owner->_browser->getLocationURL();

		Stamina::RegEx re;
		re.setSubject(url.c_str());
		if (re.match("#^http://.+/actio/(zaloz_konto|aktywacja)#i")) {// sprawdzamy zawartoœæ na znaczniki
			Stamina::RegEx content;
			content.setSubject( _owner->_browser->getInnerHTML() );
			if (content.match("#<!--konnektTag:([^&]+)(?:(&.+))?-->#")) {
				String cmd = content.getSub(1);

				RegExParams params (content.getSub(2));

				if (cmd == "setAccount") {
					String id = params.get("id");
					String pass = params.get("pass");
					UIActionCfgSetValue(sUIAction(Actio::ACT::configGroup, CFG::userName | IMIB_CFG), id.c_str());
					UIActionCfgSetValue(sUIAction(Actio::ACT::configGroup, CFG::userPass | IMIB_CFG), pass.c_str());
					Ctrl->DTsetStr(DTCFG, 0, CFG::userName, id.c_str());
					Ctrl->DTsetStr(DTCFG, 0, CFG::userPass, pass.c_str());

					ICMessage(IMC_CFG_SAVE);
					ICMessage(IMC_SAVE_CFG);
				}

			}
		}

	}




	void __stdcall ActioBrowserSink::BeforeNavigate2(IDispatch*,VARIANT* vUrl,VARIANT* vFlags,VARIANT* vTarget,VARIANT* vPost,VARIANT* vHeaders,VARIANT_BOOL* cancel) {

		String url = vUrl->bstrVal;

		Stamina::RegEx re;
		re.setSubject(url.c_str());
		if (re.match("#^http://www.konnekt.info(/forum|.+regulamin)#i")) {// strony które powinny byæ otwierane w pe³nym oknie
			*cancel = true;
			ShellExecute(0, "open", url.a_str(), "", "", SW_SHOW);
		} else if (re.match("#^konnekt:([^&]+)(?:&(.+))?#")) {
			*cancel = true;
			String cmd = re.getSub(1);
			if (cmd == "cancel") {
				_owner->postMessage(WM_CLOSE, 0, 0);
			} else if (cmd == "close") {
				_owner->postMessage(WM_CLOSE, 0, 0);
			} else if (cmd == "refresh") {
				_owner->onCommand(1, ID_REFRESH, 0);
			} else if (cmd == "back") {
				_owner->onCommand(1, ID_GOBACK, 0);
			}
			return;
		}
		

		_owner->_wndProgress->showWindow(SW_SHOW);
		_owner->_wndInfo->showWindow(SW_SHOW);
		_owner->_wndInfo->setText(L"Chwileczkê, wczytujê...");



	}


	ActioBrowser::ActioBrowser():WindowDialog() {
	}

	void ActioBrowser::run(bool modal, HWND parent, const StringRef& title) {
		_title = title;
		if (modal) {
			this->createModal(Stamina::getInstance(), MAKEINTRESOURCE(IDD_BROWSER), parent);
		} else {
			this->create(Stamina::getInstance(), MAKEINTRESOURCE(IDD_BROWSER), parent);
			this->showWindow(SW_SHOW);
		}
	}


	void ActioBrowser::onCreateWindow() {
		__super::onCreateWindow();

		Stamina::log(logEvent, "ActioBrowser", "onCreateWindow", "title=%s", _title.c_str());

		this->setText(_title);

		_butReopen = new ButtonX(this->getChild(IDC_REOPEN));
		_wndInfo = this->getChildWindow(IDC_INFO);
		_wndProgress = this->getChildWindow(IDC_PROGRESS1);

		_sink = new ActioBrowserSink(this);
		_browser = BrowserCtrl::replaceWindow(this->getChild(IDC_BROWSER), _sink);

		_browser->navigate(_url, _postData, "User-Agent: KONNEKT - Actio (Core=" + FileVersion(Ctrl->hInst()).getFileVersion().getString(4) + "; Actio=" + FileVersion(Ctrl->hDll()).getFileVersion().getString(4) + ")\r\n");

	}

	void ActioBrowser::onDestroyWindow() {
		delete _browser;

		__super::onDestroyWindow();
	}

	void ActioBrowser::onCommand(int code, int id, HWND sender) {
		__super::onCommand(code, id, sender);
		if (code == BN_CLICKED || (code == 1 && sender == 0)) {
			switch (id) {
				case IDC_REOPEN:
					ShellExecute(0, "open", this->_originalUrl.c_str(), "", "", SW_SHOW);
					this->endDialog(IDCANCEL);
					return;
				case IDCANCEL:
					this->endDialog(IDCANCEL);
					return;
				case ID_COPY:
					this->_browser->getBrowser()->ExecWB(OLECMDID_COPY, OLECMDEXECOPT_DODEFAULT, 0, 0);
					return;
				case ID_GOBACK:
					this->_browser->getBrowser()->GoBack();
					return;
				case ID_REFRESH: {
					_variant_t type = REFRESH_COMPLETELY;
					this->_browser->getBrowser()->Refresh2(&type);
					return;}
			}
		}
	}


}