#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemInventoryPanel.generated.h"

class UBorder;
class UTFT_ItemWidget;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_ItemInventoryPanel : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
public: 
	UTFT_ItemInventoryPanel(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category="TFT|Inventory")
	void RefreshInventory(const TArray<FStruct_TFTItemInstance>& InItems);
	
	UFUNCTION()
	void HandleInventoryUpdated(const TArray<FStruct_TFTItemInstance>& InventoryItems);
	
	UBorder* GetSlotByIndex(int32 Index) const;
	void ClearAllSlots();
	class UTFT_UISubsystem* GetUISubsystem() const;
	
	// 생성할 아이템 위젯 클래스 (WBP_Item)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|Inventory")
	TSubclassOf<UTFT_ItemWidget> ItemWidgetClass;
	
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_1;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_2;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_3;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_4;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_5;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_6;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_7;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_8;
	UPROPERTY(meta=(BindWidgetOptional)) UBorder* ItemSlot_9;
};
