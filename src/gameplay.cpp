#include <dllentry.h>
#include "main.h"

DEFAULT_SETTINGS(settings);

void dllenter() {}
void dllexit() {}

void teleportToWithInterpolatedPacket(Player* _this, Vec3 const& pos, bool shouldStopRiding) {

	CallServerClassMethod<void>("?teleportTo@Mob@@UEAAXAEBVVec3@@_NHHAEBUActorUniqueID@@@Z",
		_this, pos, shouldStopRiding, 0, 1, ActorUniqueID::INVALID_ID);

	MovePlayerPacket teleportPkt(*_this, Player::PositionMode::Normal, 0, 0);
	_this->sendNetworkPacket(teleportPkt);

	if (_this->hasRider()) {
		_this->teleportRidersTo(_this->getPos(), 0, 1);
	}
}

TClasslessInstanceHook(bool, "?trySpawnPortal@PortalBlock@@QEBA_NAEAVBlockSource@@AEBVBlockPos@@@Z", void* region, void* pos) {
  	if (!settings.netherPortalIgniting) return false;
  	return original(this, region, pos);
}

TClasslessInstanceHook(bool, "?use@EndPortalFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@@Z", void* player, void* pos) {
  	if (!settings.endPortalActivating) return false; // for some reason it still places if player is in creative
  	return original(this, player, pos);
}

// neither of these seem to do anything
/*THook(void, "?createPortal@EndPortalFrameBlock@@CAXAEAVBlockSource@@AEBVBlockPos@@@Z", void* region, void* origin) {
	if (settings.endPortalActivating) return;
	original(region, origin);
}
TClasslessInstanceHook(bool, "?isValid@EndPortalShape@@QEAA_NAEAVBlockSource@@@Z", void* region) {
	if (!settings.endPortalActivating) return false;
	return original(this, region);
}*/

// returning from this function causes a crash and I have no idea why
/*TClasslessInstanceHook(void, "?setBedRespawnPosition@Player@@QEAAXAEBVBlockPos@@@Z", void* bedPosition) {
	if (!settings.setBedRespawnPosition) return;
	original(this, bedPosition);
}*/

TClasslessInstanceHook(bool, "?checkMobSpawn@SkullBlock@@QEBA_NAEAVLevel@@AEAVBlockSource@@AEBVBlockPos@@AEAVSkullBlockActor@@@Z",
	void* level, void* region, void* pos, void* placedSkull) {
	if (!settings.spawnWithersFromBlockPattern) return false;
	return original(this, level, region, pos, placedSkull);
}

TClasslessInstanceHook(void, "?startSwimming@Player@@UEAAXXZ") {
	if (!settings.playerSwimming) return;
	original(this);
}

