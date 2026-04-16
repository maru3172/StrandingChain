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
	TeamAConfig =
	{
		{ ESCPosition::Dealer, FLinearColor(1.f, 0.2f, 0.2f), 80  },
		{ ESCPosition::Dealer, FLinearColor(1.f, 0.6f, 0.1f), 80  },
		{ ESCPosition::Buffer, FLinearColor(0.2f, 1.f, 0.2f), 90  },
		{ ESCPosition::Tanker, FLinearColor(0.2f, 0.4f, 1.f), 150 }
	};

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
			TEXT("[SCBattleAutoSetup] ArenaClass 미설정."));
		return;
	}
	if (!IsValid(CharacterClass))
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] CharacterClass 미설정."));
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
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedArena = World->SpawnActor<ASCBattleArena>(
		ArenaClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (!IsValid(SpawnedArena))
	{
		UE_LOG(LogStrandingChain, Error, TEXT("[SCBattleAutoSetup] 아레나 스폰 실패."));
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
		ASCBattleSlot* Slot = SpawnedArena->GetSlot(Team, i);
		FVector SpawnLoc = IsValid(Slot)
			? Slot->GetSpawnWorldLocation()
			: FVector((Team == ESCTeam::TeamA ? -300.f : 300.f), i * -200.f, 92.f);
		SpawnLoc.Z = 92.f;

		const FSCCharacterSlotConfig& Cfg = Configs[i];

		// ── SpawnActorDeferred: BeginPlay 이전에 프로퍼티 설정 ──────────────
		ASCPlaceholderCharacter* Char = World->SpawnActorDeferred<ASCPlaceholderCharacter>(
			CharacterClass,
			FTransform(FRotator::ZeroRotator, SpawnLoc),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (!IsValid(Char))
		{
			UE_LOG(LogStrandingChain, Error,
				TEXT("[SCBattleAutoSetup] 캐릭터 SpawnDeferred 실패. Team=%d Slot=%d"),
				static_cast<int32>(Team), i);
			continue;
		}

		// BeginPlay 전 설정 → DebugColor·MaxHP가 올바르게 반영됨
		Char->Team       = Team;
		Char->Position   = Cfg.Position;
		Char->DebugColor = Cfg.DebugColor;
		Char->MaxHP      = Cfg.MaxHP;

		Char->SkillClasses.Empty();
		Char->SkillClasses.Add(USCSkill_BasicAttack::StaticClass());
		Char->SkillClasses.Add(USCSkill_HeavyAttack::StaticClass());
		Char->SkillClasses.Add(USCSkill_CostGenerate::StaticClass());
		if (Cfg.Position == ESCPosition::Buffer || Cfg.Position == ESCPosition::Tanker)
			Char->SkillClasses.Add(USCSkill_Heal::StaticClass());
		else
			Char->SkillClasses.Add(USCSkill_Shield::StaticClass());
		Char->SkillClasses.Add(USCSkill_Shield::StaticClass());

		// FinishSpawning → BeginPlay 호출 (이 시점에 DebugColor, MaxHP 정상 반영)
		Char->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLoc));

		// ── 포스트 스폰: 위치·회전 보정 ──────────────────────────────────
		if (IsValid(Slot))
		{
			Slot->PlaceCharacter(Char);
		}
		if (Team == ESCTeam::TeamB)
		{
			Char->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		}

		FVector FixedLoc = Char->GetActorLocation();
		FixedLoc.Z = 92.f;
		Char->SetActorLocation(FixedLoc);

		OutChars.Add(Char);

		UE_LOG(LogStrandingChain, Log,
			TEXT("[SCBattleAutoSetup] 캐릭터 스폰: %s Team=%d Slot=%d HP=%d"),
			*Char->GetName(), static_cast<int32>(Team), i, Cfg.MaxHP);
	}
}

void ASCBattleAutoSetup::StartBattle()
{
	UGameInstance* GI = GetGameInstance();
	if (!IsValid(GI))
	{
		UE_LOG(LogStrandingChain, Error, TEXT("[SCBattleAutoSetup] GameInstance 유효하지 않음."));
		return;
	}

	USCTurnManager* TM = GI->GetSubsystem<USCTurnManager>();
	if (!IsValid(TM))
	{
		UE_LOG(LogStrandingChain, Error, TEXT("[SCBattleAutoSetup] SCTurnManager 없음."));
		return;
	}

	TArray<ASCCharacterBase*> A, B;
	for (auto& C : TeamACharacters) { if (IsValid(C)) A.Add(C.Get()); }
	for (auto& C : TeamBCharacters) { if (IsValid(C)) B.Add(C.Get()); }

	if (A.Num() != 4 || B.Num() != 4)
	{
		UE_LOG(LogStrandingChain, Error,
			TEXT("[SCBattleAutoSetup] 캐릭터 수 부족. A=%d B=%d"), A.Num(), B.Num());
		return;
	}

	TM->StartBattle(A, B);
	UE_LOG(LogStrandingChain, Log, TEXT("[SCBattleAutoSetup] 배틀 시작 완료."));
}
