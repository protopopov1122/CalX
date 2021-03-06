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

#include "calx/ctrl-lib/ctrl/MotorController.h"
#include <algorithm>

namespace CalX {

	MotorController::MotorController(ConfigurationCatalogue &conf, Motor &dev)
	    : DeviceController::DeviceController(conf, dev), motor(dev) {
		this->destination = MoveType::Stop;
		this->work = false;
	}

	Motor &MotorController::getMotor() const {
		return this->motor;
	}

	Power MotorController::getPowerState() const {
		return this->motor.getPowerState();
	}

	ErrorCode MotorController::enablePower(bool p) {
		bool res = this->motor.enablePower(p);
		return res ? ErrorCode::NoError : ErrorCode::LowLevelError;
	}

	ErrorCode MotorController::flipPower() {
		return enablePower(getPowerState() == Power::NoPower);
	}

	ErrorCode MotorController::checkTrailers() {
		ErrorCode errcode = ErrorCode::NoError;
		if (!this->motor.isRunning()) {
			return errcode;
		}
		if (this->destination == MoveType::MoveUp &&
		    this->motor.isTrailerPressed(2)) {
			errcode = ErrorCode::Trailer2Pressed;
			if (!this->motor.stop()) {
				errcode = ErrorCode::LowLevelError;
			}
			this->destination = MoveType::Stop;
			return errcode;
		}
		if (this->destination == MoveType::MoveDown &&
		    this->motor.isTrailerPressed(1)) {
			errcode = ErrorCode::Trailer1Pressed;
			if (!this->motor.stop()) {
				errcode = ErrorCode::LowLevelError;
			}
			this->destination = MoveType::Stop;
			return errcode;
		}
		return ErrorCode::NoError;
	}

	bool MotorController::isTrailerPressed(TrailerId tr) const {
		return this->motor.isTrailerPressed(static_cast<int>(tr));
	}

	ErrorCode MotorController::waitWhileRunning() {
		while (this->motor.isRunning()) {
			ErrorCode code = this->checkTrailers();
			if (code != ErrorCode::NoError) {
				return code;
			}
		}
		return ErrorCode::NoError;
	}

	ErrorCode MotorController::moveToTrailer(TrailerId tr) {
		if (this->motor.isRunning()) {
			return ErrorCode::MotorRunning;
		}
		if (this->motor.getPowerState() == Power::NoPower) {
			return ErrorCode::PowerOff;
		}
		this->work = true;
		int_conf_t roll_step =
		    config.getEntry(CalxConfiguration::Core)
		        ->getInt(CalxMotorConfiguration::RollStep, ROLL_STEP);
		int_conf_t roll_speed =
		    config.getEntry(CalxConfiguration::Core)
		        ->getInt(CalxMotorConfiguration::RollSpeed, ROLL_SPEED);
		int_conf_t comeback =
		    config.getEntry(CalxConfiguration::Core)
		        ->getInt(CalxMotorConfiguration::TrailerComeback, TRAILER_COMEBACK);

		int_conf_t dest = (tr == TrailerId::Trailer1 ? -roll_step : roll_step);
		this->destination =
		    (tr == TrailerId::Trailer1 ? MoveType::RollDown : MoveType::RollUp);
		MotorRollEvent evt = { tr };
		this->sendRollingEvent(evt);
		while (!motor.isTrailerPressed(static_cast<int>(tr)) && work) {
			if (!this->motor.isRunning()) {
				if (!this->motor.start(this->motor.getPosition() + dest, roll_speed)) {
					this->sendRolledEvent(evt);
					this->destination = MoveType::Stop;
					this->work = false;
					return ErrorCode::LowLevelError;
				}
			}
		}
		if (this->motor.isRunning()) {
			if (!this->motor.stop()) {
				this->sendRolledEvent(evt);
				this->destination = MoveType::Stop;
				this->work = false;
				return ErrorCode::LowLevelError;
			}
		}
		this->destination = MoveType::Stop;
		if (tr == TrailerId::Trailer2) {
			comeback *= -1;
		}
		ErrorCode errcode = ErrorCode::NoError;
		if (work) {
			errcode =
			    this->startMove(this->motor.getPosition() + comeback, roll_speed);
		}
		this->sendRolledEvent(evt);
		this->work = false;

		return errcode;
	}

