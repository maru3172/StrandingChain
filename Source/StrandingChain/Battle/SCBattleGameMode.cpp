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

	// TeamB AI 턴 시작 알림 바인딩
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
}

void ASCBattleGameMode::InitiateBattle(
	const TArray<ASCCharacterBase*>& TeamAChars,
	const TArray<ASCCharacterBase*>& TeamBChars)
{
	USCTurnManager* Manager = TurnManager.Get();
	if (!IsValid(Manager)) { return; }
	Manager->StartBattle(TeamAChars, TeamBChars);
}

// ── TeamB AI ─────────────────────────────────────────────────────────────────

void ASCBattleGameMode::OnCharacterTurnBeginHandler(ASCCharacterBase* InCharacter)
{
	if (!IsValid(InCharacter)) { return; }

	// TeamA 캐릭터 턴은 PlayerController가 처리
	// GameMode는 TeamB만 처리
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

	if (!IsValid(InCharacter) || InCharacter->IsDead())
	{
		// 사망한 경우 AI 턴 건너뛰고 플레이어 턴으로
		USCTurnManager* TM = TurnManager.Get();
		if (IsValid(TM)) { TM->AdvanceAfterAITurn(); }
		return;
	}

	USCTurnManager* TM = TurnManager.Get();
	if (!IsValid(TM)) { return; }

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

	// 코스트 부족 시 Generate 스킬이라도
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

	// ── 코스트 처리 ──────────────────────────────────────────────────────────
	if (IsValid(TeamState))
	{
		int32 TotalConsume = 0, TotalGenerate = 0;
		for (const TObjectPtr<USCSkillBase>& Skill : InCharacter->SkillQueue)
		{
			if (!IsValid(Skill)) { continue; }
			if (Skill->SkillData.CostType == ESCSkillCostType::Consume)
				TotalConsume += Skill->SkillData.CostAmount;
			else
				TotalGenerate += Skill->SkillData.CostAmount;
		}
		const int32 Net = TotalConsume - TotalGenerate;
		if (Net > 0) TeamState->TryConsumeCost(Net);
		else if (Net < 0) TeamState->AddCost(-Net);
	}

	// ── 스킬 실행 ────────────────────────────────────────────────────────────
	TArray<ASCCharacterBase*> AliveA = TM->GetAliveCharsOfTeam(ESCTeam::TeamA);
	TArray<AActor*> Targets;

	// 랜덤 단일 타겟 (AI는 랜덤으로 한 명 선택)
	if (AliveA.Num() > 0)
	{
		int32 RandIdx = FMath::RandRange(0, AliveA.Num() - 1);
		if (IsValid(AliveA[RandIdx]))
			Targets.Add(AliveA[RandIdx]);
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleGameMode] AI 실행: %s 큐=%d 타겟=%d"),
		*InCharacter->GetName(), InCharacter->SkillQueue.Num(), Targets.Num());

	InCharacter->ExecuteSkillQueue(Targets);
	InCharacter->bHasActedThisTurn = true;

	// ── 플레이어 턴으로 복귀 ─────────────────────────────────────────────────
	TM->AdvanceAfterAITurn();
}
