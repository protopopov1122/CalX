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

#ifndef CALX_CTRL_LIB_LOGGING_SESSION_H_
#define CALX_CTRL_LIB_LOGGING_SESSION_H_

#include "calx/ctrl-lib/logger/Journal.h"
#include "calx/ctrl-lib/logger/Sink.h"
#include <map>

namespace CalX {

	class JournalDefaultLogger : public JournalLoggerController,
	                             public JournalLogger {
	 public:
		JournalDefaultLogger();

		const JournalSink *getSink(const std::string &) const override;
		void getSinks(std::vector<std::reference_wrapper<const JournalSink>> &)
		    const override;
		void log(const LogEntry &) const override;

		void setFilter(std::function<bool(const LogEntry &)>) override;
		JournalSink &newStreamSink(const std::string &, std::ostream &) override;
		JournalSink &newFileSink(const std::string &, const std::string &) override;
		JournalSink &newNullSink(const std::string &) override;
		JournalSink &appendSink(const std::string &,
		                        std::shared_ptr<JournalSink>) override;
		void dropSink(const std::string &) override;

	 private:
		std::map<std::string, std::shared_ptr<JournalSink>> sinks;
		std::function<bool(const LogEntry &)> filter;
	};

	class DefaultJournalSession : public JournalDefaultLogger,
	                              public JournalSession {
	 public:
		using JournalDefaultLogger::JournalDefaultLogger;

		JournalLogger &getLogger() override {
			return *this;
		}

		JournalLoggerController &getController() override {
			return *this;
		}
	};
}  // namespace CalX

#endif