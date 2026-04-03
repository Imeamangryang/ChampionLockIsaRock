#include "TFT_ItemWidget.h"
#include "TFT_ItemDragDropOperation.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "SHIN/GameFramework/TFT_GameInstance.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"
#include "Dong/Public/TopDownController.h"

void UTFT_ItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshItemVisual();
	SetPadding(FMargin(0.f));
}

void UTFT_ItemWidget::SetItemInstance(const FStruct_TFTItemInstance& InItemInstance)
{
	ItemInstance = InItemInstance;
	RefreshItemVisual();
}

void UTFT_ItemWidget::RefreshItemVisual()
{
	if (!ItemIcon)
	{
		return;
	}

	UTexture2D* FoundIcon = nullptr;

	if (UWorld* World = GetWorld())
	{
		if (UTFT_GameInstance* GI = World->GetGameInstance<UTFT_GameInstance>())
		{
			if (UDataTable* ItemTable = GI->ItemDataTable.LoadSynchronous())
			{
				if (const FStruct_TFTItemDefinition* ItemDef = ItemTable->FindRow<FStruct_TFTItemDefinition>(ItemInstance.ItemId, TEXT("RefreshItemVisual")))
				{
					FoundIcon = ItemDef->Icon;
				}
			}
		}
	}

	if (FoundIcon)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(FoundIcon);
		Brush.ImageSize = FVector2D(64.f, 64.f);
		ItemIcon->SetBrush(Brush);
	}
	
	else
	{
		ItemIcon->SetBrush(FSlateBrush());
	}
}

FReply UTFT_ItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UTFT_ItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UTFT_ItemDragDropOperation* DragOp = NewObject<UTFT_ItemDragDropOperation>();
	if (!DragOp)
	{
		return;
	}

	DragOp->DraggedItemInstance = ItemInstance;

	UTFT_ItemWidget* DragVisualWidget = CreateWidget<UTFT_ItemWidget>(GetWorld(), GetClass());
	if (DragVisualWidget)
	{
		DragVisualWidget->SetItemInstance(ItemInstance);
		DragOp->DefaultDragVisual = DragVisualWidget;
	}

	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOp;

	if (ATopDownController* TDController = Cast<ATopDownController>(GetOwningPlayer()))
	{
		TDController->BeginItemDrag(ItemInstance, DragOp);
	}
}