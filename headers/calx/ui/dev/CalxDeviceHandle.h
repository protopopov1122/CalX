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

#ifndef CALX_UI_DEV_CALX_DEVICE_HANDLE_H_
#define CALX_UI_DEV_CALX_DEVICE_HANDLE_H_

#include "calx/ui/CalxApp.h"
#include "calx/ui/CalxActionQueue.h"

namespace CalX::UI {

	class CalxDeviceHandle : public wxPanel {
	 public:
		CalxDeviceHandle(wxWindow *win, wxWindowID id)
		    : wxPanel::wxPanel(win, id) {}
		virtual void updateUI() = 0;
		virtual void stop() = 0;
		virtual bool isBusy() = 0;
		virtual DeviceController *getController() = 0;
	};

	class CalxMotorHandle {
	 public:
		virtual ~CalxMotorHandle() = default;
		virtual void stopMovement() = 0;
		virtual ErrorCode setPower(bool) = 0;
		virtual CalxActionResult roll(TrailerId) = 0;
		virtual CalxActionResult move(motor_coord_t, float, bool) = 0;
	};

	class CalxInstrumentHandle {
	 public:
		virtual ~CalxInstrumentHandle() = default;
		virtual void stopInstrument() = 0;
	};

	class CalxDeviceConstructor {
	 public:
		virtual ~CalxDeviceConstructor() {}
		virtual CalxDeviceHandle *construct(wxWindow *) = 0;
	};

	class CalxDevicePool {
	 public:
		virtual void appendDevice(CalxDeviceConstructor *, bool * = nullptr) = 0;
		virtual std::size_t getMotorCount() = 0;
		virtual CalxMotorHandle *getMotor(device_id_t) = 0;
		virtual std::size_t getInstrumentCount() = 0;
		virtual CalxInstrumentHandle *getInstrument(device_id_t) = 0;
		virtual void stop() = 0;
	};
}  // namespace CalX::UI

#endif
