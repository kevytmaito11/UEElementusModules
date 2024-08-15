// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <AIController.h>
#include <AbilitySystemInterface.h>
#include <Core/PEAbilitySystemComponent.h>
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Touch.h"
#include "PEAIController.generated.h"

/**
 *
 */
DECLARE_LOG_CATEGORY_EXTERN(LogAIController, NoLogging, All);

#define AICONTROLLER_VLOG(Actor, Verbosity, Format, ...) \
{ \
	UE_LOG(LogAIController, Verbosity, Format, ##__VA_ARGS__); \
	UE_VLOG(Actor, LogAIController, Verbosity, Format, ##__VA_ARGS__); \
}
/**
 *
 */

/**
 *
 */
UCLASS(Blueprintable, NotPlaceable, Category = "Project Elementus | Classes")
class ELEMENTUSCORE_API APEAIController : public AAIController, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit APEAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
    void DeathStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const;
    void StunStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const;

	/** Sense configuration components */
	UAISenseConfig_Sight* SightConfig;
	UAISenseConfig_Hearing* HearingConfig;
	UAISenseConfig_Damage* DamageConfig;
	UAISenseConfig_Touch* TouchConfig;

protected:
    virtual void BeginPlay() override;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    /* AI associated Ability System Component */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Project Elementus | Properties", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPEAbilitySystemComponent> AbilitySystemComponent;

public:
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Perceived actor pointers to be referenced from Blueprints */
	UPROPERTY(BlueprintReadOnly, Category = "AI Perception")
	AActor* LastSensedActor;

	UPROPERTY(BlueprintReadOnly, Category = "AI Perception")
	AActor* SightedActor;

	UPROPERTY(BlueprintReadOnly, Category = "AI Perception")
	AActor* HeardActor;

	UPROPERTY(BlueprintReadOnly, Category = "AI Perception")
	AActor* DamagedActor;

	UPROPERTY(BlueprintReadOnly, Category = "AI Perception")
	AActor* TouchedActor;
};
