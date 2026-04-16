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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillQueueFinished);

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

	/** 임시 방어막 — 다음 피해를 이 값만큼 흡수 후 소멸 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Stat")
	int32 TempShield = 0;

	// ── 스킬 ────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Skill",
		meta = (ClampMin = 0, ClampMax = 5))
	TArray<TSubclassOf<USCSkillBase>> SkillClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> SkillInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> DrawnSkills;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Skill")
	TArray<TObjectPtr<USCSkillBase>> SkillQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<UWidgetComponent> HPBarComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<USCHPBarWidget> HPBarWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<UTextRenderComponent> HPTextComponent;

	// ── 턴 상태 ─────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Turn")
	bool bHasActedThisTurn = false;

	// ── 델리게이트 ───────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "SC|Skill")
	FOnSkillQueueFinished OnSkillQueueFinished;

	// ── 인터페이스 ───────────────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Turn")
	void BeginTurn();

	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	bool EnqueueSkill(int32 DrawnIndex);

	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	void DequeueSkill(int32 QueueIndex);

	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	void ExecuteSkillQueue(const TArray<AActor*>& Targets);

	/** 피해 적용 (TempShield 우선 흡수) */
	UFUNCTION(BlueprintCallable, Category = "SC|Stat")
	void ApplyDamage(int32 Amount);

	/** 회복 적용 */
	UFUNCTION(BlueprintCallable, Category = "SC|Stat")
	void ApplyHeal(int32 Amount);

	/** 방어막 추가 */
	UFUNCTION(BlueprintCallable, Category = "SC|Stat")
	void AddShield(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "SC|Stat")
	bool IsDead() const { return CurrentHP <= 0; }

	UFUNCTION(BlueprintCallable, Category = "SC|Skill")
	void InitializeSkillInstancesPublic() { InitializeSkillInstances(); }

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Stat")
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Turn")
	void OnTurnBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Skill")
	void OnSkillQueueExecuted();

protected:
	virtual void BeginPlay() override;

private:
	void InitializeSkillInstances();
	void InitHPBar();
	void RefreshHPBar();
};
