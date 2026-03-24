// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleTypes.h
// 공용 열거형 / 구조체 정의 — 런타임 전용

#pragma once

#include "CoreMinimal.h"
#include "SCBattleTypes.generated.h"

// ─────────────────────────────────────────
// 포지션
// ─────────────────────────────────────────
UENUM(BlueprintType)
enum class ESCPosition : uint8
{
	Dealer  UMETA(DisplayName = "딜러"),
	Buffer  UMETA(DisplayName = "버퍼"),
	Tanker  UMETA(DisplayName = "탱커")
};

// ─────────────────────────────────────────
// 팀 구분
// ─────────────────────────────────────────
UENUM(BlueprintType)
enum class ESCTeam : uint8
{
	TeamA UMETA(DisplayName = "팀A"),
	TeamB UMETA(DisplayName = "팀B")
};

// ─────────────────────────────────────────
// 스킬 코스트 방향 (소모 / 획득)
// ─────────────────────────────────────────
UENUM(BlueprintType)
enum class ESCSkillCostType : uint8
{
	Consume UMETA(DisplayName = "코스트 소모"),
	Generate UMETA(DisplayName = "코스트 획득")
};

// ─────────────────────────────────────────
// 배틀 페이즈
// ─────────────────────────────────────────
UENUM(BlueprintType)
enum class ESCBattlePhase : uint8
{
	Idle			UMETA(DisplayName = "대기"),
	DrawPhase		UMETA(DisplayName = "스킬 드로우"),
	PlayerQueuePhase UMETA(DisplayName = "스킬 큐 편성"),
	ExecutePhase	UMETA(DisplayName = "스킬 실행"),
	ResultPhase		UMETA(DisplayName = "결산"),
	BattleEnd		UMETA(DisplayName = "배틀 종료")
};

// ─────────────────────────────────────────
// 스킬 정적 데이터
// ─────────────────────────────────────────
USTRUCT(BlueprintType)
struct STRANDINGCHAIN_API FSCSkillData
{
	GENERATED_BODY()

	/** 스킬 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillID = NAME_None;

	/** 스킬 표시명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FText SkillName;

	/** 코스트 방향 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	ESCSkillCostType CostType = ESCSkillCostType::Consume;

	/** 코스트 절댓값 (소모 또는 획득량) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = 0))
	int32 CostAmount = 1;
};

// ─────────────────────────────────────────
// 큐에 등록된 스킬 실행 요청
// ─────────────────────────────────────────
USTRUCT(BlueprintType)
struct STRANDINGCHAIN_API FSCSkillQueueEntry
{
	GENERATED_BODY()

	/** 시전 캐릭터 (약한 참조) */
	UPROPERTY(BlueprintReadOnly, Category = "Queue")
	TWeakObjectPtr<AActor> Caster;

	/** 스킬 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "Queue")
	FSCSkillData SkillData;
};
