#include "DllModule.h"
#include "ParseCommandLine.h"
#include "FullWindowedMode.h"
#include <aero/function/fp_call.hpp>
#include <ydwe/file/steam.h>
#include <ydwe/hook/iat.h>
#include <ydwe/path/filesystem_helper.h>
#include <ydwe/path/service.h>
#include <ydwe/util/unicode.h>
#include <ydwe/win/file_version.h>
#include <ydwe/win/pe_reader.h>
#include <SlkLib/IniReader.hpp>
#include <SlkLib/IniReader.cpp>

uintptr_t RealLoadLibraryA  = 0;
uintptr_t RealCreateWindowExA = 0;
uintptr_t RealSFileOpenArchive = (uintptr_t)::GetProcAddress(LoadLibraryW(L"Storm.dll"), (LPCSTR)266);

FullWindowedMode fullWindowedMode;

const char *PluginName();

HWND WINAPI FakeCreateWindowExA(
	DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HANDLE hlnstance,
	LPVOID lpParam)
{
	HWND hWnd = aero::std_call<HWND>(RealCreateWindowExA, dwExStyle, lpClassName, lpWindowName, dwStyle, x,  y,  nWidth, nHeight, hWndParent, hMenu, hlnstance, lpParam);
	if ((0 == strcmp(lpClassName, "Warcraft III"))
		&& (0 == strcmp(lpWindowName, "Warcraft III"))
		&& (NULL == g_DllMod.hWar3Wnd))
	{
		g_DllMod.hWar3Wnd = hWnd;
		if (g_DllMod.IsFullWindowedMode)
		{
			fullWindowedMode.Start();
		}
		else
		{
			if (g_DllMod.IsFixedRatioWindowed)
			{
				RECT r;
				if (::GetWindowRect(hWnd, &r))
				{
					FullWindowedMode::FixedRatio(hWnd, r.right - r.left, r.bottom - r.top);
				}
			}
		}
		g_DllMod.ThreadStart();
	}

	return hWnd;
}

void DaemonThreadProc()
{
	try
	{
		while (true)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));

			void LockingMouse();
			LockingMouse();
		}
	}
	catch (boost::thread_interrupted const&) 
	{
	}
}


HMODULE __stdcall FakeLoadLibraryA(LPCSTR lpFilePath)
{
	fs::path lib((const char*)lpFilePath);
	if (lib == "Game.dll")
	{
		if (!g_DllMod.hGameDll)
		{
			g_DllMod.hGameDll = ::LoadLibraryW((g_DllMod.patch_path / L"Game.dll").c_str());
			if (!g_DllMod.hGameDll)
			{
				g_DllMod.hGameDll = ::LoadLibraryW(L"Game.dll");
				if (!g_DllMod.hGameDll)
				{
					return g_DllMod.hGameDll;
				}
			}

			RealCreateWindowExA = ydwe::hook::iat(L"Game.dll", "user32.dll", "CreateWindowExA", (uintptr_t)FakeCreateWindowExA);
			HANDLE hMpq;
			aero::std_call<BOOL>(RealSFileOpenArchive, (g_DllMod.patch_path / "Patch.mpq").string().c_str(), 9, 6, &hMpq);

			g_DllMod.LoadPlugins();

			return g_DllMod.hGameDll;
		}
	}

	return aero::std_call<HMODULE>(RealLoadLibraryA, lpFilePath);
}

DllModule::DllModule()
	: hWar3Wnd(NULL)
	, IsWindowMode(false)
	, IsFullWindowedMode(false)
	, IsLockingMouse(false)
	, IsFixedRatioWindowed(false)
	, hGameDll(NULL)
{ }

void DllModule::ThreadStart()
{
	try
	{
		daemon_thread_.reset(new boost::thread(DaemonThreadProc));
	}
	catch (boost::thread_resource_error const&) 
	{
	}
}

void DllModule::ThreadStop()
{
	try
	{
		if (daemon_thread_)
		{
			daemon_thread_->interrupt();
			daemon_thread_->timed_join(boost::posix_time::milliseconds(1000));
			daemon_thread_.reset();
		}
	}
	catch (...)
	{
	}
}

