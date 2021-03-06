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

#ifndef CALX_UI_CALX_TASK_HANDLE_H_
#define CALX_UI_CALX_TASK_HANDLE_H_

#include "calx/ui/CalxApp.h"
#include "calx/ui/coord/CalxCoordFilter.h"
#include <iosfwd>
#include <vector>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>

namespace CalX::UI {

	class CalxTaskDescriptor {
	 public:
		virtual ~CalxTaskDescriptor() = default;

		friend std::ostream &operator<<(std::ostream &, const CalxTaskDescriptor &);

	 protected:
		virtual void dump(std::ostream &) const = 0;
	};

	class CalxTaskHandle : public wxScrolledWindow, public CalxTaskDescriptor {
	 public:
		CalxTaskHandle(wxWindow *win, wxWindowID id)
		    : wxScrolledWindow::wxScrolledWindow(win, id) {
			this->SetScrollRate(5, 5);
		}
		virtual std::string getName() const = 0;
		virtual std::shared_ptr<CoordTask> getTask() = 0;
		virtual std::shared_ptr<CoordTranslator> getTranslator() = 0;
		virtual void update() = 0;
	};
}  // namespace CalX::UI

#endif
