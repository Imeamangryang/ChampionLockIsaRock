// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TopDownController.generated.h"

// 전방 선언 (헤더 포함을 최소화하여 컴파일 속도를 높이고 꼬임을 방지)
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class ATFT_UnitCharacter;
class AGirdManager;
class ABenchManager;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATopDownController : public APlayerController
{
    GENERATED_BODY()
    
public:
    ATopDownController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;

// =========================================================================
// 1. 입력 (Input) 및 조작 관련
// =========================================================================
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_Input")
    UInputAction* IA_Grabbed; // 좌클릭 (기물 집기/놓기)
    
    UPROPERTY(EditAnywhere, Category = "TFT_Input")
    UInputAction* MoveAction; // 우클릭 (전설이 이동)
    
    UPROPERTY()
    AActor* BoardCameraActor;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TFT_Input")
    class UInputAction* ZoomAction;
    //  카메라 수치 
    // location -70.0, 1200, 1300
    // rotation     0, -60, 90
    
    void OnGrabPressed(const FInputActionValue& Value);
    void OnDropReleased(const FInputActionValue& Value);
    void OnMoveInputReleased();
    void OnZoom(const struct FInputActionValue& Value);
    

// =========================================================================
// 2. 기물 조작 (Drag & Drop) 상태 관리
// =========================================================================
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
    bool bIsHoldingUnit = false; // 현재 기물을 들고 있는지 여부
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
    ATFT_UnitCharacter* GrabbedUnit = nullptr; // 현재 들고 있는 기물
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Interaction")
    FVector OriginalLocation; // 기물을 들기 전 원래 위치 (스왑/복귀용)

protected:
    float GrabStartTime = 0.0f; // 짧은 클릭 vs 꾹 누르기 판정용 시간 기록

    void PerformDrop(); // 실제 좌표 계산 및 기물 내려놓기 로직

// =========================================================================
// 3. 맵 매니저 (Grid & Bench) 캐싱
// =========================================================================
public:
    // 맵에 배치된 그리드 매니저
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Managers")
    AGirdManager* CachedGridManager; 
    
protected:
    // 맵에 배치된 대기석 매니저
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT_Managers")
    ABenchManager* CachedBenchManager;

// =========================================================================
// 4. 데이터 및 상태 추적 (명단 관리)
// =========================================================================
protected:
    // 대기석에 있는 기물 목록 (전투 제외 대상)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT_Data")
    TArray<ATFT_UnitCharacter*> BenchUnits;

    // 맵에 배치된 모든 기물의 초기 위치 (전투 종료 후 복귀용)
    UPROPERTY()
    TMap<ATFT_UnitCharacter*, FTransform> UnitHomeRegistry;

    FVector CachedDestination; // 전설이 우클릭 이동 목적지

// =========================================================================
// 5. UI 및 FX
// =========================================================================
protected:
    // 스냅될 위치를 미리 보여주는 하이라이트
    UPROPERTY()
    AActor* HighlightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    UStaticMesh* GridHighlightMesh; // 보드판용 육각형 메쉬

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    UStaticMesh* BenchHighlightMesh; // 대기석용 사각형 메쉬
    
    UPROPERTY(EditAnywhere, Category = "TFT_FX")
    UNiagaraSystem* FXCursor; // 우클릭 시 생성될 바닥 이펙트

    void UpdateHighlightPosition(FVector MouseLoc); // 매 프레임 하이라이트 위치 갱신

// =========================================================================
// 6. 외부 호출용 (Blueprint 등) 퍼블릭 함수
// =========================================================================
public:
    // 시작 시 맵에 깔린 기물 위치를 UnitHomeRegistry에 저장
    void SaveInitialLocations(); 

    // 모든 기물을 Original 위치(주소록 기반)로 복귀시킴
    UFUNCTION(BlueprintCallable, Category = "TFT_Combat")
    void ReturnAllUnitsToHome(); 
    
    // 특정 기물이 대기석 명단에 있는지 판별
    UFUNCTION(BlueprintPure, Category = "TFT_Bench")
    bool IsUnitOnBench(AActor* Unit) const; 
};