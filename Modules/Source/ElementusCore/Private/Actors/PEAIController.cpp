// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Actors/PEAIController.h"
#include <Core/PEAbilitySystemComponent.h>
#include <PEAbilityTags.h>
#include <Actors/PECharacter.h>
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Touch.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEAIController)

DEFINE_LOG_CATEGORY(LogAIController);

APEAIController::APEAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    AbilitySystemComponent = CreateDefaultSubobject<UPEAbilitySystemComponent>(TEXT("Ability System Component"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Data.Game.Bot"));

    // Initialize the Perception Component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Initialize Sight Config and add to Perception Component
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // Initialize Hearing Config and add to Perception Component
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 3000.0f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // Initialize Damage Config and add to Perception Component
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

    // Initialize Touch Config and add to Perception Component
    TouchConfig = CreateDefaultSubobject<UAISenseConfig_Touch>(TEXT("TouchConfig"));

    // Add all sense configurations to the perception component
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    PerceptionComponent->ConfigureSense(*TouchConfig);

    // Set the dominant sense for the perception component
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind to perception updated delegate
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &APEAIController::OnPerceptionUpdated);
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

    // Ensure all sensed actors are initially null
    LastSensedActor = nullptr;
    SightedActor = nullptr;
    HeardActor = nullptr;
    DamagedActor = nullptr;
    TouchedActor = nullptr;
}

void APEAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        FActorPerceptionBlueprintInfo Info;
        PerceptionComponent->GetActorsPerception(Actor, Info);

        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                LastSensedActor = Actor;

                if (Stimulus.Type == UAISense_Sight::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID())
                {
                    SightedActor = Actor;
                }
                else if (Stimulus.Type == UAISense_Hearing::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID())
                {
                    HeardActor = Actor;
                }
                else if (Stimulus.Type == UAISense_Damage::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID())
                {
                    DamagedActor = Actor;
                }
                else if (Stimulus.Type == UAISense_Touch::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID())
                {
                    TouchedActor = Actor;
                }
            }
        }
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
