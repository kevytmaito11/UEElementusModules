// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Components/PEInventoryComponent.h"
#include "Interfaces/IPEEquipment.h"
#include "Interfaces/PEEquipment.h"
#include "LogElementusInventorySystem.h"
#include <Core/PEAbilitySystemComponent.h>
#include <Core/PEAbilityFunctions.h>
#include <Effects/PEEffectData.h>
#include <Management/ElementusInventoryFunctions.h>
#include <GameFramework/Character.h>
#include <AbilitySystemGlobals.h>
#include <PEAbilityTags.h>
#include <MFEA_Settings.h>
#include <GameplayEffect.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEInventoryComponent)

UPEInventoryComponent::UPEInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UPEInventoryComponent::CanGiveItem(const FElementusItemInfo InItemInfo) const
{
    // We cannot give the item if it is currently equiped
    return Super::CanGiveItem(InItemInfo) && !InItemInfo.Tags.HasTag(FGameplayTag::RequestGameplayTag(GlobalTag_EquipSlot_Base));
}

const FElementusItemInfo& UPEInventoryComponent::AddTagsToItem(const FElementusItemInfo& InItem, const FGameplayTagContainer Tags, const FGameplayTagContainer IgnoreTags)
{
    if (int32 FoundInfoIndex; FindFirstItemIndexWithInfo(InItem, FoundInfoIndex, IgnoreTags))
    {
        FElementusItemInfo& ItemRef = GetItemReferenceAt(FoundInfoIndex);
        ItemRef.Tags.AppendTags(Tags);
        return ItemRef;
    }
    return InItem;
}

const FElementusItemInfo& UPEInventoryComponent::RemoveTagsFromItem(const FElementusItemInfo& InItem, const FGameplayTagContainer Tags, const FGameplayTagContainer IgnoreTags)
{
    if (int32 FoundInfoIndex; FindFirstItemIndexWithInfo(InItem, FoundInfoIndex, IgnoreTags))
    {
        FElementusItemInfo& ItemRef = GetItemReferenceAt(FoundInfoIndex);
        ItemRef.Tags.RemoveTags(Tags);
        return ItemRef;
    }
    return InItem;
}

bool UPEInventoryComponent::RemoveTagsFromItems(const FGameplayTagContainer Tags, const FGameplayTagContainer IgnoreTags)
{
    if (TArray<int32> OutIndexes; FindAllItemIndexesWithTags(Tags, OutIndexes, IgnoreTags))
    {
        for (int32 Index : OutIndexes)
        {
            FElementusItemInfo& ItemRef = GetItemReferenceAt(Index);
            ItemRef.Tags.RemoveTags(Tags);
        }
        return true;
    }
    return false;
}

void UPEInventoryComponent::EquipItem(const FElementusItemInfo& InItem)
{
    if (TryEquipItem_Internal(InItem))
    {
        NotifyInventoryChange();
    }
}

void UPEInventoryComponent::UnequipItem(FElementusItemInfo& InItem)
{
    if (TryUnequipItem_Internal(InItem))
    {
        NotifyInventoryChange();
    }
}

void UPEInventoryComponent::EquipInterfaceItem(const FElementusItemInfo& InItem)
{
    if (TryEquipInterfaceItem_Internal(InItem))
    {
        NotifyInventoryChange();
    }
}

void UPEInventoryComponent::UnequipInterfaceItem(const FElementusItemInfo& InItem)
{
    if (TryUnequipInterfaceItem_Internal(InItem))
    {
        NotifyInventoryChange();
    }
}

void UPEInventoryComponent::UnnequipAll(UAbilitySystemComponent* OwnerABSC)
{
    if (EquipmentMap.IsEmpty())
    {
        return;
    }

    TArray<FElementusItemInfo> ItemsToUnequip;
    for (auto& [SlotTag, ItemInfo] : EquipmentMap)
    {
        ItemsToUnequip.Add(ItemInfo);
    }

    for (FElementusItemInfo& ItemInfo : ItemsToUnequip)
    {
        TryUnequipItem_Internal(ItemInfo);
    }
}

void UPEInventoryComponent::ApplyItemEffect(const TSubclassOf<UGameplayEffect> EffectClass)
{
    ApplyItemEffect_Server(EffectClass);
}

