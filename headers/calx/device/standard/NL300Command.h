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

#ifndef CALX_DEVICE_STANDARD_NL300_COMMAND_H_
#define CALX_DEVICE_STANDARD_NL300_COMMAND_H_

#include "calx/ctrl-lib/device/Device.h"
#include "calx/ctrl-lib/device/DeviceManager.h"
#include "calx/device/standard/NL300Message.h"
#include <cinttypes>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace CalX {

	class StandardDeviceManager;  // Forward referencing

	class NL300SystemCommand : public NL300Message {
	 public:
		// Arguments: receiver, command, parameter, sender
		NL300SystemCommand(const std::string &, const std::string &,
		                   const std::string &, const std::string &);
		virtual ~NL300SystemCommand();
		std::string getCommand();
		std::string getParameter();

	 private:
		std::string command;
		std::string parameter;
	};

	enum class NL300GeneralAction {
		Set = 'S',
		Add = 'A',
		Program = 'P',
		Inquiry = '?'
	};

	class NL300GeneralCommand : public NL300Message {
	 public:
		// Arguments: receiver, array, index, action, parameter, sender
		NL300GeneralCommand(const std::string &, char, uint16_t, NL300GeneralAction,
		                    NL300Parameter *, const std::string &);
		virtual ~NL300GeneralCommand();
		char getArray();
		uint16_t getIndex();
		NL300GeneralAction getAction();
		NL300Parameter *getParameter();

	 private:
		char array;
		uint16_t index;
		NL300GeneralAction action;
		NL300Parameter *parameter;
	};
}  // namespace CalX

#endif