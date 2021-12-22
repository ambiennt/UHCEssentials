#include <dllentry.h>
#include "main.h"

DEFAULT_SETTINGS(settings);

void dllenter() {}
void dllexit() {}
void PreInit() {
    if (!settings.playersCanCrit) {
        GetServerSymbolWithOffset<PatchSpan<2>>("?attack@Player@@UEAA_NAEAVActor@@@Z", 0x205) // change jz to jnz
            ->VerifyPatchFunction({0x74, 0x1A}, {0x75, 0x1A}); // 74 1A
    }
}
void PostInit() {}

TClasslessInstanceHook(bool, "?trySpawnPortal@PortalBlock@@QEBA_NAEAVBlockSource@@AEBVBlockPos@@@Z", BlockSource* region, const BlockPos* pos) {
  	if (!settings.netherPortalIgniting) return false;
  	return original(this, region, pos);
}

TClasslessInstanceHook(bool, "?use@EndPortalFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@@Z", Player* player, const BlockPos* pos) {
  	if (!settings.endPortalActivating) return false; // for some reason it still places if player is in creative
  	return original(this, player, pos);
}

 // neither of these seem to do anything
/*THook(void, "?createPortal@EndPortalFrameBlock@@CAXAEAVBlockSource@@AEBVBlockPos@@@Z", BlockSource *region, const BlockPos *origin) {
    if (settings.endPortalActivating) return;
    original(region, origin);
}
TClasslessInstanceHook(bool, "?isValid@EndPortalShape@@QEAA_NAEAVBlockSource@@@Z", BlockSource* region) {
    if (!settings.endPortalActivating) return false;
    return original(this, region);
}*/

// returning from this function causes a crash and I have no idea why
/*THook(void, "?setBedRespawnPosition@Player@@QEAAXAEBVBlockPos@@@Z", Player *player, const BlockPos *bedPosition) {
	if (!settings.setBedRespawnPosition) return;
	original(player, bedPosition);
}*/

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

// only responsible for sending the animate packet
/*TClasslessInstanceHook(void, "?_crit@Player@@UEAAXAEAVActor@@@Z", Actor *actor) {
	if (!settings.playerCrits && actor->getEntityTypeId() == ActorType::Player_0) return;
	original(this, actor);
}*/

// this must be set with a mod because splash_radius doesn't work in entity JSON files
TClasslessInstanceHook(void, "?doOnHitEffect@SplashPotionEffectSubcomponent@@UEAAXAEAVActor@@AEAVProjectileComponent@@@Z",
    Actor &owner, ProjectileComponent &component) {
    component.mSplashRange = settings.splashPotionRadius;
    original(this, owner, component);
}

THook(bool, "?initialize@Level@@UEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBVLevelSettings@@PEAVLevelData@@PEBV23@@Z",
    Level *level, const std::string *levelName, const LevelSettings *levelSettings, LevelData *levelData, const std::string *levelId) {

    bool result = original(level, levelName, levelSettings, levelData, levelId);
    auto& wrapper = level->GetLevelDataWrapper();
    wrapper->mNetherScale = settings.netherScale;

    wrapper->mXBLBroadcastIntent      = 4; // public
    wrapper->mXBLBroadcastMode        = 4;
    wrapper->mPlatformBroadcastIntent = 4;
    wrapper->mPlatformBroadcastMode   = 4;
    return result;
}

//as weird as it sounds, it seems like other inlined functions that return exactly 20 also call this function
//for example: horse jump packet uses this function to get its packet id
//as a result, the wrong packet id is sent to the client for this packet. The highest packet id in 1.16.20 is 156 so sending 
//a value of 157 or higher will force disconnect the client due to it receiving an unknown packet
TClasslessInstanceHook(int, "?getCooldownTime@EnderpearlItem@@UEBAHXZ") {
    if ((int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0) == 0x727F17) { // if called when using an ender pearl
        return settings.enderPearlCooldownTime;
    }
    return original(this);
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





// non-configurable essentials / bug vanilla bug fixes go here

// make all skins trusted, regardless of whether the client has this setting enabled
THook(void, "?write@SerializedSkin@@QEBAXAEAVBinaryStream@@@Z", SerializedSkin &skin, BinaryStream &stream) {
	skin.trusted_flag = SerializedSkin::TrustedSkinFlag::YES;
	original(skin, stream);
}

// we shouldn't be sending other clients this information so it is removed
THook(void, "?write@AddPlayerPacket@@UEBAXAEAVBinaryStream@@@Z", AddPlayerPacket &pkt, BinaryStream &stream) {
    pkt.mDeviceId = "";
    pkt.mBuildPlatform = BuildPlatform::Unknown;
    original(pkt, stream);
}

// fix crash, idk if its a problem in 1.16.20 though
THook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVPlayerSkinPacket@@@Z",
    ServerNetworkHandler &snh, NetworkIdentifier &source, void *pkt) {
    if (!settings.playersCanChangeSkins) return;
    original(snh, source, pkt);
}

