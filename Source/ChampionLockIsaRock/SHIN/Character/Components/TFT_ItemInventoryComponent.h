#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemInventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHAMPIONLOCKISAROCK_API UTFT_ItemInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTFT_ItemInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	// 현재 사이드 인벤토리에 들어있는 아이템 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TFT Inventory")
	TArray<FStruct_TFTItemInstance> InventoryItems;
	
	// 아이템 추가
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	bool AddItem(const FStruct_TFTItemInstance& NewItem);

	// InstanceId 기준으로 아이템 제거
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	bool RemoveItemByInstanceId(const FGuid& InstanceId);

	// InstanceId 기준으로 아이템 찾기
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	bool FindItemByInstanceId(const FGuid& InstanceId, FStruct_TFTItemInstance& OutItem) const;

	// 현재 전체 인벤토리 반환
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	const TArray<FStruct_TFTItemInstance>& GetAllItems() const;

	// 전체 비우기
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	void ClearInventory();

	// UI 강제 갱신
	UFUNCTION(BlueprintCallable, Category = "TFT Inventory")
	void NotifyInventoryUpdated();
	
	class UTFT_UISubsystem* GetUISubsystem() const;
};
