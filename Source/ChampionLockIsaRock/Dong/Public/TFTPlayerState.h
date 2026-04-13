// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFTPlayerState.generated.h"

class UTFT_UISubsystem;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFTPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ATFTPlayerState();
	
	// --- 경제 및 레벨 데이터 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Economy")
	int32 PlayerGold;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Level")
	int32 PlayerLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Level")
	int32 CurrentXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Level")
	int32 MaxXP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundBase* SoundLevelUp;
	
	// --- 외부 호출용 함수 ---
	UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
	bool SpendGold(int32 Amount); // 돈을 쓸 수 있는지 체크 후 소모

	UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
	void AddXP(int32 Amount);

	// 레벨업 버튼용 (4골드 소모 -> 4 XP 획득)
	UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
	void BuyXP();

	// 스테이지 종료 시 호출 (자동 2 XP 획득)
	UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
	void AddStageEndXP();
	
	// 다음 레벨에 필요한 경험치 통을 계산하는 내부 함수
	void UpdateMaxXP();
	
private:
	UTFT_UISubsystem* GetUISubsystem() const;
};
