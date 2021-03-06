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

#ifndef CALX_LOGS_CALX_LOGS_PANEL_H_
#define CALX_LOGS_CALX_LOGS_PANEL_H_

#include "calx/ui/CalxApp.h"
#include "calx/ui/CalxPanelPane.h"
#include "calx/ui/logs/LogSink.h"

namespace CalX::UI {

	class CalxLogPanel : public CalxPanelPane, public CalxLogSink {
	 public:
		CalxLogPanel(wxWindow *, wxWindowID);
		bool isBusy() override;
		void shutdown() override;
		void updateUI() override;

		std::shared_ptr<JournalSink> getSink(const std::string &) override;

	 private:
		class UIJournalSink;
		void OnFlushText(wxThreadEvent &);
		void OnClose(wxCloseEvent &);

		wxTextCtrl *logPane;
		std::vector<std::weak_ptr<UIJournalSink>> sinks;
	};
}  // namespace CalX::UI

#endif