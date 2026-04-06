#pragma once
#include "CoreMinimal.h"
#include "SHIN/Struct/FStruct_TFT_Champion.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Dong/Public/TFTPlayerState.h"
#include "TFT_UISubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUIOpen, bool, bIsOpen, ATFT_UnitCharacter*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHPUpdated, ATFT_UnitCharacter*, Unit, float, MaxHP, float, CurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMPUpdated, ATFT_UnitCharacter*, Unit, float, MaxMP, float, CurrentMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemInventoryUpdated, const TArray<FStruct_TFTItemInstance>&, InventoryItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLevelInfoUpdated, int32, NewLevel, int32, NewCurrentXP, int32, NewMaxXP);

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
	
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnItemInventoryUpdated OnItemInventoryUpdated;
	
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnGoldChanged OnGoldChanged;
	
	UPROPERTY(BlueprintAssignable, Category="TFT|UI")
	FOnLevelInfoUpdated OnLevelInfoUpdated;
	
	// ======== BroadCast ==========
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastStatUIOpen(bool bIsOpen,ATFT_UnitCharacter* Unit);
	
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastHPUpdate(ATFT_UnitCharacter* Unit, float MaxHP, float CurrentHP);
	
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastMPUpdate(ATFT_UnitCharacter* Unit, float MaxMP, float CurrentMP);
	
	UFUNCTION(BlueprintCallable, Category="TFT|UI")
	void BroadcastItemInventoryUpdated(const TArray<FStruct_TFTItemInstance>& InventoryItems);
	
	UFUNCTION(BlueprintCallable, Category="TFT|Economy")
	void BroadcastGoldUpdate(int32 NewGold);

	UFUNCTION(BlueprintCallable, Category="TFT|Level")
	void BroadcastLevelInfoUpdate(int32 NewLevel, int32 NewCurrentXP, int32 NewMaxXP);
};
 