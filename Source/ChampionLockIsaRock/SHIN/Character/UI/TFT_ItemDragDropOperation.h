#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemDragDropOperation.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFT_ItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="TFT|Item")
	FStruct_TFTItemInstance DraggedItemInstance;
};