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

	// 체력바 위젯 컴포넌트 — WBP 설정 시 사용
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(GetMesh());
	HPBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	HPBarComponent->SetWidgetSpace(EWidgetSpace::World);
	HPBarComponent->SetDrawSize(FVector2D(150.f, 20.f));
	HPBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// HP 텍스트 — WBP 없이 즉시 표시되는 3D 텍스트
	HPTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPTextComponent"));
	HPTextComponent->SetupAttachment(GetRootComponent());
	HPTextComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	// 사이드뷰 카메라(-Y 방향)를 바라보도록 회전
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
	InitializeSkillInstances();
	InitHPBar();
}

void ASCCharacterBase::InitializeSkillInstances()
{
	SkillInstances.Empty();
	for (TSubclassOf<USCSkillBase> SkillClass : SkillClasses)
	{
		if (!IsValid(SkillClass))
		{
			UE_LOG(LogStrandingChain, Warning,
				TEXT("[%s] InitializeSkillInstances: 유효하지 않은 SkillClass 발견, 건너뜀."),
				*GetName());
			continue;
		}
		USCSkillBase* Instance = NewObject<USCSkillBase>(this, SkillClass);
		if (ensureMsgf(IsValid(Instance),
			TEXT("[%s] 스킬 인스턴스 생성 실패: %s"), *GetName(), *SkillClass->GetName()))
		{
			SkillInstances.Add(Instance);
		}
	}
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 스킬 초기화 완료: %d개"), *GetName(), SkillInstances.Num());
}

void ASCCharacterBase::BeginTurn()
{
	if (IsDead())
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] BeginTurn 호출됐으나 이미 사망 상태."), *GetName());
		return;
	}

	bHasActedThisTurn = false;
	SkillQueue.Empty();
	DrawnSkills.Empty();

	// 보유 스킬 중 SC_DRAW_COUNT개 랜덤 드로우 (중복 없음)
	TArray<int32> Indices;
	for (int32 i = 0; i < SkillInstances.Num(); ++i) { Indices.Add(i); }

	int32 DrawCount = FMath::Min(SC_DRAW_COUNT, Indices.Num());
	for (int32 i = 0; i < DrawCount; ++i)
	{
		int32 RandIdx = FMath::RandRange(i, Indices.Num() - 1);
		Indices.Swap(i, RandIdx);
		USCSkillBase* Drawn = SkillInstances[Indices[i]];
		if (IsValid(Drawn))
		{
			DrawnSkills.Add(Drawn);
		}
	}

	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] BeginTurn: %d개 스킬 드로우 완료."), *GetName(), DrawnSkills.Num());

	OnTurnBegin();
}

bool ASCCharacterBase::EnqueueSkill(int32 DrawnIndex)
{
	if (!DrawnSkills.IsValidIndex(DrawnIndex))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] EnqueueSkill: 잘못된 DrawnIndex=%d (DrawnSkills.Num=%d)"),
			*GetName(), DrawnIndex, DrawnSkills.Num());
		return false;
	}
	if (SkillQueue.Num() >= SC_MAX_QUEUE)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] EnqueueSkill: 큐가 가득 참 (최대 %d개)."), *GetName(), SC_MAX_QUEUE);
		return false;
	}
	USCSkillBase* Skill = DrawnSkills[DrawnIndex];
	if (!IsValid(Skill))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] EnqueueSkill: DrawnSkills[%d]가 유효하지 않음."), *GetName(), DrawnIndex);
		return false;
	}
	SkillQueue.Add(Skill);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 스킬 큐 추가: %s (큐 크기=%d)"),
		*GetName(), *Skill->SkillData.SkillID.ToString(), SkillQueue.Num());
	return true;
}

void ASCCharacterBase::DequeueSkill(int32 QueueIndex)
{
	if (!SkillQueue.IsValidIndex(QueueIndex))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] DequeueSkill: 잘못된 QueueIndex=%d"), *GetName(), QueueIndex);
		return;
	}
	SkillQueue.RemoveAt(QueueIndex);
}

void ASCCharacterBase::ExecuteSkillQueue(const TArray<AActor*>& Targets)
{
	if (bHasActedThisTurn)
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] ExecuteSkillQueue: 이미 이번 턴에 행동함."), *GetName());
		return;
	}
	if (SkillQueue.IsEmpty())
	{
		UE_LOG(LogStrandingChain, Log,
			TEXT("[%s] ExecuteSkillQueue: 큐가 비어있음."), *GetName());
		return;
	}

	for (TObjectPtr<USCSkillBase> Skill : SkillQueue)
	{
		if (!IsValid(Skill)) { continue; }

		// 코스트 처리는 SCTurnManager에서 검증 후 여기까지 오므로
		// 여기서는 실제 효과만 실행
		Skill->Execute(this, Targets);
	}

	bHasActedThisTurn = true;
	SkillQueue.Empty();

	OnSkillQueueFinished.Broadcast();
	OnSkillQueueExecuted();
}

void ASCCharacterBase::ApplyDamage(int32 Amount)
{
	if (IsDead()) { return; }
	CurrentHP = FMath::Max(0, CurrentHP - Amount);
	UE_LOG(LogStrandingChain, Log,
		TEXT("[%s] 데미지 적용: -%d, 남은HP=%d/%d"),
		*GetName(), Amount, CurrentHP, MaxHP);

	RefreshHPBar();

	if (IsDead())
	{
		UE_LOG(LogStrandingChain, Log, TEXT("[%s] 사망."), *GetName());
		OnDeath();
	}
}

void ASCCharacterBase::InitHPBar()
{
	if (!IsValid(HPBarComponent)) { return; }
	if (!IsValid(HPBarWidgetClass))
	{
		UE_LOG(LogStrandingChain, Warning,
			TEXT("[%s] HPBarWidgetClass가 설정되지 않음. Blueprint에서 WBP_HPBar 지정 필요."),
			*GetName());
		return;
	}
	HPBarComponent->SetWidgetClass(HPBarWidgetClass);
	HPBarComponent->InitWidget();
	RefreshHPBar();
}

void ASCCharacterBase::RefreshHPBar()
{
	// TextRender HP 갱신 (즉시 표시)
	if (IsValid(HPTextComponent))
	{
		FString HPStr = FString::Printf(TEXT("HP %d / %d"), CurrentHP, MaxHP);
		HPTextComponent->SetText(FText::FromString(HPStr));

		// HP 비율에 따라 색상 변경: 높음=초록, 중간=노랑, 낮음=빨강
		float Ratio = (MaxHP > 0)
			? static_cast<float>(CurrentHP) / static_cast<float>(MaxHP)
			: 0.f;
		FColor TextColor = (Ratio > 0.5f) ? FColor::Green
						 : (Ratio > 0.25f) ? FColor::Yellow
						 : FColor::Red;
		HPTextComponent->SetTextRenderColor(TextColor);
	}

	// Widget HP 갱신 (WBP 설정 시)
	if (!IsValid(HPBarComponent)) { return; }
	USCHPBarWidget* Widget = Cast<USCHPBarWidget>(HPBarComponent->GetWidget());
	if (!IsValid(Widget)) { return; }
	Widget->UpdateHP(CurrentHP, MaxHP);
}