TClasslessInstanceHook(bool, "?_updateTrackedEntityDecoration@MapItemSavedData@@AEAA_NAEAVBlockSource@@V?$shared_ptr@VMapItemTrackedActor@@@std@@@Z",
	void* region, void* trackedEntity) {
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

// this must be set with a mod because splash_radius doesn't work in entity JSON files
TClasslessInstanceHook(void, "?doOnHitEffect@SplashPotionEffectSubcomponent@@UEAAXAEAVActor@@AEAVProjectileComponent@@@Z",
	void* owner, ProjectileComponent &component) {
	component.mSplashRange = settings.splashPotionRadius;
	original(this, owner, component);
}

// unlike gamerule dofiretick, turning this setting off will still allow the source block to extinguish its flame
TClasslessInstanceHook(bool, "?isValidFireLocation@FireBlock@@AEBA_NAEAVBlockSource@@AEBVBlockPos@@@Z",
	void* region, void* pos) {
	if (!settings.fireSpreads) return false;
	return original(this, region, pos);
}

THook(void*, "??0ItemActor@@QEAA@PEAVActorDefinitionGroup@@AEBUActorDefinitionIdentifier@@@Z",
	ItemActor *actor, void* definitionGroup, void* definitionIdentifier) {
	auto ret = original(actor, definitionGroup, definitionIdentifier);
	actor->mLifeTime = settings.itemActorDespawnTime;
	return ret;
}

TInstanceHook(bool,
	"?initialize@Level@@UEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBVLevelSettings@@PEAVLevelData@@PEBV23@@Z",
	Level, void* levelName, void* levelSettings, void* levelData, void* levelId) {
	bool result = original(this, levelName, levelSettings, levelData, levelId);
	this->getLevelDataWrapper()->mNetherScale = settings.netherScale;
	return result;
}

// reimplement Player::startCooldown
// fails to hook?
/*TInstanceHook(void, "?startCooldown@Player@@UEAAXPEBVItem@@@Z", Player, Item const *item) {
	if (item) {
		CooldownType type = item->getCooldownType();
		if (type != CooldownType::None) {
			int32_t time = item->getCooldownTime();
			if (type == CooldownType::EnderPearl) {
				time = settings.enderPearlCooldownTime;
			}
			this->mCooldowns[(int32_t)type] = time;
		}
	}
}*/

// reimplement EnderpearlItem::use
TClasslessInstanceHook(ItemStack&,
	"?use@EnderpearlItem@@UEBAAEAVItemStack@@AEAV2@AEAVPlayer@@@Z", ItemStack &stack, Player &player) {

	auto& lvl = *player.mLevel;
	auto& gr = lvl.getGameRules();
	
	if (!gr.hasRule(GameRulesIndex::AllowDestructiveObjects) || gr.getGameRuleValue<bool>(GameRulesIndex::AllowDestructiveObjects)) {

		player.useItem(stack, ItemUseMethod::Throw, true);
		player.swing();

		auto attachPos = player.getAttachPos(ActorLocation::Head, 0.f);
		player.playSynchronizedSound(LevelSoundEvent::Throw, attachPos, -1, false);

		auto& spawner = *lvl.mMobSpawner.get();
		spawner.spawnProjectile(*player.mRegion, ActorDefinitionIdentifier(ActorType::Enderpearl),
			&player, player.getPos(), Vec3::ZERO);

		// inline Player::startCooldown here
		player.mCooldowns[(int32_t)CooldownType::EnderPearl] = settings.enderPearlCooldownTime;
	}
	return stack;
}

TInstanceHook(int32_t, "?getItemCooldownLeft@Player@@UEBAHW4CooldownType@@@Z", Player, CooldownType type) {
	int32_t ret = original(this, type);
	if (settings.sendEnderPearlCooldownMessage && (type == CooldownType::EnderPearl) && (ret > 0)) {

		const float cooldownSeconds = ret / 20.0f;
		std::stringstream ss;
		ss << std::setw(0) << cooldownSeconds;

		auto pkt = TextPacket::createTextPacket<TextPacketType::JukeboxPopup>("§6" + ss.str() + "§r");
		this->sendNetworkPacket(pkt);
	}
	return ret;
}

TClasslessInstanceHook(void, "?doOnHitEffect@TeleportToSubcomponent@@UEAAXAEAVActor@@AEAVProjectileComponent@@@Z",
	Actor &projectile, ProjectileComponent &component) {

	auto& lvl = *projectile.mLevel;

	auto shooter = lvl.fetchEntity(component.mOwnerId, false);
	if (shooter && shooter->hasCategory(ActorCategory::Player)) {

		auto playerShooter = (Player*)shooter;
		auto target = component.mHitResult.mEntity;

		if (target) {
			ActorDamageByChildActorSource teleportIntoDmgSource(projectile, *playerShooter, ActorDamageCause::Projectile);
			target->hurt(teleportIntoDmgSource, 0, true, false);
		}
		lvl.broadcastLevelEvent(LevelEvent::ParticlesTeleport, projectile.getPos(), 0, nullptr);

		if (projectile.mDimensionId == playerShooter->mDimensionId) {

			if (playerShooter->isRiding()) {
				playerShooter->stopRiding(true, true, false);
			}

			auto posBeforeTeleport = playerShooter->getPos(); // save for later

			// vanilla sends a MovePlayerPacket with the teleport position mode, then in the teleportTo function sends the packet
			// again unnecessarily. If we teleport the player using Player::PositionMode::Normal, it gives the "interpolated"
			// teleport effect, and looks a little bit nicer for pvp
			/*playerShooter->teleportTo(
				component.mHitResult.mPos, true, 1, (int32_t)projectile.getEntityTypeId(), ActorUniqueID::INVALID_ID);
			MovePlayerPacket teleportPkt(*playerShooter, Player::PositionMode::Teleport,
				(int32_t)MovePlayerPacket::TeleportCause::Projectile, (int32_t)projectile.getEntityTypeId());
			playerShooter->sendNetworkPacket(teleportPkt);*/
			teleportToWithInterpolatedPacket(playerShooter, component.mHitResult.mPos, true);

			auto& region = *projectile.mRegion;
			lvl.broadcastDimensionEvent(region, LevelEvent::SoundTeleportEnderPearl, posBeforeTeleport, 0, nullptr);
			lvl.broadcastDimensionEvent(region, LevelEvent::SoundTeleportEnderPearl, playerShooter->getPos(), 0, nullptr);

			// vanilla actually checks if player has instabuild ability instead of gamemode
			if (settings.enderPearlFallDamage && !playerShooter->isInCreativeMode()) {
				ActorDamageSource teleportDmgSource(ActorDamageCause::Fall);
				playerShooter->hurt(teleportDmgSource, 5, true, false); // idk why knock is true but its what vanilla does
			}
		}
	}
}