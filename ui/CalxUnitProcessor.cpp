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

#include "calx/ui/CalxUnitProcessor.h"
#include <sstream>

namespace CalX::UI {
	CalxUnitProcessor::CalxUnitProcessor(ConfigurationCatalogue &conf)
	    : config(conf) {}

	std::string CalxUnitProcessor::formatDouble(double d) {
		std::ostringstream os;
		os << d;
		return os.str();
	}

	std::string CalxUnitProcessor::getUnits() {
		return this->config.getEntry(CalxConfiguration::Units)
		    ->getString(CalxUnitConfiguration::UnitSuffix, "");
	}

	std::string CalxUnitProcessor::getSpeedUnits() {
		std::string units = this->getUnits();
		std::string timing = this->config.getEntry(CalxConfiguration::Units)
		                         ->getString(CalxUnitConfiguration::Timing, "");
		return units.empty() ? "" : units + timing;
	}

	double CalxUnitProcessor::getUnitPrecision() {
		return 1.0 / this->getUnitScale();
	}

	double CalxUnitProcessor::getSpeedPrecision() {
		return 1.0 / this->getSpeedScale();
	}

	double CalxUnitProcessor::getUnitScale() {
		return this->config.getEntry(CalxConfiguration::Units)
		    ->getReal(CalxUnitConfiguration::UnitScale, 1.0f);
	}

	double CalxUnitProcessor::getSpeedScale() {
		return this->config.getEntry(CalxConfiguration::Units)
		    ->getReal(CalxUnitConfiguration::SpeedScale, 1.0f);
	}

	coord_point_t CalxUnitProcessor::getUnitOffset() {
		coord_point_t offset = {
			this->config.getEntry(CalxConfiguration::Units)
			    ->getReal(CalxUnitConfiguration::UnitOffsetX, 0.0f),
			this->config.getEntry(CalxConfiguration::Units)
			    ->getReal(CalxUnitConfiguration::UnitOffsetY, 0.0f)
		};
		return offset;
	}
}  // namespace CalX::UI