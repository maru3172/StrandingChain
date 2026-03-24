// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCCharacterBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCBattleTypes.h"
#include "SCCharacterBase.generated.h"

class USCSkillBase;
class USCHPBarWidget;
class UWidgetComponent;
class UTextRenderComponent;

// 스킬 큐 실행 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillQueueFinished);

/**
 * SCCharacterBase
 * - 딜러/버퍼/탱커 포지션
 * - 5개 스킬 보유, 턴당 3개 랜덤 드로우
 * - 드로우된 3개 중 원하는 순서로 큐 편성 후 실행
 * - 팀 공용 코스트 소모/획득
 */
UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API ASCCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASCCharacterBase();

	// ── 포지션 ──────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Character")
	ESCPosition Position = ESCPosition::Dealer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Character")
	ESCTeam Team = ESCTeam::TeamA;

	// ── 스탯 ────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Stat", meta = (ClampMin = 1))
	int32 MaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Stat")
	int32 CurrentHP = 100;

	// ── 스킬 ────────────────────────────────
	/** 보유 스킬 클래스 목록 (최대 5개) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Skill",
		meta = (ClampMin = 0, ClampMax = 5))
	TArray<TSubclassOf<USCSkillBase>> SkillClasses;

	/** 인스턴스화된 스킬 목록 (런타임) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> SkillInstances;

	/** 이번 턴 드로우된 스킬 (3개) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> DrawnSkills;

	/** 플레이어가 편성한 실행 큐 (최대 3개, 순서 유지) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> SkillQueue;

	/** 체력바 위젯 컴포넌트 (캐릭터 머리 위 World Space) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<UWidgetComponent> HPBarComponent;

	/** 체력바 위젯 클래스 (Blueprint에서 WBP_HPBar 지정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<USCHPBarWidget> HPBarWidgetClass;

	/** HP 텍스트 (WBP 없이도 즉시 표시되는 3D 텍스트) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<UTextRenderComponent> HPTextComponent;

	// ── 턴 상태 ─────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Turn")
	bool bHasActedThisTurn = false;

	// ── 델리게이트 ───────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "SC|Skill")
	FOnSkillQueueFinished OnSkillQueueFinished;

	// ── 인터페이스 ───────────────────────────

	/** 턴 시작 시 호출 — 스킬 드로우 수행 */
	UFUNCTION(BlueprintCallable, Category = "SC|Turn")
	void BeginTurn();

	/**
	 * 드로우된 스킬 중 하나를 큐에 추가
	 * @param DrawnIndex DrawnSkills 배열 인덱스 (0~2)
	 * @return 성공 여부 (인덱스 범위 초과 / 이미 3개 꽉 참 시 false)
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	bool EnqueueSkill(int32 DrawnIndex);

	/** 큐에서 특정 인덱스 스킬 제거 */
	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	void DequeueSkill(int32 QueueIndex);

	/** 편성된 큐를 순서대로 실행 */
	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	void ExecuteSkillQueue(const TArray<AActor*>& Targets);

	/** 데미지 적용 */
	UFUNCTION(BlueprintCallable, Category = "SC|Stat")
	void ApplyDamage(int32 Amount);

	/** 사망 여부 */
	UFUNCTION(BlueprintPure, Category = "SC|Stat")
	bool IsDead() const { return CurrentHP <= 0; }

	/** Blueprint 이벤트: 사망 시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Stat")
	void OnDeath();

	/** Blueprint 이벤트: 턴 시작 시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Turn")
	void OnTurnBegin();

	/** Blueprint 이벤트: 스킬 큐 실행 완료 시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Skill")
	void OnSkillQueueExecuted();

protected:
	virtual void BeginPlay() override;

private:
	/** 스킬 클래스로부터 인스턴스 생성 */
	void InitializeSkillInstances();

	/** 체력바 위젯 초기화 및 갱신 */
	void InitHPBar();
	void RefreshHPBar();
};
