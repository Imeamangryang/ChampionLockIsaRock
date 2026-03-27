#pragma once
#include "CoreMinimal.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "TFT_UISubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatUIOpen, bool, bIsOpen, FStruct_TFT_Champion, ChampionData, ATFT_UnitCharacter*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHPUpdated, ATFT_UnitCharacter*, Unit, float, MaxHP, float, CurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMPUpdated, ATFT_UnitCharacter*, Unit, float, MaxMP, float, CurrentMP);

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_UISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	// ======== Delegate ==========
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnStatUIOpen OnStatUIOpen;
	
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnHPUpdated OnHPUpdated;
	
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnMPUpdated OnMPUpdated;
	
	// ======== BroadCast ==========
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastStatUIOpen(bool bIsOpen, FStruct_TFT_Champion championdata, ATFT_UnitCharacter* Unit);
	
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastHPUpdate(ATFT_UnitCharacter* Unit, float MaxHP, float CurrentHP);
	
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastMPUpdate(ATFT_UnitCharacter* Unit, float MaxMP, float CurrentMP);
};
