/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>

#include <lua/lua.hpp>
#include "LuaScript.h"

namespace OpenXcom
{

int _logErrorFunction(lua_State* L);
int _logInfoFunction(lua_State* L);
int _logDebugFunction(lua_State* L);
int _logVerboseFunction(lua_State* L);

LuaScript::LuaScript(const std::string &id) : _id(id), _started(false)
{}

LuaScript::~LuaScript()
{
	stop();
}

void LuaScript::load(std::string ruleFilePath, const YAML::Node &node)
{
	// Fix windows style file paths and pop the file off of the ruleFilePath
	std::replace(ruleFilePath.begin(), ruleFilePath.end(), '\\', '/');
	_ruleFilePath = ruleFilePath.substr(0, ruleFilePath.find_last_of('/'));
	_relativeScriptPath = node["file"].as<std::string>();

	start();
}

std::string LuaScript::getScriptPath()
{
	return _ruleFilePath + "/" + _relativeScriptPath;
}


void LuaScript::start()
{
	if(!_started) {
		_luaState = luaL_newstate();

		// Expose some logging functions
		lua_register(_luaState, "log_info", _logInfoFunction);
		lua_register(_luaState, "log_debug", _logDebugFunction);
		lua_register(_luaState, "log_verbose", _logVerboseFunction);
		lua_register(_luaState, "log_error", _logErrorFunction);

		luaL_dofile(_luaState, getScriptPath().c_str());

		_started = true;
	}
}

void LuaScript::call(std::string fn)
{
	if(_started)
	{
		int exists = lua_getglobal(_luaState, fn.c_str());
		if(exists)
		{
			lua_call(_luaState, 0, 0);
			clearStack();
		}
	}
}

void LuaScript::stop()
{
	if (_started) {
		lua_close(_luaState);
	}
}

void LuaScript::clearStack() {
	if(_started) {
		int n = lua_gettop(_luaState);
		lua_pop(_luaState, n);
	}
}

// LuaCFunctions

int _logErrorFunction(lua_State* L)
{
	Log(LOG_ERROR) << lua_tostring(L, 1);
	return 0;
}

int _logInfoFunction(lua_State* L)
{
	Log(LOG_INFO) << lua_tostring(L, 1);
	return 0;
}

int _logDebugFunction(lua_State* L)
{
	Log(LOG_DEBUG) << lua_tostring(L, 1);
	return 0;
}

int _logVerboseFunction(lua_State* L)
{
	Log(LOG_VERBOSE) << lua_tostring(L, 1);
	return 0;
}

}
