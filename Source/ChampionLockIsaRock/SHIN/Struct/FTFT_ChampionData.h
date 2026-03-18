#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FTFT_ChampionData.generated.h"

USTRUCT()
struct FTFT_ChampionData : public FTableRowBase
{
	GENERATED_BODY()
	
	// ===== Basic Info =====

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Cost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Origins; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Classes;

	// ===== Stats =====

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AbilityPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Armor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MagicResist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartingMana;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxMana;

	// ===== Skill =====

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SkillStats; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SkillImage;
	
};
