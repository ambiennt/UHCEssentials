#include "main.h"

// make these commands visible to operators in game and work without cheats
// setmaxplayers - sets max player count
// reload - reloads behavior pack functions and DynamicMOTD/SpawnProtection if enabled, respectively
// permission - lists all the permissions of each player
// listd - verbose version of /list
// agent - education edition agent

// lock these commands to permission level 4 only (server console usage)
// whitelist - adds a player to the whitelist (takes effect if whitelist is enabled, on/off enums don't work)
// op - promotes a player's permission to "GameMasters" (permission level 1)
// deop - demotes a player's permission to "Any" (permission level 0)
TClasslessInstanceHook(void,
	"?registerCommand@CommandRegistry@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBDW4CommandPermissionLevel@@UCommandFlag@@3@Z",
	const std::string &name, const char *description, CommandPermissionLevel requirement, CommandFlagValue f1, CommandFlagValue f2) {

	//std::cout << " command: " << *name << "   level: " << (int)requirement << "   f1: " << (int)f1 << "   f2: " << (int)f2 << std::endl;
	//std::cout << "0x" << std::hex << (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0) << " - " << *name << std::endl;
	//int64_t address = ((int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0));

	const char *cName = name.c_str();
	
    if ((strcmp(cName, "setmaxplayers") == 0) ||
    	(strcmp(cName, "reload") == 0) ||
    	(strcmp(cName, "permission") == 0) ||
    	(strcmp(cName, "listd") == 0) ||
    	(strcmp(cName, "agent") == 0) ||
    	(strcmp(cName, "whitelist") == 0)) {
  		return original(this, name, description, CommandPermissionLevel::GameMasters, CommandFlagValue::None, CommandFlagValue::None);
	}
	else if ((strcmp(cName, "op") == 0) ||
		(strcmp(cName, "deop") == 0)) {
  		return original(this, name, description, CommandPermissionLevel::Owner, CommandFlagValue::None, CommandFlagValue::None);
	}

	original(this, name, description, requirement, f1, f2);
}


// remove unnecessary commands

// websocket server docs: https://github.com/Sandertv/mcwss
// /wsserver - websocket server (idk if it works on BDS)
THook(void, "?setup@WSServerCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /gettopsolidblock - gets top solid block in the world (silently fails)
THook(void, "?setup@GetTopSolidBlockCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /querytarget - prints json of selector position and rotation, but shows to all clients (seems to be for websockets?)
THook(void, "?setup@QueryTargetCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /changesetting - change difficulty and toggle cheats, can be easily recreated more simply
THook(void, "?setup@ChangeSettingCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /worldbuilder (or /wb) - supposedly changes worldbuilder adventuresetting but doesn't seem to work
// use /abilities instead
THook(void, "?setup@WorldBuilderCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /ability - actually useful but same with worldbuilder
// use /abilities instead
THook(void, "?setup@AbilityCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /help - its actually a clientside command but still unnecessarily registered in BDS
// never will be executed serverside because vanilla client wont send a commandrequestpacket for this
THook(void, "?setup@HelpCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }

// /me - just prints your name with an asterisk? very useless and annoying to see in chat
THook(void, "?setup@MeCommand@@SAXAEAVCommandRegistry@@@Z", void* registry) { return; }