// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include "ViewModels/Attributes/PEVM_AttributeBase.h"
#include "PEVM_AttributeCustom.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Category = "Project Elementus | Classes")
class ELEMENTUSABILITYSYSTEM_API UPEVM_AttributeCustom : public UPEVM_AttributeBase
{
    GENERATED_BODY()

public:
    explicit UPEVM_AttributeCustom(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float AttackRate;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float DefenseRate;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float SpeedRate;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float JumpRate;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category = "Project Elementus | Properties")
    float Gold;

    virtual void NotifyAttributeChange(const FGameplayAttribute& Attribute, const float& NewValue) override;

protected:

private:
    void SetAttackRate(const float Value);
    float GetAttackRate() const;

    void SetDefenseRate(const float Value);
    float GetDefenseRate() const;

    void SetSpeedRate(const float Value);
    float GetSpeedRate() const;

    void SetJumpRate(const float Value);
    float GetJumpRate() const;

    void SetGold(const float Value);
    float GetGold() const;
};
