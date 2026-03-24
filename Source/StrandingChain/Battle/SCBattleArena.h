// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleArena.h
// 역할: 전투 씬 전체 레이아웃 관리
//       - TeamA(좌) 4슬롯 / TeamB(우) 4슬롯
//       - 고정 사이드뷰 카메라
//       - 캐릭터 배치 및 슬롯 조회

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCBattleTypes.h"
#include "SCBattleArena.generated.h"

class ASCBattleSlot;
class ASCCharacterBase;
class UCameraComponent;
class USpringArmComponent;

UCLASS(Blueprintable)
class STRANDINGCHAIN_API ASCBattleArena : public AActor
{
	GENERATED_BODY()

public:
	ASCBattleArena();

	// ── 카메라 ──────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Camera")
	TObjectPtr<UCameraComponent> Camera;

	// ── 슬롯 배치 설정 ──────────────────────
	/** TeamA 슬롯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Arena")
	TSubclassOf<ASCBattleSlot> SlotClass;

	/** TeamA 슬롯 4개 월드 오프셋 (아레나 원점 기준, 좌측) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Arena")
	TArray<FVector> TeamASlotOffsets;

	/** TeamB 슬롯 4개 월드 오프셋 (아레나 원점 기준, 우측) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SC|Arena")
	TArray<FVector> TeamBSlotOffsets;

	// ── 런타임 슬롯 ─────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Arena")
	TArray<TObjectPtr<ASCBattleSlot>> TeamASlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SC|Arena")
	TArray<TObjectPtr<ASCBattleSlot>> TeamBSlots;

	// ── 슬롯 스폰 및 조회 ────────────────────
	/** BeginPlay에서 자동 호출 — 슬롯 스폰 */
	UFUNCTION(BlueprintCallable, Category = "SC|Arena")
	void SpawnSlots();

	/**
	 * 캐릭터 배열을 받아 해당 팀 슬롯에 순서대로 배치
	 * @param Characters 배치할 캐릭터 목록 (최대 4명)
	 * @param Team       배치할 팀
	 */
	UFUNCTION(BlueprintCallable, Category = "SC|Arena")
	void PlaceCharactersToSlots(const TArray<ASCCharacterBase*>& Characters, ESCTeam Team);

	/** 팀/인덱스로 슬롯 반환 */
	UFUNCTION(BlueprintPure, Category = "SC|Arena")
	ASCBattleSlot* GetSlot(ESCTeam Team, int32 Index) const;

	/** 팀 슬롯 전체 반환 */
	UFUNCTION(BlueprintPure, Category = "SC|Arena")
	TArray<ASCBattleSlot*> GetTeamSlots(ESCTeam Team) const;

protected:
	virtual void BeginPlay() override;

private:
	void SetDefaultSlotOffsets();
};
