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

#include "calx/device/emulated/EmuMotor.h"
#include "calx/device/emulated/EmuDeviceManager.h"

namespace CalX {

	EmuMotor::EmuMotor(device_id_t id, EmuDeviceManager &devman)
	    : Motor::Motor(id), devman(devman) {
		this->pos = 0;
		this->power = Power::NoPower;
	}

	DeviceManager &EmuMotor::getDeviceManager() {
		return this->devman;
	}

	std::string EmuMotor::getDeviceInfo() {
		return "";
	}

	std::string EmuMotor::getRuntimeInfo() {
		return "";
	}

	motor_coord_t EmuMotor::getPosition() {
		return this->pos;
	}

	bool EmuMotor::isTrailerPressed(int tr) {
		if (tr == 1) {
			return this->pos <= -EMU_MOTOR_SIZE / 2;
		} else if (tr == 2) {
			return this->pos >= EMU_MOTOR_SIZE / 2;
		} else {
			return false;
		}
	}

	bool EmuMotor::isRunning() {
		return false;
	}

	Power EmuMotor::getPowerState() {
		return this->power;
	}

	bool EmuMotor::start(motor_coord_t dest, float speed) {
		if (this->power == Power::NoPower) {
			return true;
		}
		this->power = Power::FullPower;
		this->pos = dest;
		return true;
	}

	bool EmuMotor::stop() {
		return true;
	}

	bool EmuMotor::enablePower(bool p) {
		this->power = p ? Power::HalfPower : Power::NoPower;
		return true;
	}

	void EmuMotor::terminate() {}
}  // namespace CalX
