// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <AbilitySystemInterface.h>
#include <GameplayTagContainer.h>
#include <GameFramework/Character.h>
#include "PECharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterInit, APECharacter*, InitializedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

class UPEEquipment;
class UPEAbilitySystemComponent;
class UGameplayAbility;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UPEInventoryComponent;

/**
 *
 */
UCLASS(config = Game, Category = "Project Elementus | Classes")
class ELEMENTUSCORE_API APECharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

protected:
    virtual void PossessedBy(AController* InController) override;
    virtual void OnRep_PlayerState() override;
    virtual void OnRep_Controller() override;

    virtual void ApplyExtraSettings();

private:
    TObjectPtr<UPEAbilitySystemComponent> AbilitySystemComponent;

public:
    explicit APECharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    static const FName PEInventoryComponentName;
    static const FVector PECameraDefaultPosition;

    /** Returns CameraBoom sub object **/
    FORCEINLINE USpringArmComponent* GetCameraBoom() const
    {
        return CameraBoom;
    }

    /** Returns FollowCamera sub object **/
    FORCEINLINE UCameraComponent* GetFollowCamera() const
    {
        return FollowCamera;
    }

    /* Returns character associated Ability System Component */
    UFUNCTION(BlueprintPure, Category = "Project Elementus | Functions")
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /* Initialize the specified Ability System Component with the given owner actor in this character (AvatarActor) */
    void InitializeAbilitySystemComponent(UAbilitySystemComponent* InABSC, AActor* InOwnerActor);

    UFUNCTION(BlueprintPure, Category = "Project Elementus | Functions")
    UPEInventoryComponent* GetInventoryComponent() const;

protected:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Project Elementus | Properties", meta = (Getter = "GetInventoryComponent"))
    TObjectPtr<UPEInventoryComponent> InventoryComponent;

    virtual void PreInitializeComponents() override;
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void Server_InitializeCharacter();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_InitializeCharacter();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /* Metadata for unique traits or affiliations */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Project Elementus | Properties")
    FGameplayTagContainer CharacterTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project Elementus | Properties")
    bool RagdollAfterDeath = true;

    /* Init a death state with this character */
    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    void PerformDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Project Elementus | Functions | Callbacks")
    void AbilityActivated(UGameplayAbility* Ability);

    UFUNCTION(BlueprintImplementableEvent, Category = "Project Elementus | Functions | Callbacks")
    void AbilityCommited(UGameplayAbility* Ability);

    UFUNCTION(BlueprintImplementableEvent, Category = "Project Elementus | Functions | Callbacks")
    void AbilityEnded(UGameplayAbility* Ability);

    UFUNCTION(BlueprintNativeEvent, Category = "Project Elementus | Functions | Callbacks")
    void AbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& TagContainer);

    UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
    FOnCharacterDeath OnCharacterDeath;

    UPROPERTY(BlueprintAssignable, Category = "Project Elementus | Delegates")
    FOnCharacterInit OnCharacterInit;

private:
    UFUNCTION(Server, Reliable)
    void Server_DestroyCharacter();

    UFUNCTION(Client, Reliable)
    void Client_DeathSetup();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DeathSetup();

    UFUNCTION(Server, Reliable)
    virtual void Server_SpawnInventoryPackage();

    virtual void Landed(const FHitResult& Hit) override;

#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Project Elementus | Debug", meta = (AllowPrivateAccess = "true"))
    bool bDebugAbilities = false;
#endif
};
