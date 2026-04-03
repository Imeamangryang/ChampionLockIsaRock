#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFT_ItemDragLayerWidget.generated.h"

class ATopDownController;
class UDragDropOperation;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_ItemDragLayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOwningController(ATopDownController* InController);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY()
	ATopDownController* OwningController = nullptr;
};
