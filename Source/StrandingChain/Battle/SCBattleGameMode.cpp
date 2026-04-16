// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleGameMode.cpp

#include "SCBattleGameMode.h"
#include "SCTurnManager.h"
#include "SCCharacterBase.h"
#include "SCTeamState.h"
#include "SCSkillBase.h"
#include "SCBattleArena.h"
#include "SCBattlePlayerController.h"
#include "SCBattleTypes.h"
#include "StrandingChain.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ASCBattleGameMode::ASCBattleGameMode()
{
	PlayerControllerClass = ASCBattlePlayerController::StaticClass();
	DefaultPawnClass      = nullptr;
}

void ASCBattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetGameInstance();
	if (!ensureMsgf(IsValid(GI), TEXT("[SCBattleGameMode] GameInstance 없음.")))
		return;

	USCTurnManager* Manager = GI->GetSubsystem<USCTurnManager>();
	if (!ensureMsgf(IsValid(Manager), TEXT("[SCBattleGameMode] SCTurnManager 없음.")))
		return;

	TurnManager = Manager;

	Manager->OnCharacterTurnBegin.AddDynamic(
		this, &ASCBattleGameMode::OnCharacterTurnBeginHandler);

	UE_LOG(LogStrandingChain, Log, TEXT("[SCBattleGameMode] BeginPlay 완료."));
	SetArenaCamera();
}

void ASCBattleGameMode::SetArenaCamera()
{
	AActor* ArenaActor = UGameplayStatics::GetActorOfClass(
		GetWorld(), ASCBattleArena::StaticClass());
	if (!IsValid(ArenaActor)) { return; }

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!IsValid(PC)) { return; }

	PC->SetViewTargetWithBlend(ArenaActor, 0.f);
	UE_LOG(LogStrandingChain, Log, TEXT("[SCBattleGameMode] 아레나 카메라 전환."));
}

void ASCBattleGameMode::InitiateBattle(
	const TArray<ASCCharacterBase*>& TeamAChars,
	const TArray<ASCCharacterBase*>& TeamBChars)
{
	USCTurnManager* Manager = TurnManager.Get();
	if (!IsValid(Manager))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleGameMode] InitiateBattle: TurnManager 없음."));
		return;
	}
	Manager->StartBattle(TeamAChars, TeamBChars);
}

// ── TeamB AI ─────────────────────────────────────────────────────────────────

void ASCBattleGameMode::OnCharacterTurnBeginHandler(ASCCharacterBase* InCharacter)
{
	if (!IsValid(InCharacter)) { return; }
	if (InCharacter->Team != ESCTeam::TeamB || InCharacter->IsDead()) { return; }

	PendingAICharacter = InCharacter;

	GetWorldTimerManager().SetTimer(
		AITimerHandle,
		this,
		&ASCBattleGameMode::ExecuteAITurn,
		1.2f,
		false);

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleGameMode] TeamB AI 예약: %s"), *InCharacter->GetName());
}

void ASCBattleGameMode::ExecuteAITurn()
{
	ASCCharacterBase* InCharacter = PendingAICharacter.Get();
	PendingAICharacter.Reset();

	if (!IsValid(InCharacter) || InCharacter->IsDead()) { return; }

	USCTurnManager* TM = TurnManager.Get();
	if (!IsValid(TM)) { return; }

	if (TM->GetCurrentActor() != InCharacter)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCBattleGameMode] ExecuteAITurn: 현재 액터 불일치."));
		return;
	}

	// ── 스킬 선택 ────────────────────────────────────────────────────────────
	USCTeamState* TeamState = TM->GetTeamState(ESCTeam::TeamB);
	int32 Budget = IsValid(TeamState) ? TeamState->CurrentCost : 0;

	TArray<int32> DrawIndices;
	for (int32 i = 0; i < InCharacter->DrawnSkills.Num(); ++i)
		DrawIndices.Add(i);

	// Fisher-Yates 셔플
	for (int32 i = DrawIndices.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		DrawIndices.Swap(i, j);
	}

	InCharacter->SkillQueue.Empty();

	for (int32 DrawIdx : DrawIndices)
	{
		if (InCharacter->SkillQueue.Num() >= 2) { break; }

		USCSkillBase* Sk = InCharacter->DrawnSkills.IsValidIndex(DrawIdx)
			? InCharacter->DrawnSkills[DrawIdx].Get() : nullptr;
		if (!IsValid(Sk)) { continue; }

		if (Sk->SkillData.CostType == ESCSkillCostType::Consume)
		{
			if (Budget >= Sk->SkillData.CostAmount)
			{
				if (InCharacter->EnqueueSkill(DrawIdx))
					Budget -= Sk->SkillData.CostAmount;
			}
		}
		else
		{
			InCharacter->EnqueueSkill(DrawIdx);
		}
	}

	// 코스트 부족으로 아무것도 못 넣은 경우 Generate 스킬이라도
	if (InCharacter->SkillQueue.IsEmpty())
	{
		for (int32 DrawIdx : DrawIndices)
		{
			USCSkillBase* Sk = InCharacter->DrawnSkills.IsValidIndex(DrawIdx)
				? InCharacter->DrawnSkills[DrawIdx].Get() : nullptr;
			if (IsValid(Sk) && Sk->SkillData.CostType == ESCSkillCostType::Generate)
			{
				InCharacter->EnqueueSkill(DrawIdx);
				break;
			}
		}
	}

	// 타겟: TeamA 살아있는 전원
	TArray<ASCCharacterBase*> AliveA = TM->GetAliveCharsOfTeam(ESCTeam::TeamA);
	TArray<AActor*> Targets;
	for (ASCCharacterBase* T : AliveA) { if (IsValid(T)) Targets.Add(T); }

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleGameMode] AI 실행: %s 큐=%d 타겟=%d"),
		*InCharacter->GetName(), InCharacter->SkillQueue.Num(), Targets.Num());

	TM->ConfirmAndExecuteQueue(Targets);
}
