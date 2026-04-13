// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/Public/TFTPlayerState.h"
#include "Dong/Public/TopDownController.h"
#include "SHIN/Subsystem/TFT_UISubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ATFTPlayerState::ATFTPlayerState()
{
	// 초기값 설정 (TFT 시험 모드 기준)
	PlayerGold = 10;
	PlayerLevel = 3;
	CurrentXP = 0;
	UpdateMaxXP(); // 레벨 1의 MaxXP 설정
}

// 헬퍼 함수 구현
UTFT_UISubsystem* ATFTPlayerState::GetUISubsystem() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			return LP->GetSubsystem<UTFT_UISubsystem>();
		}
	}
	return nullptr;
}

void ATFTPlayerState::AddGold(int32 Amount)
{
	PlayerGold += Amount;
	if (auto* UISub = GetUISubsystem())
	{
		UISub->BroadcastGoldUpdate(PlayerGold); // 함수 이름 확인: BroadcastGoldUpdate
	}
}

bool ATFTPlayerState::SpendGold(int32 Amount)
{
	if (PlayerGold >= Amount)
	{
		PlayerGold -= Amount;
		if (auto* UISub = GetUISubsystem())
		{
			UISub->BroadcastGoldUpdate(PlayerGold); // 돈을 쓸 때도 UI에 알려줘야 함!
		}
		return true;
	}
	return false;
}

void ATFTPlayerState::AddXP(int32 Amount)
{
	// 레벨업 전의 레벨 미리 저장
	int32 OldLevel = PlayerLevel;
	CurrentXP += Amount;

	// 레벨업 체크
	while (CurrentXP >= MaxXP && PlayerLevel < 10) // 최대 레벨 10 가정
	{
		CurrentXP -= MaxXP;
		PlayerLevel++;
		UpdateMaxXP(); // 레벨에 맞는 새로운 MaxXP 갱신
	}
	// 레벨이 올랐는지 확인
	if (OldLevel != PlayerLevel)
	{
		if (SoundLevelUp)
		{
			UGameplayStatics::PlaySound2D(this, SoundLevelUp);
		}
		
		if (auto* PC = Cast<ATopDownController>(GetWorld()->GetFirstPlayerController()))
		{
			PC->BroadcastUnitCount();
		}
	}
	// 경험치/레벨 UI 업데이트 호출
	if (auto* UISub = GetUISubsystem())
	{
		UISub->BroadcastLevelInfoUpdate(PlayerLevel, CurrentXP, MaxXP);
	}
}

void ATFTPlayerState::UpdateMaxXP()
{
	// 제공해주신 TFT 레벨별 경험치 테이블 적용
	switch (PlayerLevel)
	{
		case 1: MaxXP = 2; break;
		case 2: MaxXP = 2; break;
		case 3: MaxXP = 6; break;
		case 4: MaxXP = 10; break;
		case 5: MaxXP = 20; break;
		case 6: MaxXP = 36; break;
		case 7: MaxXP = 60; break;
		case 8: MaxXP = 68; break;
		case 9: MaxXP = 68; break;
		default: MaxXP = 999; break; // 만렙 이후
	} 
}

// 레벨업 버튼 클릭 시 호출될 함수 (4골드 소모 -> 4 XP 획득)
void ATFTPlayerState::BuyXP()
{
	if (SpendGold(4))
	{
		AddXP(4);
	}
	else
	{
		// 돈이 부족하다는 피드백 (UI 메시지 등)
		UE_LOG(LogTemp, Warning, TEXT("골드가 부족합니다!"));
	}
}

void ATFTPlayerState::AddStageEndXP()
{
	// 스테이지가 넘어갈 때마다 경험치 2 추가
	AddXP(2);
}

