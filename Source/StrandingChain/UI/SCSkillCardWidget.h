// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/UI/SCSkillCardWidget.h
// 역할: 드로우된 스킬 1장 카드
//       Blueprint(WBP_SkillCard)에서 텍스트/버튼 배치 후 InitCard() 호출

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Battle/SCBattleTypes.h"
#include "SCSkillCardWidget.generated.h"

class USCSkillBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCardClicked, USCSkillBase*, Skill, int32, DrawnIndex);

UCLASS(Abstract, Blueprintable)
class STRANDINGCHAIN_API USCSkillCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── 데이터 ──────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "SC|Card")
	TObjectPtr<USCSkillBase> SkillRef;

	/** DrawnSkills 배열 인덱스 (0~2) */
	UPROPERTY(BlueprintReadOnly, Category = "SC|Card")
	int32 DrawnIndex = 0;

	// ── 델리게이트 ───────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "SC|Card")
	FOnSkillCardClicked OnSkillCardClicked;

	// ── C++ API ──────────────────────────────
	/** 스킬 카드 초기화 — Panel에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "SC|Card")
	void InitCard(USCSkillBase* InSkill, int32 InDrawnIndex);

	/** Blueprint 버튼 OnClicked에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "SC|Card")
	void NotifyClicked();

	// ── Blueprint 이벤트 ─────────────────────
	/** 카드 내용 갱신 — Blueprint에서 텍스트/이미지 세팅 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SC|Card")
	void OnCardInitialized(const FSCSkillData& SkillData);

protected:
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
