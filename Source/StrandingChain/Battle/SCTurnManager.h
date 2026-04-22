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

/** 플레이어 턴 시작 — 어느 TeamA 캐릭터를 쓸지 선택할 것을 알림 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE FOnPlayerTurnStarted;

UCLASS()
class STRANDINGCHAIN_API USCTurnManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 배틀 초기화 ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void StartBattle(
		const TArray<ASCCharacterBase*>& TeamAChars,
		const TArray<ASCCharacterBase*>& TeamBChars);

	// ── 플레이어 턴 행동 ─────────────────────
	/**
	 * 플레이어가 선택한 캐릭터(PlayerChar)의 스킬 큐를 실행
	 * ConfirmAndExecuteQueue는 PlayerChar를 명시적으로 받음
	 * → 실행 후 자동으로 다음 TeamB 캐릭터 턴 시작
	 * @param PlayerChar  플레이어가 선택한 TeamA 캐릭터
	 * @param Targets     타겟 배열
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void ConfirmAndExecuteQueue(ASCCharacterBase* PlayerChar, const TArray<AActor*>& Targets);

	/** 플레이어 턴 건너뛰기 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void SkipCurrentActorTurn();

	// ── AI 턴 완료 알림 (GameMode에서 호출) ──
	/**
	 * GameMode가 AI 행동 완료 후 호출
	 * → 다음 플레이어 턴으로 전환
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void AdvanceAfterAITurn();

	// ── 조회 ────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ESCBattlePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetTurnNumber() const { return TurnNumber; }

	/** 현재 행동 중인 캐릭터 (AI 턴일 때만 유효, 플레이어 턴은 nullptr) */
	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ASCCharacterBase* GetCurrentAIActor() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	USCTeamState* GetTeamState(ESCTeam Team) const;

	/** 플레이어 턴 여부 */
	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	bool IsPlayerTurn() const { return bIsPlayerTurn; }

	/** 팀의 살아있는 캐릭터 목록 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	TArray<ASCCharacterBase*> GetAliveCharsOfTeam(ESCTeam Team) const;

	// ── 델리게이트 ───────────────────────────
	/** 플레이어 턴 시작 — PlayerController가 바인딩 */
	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnPlayerTurnStarted OnPlayerTurnStarted;

	/** AI 캐릭터 턴 시작 — GameMode가 바인딩 */
	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnCharacterTurnBegin OnCharacterTurnBegin;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnBattlePhaseChanged OnBattlePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnTurnChanged OnTurnChanged;

	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnBattleEnd OnBattleEnd;

private:
	void SetPhase(ESCBattlePhase NewPhase);
	bool CheckBattleEnd();

	ASCCharacterBase* FindNextAliveTeamBChar();

	/** 모든 TeamA/B 캐릭터 (생존 여부 관계없이) */
	UPROPERTY()
	TArray<TWeakObjectPtr<ASCCharacterBase>> TeamACharsAll;

	/** TeamB 고정 턴 순서 */
	UPROPERTY()
	TArray<TWeakObjectPtr<ASCCharacterBase>> TeamBOrder;

	/** 현재 행동할 TeamB 인덱스 */
	UPROPERTY()
	int32 TeamBIndex = 0;

	/** 현재 플레이어 턴 여부 */
	UPROPERTY()
	bool bIsPlayerTurn = true;

	UPROPERTY()
	ESCBattlePhase CurrentPhase = ESCBattlePhase::Idle;

	UPROPERTY()
	int32 TurnNumber = 1;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamAState;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamBState;
};
