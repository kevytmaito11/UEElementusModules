// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Actors/PEAIController.h"
#include <Core/PEAbilitySystemComponent.h>
#include <PEAbilityTags.h>
#include <Actors/PECharacter.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEAIController)

DEFINE_LOG_CATEGORY(LogAIController);

APEAIController::APEAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    AbilitySystemComponent = CreateDefaultSubobject<UPEAbilitySystemComponent>(TEXT("Ability System Component"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Data.Game.Bot"));
}

void APEAIController::BeginPlay()
{
    AICONTROLLER_VLOG(this, Display, TEXT("%s called."), *FString(__FUNCTION__));

    Super::BeginPlay();

    // Check if the player state have a valid ABSC and bind functions to wait Death and Stun tags
    if (ensureAlwaysMsgf(IsValid(AbilitySystemComponent), TEXT("%s have a invalid AbilitySystemComponent"), *GetName()))
    {
        AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(GlobalTag_DeadState), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APEAIController::DeathStateChanged_Callback);
        AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(GlobalTag_StunState), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APEAIController::StunStateChanged_Callback);
    }
}

UAbilitySystemComponent* APEAIController::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void APEAIController::DeathStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const
{
    // GetPlayerController() return nullptr on clients
    if (!HasAuthority())
    {
        return;
    }

    AICONTROLLER_VLOG(this, Display, TEXT("%s called with %s Callback Tag and NewCount equal to %d"), *FString(__FUNCTION__), *CallbackTag.ToString(), NewCount);

    // If death tag != 0, the player is dead
    if (NewCount == 0)
    {
        return;
    }

    // If this controller have a valid character, perform death
    if (APECharacter* const Player_Temp = GetPawn<APECharacter>(); ensureAlwaysMsgf(IsValid(Player_Temp), TEXT("%s have a invalid Player"), *GetName()))
    {
        Player_Temp->PerformDeath();
    }
}

void APEAIController::StunStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const
{
    // GetPlayerController() return nullptr on clients
    if (!HasAuthority())
    {
        return;
    }

    AICONTROLLER_VLOG(this, Display, TEXT("%s called with %s Callback Tag and NewCount equal to %d"), *FString(__FUNCTION__), *CallbackTag.ToString(), NewCount);
}
