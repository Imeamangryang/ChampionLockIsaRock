#include "TFT_StatComponent.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"
#include "TFT_CombatComponent.h"
#include "SHIN/Character/TFT_UnitCharacter.h"

UTFT_StatComponent::UTFT_StatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

float UTFT_StatComponent::EvaluateModifiers(float BaseValue, const TArray<FBaseModifier>& Modifiers)
{
	float AddSum = 0.0f;
	float MulProduct = 1.0f;
	TOptional<float> OverrideValue;

	for (const FBaseModifier& Modifier : Modifiers)
	{
		switch (Modifier.Operation)
		{
		case EModifierOperation::Add:
			AddSum += Modifier.Value;
			break;

		case EModifierOperation::Multiply:
			MulProduct *= (1.0f + Modifier.Value);
			break;

		case EModifierOperation::Override:
			OverrideValue = Modifier.Value;
			break;

		default:
			break;
		}
	}

	if (OverrideValue.IsSet())
	{
		return OverrideValue.GetValue();
	}

	return (BaseValue + AddSum) * MulProduct;
}

void UTFT_StatComponent::Initialize(const FStruct_TFT_Champion& Data, int32 StarLevel)
{
	float Multiplier;

	switch (StarLevel)
	{
	case 2: Multiplier = 1.8f; break;
	case 3: Multiplier = 1.8f * 1.8f; break;
	default: Multiplier = 1.0f; break;
	}

	BaseAttackDamage = FMath::RoundToInt(Data.Stats.AttackDamage * Multiplier);
	BaseAbilityPower = Data.Stats.AbilityPower;
	BaseAttackRange = Data.Stats.AttackRange;
	BaseAttackSpeed = Data.Stats.AttackSpeed;
	BaseCriticalChance = Data.Stats.CriticalChance;
	BaseHealth = FMath::RoundToInt(Data.Stats.Health * Multiplier);
	BaseArmor = Data.Stats.Armor;
	BaseMagicResist = Data.Stats.MagicResist;
	BaseStartingMana = Data.Stats.StartingMana;
	BaseMaxMana = Data.Stats.MaxMana;

	RecalculateFinalStats();

	Health = MaxHealth;
	StartingMana = FMath::Clamp(StartingMana, 0, MaxMana);
}

FTFTDamageResult UTFT_StatComponent::ApplyDamage(int32 Damage, ATFT_UnitCharacter* Attacker)
{
	FTFTDamageResult Result;
	Result.InputDamage = Damage;
	Result.ModifiedDamage = Damage;

	if (Damage <= 0)
	{
		return Result;
	}

	if (Attacker && Attacker->StatComponent)
	{
		float CritChance = Attacker->StatComponent->CriticalChance;

		// 25 같은 값이 들어오면 0.25로 보정
		if (CritChance > 1.0f)
		{
			CritChance *= 0.01f;
		}

		if (FMath::FRand() <= CritChance)
		{
			Result.bIsCritical = true;
			Result.ModifiedDamage = FMath::RoundToInt((float)Result.ModifiedDamage * 1.5f);
		}
	}

	const float DamageMultiplier = 100.f / (100.f + Armor);
	Result.FinalDamage = FMath::RoundToInt((float)Result.ModifiedDamage * DamageMultiplier);
	
	Health -= Result.FinalDamage;
	if (Health < 0)
	{
		Health = 0;
	}

	Result.bKilledTarget = (Health == 0);
	
	if (ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner()))
	{
		OwnerCharacter->UpdateHPBarWidget();
	}
	
	if (Result.bKilledTarget)
	{
		if (ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner()))
		{
			if (OwnerCharacter->CombatComponent && OwnerCharacter->CombatComponent->CurrentState != ECombatState::Dead)
			{
				OwnerCharacter->CombatComponent->ChangeState(OwnerCharacter->CombatComponent->GetDeadState(), ECombatState::Dead);
			}
		}
	}

	return Result;
}

