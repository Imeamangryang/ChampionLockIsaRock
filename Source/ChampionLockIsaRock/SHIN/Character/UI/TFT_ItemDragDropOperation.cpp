#include "TFT_ItemDragDropOperation.h"
#include "Kismet/GameplayStatics.h"
#include "Dong/Public/TopDownController.h"

void UTFT_ItemDragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Dragged_Implementation(PointerEvent);

	OnDragScreenPositionUpdated.Broadcast(PointerEvent.GetScreenSpacePosition());
}

void UTFT_ItemDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ATopDownController* TDController = Cast<ATopDownController>(PC))
		{
			TDController->EndItemDrag(true);
		}
	}
}

void UTFT_ItemDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ATopDownController* TDController = Cast<ATopDownController>(PC))
		{
			TDController->EndItemDrag(false);
		}
	}
}