#include "TableValueLibrary.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "MonsterTableRows.h"
#include "TuningTableRows.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogTableValue, Log, All);

namespace
{
	// 표 에셋을 찾는다. 임포트한 폴더가 달라도 후보 경로를 차례로 시도한다.
	UDataTable* FindTable(TWeakObjectPtr<UDataTable>& Cache, const TArray<FString>& Paths)
	{
		if (Cache.IsValid())
		{
			return Cache.Get();
		}
		for (const FString& Path : Paths)
		{
			if (UDataTable* Table = LoadObject<UDataTable>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet))
			{
				Cache = Table;
				return Table;
			}
		}
		return nullptr;
	}

	// 한 번만 경고한다 (매 프레임 부르는 노드에서 로그가 넘치지 않게)
	void WarnOnce(const FString& Key, const FString& Msg)
	{
		static TSet<FString> Seen;
		if (!Seen.Contains(Key))
		{
			Seen.Add(Key);
			UE_LOG(LogTableValue, Warning, TEXT("%s"), *Msg);
		}
	}

	bool ReadNumber(const UScriptStruct* Struct, const void* RowData, FName Column, double& Out)
	{
		const FProperty* Prop = Struct->FindPropertyByName(Column);
		if (!Prop)
		{
			return false;
		}
		const void* Value = Prop->ContainerPtrToValuePtr<void>(RowData);
		if (const FNumericProperty* Num = CastField<FNumericProperty>(Prop))
		{
			Out = Num->IsFloatingPoint() ? Num->GetFloatingPointPropertyValue(Value) : static_cast<double>(Num->GetSignedIntPropertyValue(Value));
			return true;
		}
		if (const FBoolProperty* Bool = CastField<FBoolProperty>(Prop))
		{
			Out = Bool->GetPropertyValue(Value) ? 1.0 : 0.0;
			return true;
		}
		return false;
	}

	float ReadFromTable(UDataTable* Table, const TCHAR* TableLabel, FName RowName, FName Column, float DefaultValue)
	{
		if (!Table)
		{
			WarnOnce(FString::Printf(TEXT("notable:%s"), TableLabel), FString::Printf(TEXT("%s 표를 찾지 못해 기본값을 씀"), TableLabel));
			return DefaultValue;
		}
		const uint8* RowData = Table->FindRowUnchecked(RowName);
		if (!RowData)
		{
			WarnOnce(FString::Printf(TEXT("norow:%s:%s"), TableLabel, *RowName.ToString()), FString::Printf(TEXT("%s 표에 줄 %s이 없어 기본값을 씀"), TableLabel, *RowName.ToString()));
			return DefaultValue;
		}
		double Out = 0.0;
		if (!ReadNumber(Table->GetRowStruct(), RowData, Column, Out))
		{
			WarnOnce(FString::Printf(TEXT("nocol:%s:%s"), TableLabel, *Column.ToString()), FString::Printf(TEXT("%s 표에 숫자 칸 %s이 없어 기본값을 씀"), TableLabel, *Column.ToString()));
			return DefaultValue;
		}
		const float Value = static_cast<float>(Out);

		// 확인용 로그: 줄·칸마다 값이 처음 읽힐 때와 바뀔 때만 한 줄 남긴다 (매 프레임 부르는 노드에서 넘치지 않게)
		// 게임 월드가 새로 만들어질 때(PIE를 다시 켤 때)마다 기록을 지워서, 시작할 때마다 처음 읽은 값이 다시 찍힌다
		static TMap<FString, float> LastLogged;
		static bool bResetBound = false;
		if (!bResetBound)
		{
			bResetBound = true;
			FWorldDelegates::OnPostWorldInitialization.AddLambda([](UWorld* World, const UWorld::InitializationValues)
			{
				if (World && World->IsGameWorld())
				{
					LastLogged.Empty();
				}
			});
		}
		const FString LogKey = FString::Printf(TEXT("%s:%s:%s"), TableLabel, *RowName.ToString(), *Column.ToString());
		const float* Prev = LastLogged.Find(LogKey);
		if (!Prev)
		{
			UE_LOG(LogTableValue, Log, TEXT("[표값] %s %s.%s = %.4f (기본값 %.4f)"), TableLabel, *RowName.ToString(), *Column.ToString(), Value, DefaultValue);
			LastLogged.Add(LogKey, Value);
		}
		else if (!FMath::IsNearlyEqual(*Prev, Value))
		{
			UE_LOG(LogTableValue, Log, TEXT("[표값] %s %s.%s 바뀜: %.4f -> %.4f (기본값 %.4f)"), TableLabel, *RowName.ToString(), *Column.ToString(), *Prev, Value, DefaultValue);
			LastLogged[LogKey] = Value;
		}
		return Value;
	}
}

