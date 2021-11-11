#include <dllentry.h>
#include "main.h"

DEFAULT_SETTINGS(settings);

void dllenter() {}
void dllexit() {}

TClasslessInstanceHook(bool, "?trySpawnPortal@PortalBlock@@QEBA_NAEAVBlockSource@@AEBVBlockPos@@@Z", BlockSource* region, const BlockPos* pos) {
  	if (!settings.netherPortalIgniting) return false;
  	return original(this, region, pos);
}

TClasslessInstanceHook(bool, "?use@EndPortalFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@@Z", Player* player, const BlockPos* pos) {
  	if (!settings.endPortalActivating) return false; // for some reason it still places if player is in creative
  	return original(this, player, pos);
}
/*THook(void, "?createPortal@EndPortalFrameBlock@@CAXAEAVBlockSource@@AEBVBlockPos@@@Z", BlockSource *region, const BlockPos *origin) {
    if (settings.endPortalActivating) return; // doesn't seem to do anything
    original(region, origin);
}
TClasslessInstanceHook(bool, "?isValid@EndPortalShape@@QEAA_NAEAVBlockSource@@@Z", BlockSource* region) {
    if (!settings.endPortalActivating) return false; // doesn't seem to do anything
    return original(this, region);
}*/

TClasslessInstanceHook(void, "?setBedRespawnPosition@Player@@QEAAXAEBVBlockPos@@@Z", const BlockPos* bedPosition) {
	if (!settings.setBedRespawnPosition) return;
	original(this, bedPosition);
}

TClasslessInstanceHook(bool, "?checkMobSpawn@SkullBlock@@QEBA_NAEAVLevel@@AEAVBlockSource@@AEBVBlockPos@@AEAVSkullBlockActor@@@Z",
    Level *level, BlockSource *region, const BlockPos *pos, class SkullBlockActor *placedSkull) {
    if (!settings.spawnWithersFromBlockPattern) return false;
    return original(this, level, region, pos, placedSkull);
}

TClasslessInstanceHook(void, "?startSwimming@Player@@UEAAXXZ") {
	if (!settings.playerSwimming) return;
	original(this);
}

TClasslessInstanceHook(bool, "?_updateTrackedEntityDecoration@MapItemSavedData@@AEAA_NAEAVBlockSource@@V?$shared_ptr@VMapItemTrackedActor@@@std@@@Z",
    BlockSource* region, std::shared_ptr<class MapItemTrackedActor> trackedEntity) {
  	if (!settings.playerMapMarkers) return false;
  	return original(this, region, trackedEntity);
}

TClasslessInstanceHook(bool, "?_shouldSetOnFire@LightningBolt@@AEBA_NXZ") {
	if (!settings.lightningDealsDamage) return false;
	return original(this);
}
TClasslessInstanceHook(void, "?onLightningHit@Actor@@UEAAXXZ") {
	if (!settings.lightningDealsDamage) return;
	original(this);
}

TClasslessInstanceHook(void, "?_crit@Player@@UEAAXAEAVActor@@@Z", Actor *actor) {
	if (!settings.playerCrits && actor->getEntityTypeId() == ActorType::Player_0) return;
	original(this, actor);
}

THook(bool, "?initialize@Level@@UEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBVLevelSettings@@PEAVLevelData@@PEBV23@@Z",
	Level *level, const std::string *levelName, const LevelSettings *levelSettings, LevelData *levelData, const std::string *levelId) {

	bool result = original(level, levelName, levelSettings, levelData, levelId);
	auto& wrapper = level->GetLevelDataWrapper();
	wrapper->mNetherScale = settings.netherScale;
	return result;
}

TClasslessInstanceHook(int, "?getCooldownTime@EnderpearlItem@@UEBAHXZ") {
    return settings.enderPearlCooldownTime;
}

THook(int, "?getItemCooldownLeft@Player@@UEBAHW4CooldownType@@@Z", Player* player, CooldownType type) {
    int ret = original(player, type);
    if (settings.sendEnderPearlCooldownMessage && type == CooldownType::EnderPearl && ret > 0) {
        
        const float cooldownSeconds = ret / 20.0f;
        std::stringstream ss;
        ss << std::setw(0) << cooldownSeconds;

        auto pkt = TextPacket::createTextPacket<TextPacketType::JukeboxPopup>("§6" + ss.str() + "§r");
        player->sendNetworkPacket(pkt);
    }
    return ret;
}

THook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVPlayerSkinPacket@@@Z",
    ServerNetworkHandler &snh, NetworkIdentifier &source, void *pkt) {
    if (!settings.playersCanChangeSkins) return;
    original(snh, source, pkt);
}

//non-configurable essentials / bug vanilla bug fixes go here

THook(void, "?write@SerializedSkin@@QEBAXAEAVBinaryStream@@@Z", SerializedSkin &skin, BinaryStream &stream) {
	
	skin.trusted_flag = SerializedSkin::TrustedFlag::YES;
	original(skin, stream);
}

THook(void, "?execute@SetMaxPlayersCommand@@UEBAXAEBVCommandOrigin@@AEAVCommandOutput@@@Z",
    class SetMaxPlayersCommand *cmd, void *origin, CommandOutput &output) {

    auto snh = LocateService<ServerNetworkHandler>();
    const auto empty = mce::UUID::EMPTY;
    int64_t activePlayers = CallServerClassMethod<int64_t>(
        "?_getActiveAndInProgressPlayerCount@ServerNetworkHandler@@AEBAHVUUID@mce@@@Z", snh, &empty);
    int cmdCount = direct_access<int>(cmd, 0x20);
    int newCount = std::clamp(cmdCount, (int)activePlayers, INT_MAX);
    snh->mMaxNumPlayers = newCount;
    snh->updateServerAnnouncement();

    output.success("commands.setmaxplayers.success", {newCount});
    if (cmdCount < activePlayers) {
        output.success("commands.setmaxplayers.success.lowerbound", {newCount});
    }
}

TClasslessInstanceHook(void, "?registerCommand@CommandRegistry@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBDW4CommandPermissionLevel@@UCommandFlag@@3@Z",
    const std::string *name, const char *description, CommandPermissionLevel requirement, CommandFlagValue f1, CommandFlagValue f2) {

    //std::cout << " command: " << *name << "   level: " << (int)requirement << "   f1: " << (int)f1 << "   f2: " << (int)f2 << std::endl;
    //std::cout << "0x" << std::hex << (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0) << " - " << *name << std::endl;
    int64_t address = (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0);
    switch (address) {
        //case 0x475ff3: // gettopsolidblock
        //case 0x4b5bea: // wsserver
        //case 0xa8cc4:  // save
        //case 0xaa75b:  // stop
        //case 0xa3db0:  // changesetting
        //case 0x453dc3: // querytarget
        //case 0x4975f9: // setmaxplayers
        case 0x480e45: // permission
        case 0x47ad28: // listd
        case 0x44ac4d: // agent
        case 0xaabfe:  // whitelist
            {
                requirement = CommandPermissionLevel::GameMasters;
                f1 = CommandFlagValue::None;
                f2 = CommandFlagValue::None;
                break;
            }

        case 0x47f4a9: // op
        case 0x468e59: // deop
            {
                requirement = CommandPermissionLevel::Owner;
                f1 = CommandFlagValue::None;
                f2 = CommandFlagValue::None;
                break;
            }

        default: break;
    }
    original(this, name, description, requirement, f1, f2);
}