#include "MonsterDataComponent.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogMonsterData, Log, All);

UMonsterDataComponent::UMonsterDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterDataComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyFromTable(true);

	if (RefreshIntervalSeconds > 0.f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RefreshTimer, this, &UMonsterDataComponent::Refresh, RefreshIntervalSeconds, true);
	}
}

void UMonsterDataComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void UMonsterDataComponent::Refresh()
{
	ApplyFromTable(false);
}

bool UMonsterDataComponent::SameRow(const FMonsterListRow& A, const FMonsterListRow& B)
{
	return A.MoveSpeedCmPerSec == B.MoveSpeedCmPerSec
		&& A.RotationRateYawDegPerSec == B.RotationRateYawDegPerSec
		&& A.SightRadiusCm == B.SightRadiusCm
		&& A.LoseSightRadiusCm == B.LoseSightRadiusCm
		&& A.StatusId == B.StatusId;
}

bool UMonsterDataComponent::SameStatus(const FMonsterStatusRow& A, const FMonsterStatusRow& B)
{
	return A.Hp == B.Hp && A.StaggerPerDamage == B.StaggerPerDamage;
}

UDataTable* UMonsterDataComponent::FindStatusTable()
{
	static TWeakObjectPtr<UDataTable> Cache;
	if (Cache.IsValid())
	{
		return Cache.Get();
	}
	const TCHAR* Paths[] = {
		TEXT("/Game/DataTable/Monster/DT_MonsterStatus.DT_MonsterStatus"),
		TEXT("/Game/DataTable/DT_MonsterStatus.DT_MonsterStatus") };
	for (const TCHAR* Path : Paths)
	{
		if (UDataTable* Table = LoadObject<UDataTable>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			Cache = Table;
			return Table;
		}
	}
	return nullptr;
}

bool UMonsterDataComponent::SetNumberVariable(UObject* Object, FName VariableName, double Value)
{
	if (!Object)
	{
		return false;
	}
	FProperty* Prop = Object->GetClass()->FindPropertyByName(VariableName);
	FNumericProperty* Num = Prop ? CastField<FNumericProperty>(Prop) : nullptr;
	if (!Num)
	{
		return false;
	}
	void* Ptr = Num->ContainerPtrToValuePtr<void>(Object);
	if (Num->IsFloatingPoint())
	{
		Num->SetFloatingPointPropertyValue(Ptr, Value);
	}
	else
	{
		Num->SetIntPropertyValue(Ptr, static_cast<int64>(Value));
	}
	return true;
}

bool UMonsterDataComponent::ApplyFromTable(bool bForce)
{
	if (!MonsterListTable || MonsterRowId.IsNone())
	{
		UE_LOG(LogMonsterData, Warning, TEXT("MonsterListTable 또는 MonsterRowId가 비어 있음 (%s)"), *GetNameSafe(GetOwner()));
		return false;
	}

	const FMonsterListRow* Row = MonsterListTable->FindRow<FMonsterListRow>(MonsterRowId, TEXT("UMonsterDataComponent::ApplyFromTable"), false);
	if (!Row)
	{
		UE_LOG(LogMonsterData, Warning, TEXT("표에서 줄을 찾지 못함: %s"), *MonsterRowId.ToString());
		return false;
	}

	// 스테이터스 줄 (없어도 걷기 속도 등은 적용한다)
	const FMonsterStatusRow* Status = nullptr;
	if (!Row->StatusId.IsNone())
	{
		if (UDataTable* StatusTable = FindStatusTable())
		{
			Status = StatusTable->FindRow<FMonsterStatusRow>(Row->StatusId, TEXT("UMonsterDataComponent::ApplyFromTable"), false);
		}
	}

	const bool bFirst = !bHasApplied;
	const bool bRowSame = bHasApplied && SameRow(*Row, LastRow);
	const bool bStatusSame = (Status == nullptr && !bHasStatus) || (Status != nullptr && bHasStatus && SameStatus(*Status, LastStatus));
	if (!bForce && bRowSame && bStatusSame)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = Row->MoveSpeedCmPerSec;
			Move->RotationRate.Yaw = Row->RotationRateYawDegPerSec;
		}
	}

	if (UAIPerceptionComponent* Perception = Owner->FindComponentByClass<UAIPerceptionComponent>())
	{
		if (UAISenseConfig_Sight* Sight = Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>())))
		{
			Sight->SightRadius = Row->SightRadiusCm;
			Sight->LoseSightRadius = Row->LoseSightRadiusCm;
			Perception->RequestStimuliListenerUpdate();
		}
	}

	FString StatusMsg;
	if (Status)
	{
		const bool bMax = SetNumberVariable(Owner, TEXT("MaxHP"), Status->Hp);
		bool bHp = false;
		if (bFirst)
		{
			bHp = SetNumberVariable(Owner, TEXT("HP"), Status->Hp); // 현재 체력은 처음에만 맞춘다
		}
		const bool bGroggy = SetNumberVariable(Owner, TEXT("GroggyPerDamage"), Status->StaggerPerDamage);
		StatusMsg = FString::Printf(TEXT(" Hp=%.0f(MaxHP%s HP%s) GroggyPerDamage=%.3f(%s)"),
			Status->Hp, bMax ? TEXT("O") : TEXT("X"), bFirst ? (bHp ? TEXT("O") : TEXT("X")) : TEXT("-"),
			Status->StaggerPerDamage, bGroggy ? TEXT("O") : TEXT("X"));
		LastStatus = *Status;
		bHasStatus = true;
	}
	else
	{
		bHasStatus = false;
	}

	LastRow = *Row;
	bHasApplied = true;

	const FString Msg = FString::Printf(TEXT("[MonsterData] %s: MoveSpeed=%.0f Rotation=%.0f Sight=%.0f/%.0f%s"),
		*MonsterRowId.ToString(), Row->MoveSpeedCmPerSec, Row->RotationRateYawDegPerSec, Row->SightRadiusCm, Row->LoseSightRadiusCm, *StatusMsg);
	UE_LOG(LogMonsterData, Log, TEXT("%s"), *Msg);
	if (bShowDebugMessage && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
	}
	return true;
}
