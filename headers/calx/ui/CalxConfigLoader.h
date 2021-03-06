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

#ifndef CALX_UI_CALX_CONFIG_LOADER_H_
#define CALX_UI_CALX_CONFIG_LOADER_H_

#include "calx/ui/CalxApp.h"
#include <vector>
#include <wx/dataview.h>
#include <wx/filesys.h>
#include <wx/textctrl.h>

namespace CalX::UI {

	struct CalxConfigFile {
		std::string path;
		std::string name;
		std::string descr;
	};

	class CalxConfigLoader : public wxDialog {
	 public:
		CalxConfigLoader(wxWindow *, wxWindowID, const std::string & = "");
		void load();
		std::string getFileName();
		bool isExiting();

	 private:
		void OnLoadClick(wxCommandEvent &);
		void OnExitClick(wxCommandEvent &);
		void OnClose(wxCloseEvent &);

		bool exiting;
		std::string filename;
		wxFileSystem filesys;
		std::vector<CalxConfigFile> found;
		wxDataViewListCtrl *configList;
	};
}  // namespace CalX::UI

#endif