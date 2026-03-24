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
			TEXT("[SCTurnManager] StartBattle: 각 팀은 정확히 4명이어야 함. A=%d, B=%d"),
			TeamAChars.Num(), TeamBChars.Num());
		return;
	}

	TurnOrder.Empty();
	for (ASCCharacterBase* Char : TeamAChars)
	{
		if (ensureMsgf(IsValid(Char), TEXT("[SCTurnManager] TeamA에 유효하지 않은 캐릭터 포함")))
			TurnOrder.Add(Char);
	}
	for (ASCCharacterBase* Char : TeamBChars)
	{
		if (ensureMsgf(IsValid(Char), TEXT("[SCTurnManager] TeamB에 유효하지 않은 캐릭터 포함")))
			TurnOrder.Add(Char);
	}

	TurnNumber = 0;
	CurrentActorIndex = 0;

	// 초기 코스트 설정
	TeamAState->ResetCost(3);
	TeamBState->ResetCost(3);

	UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 배틀 시작. 총 캐릭터=%d"), TurnOrder.Num());
	SetPhase(ESCBattlePhase::DrawPhase);
}

void USCTurnManager::AdvanceToNextPhase()
{
	switch (CurrentPhase)
	{
	case ESCBattlePhase::DrawPhase:
		SetPhase(ESCBattlePhase::PlayerQueuePhase);
		break;
	case ESCBattlePhase::PlayerQueuePhase:
		// ExecutePhase는 ConfirmAndExecuteQueue 호출로 진입
		break;
	case ESCBattlePhase::ExecutePhase:
		SetPhase(ESCBattlePhase::ResultPhase);
		break;
	case ESCBattlePhase::ResultPhase:
		AdvanceActorIndex();
		break;
	default:
		break;
	}
}

void USCTurnManager::ConfirmAndExecuteQueue(const TArray<AActor*>& Targets)
{
	if (CurrentPhase != ESCBattlePhase::PlayerQueuePhase)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: PlayerQueuePhase가 아님."));
		return;
	}

	ASCCharacterBase* Actor = GetCurrentActor();
	if (!IsValid(Actor))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: CurrentActor가 유효하지 않음."));
		return;
	}

	USCTeamState* TeamState = GetTeamState(Actor->Team);
	if (!IsValid(TeamState))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCTurnManager] ConfirmAndExecuteQueue: TeamState가 유효하지 않음."));
		return;
	}

	// 코스트 검증: 큐의 모든 스킬 코스트 합산 후 처리
	int32 TotalConsume = 0;
	int32 TotalGenerate = 0;
	for (TObjectPtr<USCSkillBase> Skill : Actor->SkillQueue)
	{
		if (!IsValid(Skill)) { continue; }
		if (Skill->SkillData.CostType == ESCSkillCostType::Consume)
			TotalConsume += Skill->SkillData.CostAmount;
		else
			TotalGenerate += Skill->SkillData.CostAmount;
	}

	// 소모량이 현재 코스트 + 획득량을 초과하면 실행 불가
	int32 NetConsume = TotalConsume - TotalGenerate;
	if (NetConsume > 0 && !TeamState->TryConsumeCost(NetConsume))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[SCTurnManager] 코스트 부족으로 스킬 큐 실행 불가. 필요 순소모=%d"), NetConsume);
		return;
	}

	// 획득만 있는 경우 코스트 추가
	if (NetConsume < 0)
	{
		TeamState->AddCost(-NetConsume);
	}

	SetPhase(ESCBattlePhase::ExecutePhase);
	Actor->ExecuteSkillQueue(Targets);
	SetPhase(ESCBattlePhase::ResultPhase);

	if (!CheckBattleEnd())
	{
		AdvanceActorIndex();
	}
}

ASCCharacterBase* USCTurnManager::GetCurrentActor() const
{
	if (!TurnOrder.IsValidIndex(CurrentActorIndex)) { return nullptr; }
	return TurnOrder[CurrentActorIndex].Get();
}

