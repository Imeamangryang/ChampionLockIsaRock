#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemDragDropOperation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDragScreenPositionUpdated, FVector2D, ScreenPosition);

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_ItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="TFT|Item")
	FStruct_TFTItemInstance DraggedItemInstance;
	
	UPROPERTY(BlueprintAssignable, Category="TFT|Item")
	FOnItemDragScreenPositionUpdated OnDragScreenPositionUpdated;

	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
};