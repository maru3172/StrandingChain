// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattlePlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SCBattlePlayerController.generated.h"

class ASCBattleArena;
class ASCCharacterBase;
class USCSkillPanelWidget;
class USCTurnManager;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyTargetSelected, int32, EnemySlotIndex);

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASCBattlePlayerController();

	// ── 카메라 ───────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float CameraMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomStep = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomMin = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomMax = 2500.f;

	// ── 위젯 클래스 ──────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<USCSkillPanelWidget> SkillPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<UUserWidget> EnemyTargetWidgetClass;

	// ── 위젯 인스턴스 ────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<USCSkillPanelWidget> SkillPanelWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<UUserWidget> EnemyTargetWidget;

	/** 현재 스킬 패널에서 선택된 TeamA 캐릭터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;

	// ── 적 선택 델리게이트 ───────────────────
	UPROPERTY(BlueprintAssignable, Category = "SC|Battle")
	FOnEnemyTargetSelected OnEnemyTargetSelected;

	// ── 배틀 액션 ────────────────────────────

	/**
	 * 번호 버튼 클릭 → 적 선택 저장
	 * WBP_EnemyTargetPanel 버튼에 연결
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void SelectEnemyTarget(int32 EnemySlotIndex);

	/**
	 * 실행 버튼 클릭
	 * SelectedCharacter + SelectedEnemyIndex 기반으로 스킬 실행
	 * → TurnManager::ConfirmAndExecuteQueue(PlayerChar, Targets) 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void ConfirmSkillQueue();

	/** 건너뛰기 버튼 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void SkipCurrentTurn();

	UFUNCTION(BlueprintCallable, Category = "SC|UI")
	void CloseSkillPanel();

	// ── 조회 ─────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	TArray<ASCCharacterBase*> GetAliveEnemies() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetSelectedEnemyIndex() const { return SelectedEnemyIndex; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	bool IsEnemySelected() const { return SelectedEnemyIndex >= 0; }

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetTeamACost() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetTeamBCost() const;

	UFUNCTION(BlueprintPure, Category = "SC|Battle")
	int32 GetCurrentTurnNumber() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	TWeakObjectPtr<ASCBattleArena>  ArenaRef;
	TWeakObjectPtr<USCTurnManager>  TurnManagerRef;
	float MoveAxisValue = 0.f;
	int32 SelectedEnemyIndex = -1;

	void ZoomIn();
	void ZoomOut();
	void OnCameraMoveAxis(float Value);
	void OnMouseLeftClick();
	void TrySelectCharacter();
	void OpenSkillPanel(ASCCharacterBase* InCharacter);
	void ShowEnemyTargetPanel();
	void HideEnemyTargetPanel();
	void ResetEnemySelection();
	bool IsMouseOverWidget(UUserWidget* InWidget) const;

	/**
	 * OnPlayerTurnStarted 수신
	 * 특정 캐릭터 자동선택 없이 플레이어가 직접 클릭해서 선택하도록 대기
	 */
	UFUNCTION()
	void OnPlayerTurnStartedHandler();
};
