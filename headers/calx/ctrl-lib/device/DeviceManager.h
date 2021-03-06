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

#ifndef CALX_DEVICE_MANAGER_H_
#define CALX_DEVICE_MANAGER_H_

#include "calx/ctrl-lib/device/Device.h"
#include "calx/platform.h"
#include <cinttypes>
#include <mutex>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <vector>

/* Device manager initialise existing controllers and keep track of usable
   devices and instruments.
   Some of device manager functionality are common for all possible
   managers(however it can be overrided), some is specific.*/

namespace CalX {

	class DeviceManager {
	 public:
		virtual ~DeviceManager() = default;
		virtual void refresh() = 0;  // Refresh device list, reinitialise them.
		virtual Motor *getMotor(device_id_t) const;      // Return device by id
		virtual std::size_t getMotorCount() const;       // Get device count
		virtual std::size_t getInstrumentCount() const;  // Get instrument count
		virtual Instrument *getInstrument(
		    device_id_t) const;           // Get instrument by id
		virtual bool hasError() const;    // Check errors
		virtual std::string pollError();  // Return error from queue

		virtual void terminate();
		virtual Motor *connectMotor(DeviceConnectionPrms *) = 0;
		virtual Instrument *connectInstrument(DeviceConnectionPrms *) = 0;
		virtual void getConnectionTypes(std::vector<DeviceConnectionType> &,
		                                std::vector<DeviceConnectionType> &) const;
		virtual bool canMotorConnect(DeviceConnectionType) const;
		virtual bool canInstrumentConnect(DeviceConnectionType) const;
		virtual bool loadConfiguration(const std::string &,
		                               ConfigurationCatalogue &);
		void lock();
		void unlock();
		bool tryLock();

	 protected:
		virtual void log(const std::string &);

		std::vector<std::string> error_queue;
		std::vector<std::unique_ptr<Motor>> motors;
		std::vector<std::unique_ptr<Instrument>> instruments;

		std::vector<DeviceConnectionType> motorConnectionType;
		std::vector<DeviceConnectionType> instrumentConnectionType;

	 private:
		std::mutex devman_mutex;
	};

	/* Used as exported function in Device API implementations.*/
	extern "C" LIBEXPORT DeviceManager *getDeviceManager();
	typedef DeviceManager *(*DeviceManager_getter)();
}  // namespace CalX

#endif
