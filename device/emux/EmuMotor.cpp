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

#include <thread>
#include <chrono>
#include <cmath>
#include "calx/device/emux/EmuMotor.h"
#include "calx/device/emux/EmuDeviceManager.h"

namespace CalX {

	static constexpr unsigned int SPEED_FACTOR = 10;

	EmuMotor::EmuMotor(device_id_t id, EmuDeviceManager &devman)
	    : Motor::Motor(id), devman(devman) {
		this->pos = 0;
		this->destination = 0;
		this->speed = 0.0f;
		this->power = Power::NoPower;
		this->motorWorks = true;

		std::thread thr(&EmuMotor::motorThread, this);
		thr.detach();
	}

	EmuMotor::~EmuMotor() {
		this->motorWorks = false;
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
		return this->pos != this->destination;
	}

	Power EmuMotor::getPowerState() {
		return this->power;
	}

	bool EmuMotor::start(motor_coord_t dest, float speed) {
		if (this->power == Power::NoPower) {
			return true;
		}
		this->power = Power::FullPower;
		this->destination = dest;
		this->speed = speed * SPEED_FACTOR;
		return true;
	}

	bool EmuMotor::stop() {
		this->destination = this->pos;
		return true;
	}

	bool EmuMotor::enablePower(bool p) {
		this->power = p ? Power::HalfPower : Power::NoPower;
		return true;
	}

	void EmuMotor::terminate() {}

	void EmuMotor::motorThread() {
		const float MAX_SPEED = 4000 * SPEED_FACTOR;
		const int DISCR = 100;  // times per second
		while (this->motorWorks) {
			if (this->destination != this->pos) {
				const float MOTOR_SPEED = fmin(MAX_SPEED, this->speed);
				const int MOTOR_STEP = static_cast<int>(ceil(MOTOR_SPEED / DISCR));
				if (abs(this->destination - this->pos) <= MOTOR_STEP) {
					this->pos = this->destination;
				} else {
					if (this->destination < this->pos) {
						this->pos -= MOTOR_STEP;
					} else {
						this->pos += MOTOR_STEP;
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / DISCR));
		}
	}
}  // namespace CalX
