#include "TFT_ItemDragLayerWidget.h"
#include "Dong/Public/TopDownController.h"

void UTFT_ItemDragLayerWidget::SetOwningController(ATopDownController* InController)
{
	OwningController = InController;
}

bool UTFT_ItemDragLayerWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (OwningController)
	{
		OwningController->EndItemDrag(true);
	}

	return true;
}