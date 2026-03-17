// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/Data/CameraStateHelpers.h"

// ========== Category枚举转换 ==========

ECameraStateCategory UCameraStateHelpers::StringToCategory(const FString& CategoryString)
{
	FString TrimmedString = CategoryString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("FreeExploration"), ESearchCase::IgnoreCase) ||
	    TrimmedString.Equals(TEXT("Explore"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::FreeExploration;
	else if (TrimmedString.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Combat;
	else if (TrimmedString.Equals(TEXT("LockOn"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::LockOn;
	else if (TrimmedString.Equals(TEXT("Boss"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Boss;
	else if (TrimmedString.Equals(TEXT("Sprint"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Sprint;
	else if (TrimmedString.Equals(TEXT("Mount"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Mount;
	else if (TrimmedString.Equals(TEXT("Swim"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Swim;
	else if (TrimmedString.Equals(TEXT("Climb"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Climb;
	else if (TrimmedString.Equals(TEXT("Environment"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Environment;
	else if (TrimmedString.Equals(TEXT("Item"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Item;
	else if (TrimmedString.Equals(TEXT("NPC"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::NPC;
	else if (TrimmedString.Equals(TEXT("RestPoint"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::RestPoint;
	else if (TrimmedString.Equals(TEXT("Death"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Death;
	else if (TrimmedString.Equals(TEXT("Cinematic"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Cinematic;
	else if (TrimmedString.Equals(TEXT("Magic"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Magic;
	else if (TrimmedString.Equals(TEXT("Multiplayer"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Multiplayer;
	else if (TrimmedString.Equals(TEXT("UI"), ESearchCase::IgnoreCase) ||
	         TrimmedString.Equals(TEXT("UserInterface"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::UserInterface;
	else if (TrimmedString.Equals(TEXT("Modifier"), ESearchCase::IgnoreCase))
		return ECameraStateCategory::Modifier;
	
	return ECameraStateCategory::None;
}

FString UCameraStateHelpers::CategoryToString(ECameraStateCategory Category)
{
	switch (Category)
	{
	case ECameraStateCategory::Explore:				return TEXT("Explore");
	case ECameraStateCategory::FreeExploration:		return TEXT("FreeExploration");
	case ECameraStateCategory::Combat:				return TEXT("Combat");
	case ECameraStateCategory::LockOn:				return TEXT("LockOn");
	case ECameraStateCategory::Boss:				return TEXT("Boss");
	case ECameraStateCategory::Sprint:				return TEXT("Sprint");
	case ECameraStateCategory::Mount:				return TEXT("Mount");
	case ECameraStateCategory::Swim:				return TEXT("Swim");
	case ECameraStateCategory::Climb:				return TEXT("Climb");
	case ECameraStateCategory::Environment:			return TEXT("Environment");
	case ECameraStateCategory::Item:				return TEXT("Item");
	case ECameraStateCategory::NPC:					return TEXT("NPC");
	case ECameraStateCategory::RestPoint:			return TEXT("RestPoint");
	case ECameraStateCategory::Death:				return TEXT("Death");
	case ECameraStateCategory::Cinematic:			return TEXT("Cinematic");
	case ECameraStateCategory::Magic:				return TEXT("Magic");
	case ECameraStateCategory::Multiplayer:			return TEXT("Multiplayer");
	case ECameraStateCategory::UserInterface:		return TEXT("UI");
	case ECameraStateCategory::Modifier:			return TEXT("Modifier");
	case ECameraStateCategory::None:
	default:										return TEXT("None");
	}
}

// ========== SubCategory枚举转换 ==========

ECameraStateSubCategory UCameraStateHelpers::StringToSubCategory(const FString& SubCategoryString)
{
	FString TrimmedString = SubCategoryString.TrimStartAndEnd();
	
	// FreeExploration相关
	if (TrimmedString.Equals(TEXT("BasicMovement"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::BasicMovement;
	else if (TrimmedString.Equals(TEXT("Idle"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Idle;
	else if (TrimmedString.Equals(TEXT("Alignment"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Alignment;
	else if (TrimmedString.Equals(TEXT("Shoulder"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Shoulder;
	else if (TrimmedString.Equals(TEXT("Collision"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Collision;
	else if (TrimmedString.Equals(TEXT("Movement"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Movement;
	else if (TrimmedString.Equals(TEXT("Climbing"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Climbing;
	else if (TrimmedString.Equals(TEXT("Mount"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Mount;
	else if (TrimmedString.Equals(TEXT("Edge"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Edge;
	else if (TrimmedString.Equals(TEXT("Stealth"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Stealth;
	else if (TrimmedString.Equals(TEXT("AreaTransition"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::AreaTransition;
	
	// Combat相关
	else if (TrimmedString.Equals(TEXT("BasicCombat"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::BasicCombat;
	else if (TrimmedString.Equals(TEXT("Threat"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Threat;
	else if (TrimmedString.Equals(TEXT("LockOn"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::LockOn;
	else if (TrimmedString.Equals(TEXT("Attack"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Attack;
	else if (TrimmedString.Equals(TEXT("Defend"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Defend;
	else if (TrimmedString.Equals(TEXT("Dodge"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Dodge;
	else if (TrimmedString.Equals(TEXT("Hit"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Hit;
	else if (TrimmedString.Equals(TEXT("Status"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Status;
	else if (TrimmedString.Equals(TEXT("Special"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Special;
	else if (TrimmedString.Equals(TEXT("Boss"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Boss;
	else if (TrimmedString.Equals(TEXT("Ranged"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Ranged;
	else if (TrimmedString.Equals(TEXT("CombatMultiplayer"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::CombatMultiplayer;
	else if (TrimmedString.Equals(TEXT("DeadAngle"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::DeadAngle;
	else if (TrimmedString.Equals(TEXT("Summon"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Summon;
	
	// Environment相关
	else if (TrimmedString.Equals(TEXT("Door"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Door;
	else if (TrimmedString.Equals(TEXT("Destructible"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Destructible;
	else if (TrimmedString.Equals(TEXT("Mechanism"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Mechanism;
	else if (TrimmedString.Equals(TEXT("Trap"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Trap;
	else if (TrimmedString.Equals(TEXT("Elevator"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Elevator;
	else if (TrimmedString.Equals(TEXT("Light"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Light;
	else if (TrimmedString.Equals(TEXT("POI"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::POI;
	else if (TrimmedString.Equals(TEXT("Puzzle"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Puzzle;
	else if (TrimmedString.Equals(TEXT("Hazard"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Hazard;
	else if (TrimmedString.Equals(TEXT("Altar"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Altar;
	
	// Item相关
	else if (TrimmedString.Equals(TEXT("Pickup"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Pickup;
	else if (TrimmedString.Equals(TEXT("Bloodstain"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Bloodstain;
	else if (TrimmedString.Equals(TEXT("Message"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Message;
	else if (TrimmedString.Equals(TEXT("Use"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Use;
	else if (TrimmedString.Equals(TEXT("Equip"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Equip;
	else if (TrimmedString.Equals(TEXT("Upgrade"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Upgrade;
	else if (TrimmedString.Equals(TEXT("Craft"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Craft;
	else if (TrimmedString.Equals(TEXT("Map"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Map;
	
	// NPC相关
	else if (TrimmedString.Equals(TEXT("Dialogue"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Dialogue;
	else if (TrimmedString.Equals(TEXT("Service"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Service;
	else if (TrimmedString.Equals(TEXT("NPCSpecial"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::NPCSpecial;
	
	// RestPoint相关
	else if (TrimmedString.Equals(TEXT("Bonfire"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Bonfire;
	else if (TrimmedString.Equals(TEXT("Checkpoint"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Checkpoint;
	else if (TrimmedString.Equals(TEXT("Hub"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Hub;
	
	// Death相关
	else if (TrimmedString.Equals(TEXT("DeathProcess"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::DeathProcess;
	else if (TrimmedString.Equals(TEXT("DeathCamera"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::DeathCamera;
	else if (TrimmedString.Equals(TEXT("Respawn"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Respawn;
	else if (TrimmedString.Equals(TEXT("Revive"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Revive;
	else if (TrimmedString.Equals(TEXT("Ghost"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Ghost;
	
	// Cinematic相关
	else if (TrimmedString.Equals(TEXT("CutsceneType"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::CutsceneType;
	else if (TrimmedString.Equals(TEXT("Reveal"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Reveal;
	else if (TrimmedString.Equals(TEXT("Character"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Character;
	else if (TrimmedString.Equals(TEXT("WorldEvent"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::WorldEvent;
	else if (TrimmedString.Equals(TEXT("Flashback"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Flashback;
	else if (TrimmedString.Equals(TEXT("CinematicTransition"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::CinematicTransition;
	
	// Magic相关
	else if (TrimmedString.Equals(TEXT("SpellCast"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::SpellCast;
	else if (TrimmedString.Equals(TEXT("MagicSummon"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::MagicSummon;
	else if (TrimmedString.Equals(TEXT("Skill"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Skill;
	
	// Multiplayer相关
	else if (TrimmedString.Equals(TEXT("MultiplayerSummon"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::MultiplayerSummon;
	else if (TrimmedString.Equals(TEXT("PvP"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::PvP;
	else if (TrimmedString.Equals(TEXT("Coop"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Coop;
	
	// UI相关
	else if (TrimmedString.Equals(TEXT("Menu"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Menu;
	else if (TrimmedString.Equals(TEXT("PhotoMode"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::PhotoMode;
	
	// Modifier相关
	else if (TrimmedString.Equals(TEXT("Core"), ESearchCase::IgnoreCase))
		return ECameraStateSubCategory::Core;
	
	return ECameraStateSubCategory::None;
}

FString UCameraStateHelpers::SubCategoryToString(ECameraStateSubCategory SubCategory)
{
	switch (SubCategory)
	{
	// FreeExploration相关
	case ECameraStateSubCategory::BasicMovement:		return TEXT("BasicMovement");
	case ECameraStateSubCategory::Idle:					return TEXT("Idle");
	case ECameraStateSubCategory::Alignment:			return TEXT("Alignment");
	case ECameraStateSubCategory::Shoulder:				return TEXT("Shoulder");
	case ECameraStateSubCategory::Collision:			return TEXT("Collision");
	case ECameraStateSubCategory::Movement:				return TEXT("Movement");
	case ECameraStateSubCategory::Climbing:				return TEXT("Climbing");
	case ECameraStateSubCategory::Mount:				return TEXT("Mount");
	case ECameraStateSubCategory::Edge:					return TEXT("Edge");
	case ECameraStateSubCategory::Stealth:				return TEXT("Stealth");
	case ECameraStateSubCategory::AreaTransition:		return TEXT("AreaTransition");
	
	// Combat相关
	case ECameraStateSubCategory::BasicCombat:			return TEXT("BasicCombat");
	case ECameraStateSubCategory::Threat:				return TEXT("Threat");
	case ECameraStateSubCategory::LockOn:				return TEXT("LockOn");
	case ECameraStateSubCategory::Attack:				return TEXT("Attack");
	case ECameraStateSubCategory::Defend:				return TEXT("Defend");
	case ECameraStateSubCategory::Dodge:				return TEXT("Dodge");
	case ECameraStateSubCategory::Hit:					return TEXT("Hit");
	case ECameraStateSubCategory::Status:				return TEXT("Status");
	case ECameraStateSubCategory::Special:				return TEXT("Special");
	case ECameraStateSubCategory::Boss:					return TEXT("Boss");
	case ECameraStateSubCategory::Ranged:				return TEXT("Ranged");
	case ECameraStateSubCategory::CombatMultiplayer:	return TEXT("CombatMultiplayer");
	case ECameraStateSubCategory::DeadAngle:			return TEXT("DeadAngle");
	case ECameraStateSubCategory::Summon:				return TEXT("Summon");
	
	// Environment相关
	case ECameraStateSubCategory::Door:					return TEXT("Door");
	case ECameraStateSubCategory::Destructible:			return TEXT("Destructible");
	case ECameraStateSubCategory::Mechanism:			return TEXT("Mechanism");
	case ECameraStateSubCategory::Trap:					return TEXT("Trap");
	case ECameraStateSubCategory::Elevator:				return TEXT("Elevator");
	case ECameraStateSubCategory::Light:				return TEXT("Light");
	case ECameraStateSubCategory::POI:					return TEXT("POI");
	case ECameraStateSubCategory::Puzzle:				return TEXT("Puzzle");
	case ECameraStateSubCategory::Hazard:				return TEXT("Hazard");
	case ECameraStateSubCategory::Altar:				return TEXT("Altar");
	
	// Item相关
	case ECameraStateSubCategory::Pickup:				return TEXT("Pickup");
	case ECameraStateSubCategory::Bloodstain:			return TEXT("Bloodstain");
	case ECameraStateSubCategory::Message:				return TEXT("Message");
	case ECameraStateSubCategory::Use:					return TEXT("Use");
	case ECameraStateSubCategory::Equip:				return TEXT("Equip");
	case ECameraStateSubCategory::Upgrade:				return TEXT("Upgrade");
	case ECameraStateSubCategory::Craft:				return TEXT("Craft");
	case ECameraStateSubCategory::Map:					return TEXT("Map");
	
	// NPC相关
	case ECameraStateSubCategory::Dialogue:				return TEXT("Dialogue");
	case ECameraStateSubCategory::Service:				return TEXT("Service");
	case ECameraStateSubCategory::NPCSpecial:			return TEXT("NPCSpecial");
	
	// RestPoint相关
	case ECameraStateSubCategory::Bonfire:				return TEXT("Bonfire");
	case ECameraStateSubCategory::Checkpoint:			return TEXT("Checkpoint");
	case ECameraStateSubCategory::Hub:					return TEXT("Hub");
	
	// Death相关
	case ECameraStateSubCategory::DeathProcess:			return TEXT("DeathProcess");
	case ECameraStateSubCategory::DeathCamera:			return TEXT("DeathCamera");
	case ECameraStateSubCategory::Respawn:				return TEXT("Respawn");
	case ECameraStateSubCategory::Revive:				return TEXT("Revive");
	case ECameraStateSubCategory::Ghost:				return TEXT("Ghost");
	
	// Cinematic相关
	case ECameraStateSubCategory::CutsceneType:			return TEXT("CutsceneType");
	case ECameraStateSubCategory::Reveal:				return TEXT("Reveal");
	case ECameraStateSubCategory::Character:			return TEXT("Character");
	case ECameraStateSubCategory::WorldEvent:			return TEXT("WorldEvent");
	case ECameraStateSubCategory::Flashback:			return TEXT("Flashback");
	case ECameraStateSubCategory::CinematicTransition:	return TEXT("CinematicTransition");
	
	// Magic相关
	case ECameraStateSubCategory::SpellCast:			return TEXT("SpellCast");
	case ECameraStateSubCategory::MagicSummon:			return TEXT("MagicSummon");
	case ECameraStateSubCategory::Skill:				return TEXT("Skill");
	
	// Multiplayer相关
	case ECameraStateSubCategory::MultiplayerSummon:	return TEXT("MultiplayerSummon");
	case ECameraStateSubCategory::PvP:					return TEXT("PvP");
	case ECameraStateSubCategory::Coop:					return TEXT("Coop");
	
	// UI相关
	case ECameraStateSubCategory::Menu:					return TEXT("Menu");
	case ECameraStateSubCategory::PhotoMode:			return TEXT("PhotoMode");
	
	// Modifier相关
	case ECameraStateSubCategory::Core:					return TEXT("Core");
	
	case ECameraStateSubCategory::None:
	default:											return TEXT("None");
	}
}

// ========== Reference枚举转换 ==========

ECameraStateReference UCameraStateHelpers::StringToReference(const FString& ReferenceString)
{
	FString TrimmedString = ReferenceString.TrimStartAndEnd();
	
	if (TrimmedString.Equals(TEXT("All"), ESearchCase::IgnoreCase))
		return ECameraStateReference::All;
	else if (TrimmedString.Equals(TEXT("EldenRing"), ESearchCase::IgnoreCase))
		return ECameraStateReference::EldenRing;
	else if (TrimmedString.Equals(TEXT("Bloodborne"), ESearchCase::IgnoreCase))
		return ECameraStateReference::Bloodborne;
	else if (TrimmedString.Equals(TEXT("DarkSouls"), ESearchCase::IgnoreCase))
		return ECameraStateReference::DarkSouls;
	else if (TrimmedString.Equals(TEXT("DarkSouls3"), ESearchCase::IgnoreCase))
		return ECameraStateReference::DarkSouls3;
	else if (TrimmedString.Equals(TEXT("Sekiro"), ESearchCase::IgnoreCase))
		return ECameraStateReference::Sekiro;
	else if (TrimmedString.Equals(TEXT("LiesOfP"), ESearchCase::IgnoreCase))
		return ECameraStateReference::LiesOfP;
	else if (TrimmedString.Equals(TEXT("Some"), ESearchCase::IgnoreCase))
		return ECameraStateReference::Some;
	else if (TrimmedString.Equals(TEXT("Few"), ESearchCase::IgnoreCase))
		return ECameraStateReference::Few;
	
	return ECameraStateReference::None;
}

FString UCameraStateHelpers::ReferenceToString(ECameraStateReference Reference)
{
	switch (Reference)
	{
	case ECameraStateReference::All:		return TEXT("All");
	case ECameraStateReference::EldenRing:	return TEXT("EldenRing");
	case ECameraStateReference::Bloodborne:	return TEXT("Bloodborne");
	case ECameraStateReference::DarkSouls:	return TEXT("DarkSouls");
	case ECameraStateReference::DarkSouls3:	return TEXT("DarkSouls3");
	case ECameraStateReference::Sekiro:		return TEXT("Sekiro");
	case ECameraStateReference::LiesOfP:	return TEXT("LiesOfP");
	case ECameraStateReference::Some:		return TEXT("Some");
	case ECameraStateReference::Few:		return TEXT("Few");
	case ECameraStateReference::None:
	default:								return TEXT("None");
	}
}

// ========== 分类检查函数 ==========
// 更新为使用 ECameraStateCategory 枚举而非 FString 比较

bool UCameraStateHelpers::IsFreeExplorationCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::FreeExploration;
}

bool UCameraStateHelpers::IsCombatCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Combat;
}

bool UCameraStateHelpers::IsEnvironmentCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Environment;
}

bool UCameraStateHelpers::IsItemCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Item;
}

bool UCameraStateHelpers::IsNPCCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::NPC;
}

bool UCameraStateHelpers::IsRestPointCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::RestPoint;
}

bool UCameraStateHelpers::IsDeathCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Death;
}

bool UCameraStateHelpers::IsCinematicCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Cinematic;
}

bool UCameraStateHelpers::IsMagicCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Magic;
}

bool UCameraStateHelpers::IsMultiplayerCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Multiplayer;
}

bool UCameraStateHelpers::IsUICategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::UI || Row.Category == ECameraStateCategory::UserInterface;
}

bool UCameraStateHelpers::IsModifierCategory(const FCameraStateRow& Row)
{
	return Row.Category == ECameraStateCategory::Modifier;
}

// ========== 优先级检查函数 ==========

bool UCameraStateHelpers::IsHighestPriority(const FCameraStateRow& Row)
{
	return Row.Priority == 1;
}

bool UCameraStateHelpers::IsHighPriority(const FCameraStateRow& Row)
{
	return Row.Priority == 2;
}

bool UCameraStateHelpers::IsMediumPriority(const FCameraStateRow& Row)
{
	return Row.Priority == 3;
}

bool UCameraStateHelpers::IsLowPriority(const FCameraStateRow& Row)
{
	return Row.Priority == 4;
}

// ========== 参考来源检查函数 ==========
// 更新为使用 FName 比较而非 FString::Equals

bool UCameraStateHelpers::IsUniversalReference(const FCameraStateRow& Row)
{
	return Row.Reference == FName(TEXT("All"));
}

bool UCameraStateHelpers::IsFromEldenRing(const FCameraStateRow& Row)
{
	return Row.Reference == FName(TEXT("EldenRing"));
}

bool UCameraStateHelpers::IsFromBloodborne(const FCameraStateRow& Row)
{
	return Row.Reference == FName(TEXT("Bloodborne"));
}

bool UCameraStateHelpers::IsFromDarkSouls(const FCameraStateRow& Row)
{
	return Row.Reference == FName(TEXT("DarkSouls")) ||
		   Row.Reference == FName(TEXT("DarkSouls3"));
}

bool UCameraStateHelpers::IsFromSekiro(const FCameraStateRow& Row)
{
	return Row.Reference == FName(TEXT("Sekiro"));
}

// ========== English Display Names ==========

FString UCameraStateHelpers::GetCategoryDisplayName(ECameraStateCategory Category)
{
	switch (Category)
	{
	case ECameraStateCategory::Explore:				return TEXT("Explore");
	case ECameraStateCategory::FreeExploration:		return TEXT("Free Exploration");
	case ECameraStateCategory::Combat:				return TEXT("Combat");
	case ECameraStateCategory::LockOn:				return TEXT("Lock On");
	case ECameraStateCategory::Boss:				return TEXT("Boss");
	case ECameraStateCategory::Sprint:				return TEXT("Sprint");
	case ECameraStateCategory::Mount:				return TEXT("Mount");
	case ECameraStateCategory::Swim:				return TEXT("Swim");
	case ECameraStateCategory::Climb:				return TEXT("Climb");
	case ECameraStateCategory::Environment:			return TEXT("Environment");
	case ECameraStateCategory::Item:				return TEXT("Item");
	case ECameraStateCategory::NPC:					return TEXT("NPC");
	case ECameraStateCategory::RestPoint:			return TEXT("Rest Point");
	case ECameraStateCategory::Death:				return TEXT("Death");
	case ECameraStateCategory::Cinematic:			return TEXT("Cinematic");
	case ECameraStateCategory::Magic:				return TEXT("Magic");
	case ECameraStateCategory::Multiplayer:			return TEXT("Multiplayer");
	case ECameraStateCategory::UserInterface:		return TEXT("User Interface");
	case ECameraStateCategory::Modifier:			return TEXT("Modifier");
	case ECameraStateCategory::None:
	default:										return TEXT("None");
	}
}

FString UCameraStateHelpers::GetSubCategoryDisplayName(ECameraStateSubCategory SubCategory)
{
	switch (SubCategory)
	{
	// FreeExploration related
	case ECameraStateSubCategory::BasicMovement:		return TEXT("Basic Movement");
	case ECameraStateSubCategory::Idle:					return TEXT("Idle");
	case ECameraStateSubCategory::Alignment:			return TEXT("Alignment");
	case ECameraStateSubCategory::Shoulder:				return TEXT("Shoulder");
	case ECameraStateSubCategory::Collision:			return TEXT("Collision");
	case ECameraStateSubCategory::Movement:				return TEXT("Movement");
	case ECameraStateSubCategory::Climbing:				return TEXT("Climbing");
	case ECameraStateSubCategory::Mount:				return TEXT("Mount");
	case ECameraStateSubCategory::Edge:					return TEXT("Edge");
	case ECameraStateSubCategory::Stealth:				return TEXT("Stealth");
	case ECameraStateSubCategory::AreaTransition:		return TEXT("Area Transition");
	
	// Combat related
	case ECameraStateSubCategory::BasicCombat:			return TEXT("Basic Combat");
	case ECameraStateSubCategory::Threat:				return TEXT("Threat");
	case ECameraStateSubCategory::LockOn:				return TEXT("Lock On");
	case ECameraStateSubCategory::Attack:				return TEXT("Attack");
	case ECameraStateSubCategory::Defend:				return TEXT("Defend");
	case ECameraStateSubCategory::Dodge:				return TEXT("Dodge");
	case ECameraStateSubCategory::Hit:					return TEXT("Hit");
	case ECameraStateSubCategory::Status:				return TEXT("Status");
	case ECameraStateSubCategory::Special:				return TEXT("Special");
	case ECameraStateSubCategory::Boss:					return TEXT("Boss");
	case ECameraStateSubCategory::Ranged:				return TEXT("Ranged");
	case ECameraStateSubCategory::CombatMultiplayer:	return TEXT("Combat Multiplayer");
	case ECameraStateSubCategory::DeadAngle:			return TEXT("Dead Angle");
	case ECameraStateSubCategory::Summon:				return TEXT("Summon");
	
	// Environment related
	case ECameraStateSubCategory::Door:					return TEXT("Door");
	case ECameraStateSubCategory::Destructible:			return TEXT("Destructible");
	case ECameraStateSubCategory::Mechanism:			return TEXT("Mechanism");
	case ECameraStateSubCategory::Trap:					return TEXT("Trap");
	case ECameraStateSubCategory::Elevator:				return TEXT("Elevator");
	case ECameraStateSubCategory::Light:				return TEXT("Light");
	case ECameraStateSubCategory::POI:					return TEXT("POI");
	case ECameraStateSubCategory::Puzzle:				return TEXT("Puzzle");
	case ECameraStateSubCategory::Hazard:				return TEXT("Hazard");
	case ECameraStateSubCategory::Altar:				return TEXT("Altar");
	
	// Item related
	case ECameraStateSubCategory::Pickup:				return TEXT("Pickup");
	case ECameraStateSubCategory::Bloodstain:			return TEXT("Bloodstain");
	case ECameraStateSubCategory::Message:				return TEXT("Message");
	case ECameraStateSubCategory::Use:					return TEXT("Use");
	case ECameraStateSubCategory::Equip:				return TEXT("Equip");
	case ECameraStateSubCategory::Upgrade:				return TEXT("Upgrade");
	case ECameraStateSubCategory::Craft:				return TEXT("Craft");
	case ECameraStateSubCategory::Map:					return TEXT("Map");
	
	// NPC related
	case ECameraStateSubCategory::Dialogue:				return TEXT("Dialogue");
	case ECameraStateSubCategory::Service:				return TEXT("Service");
	case ECameraStateSubCategory::NPCSpecial:			return TEXT("NPC Special");
	
	// RestPoint related
	case ECameraStateSubCategory::Bonfire:				return TEXT("Bonfire");
	case ECameraStateSubCategory::Checkpoint:			return TEXT("Checkpoint");
	case ECameraStateSubCategory::Hub:					return TEXT("Hub");
	
	// Death related
	case ECameraStateSubCategory::DeathProcess:			return TEXT("Death Process");
	case ECameraStateSubCategory::DeathCamera:			return TEXT("Death Camera");
	case ECameraStateSubCategory::Respawn:				return TEXT("Respawn");
	case ECameraStateSubCategory::Revive:				return TEXT("Revive");
	case ECameraStateSubCategory::Ghost:				return TEXT("Ghost");
	
	// Cinematic related
	case ECameraStateSubCategory::CutsceneType:			return TEXT("Cutscene Type");
	case ECameraStateSubCategory::Reveal:				return TEXT("Reveal");
	case ECameraStateSubCategory::Character:			return TEXT("Character");
	case ECameraStateSubCategory::WorldEvent:			return TEXT("World Event");
	case ECameraStateSubCategory::Flashback:			return TEXT("Flashback");
	case ECameraStateSubCategory::CinematicTransition:	return TEXT("Cinematic Transition");
	
	// Magic related
	case ECameraStateSubCategory::SpellCast:			return TEXT("Spell Cast");
	case ECameraStateSubCategory::MagicSummon:			return TEXT("Magic Summon");
	case ECameraStateSubCategory::Skill:				return TEXT("Skill");
	
	// Multiplayer related
	case ECameraStateSubCategory::MultiplayerSummon:	return TEXT("Multiplayer Summon");
	case ECameraStateSubCategory::PvP:					return TEXT("PvP");
	case ECameraStateSubCategory::Coop:					return TEXT("Coop");
	
	// UI related
	case ECameraStateSubCategory::Menu:					return TEXT("Menu");
	case ECameraStateSubCategory::PhotoMode:			return TEXT("Photo Mode");
	
	// Modifier related
	case ECameraStateSubCategory::Core:					return TEXT("Core");
	
	case ECameraStateSubCategory::None:
	default:											return TEXT("None");
	}
}

FString UCameraStateHelpers::GetReferenceDisplayName(ECameraStateReference Reference)
{
	switch (Reference)
	{
	case ECameraStateReference::All:		return TEXT("All Games");
	case ECameraStateReference::EldenRing:	return TEXT("Elden Ring");
	case ECameraStateReference::Bloodborne:	return TEXT("Bloodborne");
	case ECameraStateReference::DarkSouls:	return TEXT("Dark Souls");
	case ECameraStateReference::DarkSouls3:	return TEXT("Dark Souls 3");
	case ECameraStateReference::Sekiro:		return TEXT("Sekiro");
	case ECameraStateReference::LiesOfP:	return TEXT("Lies of P");
	case ECameraStateReference::Some:		return TEXT("Some Games");
	case ECameraStateReference::Few:		return TEXT("Few Games");
	case ECameraStateReference::None:
	default:								return TEXT("None");
	}
}
