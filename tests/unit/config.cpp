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

#include "catch.hpp"
#include "calx/ctrl-lib/conf/ConfigManager.h"

using namespace CalX;

TEST_CASE("Configuration manager entry management") {
	ConfigManager config;
	std::vector<ConfiguationFlatDictionary *> entries;
	const std::string ENTRY = "test_entry";
	REQUIRE(!config.hasEntry(ENTRY));
	config.visit([&](const std::string &name, ConfiguationFlatDictionary &entry) {
		entries.push_back(&entry);
	});
	REQUIRE(entries.size() == 0);
	REQUIRE(!config.removeEntry(ENTRY));
	REQUIRE(config.getEntry(ENTRY, false) == nullptr);

	ConfiguationFlatDictionary *entry = config.getEntry(ENTRY);
	REQUIRE(entry != nullptr);
	REQUIRE(config.hasEntry(ENTRY));
	REQUIRE(entry == config.getEntry(ENTRY));
	for (int i = 0; i < 10; i++) {
		config.getEntry(ENTRY);
	}
	config.visit([&](const std::string &name, ConfiguationFlatDictionary &entry) {
		entries.push_back(&entry);
	});
	REQUIRE(entries.size() == 1);

	REQUIRE(config.removeEntry(ENTRY));
	REQUIRE(!config.hasEntry(ENTRY));
	entries.clear();
	config.visit([&](const std::string &name, ConfiguationFlatDictionary &entry) {
		entries.push_back(&entry);
	});
	REQUIRE(entries.size() == 0);

	const int ENTRY_COUNT = 10;
	for (int i = 0; i < ENTRY_COUNT; i++) {
		REQUIRE(!config.removeEntry(ENTRY + std::to_string(i)));
		config.getEntry(ENTRY + std::to_string(i));
	}
	entries.clear();
	config.visit([&](const std::string &name, ConfiguationFlatDictionary &entry) {
		entries.push_back(&entry);
	});
	REQUIRE(entries.size() == ENTRY_COUNT);
	for (int i = 0; i < ENTRY_COUNT; i++) {
		REQUIRE(config.hasEntry(ENTRY + std::to_string(i)));
		REQUIRE(config.removeEntry(ENTRY + std::to_string(i)));
	}
}

TEST_CASE("Configuration manager value objects") {
	const int_conf_t INTEGER = 10;
	const double REAL = 3.14;
	const bool BOOLEAN = true;
	const std::string STRING = "Hello, world!";

	ConfigurationValue integer(INTEGER);
	ConfigurationValue real(REAL);
	ConfigurationValue boolean(BOOLEAN);
	ConfigurationValue str(STRING);

	REQUIRE(integer.getInt() == INTEGER);
	REQUIRE(integer.getType() == ConfigurationValueType::Integer);
	REQUIRE(integer.toString().compare(std::to_string(INTEGER)) == 0);
	REQUIRE(real.getReal() == REAL);
	REQUIRE(real.getType() == ConfigurationValueType::Real);
	REQUIRE(real.toString().compare(std::to_string(REAL)) == 0);
	REQUIRE(boolean.getBool() == BOOLEAN);
	REQUIRE(boolean.getType() == ConfigurationValueType::Boolean);
	REQUIRE(boolean.toString().compare(BOOLEAN ? "true" : "false") == 0);
	REQUIRE(str.getString().compare(STRING) == 0);
	REQUIRE(str.getType() == ConfigurationValueType::String);
	REQUIRE(str.toString().compare(STRING) == 0);
}

TEST_CASE("Configuration manager entry") {
	ConfigManager config;
	const std::string ENTRY = "test_entry";
	ConfiguationFlatDictionary *entry = config.getEntry(ENTRY);
	SECTION("Entry parameters") {
		REQUIRE(entry != nullptr);
	}
	SECTION("Operations with entry variables") {
		const std::string INTEGER_KEY = "integer";
		const std::string REAL_KEY = "real";
		const std::string BOOLEAN_KEY = "boolean";
		const std::string STRING_KEY = "string";
		const int_conf_t INTEGER_VALUE = 42;
		const double REAL_VALUE = 3.14;
		const bool BOOLEAN_VALUE = false;
		const std::string STRING_VALUE = "Hello, sailor!";

		REQUIRE(!entry->has(INTEGER_KEY));
		REQUIRE(!entry->has(REAL_KEY));
		REQUIRE(!entry->has(BOOLEAN_KEY));
		REQUIRE(!entry->has(STRING_KEY));

		REQUIRE(!entry->remove(INTEGER_KEY));
		REQUIRE(!entry->remove(REAL_KEY));
		REQUIRE(!entry->remove(BOOLEAN_KEY));
		REQUIRE(!entry->remove(STRING_KEY));

		entry->put(INTEGER_KEY, ConfigurationValue(INTEGER_VALUE));
		entry->put(REAL_KEY, ConfigurationValue(REAL_VALUE));
		entry->put(BOOLEAN_KEY, ConfigurationValue(BOOLEAN_VALUE));
		entry->put(STRING_KEY, ConfigurationValue(STRING_VALUE));

		REQUIRE(entry->has(INTEGER_KEY));
		REQUIRE(entry->has(REAL_KEY));
		REQUIRE(entry->has(BOOLEAN_KEY));
		REQUIRE(entry->has(STRING_KEY));

		REQUIRE(entry->is(INTEGER_KEY, ConfigurationValueType::Integer));
		REQUIRE(entry->is(REAL_KEY, ConfigurationValueType::Real));
		REQUIRE(entry->is(BOOLEAN_KEY, ConfigurationValueType::Boolean));
		REQUIRE(entry->is(STRING_KEY, ConfigurationValueType::String));

		REQUIRE(entry->getInt(INTEGER_KEY) == INTEGER_VALUE);
		REQUIRE(entry->getReal(REAL_KEY) == REAL_VALUE);
		REQUIRE(entry->getBool(BOOLEAN_KEY) == BOOLEAN_VALUE);
		REQUIRE(STRING_VALUE.compare(entry->getString(STRING_KEY)) == 0);

		std::vector<std::pair<std::string, ConfigurationValue>> content;
		entry->visit([&](const std::string &key, const ConfigurationValue &value) {
			content.push_back(std::make_pair(key, value));
		});
		REQUIRE(content.size() == 4);

		REQUIRE(entry->remove(INTEGER_KEY));
		REQUIRE(entry->remove(REAL_KEY));
		REQUIRE(entry->remove(BOOLEAN_KEY));
		REQUIRE(entry->remove(STRING_KEY));
	}
}