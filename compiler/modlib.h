
#pragma once

#include <map>
#include <shared_mutex>
#include <vector>

#include "module.h"

namespace caliburn
{
	class ModuleLibrary
	{
		std::map<std::string, Module*> modules;
		std::shared_mutex lock;

		void addModule(std::string name, Module* mod)
		{
			lock.lock();
			modules.emplace(name, mod);
			lock.unlock();

		}

		Module* getModule(std::string name)
		{
			lock.lock_shared();
			Module* mod = modules[name];
			lock.unlock_shared();
			return mod;
		}

		void getModules(std::vector<std::string>& names, std::vector<Module*>& mods)
		{
			lock.lock_shared();
			for (auto name : names)
			{
				mods.push_back(modules[name]);
			}
			lock.unlock_shared();

		}

	};
}
