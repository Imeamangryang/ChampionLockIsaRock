#include "TFT_Coin.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dong/Public/TopDownController.h"
#include "Dong/Public/TFTPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"

ATFT_Coin::ATFT_Coin()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(Root);
	PickupSphere->InitSphereRadius(60.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
	CoinMesh->SetupAttachment(Root);
	CoinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoinMesh->SetRelativeScale3D(FVector(2.0f));

	CoinEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("CoinEffect"));
	CoinEffect->SetupAttachment(Root);
	CoinEffect->SetRelativeLocation(FVector::ZeroVector);
	CoinEffect->bAutoActivate = true;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CoinMeshAsset(TEXT("/Game/SHIN/Blueprints/Items/Coin/Coin/fbx/coin_extracted/source/Coin1.Coin1"));
	if (CoinMeshAsset.Succeeded())
	{
		CoinMesh->SetStaticMesh(CoinMeshAsset.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> CoinEffectAsset(
	TEXT("/Game/VFX/DrapEffet/VFX/NE_drop_effects03.NE_drop_effects03"));
	if (CoinEffectAsset.Succeeded())
	{
		CoinEffect->SetAsset(CoinEffectAsset.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundAsset(TEXT("/Game/SHIN/Sound/AddGold.AddGold"));
	if (PickupSoundAsset.Succeeded())
	{
		PickupSound = PickupSoundAsset.Object;
	}
}

void ATFT_Coin::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();

	const FVector2D Random2D = FMath::RandPointInCircle(RandomOffsetRadius);
	TargetLocation = StartLocation + FVector(Random2D.X, Random2D.Y, 0.f);

	ElapsedMoveTime = 0.f;
	bPlaySpawnArc = true;

	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ATFT_Coin::OnPickupSphereBeginOverlap);
	}
}

void ATFT_Coin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPlaySpawnArc)
	{
		ElapsedMoveTime += DeltaTime;

		const float Alpha = FMath::Clamp(ElapsedMoveTime / MoveDuration, 0.f, 1.f);

		const FVector BaseLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
		const float ArcOffsetZ = 4.f * ArcHeight * Alpha * (1.f - Alpha);

		const FVector NewLocation = BaseLocation + FVector(0.f, 0.f, ArcOffsetZ);
		SetActorLocation(NewLocation);

		if (Alpha >= 1.f)
		{
			SetActorLocation(TargetLocation);
			bPlaySpawnArc = false;
		}
	}
	
	CoinMesh->AddLocalRotation(FRotator(0.f, 180 * DeltaTime, 0.f));
}

void ATFT_Coin::OnPickupSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected || bPlaySpawnArc)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || OtherActor != PlayerPawn)
	{
		return;
	}

	ATopDownController* TDController = Cast<ATopDownController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!TDController)
	{
		return;
	}

	ATFTPlayerState* PS = TDController->GetPlayerState<ATFTPlayerState>();
	if (!PS)
	{
		return;
	}
	
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}
	
	PS->AddGold(GoldAmount);

	bCollected = true;
	Destroy();
}