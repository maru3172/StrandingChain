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

	// ── 페이즈 전환 ─────────────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void AdvanceToNextPhase();

	// ── 스킬 큐 확정 및 실행 ────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void ConfirmAndExecuteQueue(const TArray<AActor*>& Targets);

	// ── 현재 액터 턴 건너뜀 (플레이어 스킵 / AI 우회용) ──
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void SkipCurrentActorTurn();

	// ── 조회 ────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ESCBattlePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetTurnNumber() const { return TurnNumber; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	ASCCharacterBase* GetCurrentActor() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	USCTeamState* GetTeamState(ESCTeam Team) const;

	/** 팀의 살아있는 캐릭터 목록 반환 (PlayerController / GameMode에서 타겟 구성에 사용) */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	TArray<ASCCharacterBase*> GetAliveCharsOfTeam(ESCTeam Team) const;

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

	UPROPERTY()
	TArray<TWeakObjectPtr<ASCCharacterBase>> TurnOrder;

	UPROPERTY()
	int32 CurrentActorIndex = 0;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamAState;

	UPROPERTY()
	TObjectPtr<USCTeamState> TeamBState;
};
