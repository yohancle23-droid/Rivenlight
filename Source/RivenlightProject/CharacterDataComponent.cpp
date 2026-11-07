#include "CharacterDataComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogCharacterData, Log, All);

UCharacterDataComponent::UCharacterDataComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // 진단 로그용. 매 프레임 값 두 개만 비교한다
}

void UCharacterDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bLogFovChanges)
	{
		return;
	}
	const double RealSec = FPlatformTime::Seconds();
	const float Dilation = UGameplayStatics::GetGlobalTimeDilation(this);
	if (LastWatchedDilation < 0.f || !FMath::IsNearlyEqual(Dilation, LastWatchedDilation, 0.0001f))
	{
		UE_LOG(LogCharacterData, Log, TEXT("[시간배율감시] %.4f -> %.4f (실시간 %.3f초)"), LastWatchedDilation, Dilation, RealSec);
		LastWatchedDilation = Dilation;
	}
	if (AActor* Owner = GetOwner())
	{
		if (UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>())
		{
			const float Fov = Camera->FieldOfView;
			if (LastWatchedFov < 0.f || !FMath::IsNearlyEqual(Fov, LastWatchedFov, 0.01f))
			{
				UE_LOG(LogCharacterData, Log, TEXT("[시야각감시] %.2f -> %.2f (실시간 %.3f초, 시간배율 %.4f)"), LastWatchedFov, Fov, RealSec, Dilation);
				LastWatchedFov = Fov;
			}
		}
	}
}

void UCharacterDataComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyFromTable(true);

	if (RefreshIntervalSeconds > 0.f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RefreshTimer, this, &UCharacterDataComponent::Refresh, RefreshIntervalSeconds, true);
	}
}

void UCharacterDataComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void UCharacterDataComponent::Refresh()
{
	ApplyFromTable(false);
}

bool UCharacterDataComponent::SameRow(const FCharacterRow& A, const FCharacterRow& B)
{
	return A.MaxHp == B.MaxHp && A.CameraFov == B.CameraFov && A.CameraArmLengthCm == B.CameraArmLengthCm;
}

bool UCharacterDataComponent::SetNumberVariable(UObject* Object, FName VariableName, double Value)
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

bool UCharacterDataComponent::ApplyFromTable(bool bForce)
{
	if (!CharacterTable || CharacterRowId.IsNone())
	{
		UE_LOG(LogCharacterData, Warning, TEXT("CharacterTable 또는 CharacterRowId가 비어 있음 (%s)"), *GetNameSafe(GetOwner()));
		return false;
	}

	const FCharacterRow* Row = CharacterTable->FindRow<FCharacterRow>(CharacterRowId, TEXT("UCharacterDataComponent::ApplyFromTable"), false);
	if (!Row)
	{
		UE_LOG(LogCharacterData, Warning, TEXT("표에서 줄을 찾지 못함: %s"), *CharacterRowId.ToString());
		return false;
	}

	if (!bForce && bHasApplied && SameRow(*Row, LastRow))
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	const bool bFirst = !bHasApplied;

	bool bFov = false;
	if (UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>())
	{
		Camera->SetFieldOfView(Row->CameraFov);
		bFov = true;
	}

	bool bArm = false;
	if (USpringArmComponent* Arm = Owner->FindComponentByClass<USpringArmComponent>())
	{
		Arm->TargetArmLength = Row->CameraArmLengthCm;
		bArm = true;
	}

	bool bHp = false;
	if (bFirst)
	{
		bHp = SetNumberVariable(Owner, TEXT("HP"), Row->MaxHp); // 현재 체력은 처음에만 맞춘다
	}

	LastRow = *Row;
	bHasApplied = true;

	const FString Msg = FString::Printf(TEXT("[CharacterData] %s: Fov=%.0f(%s) Arm=%.0f(%s) Hp=%.0f(%s)"),
		*CharacterRowId.ToString(), Row->CameraFov, bFov ? TEXT("O") : TEXT("X"),
		Row->CameraArmLengthCm, bArm ? TEXT("O") : TEXT("X"),
		Row->MaxHp, bFirst ? (bHp ? TEXT("O") : TEXT("X")) : TEXT("-"));
	UE_LOG(LogCharacterData, Log, TEXT("%s"), *Msg);
	if (bShowDebugMessage && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Msg);
	}
	return true;
}
