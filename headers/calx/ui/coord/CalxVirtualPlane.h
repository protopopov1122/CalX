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

#ifndef CALX_UI_CALX_VIRTUAL_PLANE_H_
#define CALX_UI_CALX_VIRTUAL_PLANE_H_

#include "calx/ui/CalxApp.h"
#include <utility>
#include <vector>
#include <wx/stattext.h>

namespace CalX::UI {

	class CalxPlaneTracker : public VirtualCoordPlane {
	 public:
		CalxPlaneTracker(motor_point_t, motor_rect_t);
		virtual ~CalxPlaneTracker();

		std::unique_ptr<CoordPlane> clone(std::shared_ptr<CoordPlane>) override;
		std::vector<std::pair<motor_point_t, bool>> *getPath();
		void reset();

	 protected:
		bool jump(motor_point_t, bool) override;

	 private:
		std::vector<std::pair<motor_point_t, bool>> path;
	};

	class CalxVirtualPlane : public wxWindow {
	 public:
		CalxVirtualPlane(wxWindow *, wxWindowID, std::shared_ptr<CoordHandle>,
		                 wxSize);

		std::shared_ptr<CoordPlaneStack> getPlane();
		std::shared_ptr<CalxPlaneTracker> getTracker();
		std::shared_ptr<FloatCoordPlane> getFloatPlane();
		void repaint();

	 private:
		void OnExit(wxCloseEvent &);
		void OnPaintEvent(wxPaintEvent &);
		void OnResizeEvent(wxSizeEvent &);
		void render(wxDC &);

		std::shared_ptr<CalxPlaneTracker> tracker;
		std::shared_ptr<CoordPlaneStack> stack;
		std::shared_ptr<FloatCoordPlane> float_plane;
		std::shared_ptr<CoordHandle> base;

		wxColour pointer_colour;
		wxColour jump_colour;
		wxColour move_colour;
	};

	class CalxVirtualPlaneDialog : public wxDialog {
	 public:
		CalxVirtualPlaneDialog(wxWindow *, wxWindowID, std::shared_ptr<CoordHandle>,
		                       wxSize);

		std::shared_ptr<CoordPlaneStack> getPlane();
		std::shared_ptr<FloatCoordPlane> getFloatPlane();
		void setEnabled(bool);

	 private:
		void OnOkClick(wxCommandEvent &);
		void OnMouseMove(wxMouseEvent &);
		CalxVirtualPlane *plane;
		wxStaticText *mouseCoords;
		wxButton *okButton;
	};
}  // namespace CalX::UI

#endif
