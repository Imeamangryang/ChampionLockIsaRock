#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "Engine/DataTable.h"
#include "TestEditorValidatorBase.generated.h"

UCLASS()
class CHAMPIONLOCKISAROCK_API UTestEditorValidatorBase : public UEditorValidatorBase
{
	GENERATED_BODY()
	
public:

	// 현재 에셋이 이 Validator의 검사 대상인지 판단하는 함수
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	// 실제 데이터 검증 로직이 실행되는 함수
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	
private:
	// Data Table 검증을 위한 커스텀 함수
	static bool IsPropertyMissing(const FProperty* Property, const void* ValuePtr);
	static bool IsStatOutOfRange(const FProperty* Property, const void* ValuePtr, FString& OutErrorMessage);
};
