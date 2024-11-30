// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Attributes/PEAttributeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEAttributeBase)

UPEAttributeBase::UPEAttributeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

FGameplayTag UPEAttributeBase::CreateSetByCallerTag(const FGameplayAttribute& Attribute) const
{
    if (!Attribute.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateSetByCallerTag: Invalid Gameplay Attribute provided."));
        return FGameplayTag();
    }

    // Get the attribute name
    FString AttributeName = Attribute.GetName();

    // Construct the tag string
    FString TagString = FString::Printf(TEXT("SetByCaller.%s"), *AttributeName);

    // Use the GameplayTagsManager to create the tag
    FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound=*/false);

    if (!Tag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateSetByCallerTag: Could not create or find GameplayTag '%s'."), *TagString);
    }

    return Tag;
}

TArray<FGameplayAttribute> UPEAttributeBase::GetAllAttributesFromOwner() const
{
    TArray<FGameplayAttribute> Attributes;

    // Get the owning AbilitySystemComponent
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetAllAttributesFromOwner: No owning AbilitySystemComponent found."));
        return Attributes;
    }

    // Access the attribute sets owned by the AbilitySystemComponent
    TArray<UAttributeSet*> AttributeSets;
    for (TFieldIterator<FProperty> PropIt(ASC->GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;

        // Look for the array of UAttributeSet* (it’s typically private)
        if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
        {
            if (ArrayProperty->Inner && ArrayProperty->Inner->IsA(FObjectProperty::StaticClass()))
            {
                FObjectProperty* InnerProperty = CastField<FObjectProperty>(ArrayProperty->Inner);
                if (InnerProperty && InnerProperty->PropertyClass->IsChildOf(UAttributeSet::StaticClass()))
                {
                    // Access the array and cast it to TArray<UAttributeSet*>
                    void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(ASC);
                    FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
                    for (int32 i = 0; i < ArrayHelper.Num(); ++i)
                    {
                        UObject* Object = InnerProperty->GetObjectPropertyValue(ArrayHelper.GetRawPtr(i));
                        if (UAttributeSet* AttributeSet = Cast<UAttributeSet>(Object))
                        {
                            // Only add AttributeSets that match the calling class
                            if (AttributeSet->GetClass() == GetClass())
                            {
                                AttributeSets.Add(AttributeSet);
                            }
                        }
                    }
                }
            }
        }
    }

    // Iterate over all matching AttributeSets and collect their attributes
    for (UAttributeSet* AttributeSet : AttributeSets)
    {
        if (!AttributeSet)
        {
            continue;
        }

        // Use reflection to find all attributes in the AttributeSet
        for (TFieldIterator<FProperty> PropIt(AttributeSet->GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
        {
            FProperty* Property = *PropIt;

            // Check if the property is of type FGameplayAttributeData
            if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
            {
                if (StructProp->Struct == FGameplayAttributeData::StaticStruct())
                {
                    // Create a gameplay attribute for this property
                    FGameplayAttribute Attribute(Property);
                    Attributes.Add(Attribute);
                }
            }
        }
    }

    return Attributes;
}

FGameplayAttribute UPEAttributeBase::GetAttributeByName(const FString& AttributeName) const
{
    UClass* Class = GetClass(); // Get the class of this instance

    // Iterate through all properties of the class
    for (TFieldIterator<FProperty> It(Class); It; ++It)
    {
        FProperty* Property = *It;

        // Check if the property is of type FGameplayAttributeData
        if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
        {
            if (StructProperty->Struct == FGameplayAttributeData::StaticStruct())
            {
                // Match the property name to the provided name
                if (Property->GetName().Equals(AttributeName, ESearchCase::IgnoreCase))
                {
                    return FGameplayAttribute(Property);
                }
            }
        }
    }

    // If no matching attribute is found, return an invalid attribute
    return FGameplayAttribute();
}

void UPEAttributeBase::InitFromMetaDataTable(const UDataTable* DataTable)
{
    Super::InitFromMetaDataTable(DataTable);
}