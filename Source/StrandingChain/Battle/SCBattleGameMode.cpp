// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleGameMode.cpp

#include "SCBattleGameMode.h"
#include "SCTurnManager.h"
#include "SCCharacterBase.h"
#include "SCBattleArena.h"
#include "SCBattlePlayerController.h"
#include "StrandingChain.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"

ASCBattleGameMode::ASCBattleGameMode()
{
	// 배틀 전용 PlayerController 등록
	PlayerControllerClass = ASCBattlePlayerController::StaticClass();

	// DefaultPawn 비활성화 — 스펙테이터 구체 제거
	DefaultPawnClass = nullptr;
}

void ASCBattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetGameInstance();
	if (!ensureMsgf(IsValid(GI),
		TEXT("[SCBattleGameMode] GameInstance가 유효하지 않음.")))
		return;

	USCTurnManager* Manager = GI->GetSubsystem<USCTurnManager>();
	if (!ensureMsgf(IsValid(Manager),
		TEXT("[SCBattleGameMode] SCTurnManager 서브시스템을 찾을 수 없음.")))
		return;

	TurnManager = Manager;
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleGameMode] BeginPlay 완료. TurnManager 참조 확인됨."));

	// 아레나 카메라로 플레이어 뷰타겟 전환
	SetArenaCamera();
}

void ASCBattleGameMode::SetArenaCamera()
{
	// 레벨에 배치된 SCBattleArena 검색
	AActor* ArenaActor = UGameplayStatics::GetActorOfClass(
		GetWorld(), ASCBattleArena::StaticClass());

	if (!IsValid(ArenaActor)) { return; }

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(PC)) { return; }

	// 아레나의 카메라 액터를 뷰타겟으로 설정
	PC->SetViewTargetWithBlend(ArenaActor, 0.f);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleGameMode] 아레나 카메라로 뷰타겟 전환 완료."));
}

void ASCBattleGameMode::InitiateBattle(
	const TArray<ASCCharacterBase*>& TeamAChars,
	const TArray<ASCCharacterBase*>& TeamBChars)
{
	USCTurnManager* Manager = TurnManager.Get();
	if (!IsValid(Manager))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleGameMode] InitiateBattle: TurnManager가 유효하지 않음."));
		return;
	}
	Manager->StartBattle(TeamAChars, TeamBChars);
}
