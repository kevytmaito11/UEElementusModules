

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Effects/PEEffectData.h"
#include <Abilities/GameplayAbility.h>
#include "IPEEquipment.generated.h"

// This class does not need to be modified.
UINTERFACE(NotBlueprintable, Category = "Project Elementus | Interfaces")
class ELEMENTUSINVENTORYSYSTEM_API UIPEEquipment : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ELEMENTUSINVENTORYSYSTEM_API IIPEEquipment
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	virtual TArray<FGameplayEffectGroupedData> GetEquipmentEffects();

	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	virtual FGameplayTagContainer GetEquipmentTags();

	UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
	virtual TMap<FName, TSubclassOf<UGameplayAbility>> GetEquipmentAbilities();
};
