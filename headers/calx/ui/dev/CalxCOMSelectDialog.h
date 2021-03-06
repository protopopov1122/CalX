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

#ifndef _CALX_UI_CALX_COM_SELECT_DIALOG_H_
#define _CALX_UI_CALX_COM_SELECT_DIALOG_H_

#include "calx/ui/CalxApp.h"
#include <wx/choice.h>
#include <wx/spinctrl.h>

namespace CalX::UI {

	class CalxCOMSelectDialog : public wxDialog {
	 public:
		CalxCOMSelectDialog(wxWindow *, wxWindowID);
		int16_t getPort();
		int32_t getSpeed();
		SerialPortParity getParity();

	 private:
		void OnOkClick(wxCommandEvent &);
		void OnCancelClick(wxCommandEvent &);

		wxSpinCtrl *portSpin;
		wxSpinCtrl *speedSpin;
		wxChoice *parityCh;
		int16_t port;
		int32_t speed;
		SerialPortParity parity;
	};
}  // namespace CalX::UI

#endif
