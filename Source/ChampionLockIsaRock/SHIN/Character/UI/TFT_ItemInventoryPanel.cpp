#include "TFT_ItemInventoryPanel.h"
#include "Components/Border.h"
#include "SHIN/Character/UI/TFT_ItemWidget.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"

void UTFT_ItemInventoryPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	ClearAllSlots();
	
	if (UTFT_UISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->OnItemInventoryUpdated.RemoveDynamic(this, &UTFT_ItemInventoryPanel::HandleInventoryUpdated);
		UISubsystem->OnItemInventoryUpdated.AddDynamic(this, &UTFT_ItemInventoryPanel::HandleInventoryUpdated);
	}
	
	// // Test Code
	TArray<FStruct_TFTItemInstance> TestItems;
	FStruct_TFTItemInstance Item1;
	Item1.ItemId = TEXT("LordsEdge");
	TestItems.Add(Item1);
	
	RefreshInventory(TestItems);
}

UTFT_ItemInventoryPanel::UTFT_ItemInventoryPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	if (!ItemWidgetClass)
	{
		static ConstructorHelpers::FClassFinder<UTFT_ItemWidget> ItemWidgetBPClass(
			TEXT("/Game/SHIN/UI/Blueprints/WBP_Item.WBP_Item_C")
		);

		if (ItemWidgetBPClass.Succeeded())
		{
			ItemWidgetClass = ItemWidgetBPClass.Class;
		}
	}
}

void UTFT_ItemInventoryPanel::RefreshInventory(const TArray<FStruct_TFTItemInstance>& InItems)
{
	ClearAllSlots();

	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTFT_ItemInventoryPanel::RefreshInventory - ItemWidgetClass is not set."));
		return;
	}

	const int32 MaxSlotCount = 10;
	const int32 ItemCountToShow = FMath::Min(InItems.Num(), MaxSlotCount);

	for (int32 i = 0; i < ItemCountToShow; ++i)
	{
		UBorder* TargetSlot = GetSlotByIndex(i);
		if (!TargetSlot)
		{
			continue;
		}

		UTFT_ItemWidget* ItemWidget = CreateWidget<UTFT_ItemWidget>(GetWorld(), ItemWidgetClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->SetItemInstance(InItems[i]);
		TargetSlot->SetContent(ItemWidget);
	}
}

UBorder* UTFT_ItemInventoryPanel::GetSlotByIndex(int32 Index) const
{
	switch (Index)
	{
	case 0: return ItemSlot;
	case 1: return ItemSlot_1;
	case 2: return ItemSlot_2;
	case 3: return ItemSlot_3;
	case 4: return ItemSlot_4;
	case 5: return ItemSlot_5;
	case 6: return ItemSlot_6;
	case 7: return ItemSlot_7;
	case 8: return ItemSlot_8;
	case 9: return ItemSlot_9;
	default: return nullptr;
	}
}

void UTFT_ItemInventoryPanel::ClearAllSlots()
{
	for (int32 i = 0; i < 10; ++i)
	{
		if (UBorder* CurrentSlot = GetSlotByIndex(i))
		{
			CurrentSlot->SetContent(nullptr);
		}
	}
}

UTFT_UISubsystem* UTFT_ItemInventoryPanel::GetUISubsystem() const
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		return LP->GetSubsystem<UTFT_UISubsystem>();
	}

	return nullptr;
}

void UTFT_ItemInventoryPanel::HandleInventoryUpdated(const TArray<FStruct_TFTItemInstance>& InventoryItems)
{
	RefreshInventory(InventoryItems);
}