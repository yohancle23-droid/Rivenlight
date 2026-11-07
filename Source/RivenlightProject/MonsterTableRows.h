#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterTableRows.generated.h"

// 몬스터 데이터 테이블 행 구조체.
// 현업분 방식(몬스터 목록 -> 스테이터스 -> 스킬셋 -> 스킬을 ID로 타고 들어감)을 따른다.
// 엑셀(CSV)에서 값을 고치고 에디터에서 바로 확인하는 것이 목적이다.
// 단위는 이름에 적는다: Seconds = 초, Cm = 센티미터, Deg = 도.

// DT_MonsterList: 몬스터 한 종류가 한 행. 다른 표는 ID로 연결한다.
USTRUCT(BlueprintType)
struct FMonsterListRow : public FTableRowBase
{
	GENERATED_BODY()

	// Normal 또는 Boss
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName MonsterType;

	// DT_MonsterStatus의 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName StatusId;

	// 이 몬스터가 쓰는 스킬 묶음 ID. DT_MonsterSkill에서 SkillSetId 칸이 같은 줄들이 이 몬스터의 스킬이다 (없으면 None)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName SkillSetId;

	// 최대 걷기 속도 (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float MoveSpeedCmPerSec = 0.f;

	// 회전 속도 (도/초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float RotationRateYawDegPerSec = 0.f;

	// 플레이어를 쫓아가는가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bChasePlayer = true;

	// 시각 인식 반경 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SightRadiusCm = 0.f;

	// 시각 상실 반경 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LoseSightRadiusCm = 0.f;

	// 맞았을 때 반응하는 간격 (초). 0이면 제한 없음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	float HitReactCooldownSeconds = 0.f;
};

// DT_MonsterStatus: 몬스터의 체력, 공격력, 그로기(스태거) 수치.
USTRUCT(BlueprintType)
struct FMonsterStatusRow : public FTableRowBase
{
	GENERATED_BODY()

	// 플레이어에게 주는 피해 (한 타)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Attack = 0.f;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Hp = 0.f;

	// 받는 피해 배수 (1.0 = 그대로). 갑옷 파괴 뒤 배수는 별도 표에서 다룬다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageTakenMult = 1.f;

	// 그로기(스턴) 지속 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger")
	float StaggerDurationSeconds = 0.f;

	// 그로기 게이지 최대값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger")
	float StaggerMax = 0.f;

	// 플레이어 한 타당 쌓이는 게이지 비율 (플레이어 1타 100 x 0.05 = 5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stagger")
	float StaggerPerDamage = 0.f;
};

// DT_MonsterSkill: 몬스터 스킬의 낱개 설정값. 한 줄 = 스킬 설정 하나 (줄 이름이 곧 설정 이름, 예: DASH_MIN_DISTANCE_CM).
// DT_MonsterList의 SkillSetId와 같은 SkillSetId를 가진 줄들이 그 몬스터의 스킬이다.
USTRUCT(BlueprintType)
struct FMonsterSkillRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이 줄이 속한 스킬 묶음 (DT_MonsterList의 SkillSetId와 같은 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName SkillSetId;

	// 이 줄이 속한 스킬 (예: SKILL_TALIS_DASH)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName SkillId;

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
