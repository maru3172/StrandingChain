// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCTurnManager.cpp

#include "SCTurnManager.h"
#include "SCCharacterBase.h"
#include "SCTeamState.h"
#include "SCSkillBase.h"
#include "StrandingChain.h"

void USCTurnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TeamAState = NewObject<USCTeamState>(this);
	TeamAState->Team = ESCTeam::TeamA;

	TeamBState = NewObject<USCTeamState>(this);
	TeamBState->Team = ESCTeam::TeamB;

	UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 초기화 완료."));
}

void USCTurnManager::Deinitialize()
{
	UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 해제."));
	Super::Deinitialize();
}

void USCTurnManager::StartBattle(
	const TArray<ASCCharacterBase*>& TeamAChars,
	const TArray<ASCCharacterBase*>& TeamBChars)
{
	if (TeamAChars.Num() != 4 || TeamBChars.Num() != 4)
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCTurnManager] StartBattle: 각 팀 4명 필요. A=%d, B=%d"),
			TeamAChars.Num(), TeamBChars.Num());
		return;
	}

	TeamACharsAll.Empty();
	TeamBOrder.Empty();

	for (ASCCharacterBase* A : TeamAChars)
	{
		if (ensureMsgf(IsValid(A), TEXT("[SCTurnManager] TeamA 유효하지 않음")))
			TeamACharsAll.Add(A);
	}
	for (ASCCharacterBase* B : TeamBChars)
	{
		if (ensureMsgf(IsValid(B), TEXT("[SCTurnManager] TeamB 유효하지 않음")))
			TeamBOrder.Add(B);
	}

	TeamBIndex = 0;
	TurnNumber = 1;
	bIsPlayerTurn = true;

	TeamAState->ResetCost(3);
	TeamBState->ResetCost(3);

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCTurnManager] 배틀 시작. 턴 구조: 플레이어(자유선택) → B%d → 반복"),
		TeamBIndex);

	SetPhase(ESCBattlePhase::DrawPhase);
	OnPlayerTurnStarted.Broadcast();
}

void USCTurnManager::ConfirmAndExecuteQueue(
	ASCCharacterBase* PlayerChar, const TArray<AActor*>& Targets)
{
	if (!bIsPlayerTurn)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: 지금은 AI 턴."));
		return;
	}
	if (!IsValid(PlayerChar) || PlayerChar->Team != ESCTeam::TeamA)
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: PlayerChar 유효하지 않음."));
		return;
	}
	if (PlayerChar->IsDead())
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: 사망한 캐릭터 선택."));
		return;
	}

	// ── 코스트 검증 ─────────────────────────────────────────────────────
	USCTeamState* TeamState = TeamAState;
	int32 TotalConsume = 0, TotalGenerate = 0;
	for (const TObjectPtr<USCSkillBase>& Skill : PlayerChar->SkillQueue)
	{
		if (!IsValid(Skill)) { continue; }
		if (Skill->SkillData.CostType == ESCSkillCostType::Consume)
			TotalConsume += Skill->SkillData.CostAmount;
		else
			TotalGenerate += Skill->SkillData.CostAmount;
	}

	int32 NetConsume = TotalConsume - TotalGenerate;
	if (NetConsume > 0 && !TeamState->TryConsumeCost(NetConsume))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] 코스트 부족 (순소모=%d). 실행 취소."), NetConsume);
		return;
	}
	if (NetConsume < 0) { TeamState->AddCost(-NetConsume); }

	// ── 플레이어 스킬 실행 ───────────────────────────────────────────────
	SetPhase(ESCBattlePhase::ExecutePhase);
	PlayerChar->ExecuteSkillQueue(Targets);
	SetPhase(ESCBattlePhase::ResultPhase);

	PlayerChar->bHasActedThisTurn = true;

	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCTurnManager] 플레이어 실행 완료: %s"), *PlayerChar->GetName());

	if (CheckBattleEnd()) { return; }

	// ── AI 턴으로 전환 ───────────────────────────────────────────────────
	bIsPlayerTurn = false;
	ASCCharacterBase* AIChar = FindNextAliveTeamBChar();
	if (!IsValid(AIChar))
	{
		// 살아있는 TeamB 없음 → 배틀 종료
		CheckBattleEnd();
		return;
	}

	AIChar->BeginTurn();
	SetPhase(ESCBattlePhase::DrawPhase);
	OnCharacterTurnBegin.Broadcast(AIChar);
}

