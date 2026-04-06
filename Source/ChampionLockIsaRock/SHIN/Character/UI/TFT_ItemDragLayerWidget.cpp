#include "TFT_ItemDragLayerWidget.h"
#include "Dong/Public/TopDownController.h"
#include "SHIN/Character/UI/TFT_ItemDragDropOperation.h"
#include "SHIN/Character/UI/TFT_ItemWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTFT_ItemDragLayerWidget::SetOwningController(ATopDownController* InController)
{
	OwningController = InController;
}

bool UTFT_ItemDragLayerWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	if (UTFT_ItemDragDropOperation* DragOp = Cast<UTFT_ItemDragDropOperation>(InOperation))
	{
		if (DragOp->SourceItemWidget)
		{
			DragOp->SourceItemWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (OwningController)
	{
		OwningController->EndItemDrag(true);
	}

	return true;
}