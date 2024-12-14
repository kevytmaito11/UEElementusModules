// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "Effects/PEEffectData.h"
#include "IPEEquipment.h"
#include "PEEquipment.generated.h"

class UGameplayAbility;

/**
 *
 */
UCLASS(Abstract, Blueprintable, Category = "Project Elementus | Classes")
class ELEMENTUSINVENTORYSYSTEM_API UPEEquipment : public UObject, public IIPEEquipment
{
    GENERATED_BODY()

public:
    explicit UPEEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties", Meta = (TitleProperty = "{EffectClass}"))
    TArray<FGameplayEffectGroupedData> EquipmentEffects;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    FGameplayTagContainer EquipmentSlotTags;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    TMap<FName, TSubclassOf<UGameplayAbility>> EquipmentAbilities;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    TSoftObjectPtr<USkeletalMesh> EquipmentMesh;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    FName SocketToAttach = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    FName SkeletonTag = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Project Elementus | Properties")
    bool SetLeaderPose = false;

    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    virtual TArray<FGameplayEffectGroupedData> GetEquipmentEffects() override;

    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    virtual FGameplayTagContainer GetEquipmentTags() override;

    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    virtual TMap<FName, TSubclassOf<UGameplayAbility>> GetEquipmentAbilities() override;

    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    virtual void AddEquipmentTag(const FGameplayTag& NewTag) override;
};