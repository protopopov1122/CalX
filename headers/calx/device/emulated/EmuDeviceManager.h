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

#ifndef CALX_DEVICE_EMULATED_EMU_DEVICE_MANAGER_H_
#define CALX_DEVICE_EMULATED_EMU_DEVICE_MANAGER_H_

#include "calx/ctrl-lib/device/DeviceManager.h"
#include "calx/device/emulated/EmuInstrument.h"
#include "calx/device/emulated/EmuMotor.h"
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace CalX {

	class EmuDeviceManager : public DeviceManager {
	 public:
		EmuDeviceManager();
		virtual void refresh();
		virtual Motor *connectMotor(DeviceConnectionPrms *);
		virtual Instrument *connectInstrument(DeviceConnectionPrms *);
	};
}  // namespace CalX

#endif