void DllModule::LoadPlugins()
{
	try {
		fs::path plugin_path = ydwe_path / L"plugin" / L"warcraft3";

		slk::IniTable table;
		try {
			slk::IniReader::Read(ydwe::file::read_steam(plugin_path / L"config.cfg").read<slk::buffer>(), table);
		}
		catch(...) {
		}

		std::map<std::string, HMODULE> plugin_mapping;
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr(plugin_path); itr != end_itr; ++itr)
		{
			try {
				if (!fs::is_directory(*itr))
				{
					std::string utf8_name = ydwe::util::w2u(itr->path().filename().wstring());

					if ((! ydwe::path::equal(itr->path().filename(), std::string(PluginName()) + ".dll"))
						&& ydwe::path::equal(itr->path().extension(), L".dll")
						&& ("0" != table["Enable"][utf8_name]))
					{
						HMODULE module = ::LoadLibraryW(itr->path().c_str());
						if (module)
						{
							plugin_mapping.insert(std::make_pair(utf8_name, module));
						}
					}
				}
			}
			catch(...) {
			}
		}

		for (auto it = plugin_mapping.begin(); it != plugin_mapping.end(); )
		{
			HMODULE            module = it->second;
			uintptr_t fPluginName = (uintptr_t)::GetProcAddress(module, "PluginName");
			uintptr_t fInitialize = (uintptr_t)::GetProcAddress(module, "Initialize");
			if (fPluginName 
				&& fInitialize
				&& (it->first == std::string(aero::c_call<const char *>(fPluginName)) + ".dll"))
			{
				++it;
			}
			else
			{
				::FreeLibrary(module);
				plugin_mapping.erase(it++);
			}
		}

		for (auto it = plugin_mapping.begin(); it != plugin_mapping.end(); ++it)
		{
			aero::c_call<void>(::GetProcAddress(it->second, "Initialize"));
		}
	}
	catch(...) {
	}
}

void ResetConfig(slk::IniTable& table)
{
	table["MapSave"]["Option"] = "0";
	table["War3Patch"]["Option"] = "0";
	table["MapTest"]["LaunchOpenGL"] = "0";
	table["MapTest"]["LaunchWindowed"] = "1";
	table["MapTest"]["LaunchFullWindowed"] = "0";
	table["MapTest"]["LaunchLockingMouse"] = "0";
	table["MapTest"]["LaunchFixedRatioWindowed"] = "1";
	table["ScriptCompiler"]["EnableJassHelper"] = "1";
	table["ScriptCompiler"]["EnableJassHelperDebug"] = "0";
	table["ScriptCompiler"]["EnableJassHelperOptimization"] = "1";
	table["ScriptCompiler"]["EnableCJass"] = "0";
	table["ScriptInjection"]["Option"] = "0";
	table["ThirdPartyPlugin"]["EnableDotNetSupport"] = "1";
	table["ThirdPartyPlugin"]["EnableTesh"] = "1";
	table["ThirdPartyPlugin"]["EnableYDTrigger"] = "1";
	table["FeatureToggle"]["EnableManualNewId"] = "0";
	table["FeatureToggle"]["EnableTriggerCopyEncodingAutoConversion"] = "1";
	table["FeatureToggle"]["EnableShowInternalAttributeId"] = "0";
}

bool DllModule::SearchPatch(fs::path& result, std::wstring const& fv_str)
{
	try {
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr(ydwe_path / L"share" / L"patch"); itr != end_itr; ++itr)
		{
			try {
				if (fs::is_directory(*itr))
				{
					if (fs::exists(*itr / "Game.dll") && fs::exists(*itr / "Patch.mpq"))
					{
						ydwe::win::file_version fv((*itr / "Game.dll").c_str());
						if (fv_str == fv[L"FileVersion"])
						{
							result = *itr;
							return true;
						}
					}
				}
			}
			catch(...) {
			}
		}
	}
	catch(...) {
	}

	return false;
}

void DllModule::Attach()
{
	ParseCommandLine([&](std::wstring const& key, std::wstring const& val){
		if (key == L"ydwe")
		{
			ydwe_path = val;
		}
		else if (key == L"window")
		{
			IsWindowMode = true;
		}
	});

	try {
		slk::IniTable table;
		ResetConfig(table);

		try {
			slk::IniReader::Read(ydwe::file::read_steam(ydwe_path / L"bin" / L"EverConfig.cfg").read<slk::buffer>(), table);
		} 
		catch (...) {
		}

		IsFullWindowedMode   = "0" != table["MapTest"]["LaunchFullWindowed"];
		IsLockingMouse       = "0" != table["MapTest"]["LaunchLockingMouse"];
		IsFixedRatioWindowed = "0" != table["MapTest"]["LaunchFixedRatioWindowed"];

		try {
			if (table["War3Patch"]["Option"] == "1")
			{
				if (table["MapSave"]["Option"] == "1")
				{
					SearchPatch(patch_path, L"1.20.4.6074");
				}
				else if (table["MapSave"]["Option"] == "2")
				{
					SearchPatch(patch_path, L"1.24.4.6387");
				}
			}
			else if (table["War3Patch"]["Option"] == "2")
			{
				patch_path = ydwe_path / L"share" / L"patch"/ table["War3Patch"]["DirName"];
			}
		}
		catch (...) {
		}
	} 
	catch (...) {
	}

	RealLoadLibraryA  = ydwe::hook::iat(L"War3.exe", "kernel32.dll", "LoadLibraryA", (uintptr_t)FakeLoadLibraryA);
}

void DllModule::Detach()
{
}
