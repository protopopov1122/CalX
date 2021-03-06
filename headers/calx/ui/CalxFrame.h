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

#ifndef CALX_UI_CALX_FRAME_H_
#define CALX_UI_CALX_FRAME_H_

#include "calx/ui/CalxPanel.h"
#include "calx/ui/calx.h"
#include <iosfwd>
#include <string>
#include <wx/menu.h>

namespace CalX::UI {

	class CalxDevicePool;  // Forward referencing
	class CalxPlaneList;   // Forward referencing
	class CalxTaskList;    // Forward referencing
	class CalxMathEngine;  // Forward referencing
	class CalxLogSink;     // Forward referencing

	class CalxFrame : public wxFrame {
	 public:
		CalxFrame(const std::string &);
		CalxPanel *getPanel();

		CalxDevicePool *getDevicePool();
		CalxPlaneList *getPlaneList();
		CalxTaskList *getTaskList();
		CalxMathEngine *getMathEngine();
		CalxLogSink *getLogSink();

	 private:
		void OnStopClick(wxCommandEvent &);
		void OnClose(wxCloseEvent &);
		void OnAboutMenuClick(wxCommandEvent &);

		wxMenuBar *menuBar;
		wxMenu *aboutMenu;

		CalxPanel *panel;

		CalxDevicePool *device_pool;
		CalxPlaneList *plane_list;
		CalxTaskList *task_list;
		CalxMathEngine *math_engine;
		CalxLogSink *logSink;
	};
}  // namespace CalX::UI

#endif