UPEEquipment* UPEInventoryComponent::LoadEquipmentAsset(const FPrimaryElementusItemId& ItemId)
{
    if (const UElementusItemData* const ItemData = UElementusInventoryFunctions::GetSingleItemDataById(ItemId, { "SoftData" }, false))
    {
        return Cast<UPEEquipment>(ItemData->ItemClass.LoadSynchronous()->GetDefaultObject());
    }

    UE_LOG(LogElementusInventorySystem_Internal, Error, TEXT("%s - Failed to load item %s"), *FString(__FUNCTION__), *ItemId.ToString())

    return nullptr;
}

UObject* UPEInventoryComponent::LoadInterfaceEquipmentAsset(const FPrimaryElementusItemId& ItemId)
{
    if (const UElementusItemData* const ItemData = UElementusInventoryFunctions::GetSingleItemDataById(ItemId, { "SoftData" }, false))
    {
        UObject* LoadedObject = ItemData->ItemClass.LoadSynchronous()->GetDefaultObject();
        if (LoadedObject->Implements<UIPEEquipment>())
        {
            return LoadedObject;
        }
    }

    UE_LOG(LogElementusInventorySystem_Internal, Error, TEXT("%s - Failed to load item %s"), *FString(__FUNCTION__), *ItemId.ToString())

    return nullptr;
}

bool UPEInventoryComponent::CheckInventoryAndItem(const FElementusItemInfo& InItem) const
{
    if (!IsValid(GetOwner()))
    {
        UE_LOG(LogElementusInventorySystem_Internal, Error, TEXT("%s - Invalid owning actor"), *FString(__FUNCTION__));
        return false;
    }

    if (!ContainsItem(InItem, true))
    {
        UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - The item %s is not in the %s's inventory"), *FString(__FUNCTION__), *InItem.ItemId.ToString(), *GetOwner()->GetName());
        return false;
    }

    if (!GetOwner()->GetClass()->IsChildOf<ACharacter>())
    {
        UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Owning actor isn't a Character"), *FString(__FUNCTION__));
        return false;
    }

    return true;
}

UPEAbilitySystemComponent* UPEInventoryComponent::GetCharacterPEABSC(ACharacter* Character) const
{
    if (UAbilitySystemComponent* const TargetABSC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character))
    {
        return Cast<UPEAbilitySystemComponent>(TargetABSC);
    }

    return nullptr;
}

bool UPEInventoryComponent::TryEquipInterfaceItem_Internal(const FElementusItemInfo& InItem)
{
    if (!CheckInventoryAndItem(InItem))
    {
        return false;
    }

    if (UObject* const EquipedItem = LoadInterfaceEquipmentAsset(InItem.ItemId))
    {
        IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(EquipedItem);
        if (EquippedInterface)
        {
            const FGameplayTagContainer EquipmentSlotTags = EquippedInterface->GetEquipmentTags();
            ProcessEquipmentInterfaceAddition_Internal(Cast<ACharacter>(GetOwner()), EquipedItem);
        }
        UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
    }

    return false;
}

bool UPEInventoryComponent::TryUnequipInterfaceItem_Internal(const FElementusItemInfo& InItem)
{
    if (!CheckInventoryAndItem(InItem))
    {
        return false;
    }

    if (UObject* const EquipedItem = LoadInterfaceEquipmentAsset(InItem.ItemId))
    {
        IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(EquipedItem);
        if (EquippedInterface)
        {
            const FGameplayTagContainer EquipmentSlotTags = EquippedInterface->GetEquipmentTags();
            ProcessEquipmentInterfaceRemoval_Internal(Cast<ACharacter>(GetOwner()), EquipedItem);
        }
        UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
    }

    return false;
}

void UPEInventoryComponent::ProcessEquipmentInterfaceAddition_Internal(ACharacter* OwningCharacter, UObject* Equipment)
{
    if (UPEAbilitySystemComponent* const TargetABSC = GetCharacterPEABSC(OwningCharacter))
    {
        IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(Equipment);
        if (EquippedInterface)
        {
            AddEquipmentInterfaceGASData_Server(TargetABSC, Equipment);
            TargetABSC->AddLooseGameplayTags(EquippedInterface->GetEquipmentTags());
        }
    }
}

