#include <dllentry.h>
#include "main.h"

DEFAULT_SETTINGS(settings);

void dllenter() {}
void dllexit() {}
void PreInit() {
	if (!settings.playersCanCrit) {
		// change jz to jnz
		GetServerSymbolWithOffset<PatchSpan<2>>(
			"?attack@Player@@UEAA_NAEAVActor@@@Z", 0x205)->VerifyPatchFunction({0x74, 0x1A}, {0x75, 0x1A}); // 74 1A
	}
}
void PostInit() {}

// only responsible for sending the animate packet
// damage bonus will still apply even if this is canceled
/*TClasslessInstanceHook(void, "?_crit@Player@@UEAAXAEAVActor@@@Z", Actor *actor) {
	if (!settings.playerCrits && actor->getEntityTypeId() == ActorType::Player_0) return;
	original(this, actor);
}*/

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

TClasslessInstanceHook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVPlayerSkinPacket@@@Z",
	void* source, void* pkt) {
	if (!settings.playersCanChangeSkins) return;
	original(this, source, pkt);
}

THook(bool,
	"?initialize@Level@@UEAA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBVLevelSettings@@PEAVLevelData@@PEBV23@@Z",
	Level *level, void* levelName, void* levelSettings, void* levelData, void* levelId) {
	bool result = original(level, levelName, levelSettings, levelData, levelId);
	level->getLevelDataWrapper()->mNetherScale = settings.netherScale;
	return result;
}

// reimplement Player::startCooldown
// fails to hook?
/*THook(void, "?startCooldown@Player@@UEAAXPEBVItem@@@Z", Player *player, Item const *item) {
	if (item) {
		CooldownType type = item->getCooldownType();
		if (type != CooldownType::None) {
			int32_t time = item->getCooldownTime();
			if (type == CooldownType::EnderPearl) {
				time = settings.enderPearlCooldownTime;
			}
			player->mCooldowns[(int32_t) type] = time;
		}
	}
}*/

// reimplement EnderpearlItem::use
THook(ItemStack*, "?use@EnderpearlItem@@UEBAAEAVItemStack@@AEAV2@AEAVPlayer@@@Z", EnderPearlItem *pearl, ItemStack *stack, Player *player) {

	auto lvl = LocateService<Level>();
	auto* gr = &lvl->getGameRules();
	
	if (!gr->hasRule(GameRulesIndex::AllowDestructiveObjects) || gr->getBool(GameRulesIndex::AllowDestructiveObjects)) {

		player->useItem(*stack, ItemUseMethod::Throw, true);
		player->swing();

		auto attachPos = player->getAttachPos(ActorLocation::DropAttachPoint, 0.0f);
		player->playSynchronizedSound(LevelSoundEvent::Throw, attachPos, 0, false);

		auto spawner = lvl->mMobSpawner.get();
		ActorDefinitionIdentifier def(ActorType::Enderpearl);
		CallServerClassMethod<Actor*>(
			"?spawnProjectile@Spawner@@QEAAPEAVActor@@AEAVBlockSource@@AEBUActorDefinitionIdentifier@@PEAV2@AEBVVec3@@3@Z",
			spawner, player->mRegion, def, player, player->getPos(), Vec3::ZERO);

		// inline Player::startCooldown here
		player->mCooldowns[(int32_t) CooldownType::EnderPearl] = settings.enderPearlCooldownTime;
	}
	return stack;
}

THook(int, "?getItemCooldownLeft@Player@@UEBAHW4CooldownType@@@Z", Player* player, CooldownType type) {
	int ret = original(player, type);
	if (settings.sendEnderPearlCooldownMessage && (type == CooldownType::EnderPearl) && (ret > 0)) {

		const float cooldownSeconds = ret / 20.0f;
		std::stringstream ss;
		ss << std::setw(0) << cooldownSeconds;

		auto pkt = TextPacket::createTextPacket<TextPacketType::JukeboxPopup>("§6" + ss.str() + "§r");
		player->sendNetworkPacket(pkt);
	}
	return ret;
}