#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHIN/Struct/FTFT_ChampionData.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "TFTPieceSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

UCLASS()
class CHAMPIONLOCKISAROCK_API UTFTPieceSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
	void SetChampionData(const FTFT_ChampionData& Data);

	UFUNCTION(BlueprintCallable, Category = "TFT|Shop")
	void ClearSlot();

	// 상점에서 가격을 물어볼 때 사용할 함수
	int32 GetPieceCost() const { return CurrentCost; }

protected:
	// 에디터에서 사운드를 고를 수 있게 하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT|Sound")
	FSlateSound ClickSlateSound;
    
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "TFT|UI")
	TObjectPtr<UButton> Button;

	UFUNCTION(BlueprintImplementableEvent, Category = "TFT|Shop")
	void UpdateButtonStyleByCost(int32 Cost);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_ChampionName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Origins;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Classes;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Cost;

private:
	ETFT_ChampionKey CurrentChampionKey;
    
	// [중요] 에러 해결을 위해 명시적으로 두 변수를 모두 선언합니다.
	int32 CurrentCost = 0; 
	FTFT_ChampionData CurrentChampionData;

	UFUNCTION()
	void OnSlotClicked();
};