void UPEInventoryComponent::ProcessEquipmentInterfaceRemoval_Internal(ACharacter* OwningCharacter, UObject* Equipment)
{
    if (UPEAbilitySystemComponent* const TargetABSC = GetCharacterPEABSC(OwningCharacter))
    {
        IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(Equipment);
        if (EquippedInterface)
        {
            RemoveEquipmentInterfaceGASData_Server(TargetABSC, Equipment);
            TargetABSC->RemoveLooseGameplayTags(EquippedInterface->GetEquipmentTags());
        }
    }
}

void UPEInventoryComponent::AddEquipmentInterfaceGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UObject* Equipment)
{
    IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(Equipment);
    if (EquippedInterface)
    {
        // Add equipment effects
        for (const FGameplayEffectGroupedData& Effect : EquippedInterface->GetEquipmentEffects())
        {
            TargetABSC->ApplyEffectGroupedDataToSelf(Effect);
        }

        UEnum* const InputIdEnum = UMFEA_Settings::Get()->InputIDEnumeration.LoadSynchronous();

        // Add equipment abilities
        for (const auto& [InInputID_Name, InAbilityClass] : EquippedInterface->GetEquipmentAbilities())
        {
            if (InInputID_Name.IsNone())
            {
                UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid InputID"), *FString(__FUNCTION__));
            }

            if (!IsValid(InAbilityClass))
            {
                UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid Ability Class"), *FString(__FUNCTION__));
                continue;
            }

            UE_LOG(LogElementusInventorySystem_Internal, Display, TEXT("%s - Binding ability %s with InputId %s"), *FString(__FUNCTION__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
            UPEAbilityFunctions::GiveAbility(TargetABSC, InAbilityClass, InInputID_Name, InputIdEnum, false, true);
        }
    }
}

void UPEInventoryComponent::RemoveEquipmentInterfaceGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UObject* Equipment)
{
    IIPEEquipment* EquippedInterface = Cast<IIPEEquipment>(Equipment);
    if (EquippedInterface)
    {
        // Remove equipment effects
        for (const FGameplayEffectGroupedData& Effect : EquippedInterface->GetEquipmentEffects())
        {
            TargetABSC->RemoveEffectGroupedDataFromSelf(Effect, TargetABSC, 1);
        }

        // Remove equipment abilities
        for (const auto& [InInputID_Name, InAbilityClass] : EquippedInterface->GetEquipmentAbilities())
        {
            if (InInputID_Name.IsNone())
            {
                UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid InputID"), *FString(__FUNCTION__));
                continue;
            }

            if (!IsValid(InAbilityClass))
            {
                UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid Ability Class"), *FString(__FUNCTION__));
                continue;
            }

            UE_LOG(LogElementusInventorySystem_Internal, Display, TEXT("%s - Removing ability %s with InputId %s"), *FString(__FUNCTION__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
            UPEAbilityFunctions::RemoveAbility(TargetABSC, InAbilityClass);
        }
    }
}

bool UPEInventoryComponent::TryEquipItem_Internal(const FElementusItemInfo& InItem)
{
    if (!CheckInventoryAndItem(InItem))
    {
        return false;
    }

    if (UPEEquipment* const EquipedItem = LoadEquipmentAsset(InItem.ItemId))
    {
        const FGameplayTagContainer EquipmentSlotTags = EquipedItem->EquipmentSlotTags;
        if (int32 FoundTagIndex; FindFirstItemIndexWithTags(EquipmentSlotTags, FoundTagIndex, FGameplayTagContainer::EmptyContainer))
        {
            // Already equipped
            UE_LOG(LogElementusInventorySystem_Internal, Display, TEXT("%s - Actor %s has already unequipped item %s"), *FString(__FUNCTION__), *GetOwner()->GetName(), *InItem.ItemId.ToString());

            UnequipItem(GetItemReferenceAt(FoundTagIndex));
        }
        else if (int32 FoundInfoIndex; FindFirstItemIndexWithInfo(InItem, FoundInfoIndex, FGameplayTagContainer::EmptyContainer))
        {
            FElementusItemInfo& ItemRef = GetItemReferenceAt(FoundInfoIndex);
            ItemRef.Tags.AppendTags(EquipmentSlotTags);

            ProcessEquipmentAddition_Internal(Cast<ACharacter>(GetOwner()), EquipedItem);

            for (const FGameplayTag& Iterator : EquipmentSlotTags)
            {
                EquipmentMap.Add(Iterator, InItem);
            }

            UE_LOG(LogElementusInventorySystem, Display, TEXT("%s - Actor %s equipped %s"), *FString(__FUNCTION__), *GetOwner()->GetName(), *InItem.ItemId.ToString());
        }
        else
        {
            UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Failed to find item %s in %s's inventory"), *FString(__FUNCTION__), *InItem.ItemId.ToString(), *GetOwner()->GetName());
            UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
            return false;
        }

        UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
        return true;
    }

    return false;
}

bool UPEInventoryComponent::TryUnequipItem_Internal(FElementusItemInfo& InItem)
{
    if (!CheckInventoryAndItem(InItem))
    {
        return false;
    }

    if (UPEEquipment* const EquipedItem = LoadEquipmentAsset(InItem.ItemId))
    {
        ProcessEquipmentRemoval_Internal(Cast<ACharacter>(GetOwner()), EquipedItem);

        const FGameplayTagContainer EquipmentSlotTags = EquipedItem->EquipmentSlotTags;
        for (const FGameplayTag& Iterator : EquipmentSlotTags)
        {
            EquipmentMap.Remove(Iterator);
        }

        InItem.Tags.RemoveTags(EquipmentSlotTags);

        UE_LOG(LogElementusInventorySystem, Display, TEXT("%s - Actor %s unequipped %s"), *FString(__FUNCTION__), *GetOwner()->GetName(), *InItem.ItemId.ToString());

        UElementusInventoryFunctions::UnloadElementusItem(InItem.ItemId);
        return true;
    }

    return false;
}

void UPEInventoryComponent::ProcessEquipmentAddition_Internal(ACharacter* OwningCharacter, UPEEquipment* Equipment)
{
    if (UPEAbilitySystemComponent* const TargetABSC = GetCharacterPEABSC(OwningCharacter))
    {
        AddEquipmentGASData_Server(TargetABSC, Equipment);
        TargetABSC->AddLooseGameplayTags(Equipment->EquipmentSlotTags);
    }

    USkeletalMeshComponent* TargetMesh = OwningCharacter->GetMesh();

    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    OwningCharacter->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

    for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
    {
        // Check if the component has the specified tag
        if (SkeletalMeshComponent->ComponentHasTag(Equipment->SkeletonTag))
        {
            // Found the skeletal mesh component with the tag
            UE_LOG(LogElementusInventorySystem_Internal, Log, TEXT("Found SkeletalMeshComponent with tag: %s"), *Equipment->SkeletonTag.ToString());

            TargetMesh = SkeletalMeshComponent;
        }
    }

    if (GetOwnerRole() == ROLE_Authority)
    {
        ProcessEquipmentAttachment_Multicast(TargetMesh, Equipment);
    }
    else
    {
        ProcessEquipmentAttachment_Server(TargetMesh, Equipment);
    }
}

void UPEInventoryComponent::ProcessEquipmentRemoval_Internal(ACharacter* OwningCharacter, UPEEquipment* Equipment)
{
    if (UPEAbilitySystemComponent* const TargetABSC = GetCharacterPEABSC(OwningCharacter))
    {
        RemoveEquipmentGASData_Server(TargetABSC, Equipment);
        TargetABSC->RemoveLooseGameplayTags(Equipment->EquipmentSlotTags);
    }

    if (GetOwnerRole() == ROLE_Authority)
    {
        ProcessEquipmentDettachment_Multicast(Equipment);
    }
    else
    {
        ProcessEquipmentDettachment_Server(Equipment);
    }
}

void UPEInventoryComponent::AddEquipmentGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UPEEquipment* Equipment)
{
    // Add equipment effects
    for (const FGameplayEffectGroupedData& Effect : Equipment->EquipmentEffects)
    {
        TargetABSC->ApplyEffectGroupedDataToSelf(Effect);
    }

    UEnum* const InputIdEnum = UMFEA_Settings::Get()->InputIDEnumeration.LoadSynchronous();

    // Add equipment abilities
    for (const auto& [InInputID_Name, InAbilityClass] : Equipment->EquipmentAbilities)
    {
        if (InInputID_Name.IsNone())
        {
            UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid InputID"), *FString(__FUNCTION__));
            continue;
        }

        if (!IsValid(InAbilityClass))
        {
            UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid Ability Class"), *FString(__FUNCTION__));
            continue;
        }

        UE_LOG(LogElementusInventorySystem_Internal, Display, TEXT("%s - Binding ability %s with InputId %s"), *FString(__FUNCTION__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
        UPEAbilityFunctions::GiveAbility(TargetABSC, InAbilityClass, InInputID_Name, InputIdEnum, false, true);
    }
}

void UPEInventoryComponent::RemoveEquipmentGASData_Server_Implementation(UPEAbilitySystemComponent* TargetABSC, UPEEquipment* Equipment)
{
    // Remove equipment effects
    for (const FGameplayEffectGroupedData& Effect : Equipment->EquipmentEffects)
    {
        TargetABSC->RemoveEffectGroupedDataFromSelf(Effect, TargetABSC, 1);
    }

    // Remove equipment abilities
    for (const auto& [InInputID_Name, InAbilityClass] : Equipment->EquipmentAbilities)
    {
        if (InInputID_Name.IsNone())
        {
            UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid InputID"), *FString(__FUNCTION__));
            continue;
        }

        if (!IsValid(InAbilityClass))
        {
            UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - Invalid Ability Class"), *FString(__FUNCTION__));
            continue;
        }

        UE_LOG(LogElementusInventorySystem_Internal, Display, TEXT("%s - Removing ability %s with InputId %s"), *FString(__FUNCTION__), *InAbilityClass->GetName(), *InInputID_Name.ToString());
        UPEAbilityFunctions::RemoveAbility(TargetABSC, InAbilityClass);
    }
}

void UPEInventoryComponent::ProcessEquipmentAttachment_Server_Implementation(USkeletalMeshComponent* TargetMesh, UPEEquipment* Equipment)
{
    ProcessEquipmentAttachment_Multicast(TargetMesh, Equipment);
}

void UPEInventoryComponent::ProcessEquipmentAttachment_Multicast_Implementation(USkeletalMeshComponent* TargetMesh, UPEEquipment* Equipment)
{
    USkeletalMeshComponent* const InMesh = NewObject<USkeletalMeshComponent>(GetOwner(), USkeletalMeshComponent::StaticClass(), *Equipment->GetName());
    if (!IsValid(InMesh))
    {
        UE_LOG(LogElementusInventorySystem_Internal, Error, TEXT("%s - Failed to create skeletal mesh"), *FString(__FUNCTION__));
        return;
    }

    InMesh->SetIsReplicated(true);
    InMesh->SetSkeletalMesh(Equipment->EquipmentMesh.LoadSynchronous());
    InMesh->ComponentTags.Add(*FString::Printf(TEXT("ElementusEquipment_%s"), *Equipment->GetName()));

    GetOwner()->AddOwnedComponent(InMesh);

    if (!InMesh->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Equipment->SocketToAttach))
    {
        UE_LOG(LogElementusInventorySystem_Internal, Error, TEXT("%s - Failed to attach mesh to character"), *FString(__func__));
    }
    else
    {
        if (Equipment->SetLeaderPose)
        {
            InMesh->SetLeaderPoseComponent(TargetMesh);
        }
    }

    GetOwner()->FinishAndRegisterComponent(InMesh);
}

void UPEInventoryComponent::ProcessEquipmentDettachment_Server_Implementation(UPEEquipment* Equipment)
{
    ProcessEquipmentDettachment_Multicast(Equipment);
}

void UPEInventoryComponent::ProcessEquipmentDettachment_Multicast_Implementation(UPEEquipment* Equipment)
{
    const TArray<UActorComponent*> CompArr = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), *FString::Printf(TEXT("ElementusEquipment_%s"), *Equipment->GetName()));

    if (CompArr.IsEmpty())
    {
        UE_LOG(LogElementusInventorySystem_Internal, Warning, TEXT("%s - %s have no equipment attached"), *FString(__FUNCTION__), *GetOwner()->GetName());
        return;
    }

    for (UActorComponent* const& Iterator : CompArr)
    {
        Iterator->UnregisterComponent();
        Iterator->RemoveFromRoot();
        Iterator->DestroyComponent();
    }
}

void UPEInventoryComponent::ApplyItemEffect_Server_Implementation(const TSubclassOf<UGameplayEffect> EffectClass)
{
    if (UAbilitySystemComponent* const TargetABSC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
    {
        TargetABSC->ApplyGameplayEffectToSelf(EffectClass.GetDefaultObject(), 1.f, TargetABSC->MakeEffectContext());
    }
}