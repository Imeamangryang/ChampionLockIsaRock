#include "TFT_ItemInventoryComponent.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

UTFT_ItemInventoryComponent::UTFT_ItemInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFT_ItemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	NotifyInventoryUpdated();
}

void UTFT_ItemInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UTFT_ItemInventoryComponent::AddItem(const FStruct_TFTItemInstance& NewItem)
{
	if (NewItem.ItemId.IsNone())
	{
		return false;
	}

	InventoryItems.Add(NewItem);
	NotifyInventoryUpdated();
	return true;
}

bool UTFT_ItemInventoryComponent::RemoveItemByInstanceId(const FGuid& InstanceId)
{
	if (!InstanceId.IsValid())
	{
		return false;
	}

	for (int32 i = 0; i < InventoryItems.Num(); ++i)
	{
		if (InventoryItems[i].InstanceId == InstanceId)
		{
			InventoryItems.RemoveAt(i);
			NotifyInventoryUpdated();
			return true;
		}
	}

	return false;
}

bool UTFT_ItemInventoryComponent::FindItemByInstanceId(const FGuid& InstanceId, FStruct_TFTItemInstance& OutItem) const
{
	if (!InstanceId.IsValid())
	{
		return false;
	}

	for (const FStruct_TFTItemInstance& Item : InventoryItems)
	{
		if (Item.InstanceId == InstanceId)
		{
			OutItem = Item;
			return true;
		}
	}

	return false;
}

const TArray<FStruct_TFTItemInstance>& UTFT_ItemInventoryComponent::GetAllItems() const
{
	return InventoryItems;
}

void UTFT_ItemInventoryComponent::ClearInventory()
{
	InventoryItems.Empty();
	NotifyInventoryUpdated();
}

void UTFT_ItemInventoryComponent::NotifyInventoryUpdated()
{
	if (UTFT_UISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->BroadcastItemInventoryUpdated(InventoryItems);
	}
}

UTFT_UISubsystem* UTFT_ItemInventoryComponent::GetUISubsystem() const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerActor);
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

	return LP->GetSubsystem<UTFT_UISubsystem>();
}