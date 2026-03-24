// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCBattleAutoSetup.cpp

#include "SCBattleAutoSetup.h"
#include "SCBattleArena.h"
#include "SCBattleSlot.h"
#include "SCPlaceholderCharacter.h"
#include "SCTurnManager.h"
#include "Skills/SCSkill_BasicAttack.h"
#include "Skills/SCSkill_HeavyAttack.h"
#include "Skills/SCSkill_CostGenerate.h"
#include "Skills/SCSkill_Heal.h"
#include "Skills/SCSkill_Shield.h"
#include "StrandingChain.h"
#include "Engine/GameInstance.h"

ASCBattleAutoSetup::ASCBattleAutoSetup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetDefaultConfigs();
}

void ASCBattleAutoSetup::SetDefaultConfigs()
{
	// TeamA — 딜러(빨강), 딜러(주황), 버퍼(초록), 탱커(파랑)
	TeamAConfig =
	{
		{ ESCPosition::Dealer, FLinearColor(1.f, 0.2f, 0.2f), 80  },
		{ ESCPosition::Dealer, FLinearColor(1.f, 0.6f, 0.1f), 80  },
		{ ESCPosition::Buffer, FLinearColor(0.2f, 1.f, 0.2f), 90  },
		{ ESCPosition::Tanker, FLinearColor(0.2f, 0.4f, 1.f), 150 }
	};

	// TeamB — 동일 구성, 색상만 다름 (적팀: 어두운 계열)
	TeamBConfig =
	{
		{ ESCPosition::Dealer, FLinearColor(0.8f, 0.1f, 0.1f), 80  },
		{ ESCPosition::Dealer, FLinearColor(0.8f, 0.4f, 0.0f), 80  },
		{ ESCPosition::Buffer, FLinearColor(0.1f, 0.7f, 0.1f), 90  },
		{ ESCPosition::Tanker, FLinearColor(0.1f, 0.2f, 0.8f), 150 }
	};
}

void ASCBattleAutoSetup::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(ArenaClass))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] ArenaClass가 설정되지 않음. "
			     "BP_BattleAutoSetup의 ArenaClass에 BP_BattleArena를 지정하세요."));
		return;
	}
	if (!IsValid(CharacterClass))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] CharacterClass가 설정되지 않음. "
			     "BP_BattleAutoSetup의 CharacterClass에 BP_PlaceholderCharacter를 지정하세요."));
		return;
	}

	SpawnArena();
	SpawnCharacters(TeamAConfig, ESCTeam::TeamA, TeamACharacters);
	SpawnCharacters(TeamBConfig, ESCTeam::TeamB, TeamBCharacters);
	StartBattle();
}

void ASCBattleAutoSetup::SpawnArena()
{
	UWorld* World = GetWorld();
	if (!IsValid(World)) { return; }

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedArena = World->SpawnActor<ASCBattleArena>(
		ArenaClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (!IsValid(SpawnedArena))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] 아레나 스폰 실패."));
		return;
	}
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleAutoSetup] 아레나 스폰 완료: %s"), *SpawnedArena->GetName());
}

void ASCBattleAutoSetup::SpawnCharacters(
	const TArray<FSCCharacterSlotConfig>& Configs,
	ESCTeam Team,
	TArray<TObjectPtr<ASCPlaceholderCharacter>>& OutChars)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(SpawnedArena)) { return; }

	OutChars.Empty();

	for (int32 i = 0; i < Configs.Num(); ++i)
	{
		// 슬롯 위치에서 스폰 — Z는 캡슐이 바닥 위에 서도록 고정
		ASCBattleSlot* Slot = SpawnedArena->GetSlot(Team, i);
		FVector SpawnLoc = IsValid(Slot)
			? Slot->GetSpawnWorldLocation()
			: FVector((Team == ESCTeam::TeamA ? -300.f : 300.f), i * -200.f, 0.f);

		// 캡슐 HalfHeight=90 → 바닥 위에 정확히 서도록 Z 보정
		SpawnLoc.Z = 92.f;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASCPlaceholderCharacter* Char = World->SpawnActor<ASCPlaceholderCharacter>(
			CharacterClass, SpawnLoc, FRotator::ZeroRotator, Params);

		if (!IsValid(Char))
		{
			UE_LOG(LogStrandingChain, Error,
				TEXT("[SCBattleAutoSetup] 캐릭터 스폰 실패. Team=%d, Slot=%d"),
				static_cast<int32>(Team), i);
			continue;
		}

		// 설정 적용
		const FSCCharacterSlotConfig& Cfg = Configs[i];
		Char->Team       = Team;
		Char->Position   = Cfg.Position;
		Char->DebugColor = Cfg.DebugColor;
		Char->MaxHP      = Cfg.MaxHP;

		// 더미 스킬 5개 등록 (포지션별 구성)
		Char->SkillClasses.Empty();
		Char->SkillClasses.Add(USCSkill_BasicAttack::StaticClass());
		Char->SkillClasses.Add(USCSkill_HeavyAttack::StaticClass());
		Char->SkillClasses.Add(USCSkill_CostGenerate::StaticClass());
		if (Cfg.Position == ESCPosition::Buffer || Cfg.Position == ESCPosition::Tanker)
		{
			Char->SkillClasses.Add(USCSkill_Heal::StaticClass());
		}
		else
		{
			Char->SkillClasses.Add(USCSkill_HeavyAttack::StaticClass());
		}
		Char->SkillClasses.Add(USCSkill_Shield::StaticClass());

		// 팀B는 플레이어팀을 바라보도록 180도 회전
		if (Team == ESCTeam::TeamB)
		{
			Char->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		}

		// 슬롯에 등록 + Z 강제 고정 (PlaceCharacter가 슬롯 Z로 덮어쓰므로 재보정)
		if (IsValid(Slot))
		{
			Slot->PlaceCharacter(Char);
		}

		// 캡슐 HalfHeight=90 기준 지면 위 정확한 위치로 재보정
		FVector FixedLoc = Char->GetActorLocation();
		FixedLoc.Z = 92.f;
		Char->SetActorLocation(FixedLoc);

		OutChars.Add(Char);

		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattleAutoSetup] 캐릭터 스폰: %s Team=%d Slot=%d Pos=%d HP=%d"),
			*Char->GetName(),
			static_cast<int32>(Team), i,
			static_cast<int32>(Cfg.Position),
			Cfg.MaxHP);
	}
}

void ASCBattleAutoSetup::StartBattle()
{
	UGameInstance* GI = GetGameInstance();
	if (!IsValid(GI))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] GameInstance 유효하지 않음."));
		return;
	}

	USCTurnManager* TM = GI->GetSubsystem<USCTurnManager>();
	if (!IsValid(TM))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] SCTurnManager 서브시스템을 찾을 수 없음."));
		return;
	}

	// TArray<TObjectPtr<>> → TArray<ASCCharacterBase*> 변환
	TArray<ASCCharacterBase*> A, B;
	for (auto& C : TeamACharacters) { if (IsValid(C)) A.Add(C.Get()); }
	for (auto& C : TeamBCharacters) { if (IsValid(C)) B.Add(C.Get()); }

	if (A.Num() != 4 || B.Num() != 4)
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] 스폰된 캐릭터 수 부족. A=%d B=%d"), A.Num(), B.Num());
		return;
	}

	TM->StartBattle(A, B);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[SCBattleAutoSetup] 배틀 시작 완료."));
}
