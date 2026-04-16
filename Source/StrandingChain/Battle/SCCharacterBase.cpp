// Copyright StrandingChain. All Rights Reserved.
// File: Source/StrandingChain/Battle/SCCharacterBase.cpp

#include "SCCharacterBase.h"
#include "SCSkillBase.h"
#include "StrandingChain.h"
#include "UI/SCHPBarWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/TextRenderComponent.h"

static constexpr int32 SC_MAX_SKILL_SLOTS = 5;
static constexpr int32 SC_DRAW_COUNT      = 3;
static constexpr int32 SC_MAX_QUEUE       = 3;

ASCCharacterBase::ASCCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(GetMesh());
	HPBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	HPBarComponent->SetWidgetSpace(EWidgetSpace::World);
	HPBarComponent->SetDrawSize(FVector2D(150.f, 20.f));
	HPBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HPTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPTextComponent"));
	HPTextComponent->SetupAttachment(GetRootComponent());
	HPTextComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	HPTextComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	HPTextComponent->SetHorizontalAlignment(EHTA_Center);
	HPTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	HPTextComponent->SetWorldSize(28.f);
	HPTextComponent->SetTextRenderColor(FColor::White);
	HPTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASCCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	TempShield = 0;
	InitializeSkillInstances();
	InitHPBar();
}

void ASCCharacterBase::InitializeSkillInstances()
{
	SkillInstances.Empty();
	for (TSubclassOf<USCSkillBase> SkillClass : SkillClasses)
	{
		if (!IsValid(SkillClass)) { continue; }
		USCSkillBase* Instance = NewObject<USCSkillBase>(this, SkillClass);
		if (ensureMsgf(IsValid(Instance),
			TEXT("[%s] 스킬 인스턴스 생성 실패: %s"), *GetName(), *SkillClass->GetName()))
		{
			SkillInstances.Add(Instance);
		}
	}
}

void ASCCharacterBase::BeginTurn()
{
	if (IsDead()) { return; }

	bHasActedThisTurn = false;
	SkillQueue.Empty();
	DrawnSkills.Empty();
	TempShield = 0;  // 턴 시작 시 방어막 초기화

	TArray<int32> Indices;
	for (int32 i = 0; i < SkillInstances.Num(); ++i) { Indices.Add(i); }

	int32 DrawCount = FMath::Min(SC_DRAW_COUNT, Indices.Num());
	for (int32 i = 0; i < DrawCount; ++i)
	{
		int32 RandIdx = FMath::RandRange(i, Indices.Num() - 1);
		Indices.Swap(i, RandIdx);
		USCSkillBase* Drawn = SkillInstances[Indices[i]];
		if (IsValid(Drawn)) { DrawnSkills.Add(Drawn); }
	}

	OnTurnBegin();
}

bool ASCCharacterBase::EnqueueSkill(int32 DrawnIndex)
{
	if (!DrawnSkills.IsValidIndex(DrawnIndex)) { return false; }
	if (SkillQueue.Num() >= SC_MAX_QUEUE) { return false; }
	USCSkillBase* Skill = DrawnSkills[DrawnIndex];
	if (!IsValid(Skill)) { return false; }
	SkillQueue.Add(Skill);
	return true;
}

void ASCCharacterBase::DequeueSkill(int32 QueueIndex)
{
	if (!SkillQueue.IsValidIndex(QueueIndex)) { return; }
	SkillQueue.RemoveAt(QueueIndex);
}

void ASCCharacterBase::ExecuteSkillQueue(const TArray<AActor*>& Targets)
{
	if (bHasActedThisTurn || SkillQueue.IsEmpty()) { return; }

	for (TObjectPtr<USCSkillBase> Skill : SkillQueue)
	{
		if (!IsValid(Skill)) { continue; }
		Skill->Execute(this, Targets);
	}

	bHasActedThisTurn = true;
	SkillQueue.Empty();

	OnSkillQueueFinished.Broadcast();
	OnSkillQueueExecuted();
}

void ASCCharacterBase::ApplyDamage(int32 Amount)
{
	if (IsDead() || Amount <= 0) { return; }

	// TempShield 우선 흡수
	if (TempShield > 0)
	{
		int32 Absorbed = FMath::Min(TempShield, Amount);
		TempShield -= Absorbed;
		Amount    -= Absorbed;
		UE_LOG(LogStrandingChain, Log,
			TEXT("[%s] 방어막 %d 흡수, 남은방어막=%d"), *GetName(), Absorbed, TempShield);
	}

	if (Amount <= 0) { RefreshHPBar(); return; }

	CurrentHP = FMath::Max(0, CurrentHP - Amount);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 피해: -%d, HP=%d/%d"), *GetName(), Amount, CurrentHP, MaxHP);

	RefreshHPBar();

	if (IsDead())
	{
		UE_LOG(LogStrandingChain, Log, TEXT("[%s] 사망."), *GetName());
		OnDeath();
	}
}

void ASCCharacterBase::ApplyHeal(int32 Amount)
{
	if (IsDead() || Amount <= 0) { return; }
	const int32 Before = CurrentHP;
	CurrentHP = FMath::Min(MaxHP, CurrentHP + Amount);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 회복: +%d, HP=%d/%d"), *GetName(), CurrentHP - Before, CurrentHP, MaxHP);
	RefreshHPBar();
}

void ASCCharacterBase::AddShield(int32 Amount)
{
	if (Amount <= 0) { return; }
	TempShield += Amount;
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 방어막 +%d, 총방어막=%d"), *GetName(), Amount, TempShield);
	RefreshHPBar();
}

void ASCCharacterBase::InitHPBar()
{
	if (!IsValid(HPBarComponent)) { return; }
	if (!IsValid(HPBarWidgetClass))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] HPBarWidgetClass 미설정. BP에서 WBP_HPBar 지정 필요."), *GetName());
	}
	else
	{
		HPBarComponent->SetWidgetClass(HPBarWidgetClass);
		HPBarComponent->InitWidget();
	}
	RefreshHPBar();
}

void ASCCharacterBase::RefreshHPBar()
{
	if (IsValid(HPTextComponent))
	{
		FString HPStr;
		if (TempShield > 0)
			HPStr = FString::Printf(TEXT("HP %d/%d [+%d]"), CurrentHP, MaxHP, TempShield);
		else
			HPStr = FString::Printf(TEXT("HP %d/%d"), CurrentHP, MaxHP);

		HPTextComponent->SetText(FText::FromString(HPStr));

		const float Ratio = (MaxHP > 0)
			? static_cast<float>(CurrentHP) / static_cast<float>(MaxHP) : 0.f;
		FColor TextColor = (Ratio > 0.5f) ? FColor::Green
						 : (Ratio > 0.25f) ? FColor::Yellow
						 : FColor::Red;
		HPTextComponent->SetTextRenderColor(TextColor);
	}

	if (!IsValid(HPBarComponent)) { return; }
	USCHPBarWidget* Widget = Cast<USCHPBarWidget>(HPBarComponent->GetWidget());
	if (!IsValid(Widget)) { return; }
	Widget->UpdateHP(CurrentHP, MaxHP);
}