	ErrorCode MotorController::startMove(motor_coord_t dest, float speed) {
		if (this->motor.isRunning()) {
			return ErrorCode::MotorRunning;
		}
		if (this->motor.getPowerState() == Power::NoPower) {
			return ErrorCode::PowerOff;
		}
		this->work = true;
		this->destination = dest > this->motor.getPosition() ? MoveType::MoveUp
		                                                     : MoveType::MoveDown;
		MotorMoveEvent evt = { dest, speed };
		this->sendMovingEvent(evt);
		ErrorCode errcode = ErrorCode::NoError;
		if (!this->motor.start(dest, speed)) {
			errcode = ErrorCode::LowLevelError;
		}
		if (errcode == ErrorCode::NoError) {
			errcode = this->waitWhileRunning();
		}
		if (errcode != ErrorCode::NoError) {
			MotorErrorEvent sevt = { errcode };
			this->sendStoppedEvent(sevt);
		} else {
			this->sendMovedEvent(evt);
		}
		this->destination = MoveType::Stop;
		this->work = false;
		return errcode;
	}

	ErrorCode MotorController::startRelativeMove(motor_coord_t reldest,
	                                             float speed) {
		motor_coord_t dest = getPosition() + reldest;
		return startMove(dest, speed);
	}

	ErrorCode MotorController::asyncMove(motor_coord_t dest, float speed,
	                                     bool rollEvent) {
		if (this->motor.isRunning()) {
			return ErrorCode::MotorRunning;
		}
		if (this->motor.getPowerState() == Power::NoPower) {
			return ErrorCode::PowerOff;
		}
		this->work = true;
		this->destination = dest > this->motor.getPosition() ? MoveType::MoveUp
		                                                     : MoveType::MoveDown;
		if (!rollEvent) {
			MotorMoveEvent evt = { dest, speed };
			this->sendMovingEvent(evt);
		} else {
			TrailerId tid = this->destination == MoveType::MoveUp
			                    ? TrailerId::Trailer2
			                    : TrailerId::Trailer1;
			MotorRollEvent evt = { tid };
			this->sendRollingEvent(evt);
		}
		ErrorCode errcode = ErrorCode::NoError;
		if (!this->motor.start(dest, speed)) {
			errcode = ErrorCode::LowLevelError;
			MotorErrorEvent sevt = { errcode };
			this->sendStoppedEvent(sevt);
		}
		return errcode;
	}

	ErrorCode MotorController::asyncRelativeMove(motor_coord_t reldest,
	                                             float speed, bool rollEvent) {
		motor_coord_t dest = getPosition() + reldest;
		return asyncMove(dest, speed, rollEvent);
	}

	void MotorController::asyncStop(ErrorCode errcode, motor_coord_t dest,
	                                float speed, bool rollEvent) {
		if (errcode != ErrorCode::NoError) {
			MotorErrorEvent sevt = { errcode };
			this->sendStoppedEvent(sevt);
		} else if (!rollEvent) {
			MotorMoveEvent evt = { dest, speed };
			this->sendMovedEvent(evt);
		} else {
			TrailerId tid = this->destination == MoveType::MoveUp
			                    ? TrailerId::Trailer2
			                    : TrailerId::Trailer1;
			MotorRollEvent evt = { tid };
			this->sendRolledEvent(evt);
		}
		this->destination = MoveType::Stop;
		this->motor.stop();
		this->work = false;
	}

	void MotorController::stop() {
		this->destination = MoveType::Stop;
		this->motor.stop();
		this->work = false;
	}

	motor_coord_t MotorController::getPosition() const {
		return this->motor.getPosition();
	}

	bool MotorController::isMoving() const {
		return this->motor.isRunning();
	}

	void MotorController::sendMovingEvent(MotorMoveEvent &evt) {
		this->submitEvent(&MotorEventListener::moving, evt);
	}

	void MotorController::sendMovedEvent(MotorMoveEvent &evt) {
		this->submitEvent(&MotorEventListener::moved, evt);
	}

	void MotorController::sendStoppedEvent(MotorErrorEvent &evt) {
		this->submitEvent(&MotorEventListener::stopped, evt);
	}

	void MotorController::sendRollingEvent(MotorRollEvent &evt) {
		this->submitEvent(&MotorEventListener::rolling, evt);
	}

	void MotorController::sendRolledEvent(MotorRollEvent &evt) {
		this->submitEvent(&MotorEventListener::rolled, evt);
	}

	void MotorController::use() {
		UsableResource::use();
		this->submitEvent(&MotorEventListener::onUse);
	}

	void MotorController::unuse() {
		UsableResource::unuse();
		this->submitEvent(&MotorEventListener::onUnuse);
	}
}  // namespace CalX
