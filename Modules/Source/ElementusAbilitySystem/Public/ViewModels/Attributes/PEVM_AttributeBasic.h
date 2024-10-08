// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include "ViewModels/Attributes/PEVM_AttributeBase.h"
#include "PEVM_AttributeBasic.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Category = "Project Elementus | Classes")
class ELEMENTUSABILITYSYSTEM_API UPEVM_AttributeBasic : public UPEVM_AttributeBase
{
    GENERATED_BODY()

public:
    explicit UPEVM_AttributeBasic(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintPure, FieldNotify, Category = "Project Elementus | Functions")
    float GetHealthPercent() const;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float Health;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties", Category = "Project Elementus | Properties")
    float MaxHealth;

    UFUNCTION(BlueprintPure, FieldNotify, Category = "Project Elementus | Functions")
    float GetManaPercent() const;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float Mana;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float MaxMana;

    UFUNCTION(BlueprintPure, FieldNotify, Category = "Project Elementus | Functions")
    float GetStaminaPercent() const;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float Stamina;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float MaxStamina;

    virtual void NotifyAttributeChange(const FGameplayAttribute& Attribute, const float& NewValue) override;

protected:

private:
    void SetHealth(const float Value);
    float GetHealth() const;

    void SetMaxHealth(const float Value);
    float GetMaxHealth() const;

    void SetMana(const float Value);
    float GetMana() const;

    void SetMaxMana(const float Value);
    float GetMaxMana() const;

    void SetStamina(const float Value);
    float GetStamina() const;

    void SetMaxStamina(const float Value);
    float GetMaxStamina() const;
};