float UTableValueLibrary::GetTuning(FName RowName, float DefaultValue)
{
	static TWeakObjectPtr<UDataTable> Cache;
	// 표 이름을 DT_Tuning에서 DT_System으로 바꾸는 중이라 새 이름을 먼저, 옛 이름을 뒤에 둔다
	UDataTable* Table = FindTable(Cache, {
		TEXT("/Game/DataTable/Monster/DT_System.DT_System"),
		TEXT("/Game/DataTable/DT_System.DT_System"),
		TEXT("/Game/DataTable/System/DT_System.DT_System"),
		TEXT("/Game/DataTable/DT_Tuning.DT_Tuning"),
		TEXT("/Game/DataTable/Tuning/DT_Tuning.DT_Tuning"),
		TEXT("/Game/DataTable/Monster/DT_Tuning.DT_Tuning") });
	return ReadFromTable(Table, TEXT("DT_System"), RowName, FName(TEXT("Value")), DefaultValue);
}

float UTableValueLibrary::GetMonsterSkill(FName RowName, float DefaultValue)
{
	static TWeakObjectPtr<UDataTable> Cache;
	UDataTable* Table = FindTable(Cache, {
		TEXT("/Game/DataTable/Monster/DT_MonsterSkill.DT_MonsterSkill"),
		TEXT("/Game/DataTable/DT_MonsterSkill.DT_MonsterSkill"),
		TEXT("/Game/DataTable/Skill/DT_MonsterSkill.DT_MonsterSkill") });
	return ReadFromTable(Table, TEXT("DT_MonsterSkill"), RowName, FName(TEXT("Value")), DefaultValue);
}

float UTableValueLibrary::GetCharacter(FName RowName, FName Column, float DefaultValue)
{
	static TWeakObjectPtr<UDataTable> Cache;
	UDataTable* Table = FindTable(Cache, {
		TEXT("/Game/DataTable/DT_Character.DT_Character"),
		TEXT("/Game/DataTable/Character/DT_Character.DT_Character"),
		TEXT("/Game/DataTable/Monster/DT_Character.DT_Character") });
	return ReadFromTable(Table, TEXT("DT_Character"), RowName, Column, DefaultValue);
}

float UTableValueLibrary::GetMonsterStatus(FName RowName, FName Column, float DefaultValue)
{
	static TWeakObjectPtr<UDataTable> Cache;
	UDataTable* Table = FindTable(Cache, {
		TEXT("/Game/DataTable/Monster/DT_MonsterStatus.DT_MonsterStatus"),
		TEXT("/Game/DataTable/DT_MonsterStatus.DT_MonsterStatus") });
	return ReadFromTable(Table, TEXT("DT_MonsterStatus"), RowName, Column, DefaultValue);
}

float UTableValueLibrary::GetMonsterList(FName RowName, FName Column, float DefaultValue)
{
	static TWeakObjectPtr<UDataTable> Cache;
	UDataTable* Table = FindTable(Cache, {
		TEXT("/Game/DataTable/Monster/DT_MonsterList.DT_MonsterList"),
		TEXT("/Game/DataTable/DT_MonsterList.DT_MonsterList") });
	return ReadFromTable(Table, TEXT("DT_MonsterList"), RowName, Column, DefaultValue);
}
