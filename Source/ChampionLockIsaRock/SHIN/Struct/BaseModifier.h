#pragma once

#include "CoreMinimal.h"
#include "BaseModifier.generated.h"

UENUM(BlueprintType)
enum class EModifierOperation : uint8
{
	Add			UMETA(DisplayName = "Add"),
	Multiply	UMETA(DisplayName = "Multiply"),
	Override	UMETA(DisplayName = "Override")
};

UENUM(BlueprintType)
enum class ETFTModifiedStat : uint8
{
	AttackDamage,
	AbilityPower,
	AttackRange,
	AttackSpeed,
	CriticalChance,
	Health,
	Armor,
	MagicResist,
	StartingMana,
	MaxMana
};

USTRUCT(BlueprintType)
struct FBaseModifier
{
	GENERATED_BODY()

public:
	FBaseModifier()
		: TargetStat(ETFTModifiedStat::AttackDamage)
		, Operation(EModifierOperation::Add)
		, Value(0.0f)
		, Source(nullptr)
	{
	}

	FBaseModifier(ETFTModifiedStat InTargetStat, EModifierOperation InOperation, float InValue, UObject* InSource)
		: TargetStat(InTargetStat)
		, Operation(InOperation)
		, Value(InValue)
		, Source(InSource)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	ETFTModifiedStat TargetStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	EModifierOperation Operation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier")
	float Value;

	UPROPERTY()
	TWeakObjectPtr<UObject> Source;
};