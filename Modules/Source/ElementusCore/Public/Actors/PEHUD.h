// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/HUD.h>
#include <ViewModels/Attributes/PEVM_AttributeBasic.h>
#include <ViewModels/Attributes/PEVM_AttributeCustom.h>
#include <ViewModels/Attributes/PEVM_AttributeLeveling.h>
#include "PEHUD.generated.h"

/**
 *
 */
UCLASS(Blueprintable, NotPlaceable, Category = "Project Elementus | Classes")
class ELEMENTUSCORE_API APEHUD : public AHUD
{
    GENERATED_BODY()

public:
    explicit APEHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void BeginPlay() override;

    /* Toggle HUD */
    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    void HideWidget();

    UFUNCTION(BlueprintCallable, Category = "Project Elementus | Functions")
    void ShowWidget();

protected:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Project Elementus | Properties")
    TObjectPtr<UPEVM_AttributeBasic> BasicAttributes_VM;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Project Elementus | Properties")
    TObjectPtr<UPEVM_AttributeCustom> CustomAttributes_VM;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Project Elementus | Properties")
    TObjectPtr<UPEVM_AttributeLeveling> LevelingAttributes_VM;

    /* A Blueprint Widget class to use as HUD */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Project Elementus | Properties")
    TSubclassOf<UUserWidget> HUDClass;

    UFUNCTION()
    virtual void InitializeAttributeViewModels(APECharacter* PECharacter);

    virtual void OnViewModelAttributeChange(const struct FOnAttributeChangeData& AttributeChangeData);
    void NotifyAttributeChange(const struct FGameplayAttribute& Attribute, const float& NewValue);

    TWeakObjectPtr<UUserWidget> HUDHandle;

private:
};
