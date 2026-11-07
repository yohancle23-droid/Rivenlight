#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TuningTableRows.generated.h"

// DT_Character: 플레이어 같은 캐릭터의 핵심 수치. 줄 하나가 캐릭터 하나.
USTRUCT(BlueprintType)
struct FCharacterRow : public FTableRowBase
{
	GENERATED_BODY()

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHp = 0.f;

	// 보스에게 주는 피해 (한 타)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamageToBoss = 0.f;

	// 대시 거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDistanceCm = 0.f;

	// 대시 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashDurationSeconds = 0.f;

	// 대시 쿨타임 (초). 기획서 값이고 아직 게임에 구현되지 않음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashCooldownSeconds = 0.f;

	// 카메라 시야각 (도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraFov = 90.f;

	// 카메라 붐 길이 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraArmLengthCm = 0.f;

	// 줌 최소 / 최대 붐 길이 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraArmMinCm = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraArmMaxCm = 0.f;
};

// DT_Tuning: 연출, 타격감, 스킬 타이밍 같은 낱개 설정값. 한 줄 = 설정 하나.
// 줄 이름이 곧 설정 이름이다 (예: BREAK_HITSTOP_TIME_DILATION).
USTRUCT(BlueprintType)
struct FTuningValueRow : public FTableRowBase
{
	GENERATED_BODY()

	// 묶음 이름 (Dash, ArmorBreak, BreakFx, PlayerHit, BossHit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName Category;

	// 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float Value = 0.f;

	// 단위 (sec, cm, deg, x, frame 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	FString Unit;

	// Confirmed(확정) / Temp(임시) / NeedsCheck(확인 필요)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName Status;

	// 이 값으로 정한 이유
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString Reason;

	// 이 값을 쓰는 곳 (블루프린트 › 그래프 › 노드)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString Where;
};
