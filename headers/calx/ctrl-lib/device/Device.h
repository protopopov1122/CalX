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

#ifndef CALX_DEVICE_H_
#define CALX_DEVICE_H_

#include "calx/ctrl-lib/CtrlCore.h"
#include "calx/ctrl-lib/conf/Dictionary.h"
#include <cinttypes>
#include <mutex>
#include <string>

/* This file contains Device API interfaces.
   They tend to be as simple and as compact
   as possible to cover most device models. */

namespace CalX {

	class DeviceManager;  // For forward referencing

	enum class DeviceConnectionType { SerialPort };

	struct DeviceConnectionPrms {
	 public:
		DeviceConnectionPrms(DeviceConnectionType t) {
			this->type = t;
		}
		virtual ~DeviceConnectionPrms() = default;

		DeviceConnectionType type;
	};

	enum class SerialPortParity {
		No = 0,
		Odd = 1,
		Even = 2,
		Mark = 3,
		Space = 4
	};

	struct DeviceSerialPortConnectionPrms : public DeviceConnectionPrms {
	 public:
		DeviceSerialPortConnectionPrms(
		    uint8_t port = 0, uint32_t speed = 0,
		    SerialPortParity parity = SerialPortParity::No)
		    : DeviceConnectionPrms::DeviceConnectionPrms(
		          DeviceConnectionType::SerialPort),
		      port(port),
		      speed(speed),
		      parity(parity) {}
		DeviceSerialPortConnectionPrms(const DeviceSerialPortConnectionPrms &prms)
		    : DeviceConnectionPrms::DeviceConnectionPrms(
		          DeviceConnectionType::SerialPort),
		      port(prms.port),
		      speed(prms.speed),
		      parity(prms.parity) {}
		DeviceSerialPortConnectionPrms &operator=(
		    const DeviceSerialPortConnectionPrms &) = default;

		uint8_t port;
		uint32_t speed;
		SerialPortParity parity;
	};

	typedef int64_t device_id_t;
#define DEVICE_ID_FMT PRId64

	enum class Power { NoPower, HalfPower, FullPower };

	enum class InstrumentMode { Off, Prepare, Full };

	enum class DeviceType { Motor, Instrument };

	class Device {
	 public:
		Device(DeviceType, device_id_t);
		virtual ~Device() = default;
		DeviceType getType() const;
		void lock();
		void unlock();
		bool tryLock();
		virtual device_id_t getID() const;
		virtual bool hasErrors() const;
		virtual std::string pollError();
		virtual ConfigurationCatalogue &getConfiguration() const;
		virtual DeviceManager &getDeviceManager() = 0;
		virtual std::string getDeviceInfo() = 0;
		virtual std::string getRuntimeInfo() = 0;
		virtual void terminate() = 0;

	 protected:
		virtual void log(const std::string &);

		const device_id_t id;
		std::shared_ptr<ConfigurationCatalogue> config;
		std::vector<std::string> errors;

	 private:
		const DeviceType type;
		std::mutex dev_mutex;
	};

	class Motor : public Device {
	 public:
		Motor(device_id_t);
		virtual motor_coord_t getPosition() = 0;
		virtual bool isTrailerPressed(int) = 0;
		virtual bool isRunning() = 0;
		virtual Power getPowerState() = 0;
		virtual bool start(motor_coord_t, float) = 0;
		virtual bool stop() = 0;
		virtual bool enablePower(bool) = 0;
	};

	class Instrument : public Device {
	 public:
		Instrument(device_id_t);
		virtual bool open_session() = 0;
		virtual bool close_session() = 0;
		virtual bool enable(bool) = 0;
		virtual bool enabled() = 0;
		virtual InstrumentMode getWorkingMode() = 0;
		virtual bool setWorkingMode(InstrumentMode) = 0;

	 protected:
		virtual void log(const std::string &);
	};
}  // namespace CalX

#endif
