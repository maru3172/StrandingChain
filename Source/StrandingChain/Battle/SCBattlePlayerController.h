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

	// ── 스킬 패널 ────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<USCSkillPanelWidget> SkillPanelClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<USCSkillPanelWidget> SkillPanelWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;

	// ── 배틀 액션 ────────────────────────────
	/**
	 * 큐 위젯 슬롯 내용을 읽어 캐릭터 SkillQueue에 등록하고
	 * TurnManager->ConfirmAndExecuteQueue 호출.
	 * WBP_SkillQueue의 "실행" 버튼에 연결.
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void ConfirmSkillQueue();

	/**
	 * 현재 캐릭터 턴을 건너뜀.
	 * WBP_SkillQueue의 "건너뛰기" 버튼에 연결.
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Battle")
	void SkipCurrentTurn();

	UFUNCTION(BlueprintCallable, Category = "SC|UI")
	void CloseSkillPanel();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	TWeakObjectPtr<ASCBattleArena> ArenaRef;
	TWeakObjectPtr<USCTurnManager> TurnManagerRef;
	float MoveAxisValue = 0.f;

	void ZoomIn();
	void ZoomOut();
	void OnCameraMoveAxis(float Value);
	void OnMouseLeftClick();

	void TrySelectCharacter();
	void OpenSkillPanel(ASCCharacterBase* InCharacter);

	// ── TurnManager 델리게이트 콜백 ──────────
	// 파라미터명을 InCharacter로 사용 — AController::Character 섀도잉 방지 (UE5.7 UHT 규칙)
	UFUNCTION()
	void OnCharacterTurnBeginHandler(ASCCharacterBase* InCharacter);
};
