#include "TFT_StatComponent.h"
#include "../../Struct/FTFT_ChampionData.h"
#include "../../Struct/FStruct_TFT_Champion.h"
#include "TFT_CombatComponent.h"
#include "../TFT_UnitCharacter.h"

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
	MaxHealth = Health;
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

void UTFT_StatComponent::ApplyDamage(int32 Damage)
{
	// 데미지 계산 공식 FinalDamage = RawDamage * (100 / (100 + Armor))
	float DamageMultiplier = 100.f / (100.f + Armor);
	int32 FinalDamage = FMath::RoundToInt(Damage * DamageMultiplier);
	
	Health -= FinalDamage;
	if (Health < 0) Health = 0;
	
	// HP Bar Widget 업데이트
	if (ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner()))
	{
		OwnerCharacter->UpdateHPBarWidget();
	}
	
	
	// DeadState 진입
	if (Health == 0)
	{
		ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner());
		if (OwnerCharacter && OwnerCharacter->CombatComponent && OwnerCharacter->CombatComponent->CurrentState != ECombatState::Dead)
		{
			OwnerCharacter->CombatComponent->ChangeState(OwnerCharacter->CombatComponent->GetDeadState(),ECombatState::Dead);
		}
	}
}

void UTFT_StatComponent::AddMana(int32 ManaAmount)
{
	StartingMana += ManaAmount;
	if (StartingMana > MaxMana) StartingMana = MaxMana;
	
	if (ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner()))
	{
		OwnerCharacter->UpdateMPBarWidget();
	}
}


