// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <Interfaces/PEInteractable.h>
#include "PEConsumableActor.generated.h"

class UPEConsumableData;
class UNiagaraComponent;
/**
 *
 */
UCLASS(Abstract, Blueprintable, Category = "Project Elementus | Classes")
class ELEMENTUSACTORS_API APEConsumableActor : public AActor, public IPEInteractable
{
    GENERATED_BODY()

public:
    explicit APEConsumableActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void DoInteractionBehavior_Implementation(ACharacter* CharacterInteracting, const FHitResult& HitResult, const FGameplayTagContainer& InteractionTags) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project Elementus | Properties")
    bool bDestroyAfterConsumption;

protected:
    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    void PerformConsumption(class UAbilitySystemComponent* TargetABSC);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Project Elementus | Properties")
    TObjectPtr<UPEConsumableData> ConsumableData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Project Elementus | Properties")
    TObjectPtr<UStaticMeshComponent> ObjectMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Project Elementus | Properties")
    TObjectPtr<UNiagaraComponent> ObjectVFX;

private:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif WITH_EDITOR
};
