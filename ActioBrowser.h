#pragma once

#include <Stamina\UI\BrowserCtrl.h>
#include <Stamina\UI\WindowDialog.h>
#include <Stamina\UI\ButtonX.h>
#include <Stamina\String.h>

using namespace Stamina;
using namespace Stamina::UI;

namespace Actio {

	class ActioBrowserSink: public BrowserCtrlSink {
	public:

		ActioBrowserSink(class ActioBrowser* owner) {
			_owner = owner;
		}

		void __stdcall ProgressChange(long a, long b);
		void __stdcall DocumentComplete(IDispatch*, VARIANT*);

		void __stdcall DownloadComplete();

		void __stdcall BeforeNavigate2(IDispatch*,VARIANT*,VARIANT*,VARIANT*,VARIANT*,VARIANT*,VARIANT_BOOL*);

		class ActioBrowser* _owner;
	};


	class ActioBrowser:public WindowDialog {
	public:

		friend class ActioBrowserSink;

		ActioBrowser();

		void run(bool modal, HWND parent, const StringRef& title);

		void setUrl(const StringRef& url, const StringRef& data) {
			_url = url;
			_postData = data;
		}

		void setOriginalUrl(const StringRef& url) {
			_originalUrl = url;
		}

	protected:

		virtual void onCreateWindow();
		virtual void onDestroyWindow();
		virtual void onCommand(int code, int id, HWND sender);

		BrowserCtrl* _browser;
		ActioBrowserSink* _sink;

		ButtonX* _butReopen;
		oWindow _wndInfo;
		oWindow _wndProgress;

		String _url;
		String _postData;
		String _originalUrl;
		String _title;

	};

};
