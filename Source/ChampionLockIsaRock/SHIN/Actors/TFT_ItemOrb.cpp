#include "TFT_ItemOrb.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dong/Public/TopDownController.h"
#include "SHIN/Character/Components/TFT_ItemInventoryComponent.h"
#include "SHIN/GameFramework/TFT_GameInstance.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ATFT_ItemOrb::ATFT_ItemOrb()
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

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetupAttachment(Root);
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OrbMesh->SetRelativeRotation(FRotator(0.f, -90.f, -50.f));
	
	OrbEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OrbEffect"));
	OrbEffect->SetupAttachment(Root);
	OrbEffect->bAutoActivate = true;
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> OrbMeshAsset(TEXT("/Game/SHIN/Blueprints/Items/Sphere.Sphere"));
	{
		OrbMesh->SetStaticMesh(OrbMeshAsset.Object);
	}
	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> OrbMaterialAsset(TEXT("/Game/SHIN/Blueprints/Items/MT_ItemOrb_Inst.MT_ItemOrb_Inst"));
	{
		OrbMesh->SetMaterial(0, OrbMaterialAsset.Object);
	}
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> OrbEffectAsset(TEXT("/Game/VFX/Imortal_Loot_Drop_VFX/Niagara/NS_Loot_Drop.NS_Loot_Drop"));
	if (OrbEffectAsset.Succeeded())
	{	
		OrbEffect->SetAsset(OrbEffectAsset.Object);
	}
	
	ConstructorHelpers::FObjectFinder<USoundBase> OrbSoundAsset(TEXT("/Game/SHIN/Sound/Addbooty_out.Addbooty_out"));
	if (OrbSoundAsset.Succeeded())
	{
		OrbSound = OrbSoundAsset.Object;
	}
}

void ATFT_ItemOrb::Tick(float DeltaTime)
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
}

void ATFT_ItemOrb::BeginPlay()
{
	Super::BeginPlay();

	InitializeRandomItem();
	
	StartLocation = GetActorLocation();

	const FVector2D Random2D = FMath::RandPointInCircle(RandomOffsetRadius);
	TargetLocation = StartLocation + FVector(Random2D.X, Random2D.Y, 0.f);

	ElapsedMoveTime = 0.f;
	bPlaySpawnArc = true;

	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ATFT_ItemOrb::OnPickupSphereBeginOverlap);
	}
}

void ATFT_ItemOrb::OnPickupSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PlayerController))
	{
		return;
	}

	APawn* ControlledPawn = PlayerController->GetPawn();
	if (!IsValid(ControlledPawn) || OtherActor != ControlledPawn)
	{
		return;
	}

	ATopDownController* TDController = Cast<ATopDownController>(PlayerController);
	if (!IsValid(TDController) || !IsValid(TDController->ItemInventoryComponent))
	{
		return;
	}

	if (ItemId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TFT_ItemOrb has no ItemId set."));
		return;
	}

	FStruct_TFTItemInstance NewItem;
	NewItem.ItemId = ItemId;
	NewItem.Amount = Amount;

	const bool bAdded = TDController->ItemInventoryComponent->AddItem(NewItem);
	if (!bAdded)
	{
		return;
	}

	if (OrbSound)
	{
		UGameplayStatics::PlaySound2D(this, OrbSound);
	}

	TDController->ItemInventoryComponent->NotifyInventoryUpdated();

	bCollected = true;
	Destroy();
}

void ATFT_ItemOrb::InitializeRandomItem()
{
	if (ItemId != NAME_None)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UTFT_GameInstance* GI = World->GetGameInstance<UTFT_GameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("TFT_ItemOrb: GameInstance is null"));
		return;
	}

	UDataTable* ItemTable = GI->ItemDataTable.LoadSynchronous();
	if (!ItemTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("TFT_ItemOrb: ItemDataTable load failed"));
		return;
	}

	TArray<FName> RowNames = ItemTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TFT_ItemOrb: ItemDataTable has no rows"));
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	const FName RandomRowName = RowNames[RandomIndex];

	const FStruct_TFTItemDefinition* ItemDef =
		ItemTable->FindRow<FStruct_TFTItemDefinition>(RandomRowName, TEXT("InitializeRandomItem"));

	if (!ItemDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("TFT_ItemOrb: Failed to find random row"));
		return;
	}

	ItemId = ItemDef->ItemId;

	UE_LOG(LogTemp, Log, TEXT("TFT_ItemOrb selected random item: %s"), *ItemId.ToString());
}
