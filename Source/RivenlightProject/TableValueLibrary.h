#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TableValueLibrary.generated.h"

// 블루프린트 노드 안의 숫자를 표 값으로 바꿀 때 쓰는 "표에서 값 가져오기" 노드 모음.
// 노드 이름 그대로 검색: Get Tuning / Get Character / Get Monster Status / Get Monster List
// 표나 줄이나 칸이 없으면 DefaultValue를 돌려준다. 그래서 숫자를 지우고 이 노드를 꽂을 때
// DefaultValue에 원래 숫자를 적어 두면 표가 없어도 지금과 똑같이 동작한다.
UCLASS()
class RIVENLIGHTPROJECT_API UTableValueLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// DT_Tuning의 Value 칸. RowName은 줄 이름 (예: BREAK_HITSTOP_DELAY)
	UFUNCTION(BlueprintPure, Category = "Table Value", meta = (DisplayName = "Get Tuning"))
	static float GetTuning(FName RowName, float DefaultValue);

	// DT_Character의 숫자 칸. Column은 칸 이름 (예: DashDistanceCm)
	UFUNCTION(BlueprintPure, Category = "Table Value", meta = (DisplayName = "Get Character"))
	static float GetCharacter(FName RowName, FName Column, float DefaultValue);

	// DT_MonsterStatus의 숫자 칸 (예: Hp, StaggerPerDamage)
	UFUNCTION(BlueprintPure, Category = "Table Value", meta = (DisplayName = "Get Monster Status"))
	static float GetMonsterStatus(FName RowName, FName Column, float DefaultValue);

	// DT_MonsterList의 숫자 칸 (예: HitReactCooldownSeconds)
	UFUNCTION(BlueprintPure, Category = "Table Value", meta = (DisplayName = "Get Monster List"))
	static float GetMonsterList(FName RowName, FName Column, float DefaultValue);
};
