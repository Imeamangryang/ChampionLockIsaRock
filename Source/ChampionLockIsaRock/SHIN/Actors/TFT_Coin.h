#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_Coin.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS()
class CHAMPIONLOCKISAROCK_API ATFT_Coin : public AActor
{
	GENERATED_BODY()

public:
	ATFT_Coin();

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

public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* PickupSphere; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CoinMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Coin")
	UNiagaraComponent* CoinEffect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TFT|Coin")
	bool bCollected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|Coin")
	int32 GoldAmount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|Coin|SpawnArc")
	float RandomOffsetRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|Coin|SpawnArc")
	float ArcHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TFT|Coin|SpawnArc")
	float MoveDuration = 1.0f;
	
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float ElapsedMoveTime = 0.f;

	UPROPERTY()
	bool bPlaySpawnArc = false;
};