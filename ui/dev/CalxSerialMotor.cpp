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

#include "calx/ui/dev/CalxSerialMotor.h"
#include "calx/ui/dev/CalxCOMSelectDialog.h"
#include "calx/ui/dev/CalxDeviceConstructor.h"

namespace CalX::UI {

	class CalxSerialMotorConnectAction : public CalxAction {
	 public:
		CalxSerialMotorConnectAction(CalxDevicePool *pool,
		                             DeviceConnectionPrms *prms) {
			this->pool = pool;
			this->prms = prms;
		}
		virtual ~CalxSerialMotorConnectAction() {
			delete this->prms;
		}

		ErrorCode perform(SystemManager &sysman) override {
			std::shared_ptr<MotorController> ctrl =
			    sysman.getMotorControllerSet().connectDevice(prms).lock();
			if (ctrl == nullptr) {
				wxMessageBox(__("Motor can't be connected"), __("Connection error"),
				             wxICON_WARNING);
			} else {
				pool->appendDevice(new CalxMotorConstructor(pool, ctrl));
			}
			return ErrorCode::NoError;
		}

		void stop() override {}

	 private:
		CalxDevicePool *pool;
		DeviceConnectionPrms *prms;
	};

	void CalxSerialMotorFactory::newDevice(wxWindow *win, CalxDevicePool *pool,
	                                       CalxActionQueue *queue) {
		CalxCOMSelectDialog *dialog = new CalxCOMSelectDialog(win, wxID_ANY);
		dialog->ShowModal();
		if (dialog->getPort() != -1) {
			DeviceSerialPortConnectionPrms *prms =
			    new DeviceSerialPortConnectionPrms();
			prms->port = (uint8_t) dialog->getPort();
			prms->speed = (uint32_t) dialog->getSpeed();
			prms->parity = dialog->getParity();
			queue->addAction(
			    std::make_unique<CalxSerialMotorConnectAction>(pool, prms));
		}
		dialog->Destroy();
	}
}  // namespace CalX::UI
