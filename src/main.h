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
#include <Command/CommandOutput.h>
#include <Item/ItemStack.h>
#include <Item/Item.h>
#include <Item/EnchantResult.h>
#include <Item/EnderPearlItem.h>
#include <Packet/TextPacket.h>
#include <Packet/AddPlayerPacket.h>
#include <Net/ServerNetworkHandler.h>
#include <Core/mce.h>
#include <Component/AABBShapeComponent.h>
#include <Component/ProjectileComponent.h>
#include <Block/FireBlock.h>
#include <Block/Block.h>
#include <Block/BlockLegacy.h>
#include <Block/BlockSource.h>
#include <Actor/ItemActor.h>

inline struct Settings {
    bool netherPortalIgniting            = true;
    bool endPortalActivating             = true;
    //bool setBedRespawnPosition           = true;
    bool spawnWithersFromBlockPattern    = true;
    bool playerSwimming                  = true;
    bool playerMapMarkers                = true;
    bool lightningDealsDamage            = true;
    float splashPotionRadius             = 4.0f;
    bool fireSpreads                     = true;
    bool playersCanCrit                  = true;
    uint32_t enderPearlCooldownTime      = 20;
    bool sendEnderPearlCooldownMessage   = true;
    uint32_t netherScale                 = 8;
    bool playersCanChangeSkins           = true;
    uint32_t itemActorDespawnTime        = 6000;

    template <typename IO> static inline bool io(IO f, Settings &settings, YAML::Node &node) {
  
	    return f(settings.netherPortalIgniting, node["netherPortalIgniting"]) &&
            f(settings.endPortalActivating, node["endPortalActivating"]) &&
            //f(settings.setBedRespawnPosition, node["setBedRespawnPosition"]) &&
            f(settings.spawnWithersFromBlockPattern, node["spawnWithersFromBlockPattern"]) &&
            f(settings.playerSwimming, node["playerSwimming"]) &&
            f(settings.playerMapMarkers, node["playerMapMarkers"]) &&
            f(settings.lightningDealsDamage, node["lightningDealsDamage"]) &&
            f(settings.splashPotionRadius, node["splashPotionRadius"]) &&
            f(settings.fireSpreads, node["fireSpreads"]) &&
            f(settings.playersCanCrit, node["playersCanCrit"]) &&
            f(settings.enderPearlCooldownTime, node["enderPearlCooldownTime"]) &&
            f(settings.sendEnderPearlCooldownMessage, node["sendEnderPearlCooldownMessage"]) &&
            f(settings.netherScale, node["netherScale"]) &&
            f(settings.playersCanChangeSkins, node["playersCanChangeSkins"]) &&
            f(settings.itemActorDespawnTime, node["itemActorDespawnTime"]);
	}
} settings;

DEF_LOGGER("UHCEssentials");