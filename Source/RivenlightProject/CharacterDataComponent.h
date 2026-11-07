#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TuningTableRows.h"
#include "CharacterDataComponent.generated.h"

// 플레이어 블루프린트에 붙이는 부품.
// 시작할 때 DT_Character에서 자기 줄(CharacterRowId)을 읽어서 적용한다.
// 게임 중에 표 값이 바뀌면(RefreshIntervalSeconds마다 확인) 바뀐 값을 다시 적용한다.
//
// 적용하는 값
//   CameraFov           -> 카메라 컴포넌트의 시야각
//   CameraArmLengthCm   -> 스프링 암의 길이
//   MaxHp               -> 블루프린트 변수 HP (처음 적용할 때만)
// 아직 적용하지 않는 값: 대시 거리/시간/쿨타임, 보스에게 주는 피해 (블루프린트 노드 안에 숫자가 있음)
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RIVENLIGHTPROJECT_API UCharacterDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterDataComponent();

	// 읽을 표 (DT_Character)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TObjectPtr<UDataTable> CharacterTable;

	// 이 캐릭터의 줄 이름 (예: CHAR_PLAYER_KATANA)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	FName CharacterRowId;

	// 표 값이 바뀌었는지 확인하는 간격 (초). 0이면 시작할 때 한 번만 읽는다
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	float RefreshIntervalSeconds = 0.5f;

	// 적용했을 때 화면에 한 줄 보여 준다 (확인용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	bool bShowDebugMessage = true;

	// 카메라 시야각과 시간 배율이 바뀔 때마다 출력 로그에 한 줄 남긴다 (진단용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	bool bLogFovChanges = true;

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	bool ApplyFromTable(bool bForce);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTimerHandle RefreshTimer;
	bool bHasApplied = false;
	FCharacterRow LastRow;
	float LastWatchedFov = -1.f;
	float LastWatchedDilation = -1.f;
	float LastWatchedArm = -1.f;

	void Refresh();
	static bool SameRow(const FCharacterRow& A, const FCharacterRow& B);
	static bool SetNumberVariable(UObject* Object, FName VariableName, double Value);
};
