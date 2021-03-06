//
// KeePassHotKey
// Main application entry point
//
// Copyright 2022 SGrottel (https://www.sgrottel.de)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//

#include "Common.h"
#include "Config.h"
#include "KeePassDetector.h"
#include "KeePassRunner.h"
#include "InstanceControl.h"
#include "ConfirmationDialog.h"
#include "TraceFile.h"

void reportException(std::string const& msgUtf8) {
	_tstringstream text;
	text << _T("Error: ") << fromUtf8(msgUtf8.c_str());

	TraceFile::Instance().log(text.str());

	MessageBox(NULL,
		text.str().c_str(),
		k_caption,
		MB_ICONERROR | MB_OK | MB_APPLMODAL);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	Config config;
	InstanceControl instCtrl;
	try {
		TraceFile::Instance().log() << _T("KeePass'HotKey started");

		config.init(lpCmdLine);
		if (!config.continueProgram()) {
			TraceFile::Instance().log() << _T("config.continueProgram == false");
			return 0;
		}

		bool isMainInst = instCtrl.initOrSignal();
		if (!isMainInst) {
			TraceFile::Instance().log() << _T("(isMainInst = instCtrl.initOrSignal() ) == false");
			return 0;
		}

		if (config.playStartSound()) {
			::MessageBeep(MB_OK);
		}

		KeePassDetector detector{ config };
		detector.Detect();

		KeePassRunner runner{ config };

		if (detector.getResult() == KeePassDetector::Result::FoundOk)
		{
			TraceFile::Instance().log() << _T("detector.getResult() == KeePassDetector::Result::FoundOk");
			if (config.needConfirmationForAutoType()) {
				ConfirmationDialog cDlg{ config, instCtrl };
				if (!cDlg.confirm(hInstance)) {
					return 0;
				}

			}

			runner.RunAutoTypeSelected();
		}
		else
		{
			TraceFile::Instance().log() << _T("detector.getResult() != KeePassDetector::Result::FoundOk\n")
				<< _T("\tinstead: ") << static_cast<uint32_t>(detector.getResult());
			runner.OpenKdbx();
		}

	}
	catch (std::exception const& ex) {
		reportException(ex.what());
	}
	catch (...) {
		reportException("Unexpected Exception");
	}

	return 0;
}
