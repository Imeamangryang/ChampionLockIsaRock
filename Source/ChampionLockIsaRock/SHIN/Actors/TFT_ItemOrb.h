#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SHIN/Struct/TFT_ItemTypes.h"
#include "TFT_ItemOrb.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_ItemOrb : public AActor
{
	GENERATED_BODY()
	
public:
	ATFT_ItemOrb();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPickupSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	void InitializeRandomItem();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* OrbMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Coin")
	UNiagaraComponent* OrbEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|Audio")
	class USoundBase* OrbSound = nullptr;

	// 어떤 아이템을 주는 오브인지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|ItemOrb")
	FName ItemId = NAME_None;

	// Amount가 필요한 구조면 같이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TFT|ItemOrb")
	int32 Amount = 1;

	// 중복 획득 방지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|ItemOrb")
	bool bCollected = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|ItemOrb|SpawnArc")
	float RandomOffsetRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|ItemOrb|SpawnArc")
	float ArcHeight = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|ItemOrb|SpawnArc")
	float MoveDuration = 1.0f;
	
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bPlaySpawnArc = false;

	UPROPERTY()
	float ElapsedMoveTime = 0.f;
};