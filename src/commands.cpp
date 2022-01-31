#include "main.h"

// you can also check for each command by name but its faster to check return address than to compare strings
TClasslessInstanceHook(void,
	"?registerCommand@CommandRegistry@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBDW4CommandPermissionLevel@@UCommandFlag@@3@Z",
	const std::string *name, const char *description, CommandPermissionLevel requirement, CommandFlagValue f1, CommandFlagValue f2) {

	//std::cout << " command: " << *name << "   level: " << (int)requirement << "   f1: " << (int)f1 << "   f2: " << (int)f2 << std::endl;
	//std::cout << "0x" << std::hex << (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0) << " - " << *name << std::endl;
	int64_t address = ((int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0));
	switch (address) {

		// make these commands visible to operators in game and work without cheats
		case 0x4975f9: // setmaxplayers - sets max player count
		case 0x484987: // reload - reloads behavior pack functions and DynamicMOTD/SpawnProtection if enabled, respectively
		case 0x480e45: // permission - lists all the permissions of each player
		case 0x47ad28: // listd - verbose version of /list
		case 0x44ac4d: // agent - education edition agent
		case 0xaabfe:  // whitelist - adds a player to the whitelist (takes effect if whitelist is enabled, on/off enums don't work)
			return original(this, name, description, CommandPermissionLevel::GameMasters, CommandFlagValue::None, CommandFlagValue::None);

		// lock these commands to permission level 4 only (server console usage)
		case 0x47f4a9: // op - promotes a player's permission to "GameMasters" (permission level 1)
		case 0x468e59: // deop - demotes a player's permission to "Any" (permission level 0)
			return original(this, name, description, CommandPermissionLevel::Owner, CommandFlagValue::None, CommandFlagValue::None);

		default: break;
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