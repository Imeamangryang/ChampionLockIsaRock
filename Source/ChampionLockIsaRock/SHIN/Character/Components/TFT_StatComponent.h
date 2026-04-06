#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHIN/Struct/BaseModifier.h"
#include "TFT_StatComponent.generated.h"

struct FStruct_TFT_Champion;

USTRUCT(BlueprintType)
struct FTFTDamageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadOnly)
	int32 InputDamage = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ModifiedDamage = 0; // 크리티컬 적용 후, 방어력 적용 전

	UPROPERTY(BlueprintReadOnly)
	int32 FinalDamage = 0; // 방어력 적용 후 실제 피해량

	UPROPERTY(BlueprintReadOnly)
	bool bKilledTarget = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_StatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_StatComponent();
	
	void Initialize(const FStruct_TFT_Champion& Data, int32 StarLevel);
	
	FTFTDamageResult ApplyDamage(int32 Damage, class ATFT_UnitCharacter* Attacker = nullptr);
	void AddMana(int32 ManaAmount);

	UFUNCTION(BlueprintCallable, Category="TFT|Stats")
	void AddModifier(const FBaseModifier& Modifier);

	UFUNCTION(BlueprintCallable, Category="TFT|Stats")
	void AddModifiers(const TArray<FBaseModifier>& Modifiers);

	UFUNCTION(BlueprintCallable, Category="TFT|Stats")
	void RemoveModifiersBySource(UObject* Source);

	UFUNCTION(BlueprintCallable, Category="TFT|Stats")
	void RecalculateFinalStats();

	static float EvaluateModifiers(float BaseValue, const TArray<FBaseModifier>& Modifiers);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ===== Base Stats =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseAttackDamage = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseAbilityPower = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseAttackRange = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	float BaseAttackSpeed = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	float BaseCriticalChance = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseHealth = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseArmor = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseMagicResist = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseStartingMana = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	int32 BaseMaxMana = 0;

	// ===== Final Stats =====
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
	int32 MaxHealth;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Armor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MagicResist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartingMana;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxMana;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Stats")
	TArray<FBaseModifier> ActiveModifiers;
};