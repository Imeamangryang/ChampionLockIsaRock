#pragma once

#include "CoreMinimal.h"
#include "TFT_ItemTypes.generated.h"

UENUM(BlueprintType)
enum class ETFTItemType : uint8
{
	Component	UMETA(DisplayName = "Component"),
	Complete	UMETA(DisplayName = "Complete"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Special		UMETA(DisplayName = "Special")
};

UENUM(BlueprintType)
enum class ETFTItemEquipResult : uint8
{
	Success				UMETA(DisplayName = "Success"),
	InvalidItem			UMETA(DisplayName = "Invalid Item"),
	InvalidTarget		UMETA(DisplayName = "Invalid Target"),
	ItemCannotEquip		UMETA(DisplayName = "Item Cannot Equip"),
	NoEmptySlot			UMETA(DisplayName = "No Empty Slot"),
	DuplicateRestricted UMETA(DisplayName = "Duplicate Restricted")
};

USTRUCT(BlueprintType)
struct FStruct_TFTItemStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	int32 AttackDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	int32 AbilityPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	float AttackSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	float CriticalChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	int32 Health = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	int32 Armor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Stat")
	int32 MagicResist = 0;
};

USTRUCT(BlueprintType)
struct FStruct_TFTItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	ETFTItemType ItemType = ETFTItemType::Component;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bCanEquipToUnit = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FStruct_TFTItemStatBonus StatBonus;
};

USTRUCT(BlueprintType)
struct FStruct_TFTItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid InstanceId;

	FStruct_TFTItemInstance()
	{
		InstanceId = FGuid::NewGuid();
	}
};

USTRUCT(BlueprintType)
struct FStruct_TFTItemEquipResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	ETFTItemEquipResult Result = ETFTItemEquipResult::InvalidTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 EquippedSlotIndex = -1;
};

USTRUCT(BlueprintType)
struct FStruct_TFTEquippedItemSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FStruct_TFTItemInstance ItemInstance;
};