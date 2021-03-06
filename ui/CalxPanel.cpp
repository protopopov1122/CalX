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

#include "calx/ui/CalxPanel.h"

namespace CalX::UI {

	CalxPanel::CalxPanel(wxWindow *win, wxWindowID id)
	    : wxNotebook::wxNotebook(win, id) {
		Bind(wxEVT_CLOSE_WINDOW, &CalxPanel::OnExit, this);
	}

	void CalxPanel::addPane(const std::string &title, CalxPanelPane *pane) {
		this->panes.push_back(pane);
		this->AddPage(pane, title);
	}

	void CalxPanel::updateUI() {
		for (const auto &pane : this->panes) {
			pane->updateUI();
		}
	}

	bool CalxPanel::isBusy() {
		for (const auto &pane : this->panes) {
			if (pane->isBusy()) {
				return true;
			}
		}
		return false;
	}

	void CalxPanel::OnExit(wxCloseEvent &evt) {
		for (const auto &pane : this->panes) {
			pane->shutdown();
		}

		wxMilliSleep(400);

		while (GetPageCount()) {
			RemovePage(0);
		}
		for (const auto &pane : this->panes) {
			pane->Close(true);
		}

		Destroy();
	}
}  // namespace CalX::UI
