// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "SHIN/Struct/ETFT_ChampionList.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TopDownController.generated.h"

// 전방 선언 (헤더 포함을 최소화하여 컴파일 속도를 높이고 꼬임을 방지)
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class ATFT_UnitCharacter;
class AGirdManager;
class ABenchManager;
class UTFT_ItemDragDropOperation;
class UTFT_ItemInventoryComponent;
class UTFT_ItemDragLayerWidget;
class UUserWidget;

DECLARE_MULTICAST_DELEGATE(FOnReturnAllUnitsHome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitCountChangedSignature, int32, CurrentUnits, int32, MaxUnits);

UCLASS()
class CHAMPIONLOCKISAROCK_API ATopDownController : public APlayerController
{
    GENERATED_BODY()
    
public:
    ATopDownController();
    FOnReturnAllUnitsHome OnReturnAllUnitsHome;
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
    // location -70.0, -780, 630
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
   
    // 대기석에 있는 기물 목록 (전투 제외 대상)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT_Data")
    TArray<ATFT_UnitCharacter*> BenchUnits;

protected:
    float GrabStartTime = 0.0f; // 짧은 클릭 vs 꾹 누르기 판정용 시간 기록

    

// =========================================================================
// 3. 맵 매니저 (Grid & Bench) 캐싱
// =========================================================================
public:
    // 맵에 배치된 그리드 매니저
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TFT_Managers")
    AGirdManager* CachedGridManager; 
    
    // 맵에 배치된 대기석 매니저
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT_Managers")
    ABenchManager* CachedBenchManager;
    // 맵에 배치된 모든 기물의 초기 위치 (전투 종료 후 복귀용)
    UPROPERTY()
    TMap<ATFT_UnitCharacter*, FTransform> UnitHomeRegistry;
protected:
    FVector CachedDestination; // 전설이 우클릭 이동 목적지

// =========================================================================
// 5. UI 및 FX
// =========================================================================
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
    class USoundBase* SoundUnitDrop;
    
    // 스냅될 위치를 미리 보여주는 하이라이트
    UPROPERTY()
    AActor* HighlightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    UStaticMesh* GridHighlightMesh; // 보드판용 육각형 메쉬

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    UStaticMesh* BenchHighlightMesh; // 대기석용 사각형 메쉬
    
    UPROPERTY(EditAnywhere, Category = "TFT_FX")
    UNiagaraSystem* FXCursor; // 우클릭 시 생성될 바닥 이펙트
    
    void PerformDrop(); // 실제 좌표 계산 및 기물 내려놓기 로직

    void UpdateHighlightPosition(FVector MouseLoc); // 매 프레임 하이라이트 위치 갱신

// =========================================================================
// 6. 외부 호출용 (Blueprint 등) 퍼블릭 함수
// =========================================================================
public:
    void ReturnUnitEquippedItemsToInventory(ATFT_UnitCharacter* Unit);
    
    UFUNCTION(Exec)
    void showmethemoney();
    
    // 시작 시 맵에 깔린 기물 위치를 UnitHomeRegistry에 저장
    void SaveInitialLocations(); 

    // 모든 기물을 Original 위치(주소록 기반)로 복귀시킴
    UFUNCTION(BlueprintCallable, Category = "TFT_Combat")
    void ReturnAllUnitsToHome(); 
    
    // 특정 기물이 대기석 명단에 있는지 판별
    UFUNCTION(BlueprintPure, Category = "TFT_Bench")
    bool IsUnitOnBench(AActor* Unit) const; 
    
    // 커스텀 이벤트 역할을 할 함수
    UFUNCTION(BlueprintCallable, Category = "TFT|Spawn")
    void SpawnUnitFromBP(ETFT_ChampionKey Key);
    
    // 대기석 하이라이트 크기 조절
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    FVector BenchHighlightScale = FVector(1.0f, 1.0f, 1.0f);
    
    // 대기석 하이라이트 위치 미세 조정용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    FVector BenchHighlightOffset = FVector(0.0f, 0.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    FRotator GridHighlightRotation = FRotator(0.0f, 0.0f, 30.0f); 

    // 붕 뜨는 느낌을 없애기 위한 Z축 높이 조절 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT_UI")
    float HighlightZOffset = 5.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TFT|Level")
    int32 CheckMyGold;
    
    UPROPERTY(BlueprintAssignable, Category = "TFT|Events")
    FOnUnitCountChangedSignature OnUnitCountChanged;
    
    // 전투 중 합체가 보류되었는지
    UPROPERTY(BlueprintReadWrite, Category = "TFT|Evolution")
    bool bHasPendingUpgrades = false;
    
    // 전투 중에 합체를 미뤄둔 챔피언들의 적어두는 메모장
    UPROPERTY()
    TArray<ETFT_ChampionKey> PendingUpgradeKeys;
    
    // 준비 시간이 되면 보류된 기물을 합치는 함수
    UFUNCTION()
    void ProcessPendingUpgrades();
    
    // 유닛 카운트
    UFUNCTION(BlueprintCallable, Category = "TFT|Logic")
    void BroadcastUnitCount();
    
    // 기물 판매
    void ExecuteSellUnit(class ATFT_UnitCharacter* UnitToSell);
    // 기물 판매 시 재생할 사운드 슬롯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TFT|Sounds")
    class USoundBase* SoundUnitSell;
    
protected:
    // 블루프린트의 'SpawnActor' 노드에 있는 'Class' 핀 역할
    // 에디터에서 BP_Unit을 여기에 넣어주면 됨
    UPROPERTY(EditAnywhere, Category = "TFT|Spawn")
    TSubclassOf<class ATFT_UnitCharacter> UnitClass;
    
    // 'E' 키를 담당할 입력 액션 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* IA_Sell;

    // 'E' 키가 눌렸을 때 실행될 함수
    void OnSellKeyPressed();
    // ========= 준섭이꺼 =========
public:
    
    // Inventory Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UTFT_ItemInventoryComponent* ItemInventoryComponent;

    UFUNCTION(BlueprintCallable)
    void BeginItemDrag(const FStruct_TFTItemInstance& DraggedItem, UTFT_ItemDragDropOperation* DragOperation);

    UFUNCTION(BlueprintCallable)
    void EndItemDrag(bool bDroppedSuccessfully);
    
    ATFT_UnitCharacter* FindItemDropTargetAtScreenPosition(const FVector2D& ScreenPosition) const;
    
    UFUNCTION()
    void HandleItemDragScreenPositionUpdated(FVector2D ScreenPosition);

protected:
    UPROPERTY()
    bool bIsDraggingItem = false;

    UPROPERTY()
    FStruct_TFTItemInstance CurrentDraggedItem;

    UPROPERTY()
    TObjectPtr<ATFT_UnitCharacter> CurrentItemDropTargetUnit = nullptr; 
    
    UPROPERTY(EditDefaultsOnly, Category="TFT|UI")
    TSubclassOf<UUserWidget> ItemDragLayerClass;

    UPROPERTY()
    UTFT_ItemDragLayerWidget* ItemDragLayerWidget = nullptr;
    
    UPROPERTY()
    FVector2D LastKnownItemDragScreenPosition = FVector2D::ZeroVector;
    
    // 치트키용 입력 액션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* IA_CheatUnits;

    // 5코 3성을 소환할 함수
    void OnCheatUnitsPressed();

    // 치트 전용 스폰 헬퍼 함수
    void SpawnCheatUnit(ETFT_ChampionKey Key, int32 StarLevel);
    
};
