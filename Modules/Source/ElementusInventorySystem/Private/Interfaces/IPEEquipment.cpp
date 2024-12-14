
#include "Interfaces/IPEEquipment.h"

TArray<FGameplayEffectGroupedData> IIPEEquipment::GetEquipmentEffects()
{
	return TArray<FGameplayEffectGroupedData>();
}

FGameplayTagContainer IIPEEquipment::GetEquipmentTags()
{
	return FGameplayTagContainer();
}

TMap<FName, TSubclassOf<UGameplayAbility>> IIPEEquipment::GetEquipmentAbilities()
{
	return TMap<FName, TSubclassOf<UGameplayAbility>>();
}

void IIPEEquipment::AddEquipmentTag(const FGameplayTag& NewTag)
{
}
