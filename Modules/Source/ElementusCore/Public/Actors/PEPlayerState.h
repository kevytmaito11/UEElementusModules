// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <AbilitySystemInterface.h>
#include <GameFramework/PlayerState.h>
#include <GameplayTagContainer.h>
#include <Core/PEAbilitySystemComponent.h>
#include "PEPlayerState.generated.h"

/**
 *
 */
DECLARE_LOG_CATEGORY_EXTERN(LogPlayerState, NoLogging, All);

#define PLAYERSTATE_VLOG(Actor, Verbosity, Format, ...) \
{ \
	UE_LOG(LogPlayerState, Verbosity, Format, ##__VA_ARGS__); \
	UE_VLOG(Actor, LogPlayerState, Verbosity, Format, ##__VA_ARGS__); \
}
/**
 *
 */
UCLASS(Blueprintable, NotPlaceable, Category = "Project Elementus | Classes")
class ELEMENTUSCORE_API APEPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit APEPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
    void DeathStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const;
    void StunStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const;

protected:
    virtual void BeginPlay() override;

    /* Player associated Ability System Component */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Project Elementus | Properties", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPEAbilitySystemComponent> AbilitySystemComponent;

    /* Returns a casted version of Player Controller using PEPlayerController class */
    UFUNCTION(BlueprintPure, Category = "Project Elementus | Properties", meta = (DisplayName = "Get Casted Player Controller: APEPlayerController"))
    class APEPlayerController* GetPEPlayerController() const;

public:
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
