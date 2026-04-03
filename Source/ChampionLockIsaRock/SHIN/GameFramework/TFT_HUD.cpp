#include "TFT_HUD.h"
#include "Blueprint/UserWidget.h"

ATFT_HUD::ATFT_HUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> StatUIBPClass(TEXT("/Game/SHIN/UI/Blueprints/WBP_UnitInfo.WBP_UnitInfo_C"));
	if (StatUIBPClass.Succeeded())
	{
		StatUIClass = StatUIBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget> ShopUIBPClass(TEXT("/Game/Doeun/Blueprints/WBP_ShopMain.WBP_ShopMain_C"));
	if (ShopUIBPClass.Succeeded())
	{
		ShopClass = ShopUIBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget> ItemInventoryBPClass(TEXT("/Game/SHIN/UI/Blueprints/WBP_ItemInventoryPanel.WBP_ItemInventoryPanel_C"));
	if (ItemInventoryBPClass.Succeeded())
	{
		InventoryClass = ItemInventoryBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget> StageTimerBPClass(TEXT("/Game/Dong/UI/WBP/WBP_TimeManager.WBP_TimeManager_C"));
	if (StageTimerBPClass.Succeeded())
	{
		StageTimerClass = StageTimerBPClass.Class;
	}
}

void ATFT_HUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetOwningPlayerController();
	
	StatUIInstance = CreateWidget<UUserWidget>(PC, StatUIClass);
	StatUIInstance->AddToViewport();
	
	ShopUIInstance = CreateWidget<UUserWidget>(PC, ShopClass);
	ShopUIInstance->AddToViewport();
	
	InventoryUIInstance = CreateWidget<UUserWidget>(PC, InventoryClass);
	InventoryUIInstance->AddToViewport();
	
	StageTimerUIInstance = CreateWidget<UUserWidget>(PC, StageTimerClass);
	StageTimerUIInstance->AddToViewport();
}
