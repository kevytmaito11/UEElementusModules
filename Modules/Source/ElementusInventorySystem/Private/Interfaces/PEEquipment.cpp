// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Interfaces/PEEquipment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEEquipment)

UPEEquipment::UPEEquipment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

TArray<FGameplayEffectGroupedData> UPEEquipment::GetEquipmentEffects()
{
    return EquipmentEffects;
}

FGameplayTagContainer UPEEquipment::GetEquipmentTags()
{
    return EquipmentSlotTags;
}

TMap<FName, TSubclassOf<UGameplayAbility>> UPEEquipment::GetEquipmentAbilities()
{
    return EquipmentAbilities;
}

void UPEEquipment::AddEquipmentTag(const FGameplayTag& NewTag)
{
    // Add the new tag to the EquipmentSlotTags container
    if (!EquipmentSlotTags.HasTag(NewTag))
    {
        EquipmentSlotTags.AddTag(NewTag);
    }
}
