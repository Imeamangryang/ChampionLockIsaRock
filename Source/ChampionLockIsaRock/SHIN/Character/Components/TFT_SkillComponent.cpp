#include "TFT_SkillComponent.h"
#include "../../Struct/FStruct_TFT_Champion.h"
#include "NiagaraSystem.h"

UTFT_SkillComponent::UTFT_SkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTFT_SkillComponent::Initialize(const FStruct_TFT_Champion& Data)
{
	SkillName = Data.Skill.Name;
	Type = Data.Skill.Type;
	Description = Data.Skill.Description;
	SkillImage = Data.Skill.Image;
	
	SkillValues.Empty();

	for (const FString& StatString : Data.Skill.Stats)
	{
		const FString Trimmed = StatString.TrimStartAndEnd();

		if (!Trimmed.IsEmpty())
		{
			SkillValues.Add(FCString::Atof(*Trimmed));
		}
	}
	
	SkillEffect = Data.Skill.Effect.LoadSynchronous();
}

void UTFT_SkillComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTFT_SkillComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

