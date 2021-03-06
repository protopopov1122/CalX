/*
        SPDX-License-Identifier: LGPL-3.0

        Copyright (c) 2017-2019 Jevgenijs Protopopovs

        This file is part of CalX project.

        CalX is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License version 3 as published
   by
        the Free Software Foundation.


        CalX is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with CalX.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "calx/ui/CalxApp.h"
#ifdef OS_WIN
#include <windows.h>
#endif
#include "calx/ctrl-lib/device/DeviceManager.h"
#include "calx/ui/CalxConfigLoader.h"
#include "calx/ui/CalxDebugConsole.h"
#include "calx/ui/CalxErrorHandler.h"
#include "calx/ui/coord/CalxCoordPanel.h"
#include "calx/ui/script/CalXScriptEngine.h"
#include <wx/filedlg.h>

#include "calx/ctrl-lib/SignalHandler.h"
#include "calx/ctrl-lib/conf/ConfigManager.h"
#include "calx/ctrl-lib/conf/ConfigValidator.h"
#include "calx/ctrl-lib/logger/Global.h"
#include "calx/ctrl-lib/logger/Filter.h"
#include "calx/ctrl-lib/logger/Shortcuts.h"
#include "calx/ui/logs/LogSink.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <tuple>
#include <string>

namespace CalX::UI {

	wxDEFINE_EVENT(wxEVT_APP_ERROR, wxThreadEvent);
	wxDEFINE_EVENT(wxEVT_APP_ALERT, wxThreadEvent);

	// Utility startup methods
	// System startup is quite complex action
	// so it is split into several methods
	std::unique_ptr<ConfigurationCatalogue> CalxApp::loadConfiguration() {
		// Determine which configuration to use
		CalxConfigLoader *confLoader = new CalxConfigLoader(nullptr, wxID_ANY);
		confLoader->load();
		if (confLoader->isExiting()) {
			confLoader->Destroy();
			return nullptr;
		}
		std::string path = confLoader->getFileName();
		confLoader->Destroy();

		// Load selected configuration
		std::unique_ptr<ConfigurationCatalogue> conf_ptr = nullptr;
		std::ifstream cnf(path);
		if (!cnf.good()) {
			wxMessageBox(__("Can't open configuration. Using default values."),
			             __("Warning"), wxICON_WARNING);
			conf_ptr = std::make_unique<ConfigManager>();
		} else {
			conf_ptr = INIConfiguration::load(cnf, std::cout);
		}
		cnf.close();
		return conf_ptr;
	}

	DeviceManager_getter CalxApp::loadDeviceDrivers(
	    ConfigurationCatalogue &conf) {
		// Load specified dynamic library
		std::string lib_addr = conf.getEntry(CalxConfiguration::UserInterface)
		                           ->getString(CalxUIConfiguration::DeviceLibrary,
		                                       STRINGIZE(DEVICES_LIB));

		this->dynlib =
		    new wxDynamicLibrary(wxDynamicLibrary::CanonicalizeName(lib_addr),
		                         wxDL_DEFAULT | wxDL_QUIET);
		if (!dynlib->IsLoaded()) {
			wxMessageBox(__("Device API plugin not found\nSpecify library location"),
			             __("Warning"), wxOK | wxICON_WARNING);
			loadDevicesPlugin();
		}
		if (!dynlib->IsLoaded()) {
			wxMessageBox(__("Dynamic library not found"), __("Error"),
			             wxOK | wxICON_ERROR);
			return nullptr;
		}

		//
		bool suc;
		void *raw_getter = dynlib->GetSymbol("getDeviceManager", &suc);
		DeviceManager_getter getter = *((DeviceManager_getter *) &raw_getter);
		if (!suc) {
			wxMessageBox(__("Dynamic library is corrupt"), __("Error"),
			             wxOK | wxICON_ERROR);
			return nullptr;
		}
		return getter;
	}

	void CalxApp::initDebugConsole(ConfigurationCatalogue &conf) {
		this->debug_mode = conf.getEntry(CalxConfiguration::UserInterface)
		                       ->getBool(CalxUIConfiguration::Debug, false);
		if (this->debug_mode) {
#ifdef OS_WIN
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
#endif
		}
	}

	void CalxApp::startDebugConsole(ConfigurationCatalogue &conf) {
		if (this->debug_mode &&
		    this->sysman->getConfiguration()
		        .getEntry(CalxConfiguration::UserInterface)
		        ->getBool(CalxUIConfiguration::Console, false)) {
			this->debug_console = std::make_unique<CalxDebugConsole>(*this->sysman);
			this->debug_console->Run();
		} else {
			this->debug_console = nullptr;
		}
		this->Bind(wxEVT_APP_ERROR, &CalxApp::OnErrorEvent, this);
		this->Bind(wxEVT_APP_ALERT, &CalxApp::OnAlertEvent, this);
	}

	void CalxApp::initLogging(ConfigurationCatalogue &conf) {
#define SETUP_LOG(id, dest, sever)                                             \
	{                                                                            \
		std::string logger =                                                       \
		    conf.getEntry(CalxConfiguration::Logging)->getString(id, "");          \
		auto filter = LoggerFilter::severity_exact(sever);                         \
		if (logger.compare("stdout") == 0) {                                       \
			JournalSink &sink = GlobalLogger::getController().newStreamSink(         \
			    GlobalLogger::getSink(dest), std::cout);                             \
			sink.setFilter(filter);                                                  \
		} else if (logger.compare("uiout") != 0 && logger.length() > 0) {          \
			JournalSink &sink = GlobalLogger::getController().newFileSink(           \
			    GlobalLogger::getSink(dest), logger);                                \
			sink.setFilter(filter);                                                  \
		}                                                                          \
	}

		SETUP_LOG(CalxLoggingConfiguration::Errors, GlobalLoggingSink::Errors,
		          LoggingSeverity::Error)
		SETUP_LOG(CalxLoggingConfiguration::Warnings, GlobalLoggingSink::Warnings,
		          LoggingSeverity::Warning)
		SETUP_LOG(CalxLoggingConfiguration::Debug, GlobalLoggingSink::Debug,
		          LoggingSeverity::Debug)
		SETUP_LOG(CalxLoggingConfiguration::Info, GlobalLoggingSink::Information,
		          LoggingSeverity::Info)
		GlobalLogger::getController().dropSink("default");

#undef SETUP_LOG

		this->journalManager = std::make_unique<CalxJournalManager>(conf);
	}

	void CalxApp::updateLogging(ConfigurationCatalogue &conf) {
		CalxLogSink *logSink = this->frame->getLogSink();
		if (logSink != nullptr) {
			bool replaceStdout =
			    conf.getEntry(CalxConfiguration::Logging)
			        ->getBool(CalxLoggingConfiguration::RedirectStdout, false);
#define SETUP_LOG(id, dest, sever)                                             \
	{                                                                            \
		std::string logger =                                                       \
		    conf.getEntry(CalxConfiguration::Logging)->getString(id, "");          \
		auto filter = LoggerFilter::severity_exact(sever);                         \
		if (logger.compare("uiout") == 0 ||                                        \
		    (logger.compare("stdout") == 0 && replaceStdout)) {                    \
			GlobalLogger::getController().dropSink(GlobalLogger::getSink(dest));     \
			JournalSink &sink = GlobalLogger::getController().appendSink(            \
			    GlobalLogger::getSink(dest),                                         \
			    logSink->getSink(GlobalLogger::getSink(dest)));                      \
			sink.setFilter(filter);                                                  \
		}                                                                          \
	}
			SETUP_LOG(CalxLoggingConfiguration::Errors, GlobalLoggingSink::Errors,
			          LoggingSeverity::Error)
			SETUP_LOG(CalxLoggingConfiguration::Warnings, GlobalLoggingSink::Warnings,
			          LoggingSeverity::Warning)
			SETUP_LOG(CalxLoggingConfiguration::Debug, GlobalLoggingSink::Debug,
			          LoggingSeverity::Debug)
			SETUP_LOG(CalxLoggingConfiguration::Info, GlobalLoggingSink::Information,
			          LoggingSeverity::Info)
#undef SETUP_LOG
		}
	}

	std::unique_ptr<ExtEngine> CalxApp::loadExtensionEngine(
	    ConfigurationCatalogue &conf) {
		std::string ext_addr =
		    conf.getEntry(CalxConfiguration::Extensions)
		        ->getString(CalxExtensionsConfiguration::Engine, "");
		std::unique_ptr<ExtEngine> ext = nullptr;
		if (!ext_addr.empty()) {
			this->extLib =
			    new wxDynamicLibrary(wxDynamicLibrary::CanonicalizeName(ext_addr),
			                         wxDL_DEFAULT | wxDL_QUIET);
			if (!this->extLib->IsLoaded()) {
				wxMessageBox(__("Extension engine can't be loaded"), __("Warning"),
				             wxOK | wxICON_WARNING);
			} else {
				bool ext_suc;
				if (extLib->HasSymbol("getUIExtEngine")) {
					void *ext_raw_getter = extLib->GetSymbol("getUIExtEngine", &ext_suc);
					if (!ext_suc) {
						wxMessageBox(__("Extension engine can't be loaded"), __("Warning"),
						             wxOK | wxICON_WARNING);
					} else {
						UIExtEngine_getter ext_getter =
						    *((UIExtEngine_getter *) &ext_raw_getter);
						UIExtEngine *uiext = ext_getter();
						uiext->uiInit(wxGetApp());
						ext = std::unique_ptr<ExtEngine>(uiext);
					}
				} else {
					void *ext_raw_getter = extLib->GetSymbol("getExtEngine", &ext_suc);
					if (!ext_suc) {
						wxMessageBox(__("Extension engine can't be loaded"), __("Warning"),
						             wxOK | wxICON_WARNING);
					} else {
						ExtEngine_getter ext_getter =
						    *((ExtEngine_getter *) &ext_raw_getter);
						ext = std::unique_ptr<ExtEngine>(ext_getter());
					}
				}
			}
		} else {
			this->extLib = nullptr;
		}
		return ext;
	}

	void CalxApp::loadScriptEngine(ConfigurationCatalogue &conf) {
		std::string script_eng_addr =
		    conf.getEntry(CalxConfiguration::Scripting)
		        ->getString(CalxScriptingConfiguration::Engine, "");
		this->scriptFactory = nullptr;
		this->script_env = nullptr;
		this->scriptLib = nullptr;
		if (!script_eng_addr.empty()) {
			this->scriptLib = new wxDynamicLibrary(
			    wxDynamicLibrary::CanonicalizeName(script_eng_addr),
			    wxDL_DEFAULT | wxDL_QUIET);
			if (!this->scriptLib->IsLoaded()) {
				wxMessageBox(__("Scripting engine can't be loaded"), __("Warning"),
				             wxOK | wxICON_WARNING);
			} else {
				bool scr_success;
				if (this->scriptLib->HasSymbol("getScriptUIEngine")) {
					void *scr_raw_getter =
					    this->scriptLib->GetSymbol("getScriptUIEngine", &scr_success);
					ScriptUIEngine_getter scr_getter =
					    *((ScriptUIEngine_getter *) &scr_raw_getter);
					this->script_env =
					    std::make_unique<CalXAppScriptEnvironment>(wxGetApp());
					this->scriptFactory =
					    std::unique_ptr<CalXScriptUIFactory>(scr_getter());
				} else {
					wxMessageBox(__("Scripting engine can't be loaded"), __("Warning"),
					             wxOK | wxICON_WARNING);
				}
			}
		}
	}

	void CalxApp::startInitScript(ConfigurationCatalogue &conf) {
		if (this->scriptFactory != nullptr &&
		    this->sysman->getConfiguration()
		        .getEntry(CalxConfiguration::Scripting)
		        ->getBool(CalxScriptingConfiguration::AutoInit, false)) {
			CalXScriptHookThread *th = new CalXScriptHookThread(
			    this->sysman->getConfiguration()
			        .getEntry(CalxConfiguration::Scripting)
			        ->getString(CalxScriptingConfiguration::Main, "scripts/main.lua"),
			    this->sysman->getConfiguration()
			        .getEntry(CalxConfiguration::Scripting)
			        ->getString(CalxScriptingConfiguration::InitEntry, "init"));
			th->Run();
		}
	}

	// Application entry-point. It performs system startup and
	// calls methods above
	bool CalxApp::OnInit() {
		// Load configuration
		std::unique_ptr<ConfigurationCatalogue> conf_ptr =
		    this->loadConfiguration();
		if (conf_ptr == nullptr) {
			return false;
		}
		ConfigurationCatalogue &conf = *conf_ptr;

		// Init device drivers
		DeviceManager_getter getter = this->loadDeviceDrivers(conf);
		if (getter == nullptr) {
			return false;
		}

		// Open debug console if necessary
		this->initDebugConsole(conf);

		// Init logging subsystem
		this->initLogging(conf);

		// Load extension engine
		std::unique_ptr<ExtEngine> ext = this->loadExtensionEngine(conf);

		// Initialize main system structures
		std::unique_ptr<DeviceManager> devman =
		    std::unique_ptr<DeviceManager>(getter());
		this->sysman = std::make_unique<DefaultSystemManager>(
		    std::move(devman), std::move(conf_ptr), std::move(ext));

		// Error handler will be released in the OnExit method
		this->error_handler =
		    new CalxErrorHandler(  // lgtm [cpp/resource-not-released-in-destructor]
		        *this->sysman);

		// Start debug console if necessary
		this->startDebugConsole(conf);

		// Init unit processor
		this->unit_processor = std::make_unique<CalxUnitProcessor>(conf);

		// Load script engine
		this->loadScriptEngine(conf);

		// Show main program window
		// wxWidgets will release it automatically
		this->frame = new CalxFrame(
		    __("CalX UI"));  // lgtm [cpp/resource-not-released-in-destructor]
		this->frame->Show(true);
		this->frame->Maximize(true);

		// Update logging
		this->updateLogging(conf);

		// Init signal and SEH handlers which perform emergency shutdown if needed
		setup_signals(*this->sysman);

		// System started. Write it to journal
		Info(this->getJournal()) << "System started" << Flush();

		// Start initialization script if it is defined
		this->startInitScript(conf);

		return true;
	}

	bool CalxApp::hasScriptEngine() {
		return this->scriptFactory != nullptr;
	}

	std::shared_ptr<CalXScriptUIEnvironment> CalxApp::getScriptEnvironment() {
		return this->script_env;
	}

	std::shared_ptr<CalXScriptUIFactory> CalxApp::getScriptFactory() {
		return this->scriptFactory;
	}

	std::unique_ptr<CalXScript> CalxApp::loadScript(const std::string &path) {
		if (this->scriptFactory == nullptr) {
			wxMessageBox(FORMAT(__("Scripting engine is not loaded! Script '%s' "
			                       "can\'t be loaded"),
			                    path),
			             __("Warning"), wxOK | wxICON_WARNING);
			return nullptr;
		} else {
			auto script = this->scriptFactory->newScript(*this->script_env);
			script->loadScript(path);
			return script;
		}
	}

	// This method is called before system shuts down
	// So it frees resources and stops all actions
	int CalxApp::OnExit() {
		Info(this->getJournal()) << "System shutdown" << Flush();
		if (this->debug_console != nullptr) {
			this->debug_console->Kill();
		}

		delete this->error_handler;
		this->sysman.reset(nullptr);
		this->dynlib->Detach();
		this->dynlib->Unload();
		if (this->extLib != nullptr) {
			this->extLib->Detach();
			this->extLib->Unload();
		}
		if (this->scriptLib != nullptr) {
			this->scriptLib->Detach();
			this->scriptLib->Unload();
		}

#ifdef OS_WIN
		if (this->debug_mode) {
			FreeConsole();
		}
#endif

		return 0;
	}

	// Fatal exception handlers. They will try to gently stop all actions
	// to prevent physical corruption
	void CalxApp::OnUnhandledException() {
		calx_terminate();
		exit(-1);
	}

	void CalxApp::OnFatalException() {
		calx_terminate();
		exit(-1);
	}

	SystemManager &CalxApp::getSystemManager() {
		return *this->sysman;
	}

	CalxErrorHandler *CalxApp::getErrorHandler() {
		return this->error_handler;
	}

	CalxUnitProcessor &CalxApp::getUnitProcessor() {
		return *this->unit_processor;
	}

	CalxFrame *CalxApp::getMainFrame() {
		return this->frame;
	}

	JournalLogger &CalxApp::getJournal() {
		if (this->journalManager) {
			return this->journalManager->getJournal();
		} else {
			return GlobalLogger::getLogger();
		}
	}

	void CalxApp::loadDevicesPlugin() {
		wxFileDialog openDialog(nullptr, __("Load devices plugin"));
		if (openDialog.ShowModal() == wxID_CANCEL) {
			return;
		}
		dynlib->Load(openDialog.GetPath(), wxDL_DEFAULT | wxDL_QUIET);
	}

	void CalxApp::OnErrorEvent(wxThreadEvent &evt) {
		this->error_handler->display(evt.GetPayload<ErrorCode>());
	}

	void CalxApp::OnAlertEvent(wxThreadEvent &evt) {
		auto params = evt.GetPayload<std::tuple<std::string, std::string, long>>();
		wxMessageBox(std::get<0>(params), std::get<0>(params), std::get<2>(params));
	}
}  // namespace CalX::UI

wxIMPLEMENT_APP(CalX::UI::CalxApp);
