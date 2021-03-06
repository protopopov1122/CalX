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

#ifndef CALX_CTRL_LIB_LOGGER_SHORTCUTS_H_
#define CALX_CTRL_LIB_LOGGER_SHORTCUTS_H_

#include "calx/ctrl-lib/logger/Journal.h"

namespace CalX {

	JournalSinkStream Debug(JournalLogger &, const std::string & = "",
	                        const SourcePosition & = SourcePosition());
	JournalSinkStream Info(JournalLogger &, const std::string & = "",
	                       const SourcePosition & = SourcePosition());
	JournalSinkStream Warning(JournalLogger &, const std::string & = "",
	                          const SourcePosition & = SourcePosition());
	JournalSinkStream Error(JournalLogger &, const std::string & = "",
	                        const SourcePosition & = SourcePosition());
	JournalSinkStream Critical(JournalLogger &, const std::string & = "",
	                           const SourcePosition & = SourcePosition());
}  // namespace CalX

#endif