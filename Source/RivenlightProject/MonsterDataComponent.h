#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterTableRows.h"
#include "MonsterDataComponent.generated.h"

// 몬스터 블루프린트에 붙이는 부품.
// 시작할 때 DT_MonsterList에서 자기 줄(MonsterRowId)을 읽고, 그 줄이 가리키는 DT_MonsterStatus 줄도 읽어서 적용한다.
// 게임 중에 표 값이 바뀌면(RefreshIntervalSeconds마다 확인) 바뀐 값을 다시 적용한다.
//
// 적용하는 값
//   DT_MonsterList  : MoveSpeedCmPerSec -> 최대 걷기 속도
//                     RotationRateYawDegPerSec -> 회전 속도
//                     SightRadiusCm, LoseSightRadiusCm -> 시야
//   DT_MonsterStatus: Hp -> 블루프린트 변수 MaxHP (처음 적용할 때만 HP도 같은 값으로)
//                     StaggerPerDamage -> 블루프린트 변수 GroggyPerDamage
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RIVENLIGHTPROJECT_API UMonsterDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMonsterDataComponent();

	// 읽을 표 (DT_MonsterList)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	TObjectPtr<UDataTable> MonsterListTable;

	// 이 몬스터의 줄 이름 (예: MOB_TALIS_RIBBON)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	FName MonsterRowId;

	// 표 값이 바뀌었는지 확인하는 간격 (초). 0이면 시작할 때 한 번만 읽는다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float RefreshIntervalSeconds = 0.5f;

	// 적용했을 때 화면에 한 줄 보여 준다 (확인용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	bool bShowDebugMessage = true;

	// 표에서 값을 읽어 적용한다. bForce가 거짓이면 지난번과 값이 같을 때는 아무것도 하지 않는다
	UFUNCTION(BlueprintCallable, Category = "Monster Data")
	bool ApplyFromTable(bool bForce);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTimerHandle RefreshTimer;
	bool bHasApplied = false;
	FMonsterListRow LastRow;
	bool bHasStatus = false;
	FMonsterStatusRow LastStatus;

	void Refresh();
	static bool SameRow(const FMonsterListRow& A, const FMonsterListRow& B);
	static bool SameStatus(const FMonsterStatusRow& A, const FMonsterStatusRow& B);
	static UDataTable* FindStatusTable();
	// 블루프린트 변수를 이름으로 찾아 숫자를 넣는다. 변수가 없으면 거짓
	static bool SetNumberVariable(UObject* Object, FName VariableName, double Value);
};
