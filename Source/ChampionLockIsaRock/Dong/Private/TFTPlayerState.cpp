// Fill out your copyright notice in the Description page of Project Settings.


#include "Dong/Public/TFTPlayerState.h"

ATFTPlayerState::ATFTPlayerState()
{
	// 초기값 설정 (TFT 시험 모드 기준)
	Gold = 10;
	PlayerLevel = 1;
	CurrentXP = 0;
	UpdateMaxXP(); // 레벨 1의 MaxXP 설정
}

void ATFTPlayerState::AddGold(int32 Amount)
{
	Gold += Amount;
	OnGoldChanged.Broadcast(Gold); // UI 업데이트 신호 전송
}

bool ATFTPlayerState::SpendGold(int32 Amount)
{
	if (Gold >= Amount)
	{
		Gold -= Amount;
		OnGoldChanged.Broadcast(Gold);
		return true;
	}
	return false; // 돈이 부족함
}

void ATFTPlayerState::AddXP(int32 Amount)
{
	CurrentXP += Amount;

	// 레벨업 체크 (반복문을 써서 한 번에 여러 레벨이 오를 경우도 대비)
	while (CurrentXP >= MaxXP && PlayerLevel < 10) // 최대 레벨 10 가정
	{
		CurrentXP -= MaxXP;
		PlayerLevel++;
		UpdateMaxXP(); // 레벨에 맞는 새로운 MaxXP 갱신
	}

	// 변경된 레벨/경험치 정보를 UI에 알림
	OnLevelInfoChanged.Broadcast(PlayerLevel, CurrentXP, MaxXP);
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
