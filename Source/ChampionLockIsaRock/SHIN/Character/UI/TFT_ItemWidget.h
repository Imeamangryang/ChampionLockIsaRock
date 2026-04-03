#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemWidget.generated.h"

class UImage;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_ItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	
public:
	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	void SetItemInstance(const FStruct_TFTItemInstance& InItemInstance);

	UFUNCTION(BlueprintCallable, Category="TFT|Item")
	void RefreshItemVisual();
	
	UPROPERTY(BlueprintReadOnly, Category="TFT|Item")
	FStruct_TFTItemInstance ItemInstance;
	
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* ItemIcon;
};
