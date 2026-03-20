// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "../SHIN/Character/TFT_UnitCharacter.h"
#include "TopDownController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class CHAMPIONLOCKISAROCK_API ATopDownController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATopDownController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Grabbed;
	
	// 1. 지금 집어든 기물이 있는가?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	bool bIsHoldingUnit = false;
	
	// 2. 어떤 기물을 들고 있는가?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	ATFT_UnitCharacter* GrabbedUnit = nullptr;
	
	// 3. 원래 있던 위치 (스왑용)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
	FVector OriginalLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Grid")
	class AGirdManager* CachedGridManager; // 그리드 매니저를 담아둘 변수
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	void OnGrabPressed(const FInputActionValue& Value);
	void OnDropReleased(const FInputActionValue& Value);
	/** 오른쪽 클릭 핸들러 */
	void OnMoveInputReleased();
	
	// 클릭을 시작한 시간을 기록합니다.
	float GrabStartTime = 0.0f;

	// 기물을 실제로 내려놓는 공통 로직을 별도 함수로 뺍니다.
	void PerformDrop();
	
	
	virtual void Tick(float DeltaTime) override;

	// ... 클래스 내부 (protected)
	/** 오른쪽 클릭 시 나올 파티클 효과 (선택사항) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UNiagaraSystem> FXCursor;

	/** 이동용 입력 액션 (오른쪽 클릭) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 목적지 저장용 */
	FVector CachedDestination;
	
};
