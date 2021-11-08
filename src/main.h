#pragma once

#include <hook.h>
#include <base/log.h>
#include <yaml.h>
#include <Actor/Actor.h>
#include <Actor/Player.h>
#include <Actor/ActorType.h>
#include <Actor/ActorDamageSource.h>
#include <Actor/GameMode.h>
#include <Level/Level.h>
#include <Level/LevelSettings.h>
#include <Level/LevelDataWrapper.h>
#include <Actor/Skin/SerializedSkin.h>
#include <Command/CommandPermissionLevel.h>
#include <Command/CommandFlag.h>
#include <Item/ItemStack.h>
#include <Item/Item.h>
#include <Packet/TextPacket.h>

inline struct Settings {
    bool netherPortalIgniting            = true;
    bool endPortalActivating             = true;
    bool setBedRespawnPosition           = true;
    bool spawnWithersFromBlockPattern    = true;
    bool playerSwimming                  = true;
    bool playerMapMarkers                = true;
    bool lightningDealsDamage            = true;
    bool playerCrits                     = true;
    unsigned int enderPearlCooldownTime  = 20;
    bool sendEnderPearlCooldownMessage   = true;
    unsigned int netherScale             = 8;

    template <typename IO> static inline bool io(IO f, Settings &settings, YAML::Node &node) {
  
	    return f(settings.netherPortalIgniting, node["netherPortalIgniting"]) &&
            f(settings.endPortalActivating, node["endPortalActivating"]) &&
            f(settings.setBedRespawnPosition, node["setBedRespawnPosition"]) &&
            f(settings.spawnWithersFromBlockPattern, node["spawnWithersFromBlockPattern"]) &&
            f(settings.playerSwimming, node["playerSwimming"]) &&
            f(settings.playerMapMarkers, node["playerMapMarkers"]) &&
            f(settings.lightningDealsDamage, node["lightningDealsDamage"]) &&
            f(settings.playerCrits, node["playerCrits"]) &&
            f(settings.enderPearlCooldownTime, node["enderPearlCooldownTime"]) &&
            f(settings.sendEnderPearlCooldownMessage, node["sendEnderPearlCooldownMessage"]) &&
            f(settings.netherScale, node["netherScale"]);
	}
} settings;

DEF_LOGGER("UHCEssentials");