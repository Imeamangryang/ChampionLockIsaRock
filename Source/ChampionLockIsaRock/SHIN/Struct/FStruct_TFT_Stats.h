#pragma once

#include "CoreMinimal.h"
#include "FStruct_TFT_Stats.generated.h"

USTRUCT(BlueprintType)
struct FStruct_TFT_Stats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 공격력
	int32 AttackDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 주문력
	int32 AbilityPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 사거리
	int32 AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 공격속도
	float AttackSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 치명타 확률
	float CriticalChance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// DPS = AttackDamage * AttackSpeed (쓰지 않는 변수지만 편의상 넣어둠)
	int32 DPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 체력
	int32 Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 방어력
	int32 Armor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 마법 저항력
	int32 MagicResist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 시작 마나
	int32 StartingMana;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)	// 최대 마나
	int32 MaxMana;
};