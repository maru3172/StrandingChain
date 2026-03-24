// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCTurnManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SCBattleTypes.h"
#include "SCTurnManager.generated.h"

class ASCCharacterBase;
class USCTeamState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattlePhaseChanged, ESCBattlePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, int32, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterTurnBegin, ASCCharacterBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnd, ESCTeam, WinnerTeam);

/**
 * SCTurnManager (GameInstanceSubsystem)
 * ─ 4v4 턴제 흐름 관리
 * ─ 모든 캐릭터가 1회씩 행동 → 턴 종료 → 반복
 * ─ 스킬 큐 실행 전 팀 코스트 검증
 */
UCLASS()
class STRANDINGCHAIN_API USCTurnManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 배틀 초기화 ─────────────────────────
	/**
	 * 배틀 시작
	 * @param TeamAChars TeamA 캐릭터 4명
	 * @param TeamBChars TeamB 캐릭터 4명
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void StartBattle(
		const TArray<ASCCharacterBase*>& TeamAChars,
		const TArray<ASCCharacterBase*>& TeamBChars);

	// ── 페이즈 전환 ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void AdvanceToNextPhase();

	// ── 스킬 큐 확정 및 실행 ────────────────
	/**
	 * 현재 행동 중인 캐릭터의 스킬 큐를 실행
	 * 코스트를 먼저 검증하고 실행
	 * @param Targets 대상 목록
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void ConfirmAndExecuteQueue(const TArray<AActor*>& Targets);

	// ── 조회 ────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ESCBattlePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetTurnNumber() const { return TurnNumber; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ASCCharacterBase* GetCurrentActor() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	USCTeamState* GetTeamState(ESCTeam Team) const;

	// ── 델리게이트 ───────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnBattlePhaseChanged OnBattlePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnTurnChanged OnTurnChanged;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnCharacterTurnBegin OnCharacterTurnBegin;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnBattleEnd OnBattleEnd;

private:
	void SetPhase(ESCBattlePhase NewPhase);
	void BeginDrawPhase();
	void BeginPlayerQueuePhase();
	void BeginExecutePhase(const TArray<AActor*>& Targets);
	void BeginResultPhase();
	void AdvanceActorIndex();
	bool CheckBattleEnd();
	TArray<ASCCharacterBase*> GetAliveCharacters(ESCTeam Team) const;

	UPROPERTY()
	ESCBattlePhase CurrentPhase = ESCBattlePhase::Idle;

	UPROPERTY()
	int32 TurnNumber = 0;

	/** 이번 턴 행동 순서 (TeamA 4 → TeamB 4 순) */
	UPROPERTY()
	TArray<TWeakObjectPtr<ASCCharacterBase>> TurnOrder;

	/** 현재 행동 중인 캐릭터 인덱스 */
	UPROPERTY()
	int32 CurrentActorIndex = 0;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamAState;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamBState;
};