void UTFT_StatComponent::AddModifier(const FBaseModifier& Modifier)
{
	ActiveModifiers.Add(Modifier);
	RecalculateFinalStats();
}

void UTFT_StatComponent::AddModifiers(const TArray<FBaseModifier>& Modifiers)
{
	ActiveModifiers.Append(Modifiers);
	RecalculateFinalStats();
}

void UTFT_StatComponent::RemoveModifiersBySource(UObject* Source)
{
	if (!Source)
	{
		return;
	}

	ActiveModifiers.RemoveAll([Source](const FBaseModifier& Modifier)
	{
		return Modifier.Source.IsValid() && Modifier.Source.Get() == Source;
	});

	RecalculateFinalStats();
}

void UTFT_StatComponent::RecalculateFinalStats()
{
	auto GatherModifiers = [this](ETFTModifiedStat TargetStat)
	{
		TArray<FBaseModifier> Result;
		for (const FBaseModifier& Modifier : ActiveModifiers)
		{
			if (Modifier.TargetStat == TargetStat)
			{
				Result.Add(Modifier);
			}
		}
		return Result;
	};

	const int32 OldMaxHealth = MaxHealth;
	const int32 OldHealth = Health;
	const int32 OldMaxMana = MaxMana;
	const int32 OldMana = StartingMana;

	AttackDamage = FMath::RoundToInt(EvaluateModifiers((float)BaseAttackDamage, GatherModifiers(ETFTModifiedStat::AttackDamage)));
	AbilityPower = FMath::RoundToInt(EvaluateModifiers((float)BaseAbilityPower, GatherModifiers(ETFTModifiedStat::AbilityPower)));
	AttackRange = FMath::RoundToInt(EvaluateModifiers((float)BaseAttackRange, GatherModifiers(ETFTModifiedStat::AttackRange)));
	AttackSpeed = EvaluateModifiers(BaseAttackSpeed, GatherModifiers(ETFTModifiedStat::AttackSpeed));
	CriticalChance = EvaluateModifiers(BaseCriticalChance, GatherModifiers(ETFTModifiedStat::CriticalChance));
	MaxHealth = FMath::RoundToInt(EvaluateModifiers((float)BaseHealth, GatherModifiers(ETFTModifiedStat::Health)));
	Armor = FMath::RoundToInt(EvaluateModifiers((float)BaseArmor, GatherModifiers(ETFTModifiedStat::Armor)));
	MagicResist = FMath::RoundToInt(EvaluateModifiers((float)BaseMagicResist, GatherModifiers(ETFTModifiedStat::MagicResist)));
	MaxMana = FMath::RoundToInt(EvaluateModifiers((float)BaseMaxMana, GatherModifiers(ETFTModifiedStat::MaxMana)));
	StartingMana = FMath::RoundToInt(EvaluateModifiers((float)BaseStartingMana, GatherModifiers(ETFTModifiedStat::StartingMana)));

	DPS = FMath::RoundToInt((float)AttackDamage * AttackSpeed);

	if (OldMaxHealth > 0)
	{
		const float HealthRatio = (float)OldHealth / (float)OldMaxHealth;
		Health = FMath::Clamp(FMath::RoundToInt((float)MaxHealth * HealthRatio), 0, MaxHealth);
	}
	else
	{
		Health = MaxHealth;
	}

	if (OldMaxMana > 0)
	{
		const float ManaRatio = (float)OldMana / (float)OldMaxMana;
		StartingMana = FMath::Clamp(FMath::RoundToInt((float)MaxMana * ManaRatio), 0, MaxMana);
	}
	else
	{
		StartingMana = FMath::Clamp(StartingMana, 0, MaxMana);
	}
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

void UTFT_StatComponent::AddMana(int32 ManaAmount)
{
	StartingMana += ManaAmount;
	if (StartingMana > MaxMana) StartingMana = MaxMana;
	
	if (ATFT_UnitCharacter* OwnerCharacter = Cast<ATFT_UnitCharacter>(GetOwner()))
	{
		OwnerCharacter->UpdateMPBarWidget();
	}
}



