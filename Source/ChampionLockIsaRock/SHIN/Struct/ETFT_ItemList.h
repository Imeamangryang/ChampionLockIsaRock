#pragma once

#include "CoreMinimal.h"
#include "ETFT_ItemList.generated.h"

UENUM(BlueprintType)
enum class ETFT_ItemKey : uint8
{
	LordsEdge UMETA(DisplayName = "Lord's Edge"),
	ArcaneGauntlet UMETA(DisplayName = "Arcane Gauntlet"),
	Bloodthirster UMETA(DisplayName = "Bloodthirster"),
	BrambleVest UMETA(DisplayName = "Bramble Vest"),
	DragonsClaw UMETA(DisplayName = "Dragon's Claw"),
	GiantSlayer UMETA(DisplayName = "Giant Slayer"),
	Guardbreaker UMETA(DisplayName = "Guardbreaker"),
	GuardianAngel UMETA(DisplayName = "Guardian Angel"),
	GuinsoosRageblade UMETA(DisplayName = "Guinsoos Rageblade"),
	HextechGunblade UMETA(DisplayName = "Hextech Gunblade"),
	IronWill UMETA(DisplayName = "Iron Will"),
	NashorsTooth UMETA(DisplayName = "Nashors Tooth"),
	Quicksilver	UMETA(DisplayName = "Quicksilver"),
	RabadonsDeathcap UMETA(DisplayName = "Rabadon Deathcap"),
	RedBuffItem UMETA(DisplayName = "Redbuff Item"),
	RunaansHurricane UMETA(DisplayName = "Runaans Hurricane"),
	SteadfastHeart UMETA(DisplayName = "Steadfast Heart"),
	TitansResolve UMETA(DisplayName = "Titan Resolve"),
	VoidStaff UMETA(DisplayName = "Void Staff"),
	WarmogsArmor UMETA(DisplayName = "Warming Armor"),
};