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

#include "calx/ctrl-lib/device/DeviceManager.h"
#include "calx/ctrl-lib/conf/ConfigManager.h"

/* Implementations of API wrappers */

namespace CalX {

	Device::Device(DeviceType type, device_id_t id) : id(id), type(type) {
		this->config = std::make_shared<ConfigManager>();
	}

	DeviceType Device::getType() const {
		return this->type;
	}

	device_id_t Device::getID() const {
		return this->id;
	}

	ConfigurationCatalogue &Device::getConfiguration() const {
		return *this->config;
	}

	bool Device::hasErrors() const {
		return !this->errors.empty();
	}

	std::string Device::pollError() {
		if (this->errors.empty()) {
			return "";
		}
		std::string err = this->errors.at(0);
		this->errors.erase(this->errors.begin());
		return err;
	}

	void Device::log(const std::string &msg) {
		LOG("Device #" + std::to_string(this->getID()), msg);
	}

	void Device::lock() {
		// For better synchonization it's possible to implement locking
		// However currently it's not strictly necessary
	}

	void Device::unlock() {}

	bool Device::tryLock() {
		return true;
	}

	Motor::Motor(device_id_t id) : Device::Device(DeviceType::Motor, id) {}

	Instrument::Instrument(device_id_t id)
	    : Device::Device(DeviceType::Instrument, id) {}

	void Instrument::log(const std::string &msg) {
		LOG_INSTR(this->getID(), msg);
	}
}  // namespace CalX
