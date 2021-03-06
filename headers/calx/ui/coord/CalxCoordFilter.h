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

#ifndef CALX_UI_CALX_COORD_TRANSLATOR_H_
#define CALX_UI_CALX_COORD_TRANSLATOR_H_

#include "calx/ui/CalxApp.h"
#include <vector>
#include <wx/listbox.h>
#include <wx/spinctrl.h>

namespace CalX::UI {

	class CalxCoordFilter : public wxPanel {
	 public:
		CalxCoordFilter(wxWindow *, wxWindowID);
		virtual std::shared_ptr<CoordTranslator> getTranslator() = 0;
	};

	class CalxCoordBasicFilter : public CalxCoordFilter {
	 public:
		CalxCoordBasicFilter(wxWindow *, wxWindowID,
		                     std::shared_ptr<BasicCoordTranslator>);
		std::shared_ptr<CoordTranslator> getTranslator() override;

	 private:
		void updateData();
		std::shared_ptr<BasicCoordTranslator> translator;
		wxSpinCtrl *xoffset;
		wxSpinCtrl *yoffset;
		wxSpinCtrl *xscale;
		wxSpinCtrl *yscale;
	};

	class CalxCoordLinearFilter : public CalxCoordFilter {
	 public:
		CalxCoordLinearFilter(wxWindow *, wxWindowID,
		                      std::shared_ptr<LinearCoordTranslator>);
		std::shared_ptr<CoordTranslator> getTranslator() override;

	 private:
		void updateData();
		void OnFieldChange(wxCommandEvent &);

		std::shared_ptr<LinearCoordTranslator> translator;
		wxSpinCtrlDouble *xoffset;
		wxSpinCtrlDouble *yoffset;
		wxSpinCtrlDouble *xscale;
		wxSpinCtrlDouble *yscale;
	};

	class CalxCoordLogarithmicFilter : public CalxCoordFilter {
	 public:
		CalxCoordLogarithmicFilter(wxWindow *, wxWindowID,
		                           std::shared_ptr<LogarithmicCoordTranslator>);
		std::shared_ptr<CoordTranslator> getTranslator() override;

	 private:
		void updateData();
		void OnFieldChange(wxCommandEvent &);

		std::shared_ptr<LogarithmicCoordTranslator> translator;
		wxSpinCtrlDouble *xscale;
		wxSpinCtrlDouble *yscale;
	};

	class CalxCoordPolarFilter : public CalxCoordFilter {
	 public:
		CalxCoordPolarFilter(wxWindow *, wxWindowID,
		                     std::shared_ptr<PolarCoordTranslator>);
		std::shared_ptr<CoordTranslator> getTranslator() override;

	 private:
		std::shared_ptr<PolarCoordTranslator> translator;
	};

	class CalxCoordFilterCtrl : public CalxCoordFilter {
	 public:
		CalxCoordFilterCtrl(wxWindow *, wxWindowID,
		                    std::shared_ptr<ComplexCoordTranslator> = nullptr);
		std::shared_ptr<CoordTranslator> getTranslator() override;
		void updateUI();

	 private:
		void OnListClick(wxCommandEvent &);
		void OnAddLinearClick(wxCommandEvent &);
		void OnAddLogarithmicClick(wxCommandEvent &);
		void OnAddPolarClick(wxCommandEvent &);
		void OnRemoveClick(wxCommandEvent &);

		void addFilter(std::shared_ptr<CoordTranslator>);

		wxPanel *mainPanel;
		wxListBox *filterList;
		wxButton *removeButton;

		std::shared_ptr<ComplexCoordTranslator> trans;
		std::vector<CalxCoordFilter *> filter;
	};
}  // namespace CalX::UI

#endif
