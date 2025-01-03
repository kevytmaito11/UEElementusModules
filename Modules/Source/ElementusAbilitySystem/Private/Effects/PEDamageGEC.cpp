// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Effects/PEDamageGEC.h"
#include "Attributes/PEBasicStatusAS.h"
#include "Attributes/PECustomStatusAS.h"
#include "PEAbilityTags.h"
#include <AbilitySystemComponent.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEDamageGEC)

struct FDamageAttributesStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(StaminaDamage);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackRate);
    DECLARE_ATTRIBUTE_CAPTUREDEF(DefenseRate);

    FDamageAttributesStatics()
    {
        DEFINE_ATTRIBUTE_CAPTUREDEF(UPEBasicStatusAS, Damage, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UPEBasicStatusAS, StaminaDamage, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UPECustomStatusAS, AttackRate, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UPECustomStatusAS, DefenseRate, Target, false);
    }
};

static const FDamageAttributesStatics& GetAttributesStatics()
{
    static FDamageAttributesStatics Attributes;
    return Attributes;
}

UPEDamageGEC::UPEDamageGEC(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    RelevantAttributesToCapture.Add(GetAttributesStatics().StaminaDamageDef);
    RelevantAttributesToCapture.Add(GetAttributesStatics().DamageDef);
    RelevantAttributesToCapture.Add(GetAttributesStatics().AttackRateDef);
    RelevantAttributesToCapture.Add(GetAttributesStatics().DefenseRateDef);
}

void UPEDamageGEC::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    const FGameplayTagContainer* const SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* const TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    float BaseDamage = 0.f;
    bool bHasBaseDamage = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAttributesStatics().DamageDef, EvaluationParameters, BaseDamage);
    if (!bHasBaseDamage)
    {
        BaseDamage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(GlobalTag_Damage), false, -1.0f), 0.0f);
    }

    float AttackRate = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAttributesStatics().AttackRateDef, EvaluationParameters, AttackRate);
    AttackRate = FMath::Max<float>(AttackRate, 0.0f);

    float DefenseRate = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAttributesStatics().DefenseRateDef, EvaluationParameters, DefenseRate);
    DefenseRate = FMath::Max<float>(DefenseRate, 0.0f);

    float BaseStaminaDamage = 0.f;
    bool bHasBaseStaminaDamage = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAttributesStatics().StaminaDamageDef, EvaluationParameters, BaseStaminaDamage);
    if (!bHasBaseStaminaDamage)
    {
        BaseStaminaDamage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(GlobalTag_StaminaDamage), false, -1.0f), 0.0f);
    }

    const auto CalculateDamage = [&BaseDamage, &AttackRate, &DefenseRate]() -> float
        {
            if (DefenseRate == 0.0f) // Avoid division by zero
            {
                return 0.0f;
            }

            float DamageDone = BaseDamage * AttackRate / DefenseRate;

            if (DamageDone < 0.f)
            {
                DamageDone *= -1.f;
            }

            return DamageDone;
        };

    const auto CalculateStaminaDamage = [&BaseStaminaDamage, &AttackRate, &DefenseRate]() -> float
        {
            if (DefenseRate == 0.0f) // Avoid division by zero
            {
                return 0.0f;
            }

            float StaminaDone = BaseStaminaDamage * AttackRate / DefenseRate;

            return StaminaDone;
        };

    if (bHasBaseDamage) // Add output only if the attribute exists
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetAttributesStatics().DamageProperty, EGameplayModOp::Additive, CalculateDamage()));
    }

    if (bHasBaseStaminaDamage) // Add output only if the attribute exists
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetAttributesStatics().StaminaDamageProperty, EGameplayModOp::Additive, CalculateStaminaDamage()));
    }
}