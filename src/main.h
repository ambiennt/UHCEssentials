#pragma once

#include <hook.h>
#include <base/log.h>
#include <yaml.h>
#include <Actor/Actor.h>
#include <Actor/Player.h>
#include <Actor/ActorType.h>
#include <Actor/ActorDefinitionIdentifier.h>
#include <Level/Level.h>
#include <Level/LevelDataWrapper.h>
#include <Level/GameRules.h>
#include <Level/GameRulesIndex.h>
#include <Level/Spawner.h>
#include <Command/CommandPermissionLevel.h>
#include <Command/CommandFlag.h>
#include <Command/CommandOutput.h>
#include <Item/ItemStack.h>
#include <Item/Item.h>
#include <Item/EnderPearlItem.h>
#include <Packet/TextPacket.h>
#include <Net/ServerNetworkHandler.h>
#include <Component/ProjectileComponent.h>
#include <Actor/ItemActor.h>
#include <Math/Vec3.h>
#include <Packet/MovePlayerPacket.h>
//#include <Packet/MoveActorAbsolutePacket.h>

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
	uint32_t enderPearlCooldownTime      = 20;
	bool sendEnderPearlCooldownMessage   = true;
	bool enderPearlFallDamage            = true;
	uint32_t netherScale                 = 8;
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
			f(settings.enderPearlCooldownTime, node["enderPearlCooldownTime"]) &&
			f(settings.sendEnderPearlCooldownMessage, node["sendEnderPearlCooldownMessage"]) &&
			f(settings.enderPearlFallDamage, node["enderPearlFallDamage"]) &&
			f(settings.netherScale, node["netherScale"]) &&
			f(settings.itemActorDespawnTime, node["itemActorDespawnTime"]);
	}
} settings;

DEF_LOGGER("UHCEssentials");