// unlock setmaxplayers command to ignore the 30 player cap
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

TClasslessInstanceHook(void,
    "?registerCommand@CommandRegistry@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PEBDW4CommandPermissionLevel@@UCommandFlag@@3@Z",
    const std::string *name, const char *description, CommandPermissionLevel requirement, CommandFlagValue f1, CommandFlagValue f2) {

    //std::cout << " command: " << *name << "   level: " << (int)requirement << "   f1: " << (int)f1 << "   f2: " << (int)f2 << std::endl;
    //std::cout << "0x" << std::hex << (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0) << " - " << *name << std::endl;
    int64_t address = (int64_t)_ReturnAddress() - (int64_t)GetModuleHandle(0);
    switch (address) {
        //case 0x475ff3: // gettopsolidblock - gets top solid block in the world (silently fails)
        //case 0x4b5bea: // wsserver - websocket server (doesn't seem to work on BDS)
        //case 0xa8cc4:  // save - save level
        //case 0xaa75b:  // stop - stop server
        //case 0x453dc3: // querytarget - prints json of selector position and rotation, but shows to all clients
        //case 0xa3db0:  // changesetting - change difficulty and toggle cheats
        case 0x4975f9:   // setmaxplayers - sets max player count
        case 0x484987:   // reload - reloads behavior pack functions and DynamicMOTD/SpawnProtection if enabled, respectively
        case 0x480e45:   // permission - lists all the permissions of each player
        case 0x47ad28:   // listd - verbose version of /list
        case 0x44ac4d:   // agent - education edition agent
        case 0xaabfe:    // whitelist - adds a player to the whitelist (takes effect if whitelist is enabled, on/off enums don't work)
            {
                requirement = CommandPermissionLevel::GameMasters;
                f1 = CommandFlagValue::None;
                f2 = CommandFlagValue::None;
                break;
            }

        case 0x47f4a9: // op - promotes a player's permission to "GameMasters" (permission level 1)
        case 0x468e59: // deop - demotes a player's permission to "Any" (permission level 0)
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

// the player bounding box width is improperly initialized as 0.8 instead of 0.6
// the bounding box still gets set as 0.8 afterward but I can't find where this number is set
// however, just modifying the value at the player constructor prevents the bounding box width
// from showing up as 0.8 clientside for some reason
THook(void,
    "??0Player@@QEAA@AEAVLevel@@AEAVPacketSender@@W4GameType@@AEBVNetworkIdentifier@@EVUUID@mce@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$unique_ptr@VCertificate@@U?$default_delete@VCertificate@@@std@@@8@55@Z",
    Player* player, void *level, void *packetSender, void *playerGameType, void *owner,
    void *subid, void *uuid, void *deviceId, void *certificate, void *platformId, void *platformOnlineId) {

    original(player, level, packetSender, playerGameType, owner, subid, uuid, deviceId, certificate, platformId, platformOnlineId);
    const auto& aabb = player->mAABBComponent.mBBDim;
    player->mEntityData.set(ActorDataIDs::BOUNDING_BOX_WIDTH, aabb.x);
    player->mEntityData.set(ActorDataIDs::BOUNDING_BOX_HEIGHT, aabb.y);
}

/*THook(void, "?frostWalk@Mob@@QEAAXXZ", Mob *mob) {

    std::cout << "mServerAuthoritativeMovement: " << LocateService<Level>()->mServerAuthoritativeMovement << std::endl;
    auto boots = mob->getArmor((ArmorSlot) 3);
    if (boots.mValid) {
        std::cout << "mValid" << std::endl;
    }
    original(mob);
}*/

/*THook(bool, "?mayPlace@BlockSource@@QEAA_NAEBVBlock@@AEBVBlockPos@@EPEAVActor@@_N@Z",
    BlockSource *region, const Block *block, const BlockPos *pos, uint8_t face, Actor* placer, bool ignoreEntities) {

    bool ret = original(region, block, pos, face, placer, ignoreEntities);
    std::cout << "result: " << ret << std::endl;
    return ret;
}*/