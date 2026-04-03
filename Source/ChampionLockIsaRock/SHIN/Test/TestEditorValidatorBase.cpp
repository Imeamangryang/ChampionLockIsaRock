#include "SHIN/Test/TestEditorValidatorBase.h"
#include "Engine/DataTable.h"
#include "UObject/UnrealType.h"

// DataTable만 검증 대상으로 설정
bool UTestEditorValidatorBase::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA(UDataTable::StaticClass());
}

EDataValidationResult UTestEditorValidatorBase::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UDataTable* Table = Cast<UDataTable>(InAsset);
	if (!Table || !Table->GetRowStruct())
	{
		return EDataValidationResult::NotValidated;
	}

	bool bAllValid = true;
	const UScriptStruct* RowStruct = Table->GetRowStruct();

	// 모든 Row 순회
	for (const auto& Pair : Table->GetRowMap())
	{
		const FName RowName = Pair.Key;
		const uint8* RowData = reinterpret_cast<const uint8*>(Pair.Value);

		// Row 자체가 비어있는 경우
		if (!RowData)
		{
			bAllValid = false;
			AssetFails(
				InAsset,
				FText::FromString(
					FString::Printf(TEXT("[%s] 행(Row) 데이터가 비어 있습니다."), *RowName.ToString())
				)
			);
			continue;
		}

		int32 MissingCount = 0;
		TArray<FString> MissingFields;

		// Property 단위 순회 (리플렉션 기반)
		for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
		{
			const FProperty* Property = *It;
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			// ✅ 누락 검증
			if (IsPropertyMissing(Property, ValuePtr))
			{
				++MissingCount;
				MissingFields.Add(Property->GetName());
			}
			
			// ✅ 값 범위 검증
			FString RangeErrorMessage;
			if (IsStatOutOfRange(Property, ValuePtr, RangeErrorMessage))
			{
				bAllValid = false;
				AssetFails(
					InAsset,
					FText::FromString(
						FString::Printf(
							TEXT("[%s] %s"),
							*RowName.ToString(),
							*RangeErrorMessage
						)
					)
				);
			}
		}

		// 누락된 필드가 하나라도 있으면 실패 처리
		if (MissingCount > 0)
		{
			bAllValid = false;

			AssetFails(
				InAsset,
				FText::FromString(
					FString::Printf(
						TEXT("[%s] 누락된 필드가 %d개 있습니다: %s"),
						*RowName.ToString(),
						MissingCount,
						*FString::Join(MissingFields, TEXT(", "))
					)
				)
			);
		}
	}

	// 전체 결과 반환
	if (bAllValid)
	{
		AssetPasses(InAsset);
		return EDataValidationResult::Valid;
	}

	return EDataValidationResult::Invalid;
}

// Property가 비어있는지 검사 (Completeness Validation)
bool UTestEditorValidatorBase::IsPropertyMissing(const FProperty* Property, const void* ValuePtr)
{
	if (!Property || !ValuePtr)
	{
		return true;
	}

	if (const FStrProperty* StrProp = CastField<FStrProperty>(Property))
	{
		return StrProp->GetPropertyValue(ValuePtr).IsEmpty();
	}

	if (const FNameProperty* NameProp = CastField<FNameProperty>(Property))
	{
		return NameProp->GetPropertyValue(ValuePtr).IsNone();
	}

	if (const FTextProperty* TextProp = CastField<FTextProperty>(Property))
	{
		return TextProp->GetPropertyValue(ValuePtr).IsEmpty();
	}

	if (const FSoftObjectProperty* SoftObjProp = CastField<FSoftObjectProperty>(Property))
	{
		const FSoftObjectPtr SoftObject = SoftObjProp->GetPropertyValue(ValuePtr);
		return SoftObject.IsNull();
	}

	if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
	{
		return ObjProp->GetObjectPropertyValue(ValuePtr) == nullptr;
	}

	if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
	{
		FScriptArrayHelper Helper(ArrayProp, ValuePtr);
		return Helper.Num() == 0;
	}
	
	return false;
}

// 수치형 Property의 범위 검사 (Value Validation)
bool UTestEditorValidatorBase::IsStatOutOfRange(const FProperty* Property, const void* ValuePtr, FString& OutErrorMessage)
{
	if (!Property || !ValuePtr)
	{
		return false;
	}

	const FNumericProperty* NumProp = CastField<FNumericProperty>(Property);
	if (!NumProp)
	{
		return false;
	}

	const FString PropertyName = Property->GetName();

	// 정수형 처리
	if (NumProp->IsInteger())
	{
		const int64 Value = NumProp->GetSignedIntPropertyValue(ValuePtr);

		// 공통 규칙
		if (Value == -1)
		{
			OutErrorMessage = FString::Printf(TEXT("%s 값이 -1입니다."), *PropertyName);
			return true;
		}

		// 필드별 범위 검사
		if (PropertyName == TEXT("Cost") && (Value < 0 || Value > 5))
		{
			OutErrorMessage = FString::Printf(TEXT("%s 값이 허용 범위를 벗어났습니다. (현재값: %lld, 허용범위: 0~5)"), *PropertyName, Value);
			return true;
		}
	}

	// 실수형 처리
	if (NumProp->IsFloatingPoint())
	{
		const double Value = NumProp->GetFloatingPointPropertyValue(ValuePtr);

		if (FMath::IsNearlyEqual(Value, -1.0))
		{
			OutErrorMessage = FString::Printf(TEXT("%s 값이 -1입니다."), *PropertyName);
			return true;
		}

		if (PropertyName == TEXT("AttackSpeed") && (Value < 0.0 || Value > 5.0))
		{
			OutErrorMessage = FString::Printf(TEXT("%s 값이 허용 범위를 벗어났습니다. (현재값: %.2f, 허용범위: 0.0~5.0)"), *PropertyName, Value);
			return true;
		}
	}

	return false;
}