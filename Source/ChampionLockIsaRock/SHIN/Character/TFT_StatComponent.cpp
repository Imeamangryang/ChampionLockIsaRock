#include "TFT_StatComponent.h"
#include "../Struct/FTFT_ChampionData.h"
#include "../Struct/FStruct_TFT_Champion.h"

UTFT_StatComponent::UTFT_StatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTFT_StatComponent::Initialize(const FStruct_TFT_Champion& Data, int32 StarLevel)
{
	float Multiplier = 1.0f;

	switch (StarLevel)
	{
	case 2: Multiplier = 1.8f; break;
	case 3: Multiplier = 1.8f * 1.8f; break;
	default: Multiplier = 1.0f; break;
	}
	AttackDamage = FMath::RoundToInt(Data.Stats.AttackDamage * Multiplier);
	AbilityPower = Data.Stats.AbilityPower;
	AttackRange = Data.Stats.AttackRange;
	AttackSpeed = Data.Stats.AttackSpeed;
	CriticalChance = Data.Stats.CriticalChance;
	DPS = FMath::RoundToInt(AttackDamage * AttackSpeed);
	Health = FMath::RoundToInt(Data.Stats.Health * Multiplier);
	Armor = Data.Stats.Armor;
	MagicResist = Data.Stats.MagicResist;
	StartingMana = Data.Stats.StartingMana;
	MaxMana = Data.Stats.MaxMana;
}

void UTFT_StatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTFT_StatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

