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

#ifndef CALX_CTRL_LIB_PLANE_COORD_PLANE_VALIDATOR_H_
#define CALX_CTRL_LIB_PLANE_COORD_PLANE_VALIDATOR_H_

#include "calx/ctrl-lib/plane/ProxyCoordPlane.h"

namespace CalX {

	class CoordPlaneValidator : public ProxyCoordPlane {
	 public:
		CoordPlaneValidator(motor_point_t, motor_point_t, float,
		                    std::shared_ptr<CoordPlane>);
		motor_point_t getMinimum() const;
		motor_point_t getMaximum() const;
		float getMaxSpeed() const;
		void setMinimum(motor_point_t);
		void setMaximum(motor_point_t);
		void setMaxSpeed(float);

		ErrorCode move(motor_point_t, float, bool) override;
		ErrorCode arc(motor_point_t, motor_point_t, int, float, bool,
		              float = 1.0f) override;
		std::ostream &operator<<(std::ostream &) const override;
		std::unique_ptr<CoordPlane> clone(std::shared_ptr<CoordPlane>) override;

	 private:
		motor_point_t min;
		motor_point_t max;
		float max_speed;
	};
}  // namespace CalX

#endif
