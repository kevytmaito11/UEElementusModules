// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Actors/PEPlayerState.h"
#include "Actors/PECharacter.h"
#include "Actors/PEPlayerController.h"
#include <Core/PEAbilitySystemComponent.h>
#include <PEAbilityTags.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEPlayerState)

DEFINE_LOG_CATEGORY(LogPlayerState);

APEPlayerState::APEPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    AbilitySystemComponent = CreateDefaultSubobject<UPEAbilitySystemComponent>(TEXT("Ability System Component"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Data.Game.Player"));

    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    bReplicates = true;
    bOnlyRelevantToOwner = false;
    bAlwaysRelevant = true;
    AActor::SetReplicateMovement(false);
    SetNetUpdateFrequency(1.f);
    NetPriority = 1.f;
    NetDormancy = ENetDormancy::DORM_Awake;
}

void APEPlayerState::BeginPlay()
{
    PLAYERSTATE_VLOG(this, Display, TEXT("%s called."), *FString(__FUNCTION__));

    Super::BeginPlay();

    // Check if the player state have a valid ABSC and bind functions to wait Death and Stun tags
    if (ensureAlwaysMsgf(IsValid(AbilitySystemComponent), TEXT("%s have a invalid AbilitySystemComponent"), *GetName()))
    {
        AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(GlobalTag_DeadState), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APEPlayerState::DeathStateChanged_Callback);
        AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(GlobalTag_StunState), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APEPlayerState::StunStateChanged_Callback);
    }
}

void APEPlayerState::DeathStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const
{
    // GetPlayerController() return nullptr on clients
    if (!HasAuthority())
    {
        return;
    }

    PLAYERSTATE_VLOG(this, Display, TEXT("%s called with %s Callback Tag and NewCount equal to %d"), *FString(__FUNCTION__), *CallbackTag.ToString(), NewCount);

    // If death tag != 0, the player is dead
    if (NewCount == 0)
    {
        return;
    }

    if (APEPlayerController* const Controller_Temp = GetPEPlayerController(); ensureAlwaysMsgf(IsValid(Controller_Temp), TEXT("%s have a invalid Controller"), *GetName()))
    {
        // If this controller have a valid character, perform death
        if (APECharacter* const Player_Temp = Controller_Temp->GetPawn<APECharacter>(); ensureAlwaysMsgf(IsValid(Player_Temp), TEXT("%s have a invalid Player"), *GetName()))
        {
            Player_Temp->PerformDeath();
        }

        // Initialize the spectating state
        Controller_Temp->SetupControllerSpectator();
    }
}

void APEPlayerState::StunStateChanged_Callback(const FGameplayTag CallbackTag, const int32 NewCount) const
{
    // GetPlayerController() return nullptr on clients
    if (!HasAuthority())
    {
        return;
    }

    PLAYERSTATE_VLOG(this, Display, TEXT("%s called with %s Callback Tag and NewCount equal to %d"), *FString(__FUNCTION__), *CallbackTag.ToString(), NewCount);

    // Just ignore/activate movement inputs if have a valid player controller
    if (ensureAlwaysMsgf(IsValid(GetPlayerController()), TEXT("%s have a invalid Player"), *GetName()))
    {
        GetPlayerController()->SetIgnoreMoveInput(NewCount != 0);
    }
}

APEPlayerController* APEPlayerState::GetPEPlayerController() const
{
    return Cast<APEPlayerController>(GetPlayerController());
}

UAbilitySystemComponent* APEPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}