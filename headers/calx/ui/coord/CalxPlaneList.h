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

#ifndef CALX_UI_COORD_CALX_PLANE_LIST_H_
#define CALX_UI_COORD_CALX_PLANE_LIST_H_

#include "calx/ui/CalxApp.h"
#include "calx/ui/coord/CalxWatcherPool.h"
#include "calx/ui/coord/CalxCoordController.h"

namespace CalX::UI {

	class CalxPlaneHandle {
	 public:
		virtual ~CalxPlaneHandle() = default;
		virtual CalxCoordController *getController() = 0;
		virtual void newWatcher() = 0;
		virtual void stop() = 0;
	};

	class CalxPlaneList {
	 public:
		virtual ~CalxPlaneList() = default;
		virtual void updateList(std::shared_ptr<CoordHandle>, bool *) = 0;
		virtual std::size_t getPlaneCount() = 0;
		virtual CalxPlaneHandle *getPlaneHandle(std::size_t) = 0;
		virtual void stop() = 0;
	};
}  // namespace CalX::UI

#endif