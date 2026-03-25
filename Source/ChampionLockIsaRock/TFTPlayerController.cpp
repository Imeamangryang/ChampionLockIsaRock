// TFTPlayerController.cpp
#include "TFTPlayerController.h"
#include "Blueprint/UserWidget.h"

void ATFTPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ShopWidgetClass)
	{
		ShopWidget = CreateWidget<UUserWidget>(this, ShopWidgetClass);
		ShopWidget->AddToViewport();
	}
}
