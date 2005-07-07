#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0400

#include <windows.h>
#include <string>
#include <stdstring.h>
#include <map>
#include <stack>
#include <shellapi.h>
#include <boost\bind.hpp>

#include <tapi\sipXtapiExt.h>
#include <tapi\sipXtapiEvents.h>

#include <Stamina\Helpers.h>
#include <Stamina\WinHelper.h>
#include <Stamina\PhonoLogic\Account.h>
#include <Stamina\ThreadInvoke.h>

#include <konnekt/plug_export.h>
#include <konnekt/ui.h>
#include <konnekt/plug_func.h>
#include <konnekt/core_tables.h>
#include <konnekt/knotify.h>
#include <konnekt/ksound.h>