void USCTurnManager::SkipCurrentActorTurn()
{
	if (!bIsPlayerTurn)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] SkipCurrentActorTurn: 플레이어 턴이 아님."));
		return;
	}

	UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 플레이어 턴 건너뜀."));

	SetPhase(ESCBattlePhase::ResultPhase);
	if (CheckBattleEnd()) { return; }

	// 건너뛰어도 AI 턴은 진행
	bIsPlayerTurn = false;
	ASCCharacterBase* AIChar = FindNextAliveTeamBChar();
	if (!IsValid(AIChar))
	{
		CheckBattleEnd();
		return;
	}

	AIChar->BeginTurn();
	SetPhase(ESCBattlePhase::DrawPhase);
	OnCharacterTurnBegin.Broadcast(AIChar);
}

void USCTurnManager::AdvanceAfterAITurn()
{
	if (bIsPlayerTurn)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] AdvanceAfterAITurn: 이미 플레이어 턴."));
		return;
	}

	// ── 다음 TeamB 인덱스로 이동 ────────────────────────────────────────
	TeamBIndex++;
	if (TeamBIndex >= TeamBOrder.Num())
	{
		// 한 라운드 완료
		TeamBIndex = 0;
		TurnNumber++;
		OnTurnChanged.Broadcast(TurnNumber);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCTurnManager] 라운드 종료 → 턴 %d 시작"), TurnNumber);
	}

	if (CheckBattleEnd()) { return; }

	// ── 플레이어 턴으로 전환 ────────────────────────────────────────────
	bIsPlayerTurn = true;
	SetPhase(ESCBattlePhase::DrawPhase);

	UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 플레이어 턴 시작."));
	OnPlayerTurnStarted.Broadcast();
}

ASCCharacterBase* USCTurnManager::GetCurrentAIActor() const
{
	if (bIsPlayerTurn) { return nullptr; }
	if (!TeamBOrder.IsValidIndex(TeamBIndex)) { return nullptr; }
	return TeamBOrder[TeamBIndex].Get();
}

USCTeamState* USCTurnManager::GetTeamState(ESCTeam Team) const
{
	return (Team == ESCTeam::TeamA) ? TeamAState : TeamBState;
}

TArray<ASCCharacterBase*> USCTurnManager::GetAliveCharsOfTeam(ESCTeam Team) const
{
	TArray<ASCCharacterBase*> Result;
	const TArray<TWeakObjectPtr<ASCCharacterBase>>& Source =
		(Team == ESCTeam::TeamA) ? TeamACharsAll : TeamBOrder;
	for (const TWeakObjectPtr<ASCCharacterBase>& W : Source)
	{
		ASCCharacterBase* C = W.Get();
		if (IsValid(C) && !C->IsDead()) { Result.Add(C); }
	}
	return Result;
}

void USCTurnManager::SetPhase(ESCBattlePhase NewPhase)
{
	CurrentPhase = NewPhase;
	OnBattlePhaseChanged.Broadcast(NewPhase);
}

bool USCTurnManager::CheckBattleEnd()
{
	TArray<ASCCharacterBase*> AliveA = GetAliveCharsOfTeam(ESCTeam::TeamA);
	TArray<ASCCharacterBase*> AliveB = GetAliveCharsOfTeam(ESCTeam::TeamB);

	if (AliveA.IsEmpty() || AliveB.IsEmpty())
	{
		ESCTeam Winner = AliveA.IsEmpty() ? ESCTeam::TeamB : ESCTeam::TeamA;
		SetPhase(ESCBattlePhase::BattleEnd);
		OnBattleEnd.Broadcast(Winner);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCTurnManager] 배틀 종료. 승리=%d"), static_cast<int32>(Winner));
		return true;
	}
	return false;
}

ASCCharacterBase* USCTurnManager::FindNextAliveTeamBChar()
{
	// TeamBIndex부터 순환하며 살아있는 TeamB 캐릭터 탐색
	const int32 Total = TeamBOrder.Num();
	for (int32 i = 0; i < Total; ++i)
	{
		int32 Idx = (TeamBIndex + i) % Total;
		ASCCharacterBase* C = TeamBOrder.IsValidIndex(Idx) ? TeamBOrder[Idx].Get() : nullptr;
		if (IsValid(C) && !C->IsDead())
		{
			TeamBIndex = Idx; // 탐색 중 건너뛴 경우 인덱스 보정
			return C;
		}
	}
	return nullptr;
}
