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
}

void ATFT_HUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetOwningPlayerController();
	
	StatUIInstance = CreateWidget<UUserWidget>(PC, StatUIClass);
	StatUIInstance->AddToViewport();
	
	ShopUIInstance = CreateWidget<UUserWidget>(PC, ShopClass);
	ShopUIInstance->AddToViewport();
}
