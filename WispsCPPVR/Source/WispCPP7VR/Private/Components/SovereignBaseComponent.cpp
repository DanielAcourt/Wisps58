// Copyright (c) 2013-2026 Daniel Acourt. Version 36.4.1. Licensed under GPLv3 (See LICENSE). Last Updated: 2026-08-25

#include "Components/SovereignBaseComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"

USovereignBaseComponent::USovereignBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USovereignBaseComponent::BeginPlay()
{
	Super::BeginPlay();
}

FText USovereignBaseComponent::GetInspectorDisplayName_Implementation() const
{
	FString ClassName = GetClass()->GetName();
	if (ClassName.StartsWith(TEXT("Sovereign")))
	{
		ClassName.RemoveFromStart(TEXT("Sovereign"));
	}
	if (ClassName.EndsWith(TEXT("Component")))
	{
		ClassName.RemoveFromEnd(TEXT("Component"));
	}
	return FText::FromString(ClassName);
}

FString USovereignBaseComponent::GetInspectorCategory_Implementation() const
{
	FText DisplayName = GetInspectorDisplayName_Implementation();
	return DisplayName.ToString();
}

TMap<FString, FString> USovereignBaseComponent::GetInspectorProperties_Implementation() const
{
	TMap<FString, FString> Props;

	for (TFieldIterator<FProperty> PropIt(GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (!Property)
		{
			continue;
		}

		// Reflect UPROPERTY fields with BlueprintVisible or EditAnywhere or non-transient status
		if (Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible) || !Property->HasAnyPropertyFlags(CPF_Transient))
		{
			FString PropName = Property->GetName();
			FString ValueStr;

			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(this);

			if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
			{
				if (NumericProperty->IsFloatingPoint())
				{
					ValueStr = FString::SanitizeFloat(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
				}
				else if (NumericProperty->IsInteger())
				{
					ValueStr = FString::Printf(TEXT("%lld"), NumericProperty->GetSignedIntPropertyValue(ValuePtr));
				}
			}
			else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				ValueStr = BoolProperty->GetPropertyValue(ValuePtr) ? TEXT("True") : TEXT("False");
			}
			else if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
			{
				ValueStr = StrProperty->GetPropertyValue<FString>(ValuePtr);
			}
			else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
			{
				ValueStr = NameProperty->GetPropertyValue<FName>(ValuePtr).ToString();
			}
			else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
			{
				ValueStr = TextProperty->GetPropertyValue<FText>(ValuePtr).ToString();
			}
			else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
			{
				UEnum* EnumDef = EnumProperty->GetEnum();
				FNumericProperty* UnderlyingProp = EnumProperty->GetUnderlyingProperty();
				int64 IntVal = UnderlyingProp ? UnderlyingProp->GetSignedIntPropertyValue(ValuePtr) : 0;
				ValueStr = EnumDef ? EnumDef->GetNameStringByValue(IntVal) : FString::Printf(TEXT("%lld"), IntVal);
			}
			else if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
			{
				if (ByteProperty->Enum)
				{
					ValueStr = ByteProperty->Enum->GetNameStringByValue(ByteProperty->GetPropertyValue(ValuePtr));
				}
				else
				{
					ValueStr = FString::FromInt(ByteProperty->GetPropertyValue(ValuePtr));
				}
			}
			else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				StructProperty->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);
			}

			if (!ValueStr.IsEmpty())
			{
				Props.Add(PropName, ValueStr);
			}
		}
	}

	return Props;
}

FString USovereignBaseComponent::GetInspectorDataJson_Implementation() const
{
	TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();

	JsonObj->SetStringField(TEXT("DisplayName"), GetInspectorDisplayName_Implementation().ToString());
	JsonObj->SetStringField(TEXT("Category"), GetInspectorCategory_Implementation());

	TMap<FString, FString> Props = GetInspectorProperties_Implementation();
	TSharedPtr<FJsonObject> PropsObj = MakeShared<FJsonObject>();
	for (const auto& KVP : Props)
	{
		PropsObj->SetStringField(KVP.Key, KVP.Value);
	}
	JsonObj->SetObjectField(TEXT("Properties"), PropsObj);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

	return OutputString;
}

TSubclassOf<UUserWidget> USovereignBaseComponent::GetInspectorWidgetClass_Implementation() const
{
	return nullptr;
}

void USovereignBaseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