USCTeamState* USCTurnManager::GetTeamState(ESCTeam Team) const
{
	return (Team == ESCTeam::TeamA) ? TeamAState : TeamBState;
}

void USCTurnManager::SetPhase(ESCBattlePhase NewPhase)
{
	CurrentPhase = NewPhase;
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCTurnManager] Phase 전환: %d, Turn=%d, ActorIdx=%d"),
		static_cast<int32>(NewPhase), TurnNumber, CurrentActorIndex);
	OnBattlePhaseChanged.Broadcast(NewPhase);

	if (NewPhase == ESCBattlePhase::DrawPhase)
		BeginDrawPhase();
	else if (NewPhase == ESCBattlePhase::PlayerQueuePhase)
		BeginPlayerQueuePhase();
}

void USCTurnManager::BeginDrawPhase()
{
	ASCCharacterBase* Actor = GetCurrentActor();
	if (!IsValid(Actor)) { return; }
	Actor->BeginTurn();
	OnCharacterTurnBegin.Broadcast(Actor);
}

void USCTurnManager::BeginPlayerQueuePhase()
{
	// UI/Blueprint가 OnBattlePhaseChanged 델리게이트를 받아 큐 편성 UI를 표시
}

void USCTurnManager::AdvanceActorIndex()
{
	++CurrentActorIndex;

	// 살아있는 캐릭터만 대상으로 건너뜀
	while (TurnOrder.IsValidIndex(CurrentActorIndex))
	{
		ASCCharacterBase* Next = TurnOrder[CurrentActorIndex].Get();
		if (IsValid(Next) && !Next->IsDead()) { break; }
		++CurrentActorIndex;
	}

	// 모든 캐릭터 행동 완료 → 다음 턴
	if (!TurnOrder.IsValidIndex(CurrentActorIndex))
	{
		++TurnNumber;
		CurrentActorIndex = 0;
		OnTurnChanged.Broadcast(TurnNumber);
		UE_LOG(LogStrandingChain, Log, TEXT("[SCTurnManager] 새 턴 시작: Turn=%d"), TurnNumber);

		// 다음 턴 첫 행동 캐릭터 찾기
		while (TurnOrder.IsValidIndex(CurrentActorIndex))
		{
			ASCCharacterBase* Next = TurnOrder[CurrentActorIndex].Get();
			if (IsValid(Next) && !Next->IsDead()) { break; }
			++CurrentActorIndex;
		}
	}

	if (TurnOrder.IsValidIndex(CurrentActorIndex))
	{
		SetPhase(ESCBattlePhase::DrawPhase);
	}
}

bool USCTurnManager::CheckBattleEnd()
{
	TArray<ASCCharacterBase*> AliveA = GetAliveCharacters(ESCTeam::TeamA);
	TArray<ASCCharacterBase*> AliveB = GetAliveCharacters(ESCTeam::TeamB);

	if (AliveA.IsEmpty() || AliveB.IsEmpty())
	{
		ESCTeam Winner = AliveA.IsEmpty() ? ESCTeam::TeamB : ESCTeam::TeamA;
		SetPhase(ESCBattlePhase::BattleEnd);
		OnBattleEnd.Broadcast(Winner);
		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCTurnManager] 배틀 종료. 승리 팀=%d"), static_cast<int32>(Winner));
		return true;
	}
	return false;
}

TArray<ASCCharacterBase*> USCTurnManager::GetAliveCharacters(ESCTeam Team) const
{
	TArray<ASCCharacterBase*> Result;
	for (const TWeakObjectPtr<ASCCharacterBase>& WeakChar : TurnOrder)
	{
		ASCCharacterBase* Char = WeakChar.Get();
		if (IsValid(Char) && Char->Team == Team && !Char->IsDead())
			Result.Add(Char);
	}
	return Result;
}
