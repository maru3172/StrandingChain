// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattlePlayerController.h
// 역할: 배틀 씬 전용 PlayerController
//       - WASD 좌우 카메라 이동
//       - 마우스 휠 줌인/아웃
//       - 아군 캐릭터 클릭 → 스킬 패널 표시

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SCBattlePlayerController.generated.h"

class ASCBattleArena;
class ASCCharacterBase;
class USCSkillPanelWidget;

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASCBattlePlayerController();

	// ── 카메라 이동 설정 ─────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float CameraMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomStep = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomMin = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SC|Camera")
	float ZoomMax = 2500.f;

	// ── 스킬 패널 ────────────────────────────
	/** Blueprint에서 WBP_SkillPanel 클래스 지정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|UI")
	TSubclassOf<USCSkillPanelWidget> SkillPanelClass;

	/** 현재 열린 스킬 패널 인스턴스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TObjectPtr<USCSkillPanelWidget> SkillPanelWidget;

	/** 현재 선택된 아군 캐릭터 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|UI")
	TWeakObjectPtr<ASCCharacterBase> SelectedCharacter;

	/** 패널 닫기 (외부에서도 호출 가능) */
	UFUNCTION(BlueprintCallable, Category = "SC|UI")
	void CloseSkillPanel();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	TWeakObjectPtr<ASCBattleArena> ArenaRef;
	float MoveAxisValue = 0.f;

	void ZoomIn();
	void ZoomOut();
	void OnCameraMoveAxis(float Value);
	void OnMouseLeftClick();

	void TrySelectCharacter();
	void OpenSkillPanel(ASCCharacterBase* InCharacter);
};
