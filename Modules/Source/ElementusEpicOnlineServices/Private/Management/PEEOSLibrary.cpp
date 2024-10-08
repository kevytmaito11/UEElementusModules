// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "Management/PEEOSLibrary.h"
#include <Online/OnlineSessionNames.h>
#include <EOSVoiceChatUser.h>
#include "LogElementusEpicOnlineServices.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEEOSLibrary)

FOnlineSessionSettings FEOSSessionSettings::GetNativeEOSSettings() const
{
    FOnlineSessionSettings LocalSessionSettings;
    LocalSessionSettings.NumPublicConnections = NumPublicConnections;
    LocalSessionSettings.NumPrivateConnections = NumPrivateConnections;
    LocalSessionSettings.bShouldAdvertise = bShouldAdvertise;
    LocalSessionSettings.bAllowJoinInProgress = bAllowJoinInProgress;
    LocalSessionSettings.bIsLANMatch = bIsLANMatch;
    LocalSessionSettings.bIsDedicated = bIsDedicated;
    LocalSessionSettings.bUsesStats = bUsesStats;
    LocalSessionSettings.bAllowInvites = bAllowInvites;
    LocalSessionSettings.bUsesPresence = bUsesPresence;
    LocalSessionSettings.bAllowJoinViaPresence = bAllowJoinViaPresence;
    LocalSessionSettings.bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;
    LocalSessionSettings.bAntiCheatProtected = bAntiCheatProtected;
    LocalSessionSettings.bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
    LocalSessionSettings.bUseLobbiesVoiceChatIfAvailable = bUseLobbiesVoiceChatIfAvailable;

    LocalSessionSettings.Set(SEARCH_KEYWORDS, FString("ProjectElementus"), EOnlineDataAdvertisementType::ViaOnlineService);

    return LocalSessionSettings;
}

FOnlineSubsystemEOS* UPEEOSLibrary::GetOnlineSubsystemEOS()
{
    return static_cast<FOnlineSubsystemEOS*>(FOnlineSubsystemEOS::Get(EOS_SUBSYSTEM));
}

FEOSVoiceChatUser* UPEEOSLibrary::GetEOSVoiceChatUser(const uint8 LocalUserNum)
{
    if (FOnlineSubsystemEOS* const OnlineSubsystemEOS = GetOnlineSubsystemEOS())
    {
        if (const FUniqueNetIdPtr NetId = GetUniqueNetId(LocalUserNum, OnlineSubsystemEOS))
        {
            return OnlineSubsystemEOS->GetEOSVoiceChatUserInterface(*NetId.Get());
        }
    }

    return nullptr;
}

void UPEEOSLibrary::MuteEOSSessionVoiceChatUser(const int32 LocalUserNum, const bool bMute)
{
    UE_LOG(LogElementusEpicOnlineServices_Internal, Log, TEXT("%s - Local User Num: %d; Mute: %d"), *FString(__FUNCTION__), LocalUserNum, bMute);

    if (FEOSVoiceChatUser* const VoiceChatUserPtr = GetEOSVoiceChatUser(LocalUserNum))
    {
        VoiceChatUserPtr->SetAudioInputDeviceMuted(bMute);
    }
}

FString UPEEOSLibrary::GetEOSSessionOwningUserNameFromHandle(const FSessionDataHandler DataHandle)
{
    return DataHandle.Result.Session.OwningUserName;
}

FString UPEEOSLibrary::GetEOSSessionIdFromHandle(const FSessionDataHandler DataHandle)
{
    return DataHandle.Result.Session.GetSessionIdStr();
}

int32 UPEEOSLibrary::GetEOSSessionPingFromHandle(const FSessionDataHandler DataHandle)
{
    return DataHandle.Result.PingInMs;
}

FName UPEEOSLibrary::GetEOSSessionName()
{
    return NAME_GameSession;
}

bool UPEEOSLibrary::IsUserLoggedInEOS(const int32 LocalUserNum)
{
    if (const IOnlineIdentityPtr IdentityInterface = GetIdentityInterface())
    {
        return IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn;
    }

    return false;
}

bool UPEEOSLibrary::IsHostingEOSSession()
{
    if (const IOnlineSessionPtr SessionInterface = GetSessionInterface())
    {
        if (const FNamedOnlineSession* const CheckSession = SessionInterface->GetNamedSession(NAME_GameSession))
        {
            return CheckSession->bHosting;
        }
    }

    return false;
}

bool UPEEOSLibrary::IsUserInAEOSSession()
{
    if (const IOnlineSessionPtr SessionInterface = GetSessionInterface())
    {
        return SessionInterface->GetNamedSession(NAME_GameSession) != nullptr;
    }

    return false;
}

void UPEEOSLibrary::UpdateEOSPresence(const int32 LocalUserNum, const FString& PresenceText, const bool bOnline)
{
    IOnlineSubsystem* const OnlineSubsystem = FOnlineSubsystemEOS::Get(EOS_SUBSYSTEM);

    if (const IOnlineIdentityPtr IdentityInterface = GetIdentityInterface(OnlineSubsystem))
    {
        if (const IOnlinePresencePtr PresenceInterface = GetPresenceInterface(OnlineSubsystem))
        {
            FOnlineUserPresenceStatus NewStatus;
            NewStatus.Properties.Add(DefaultPresenceKey);
            NewStatus.State = bOnline ? EOnlinePresenceState::Online : EOnlinePresenceState::Offline;
            NewStatus.StatusStr = PresenceText;

            PresenceInterface->SetPresence(*IdentityInterface->GetUniquePlayerId(LocalUserNum).Get(), NewStatus);
        }
    }
}

void UPEEOSLibrary::IngestEOSStats(const int32 LocalUserNum, const TMap<FName, int32>& StatsMap)
{
    IOnlineSubsystem* const OnlineSubsystem = FOnlineSubsystemEOS::Get(EOS_SUBSYSTEM);

    if (const IOnlineStatsPtr StatsInterface = GetStatsInterface(OnlineSubsystem))
    {
        const FUniqueNetIdRef UserNetIdRef = GetUniqueNetId(LocalUserNum, OnlineSubsystem).ToSharedRef();

        FOnlineStatsUserUpdatedStats UpdatedStats = FOnlineStatsUserUpdatedStats(UserNetIdRef);

        for (const auto& [StatName, StatValue] : StatsMap)
        {
            if (StatName.IsNone())
            {
                continue;
            }

            UE_LOG(LogElementusEpicOnlineServices_Internal, Log, TEXT("%s - Local User Num: %d; Stat Name: %s; Value: %d"), *FString(__FUNCTION__), LocalUserNum, *StatName.ToString(), StatValue);

            const FOnlineStatUpdate StatUpdate(StatValue, FOnlineStatUpdate::EOnlineStatModificationType::Unknown);
            UpdatedStats.Stats.Add(StatName.ToString(), StatUpdate);
        }

        const FOnlineStatsUpdateStatsComplete UpdateStatsDelegate = FOnlineStatsUpdateStatsComplete::CreateLambda(
            [UserNetIdRef, FuncName = __func__](const FOnlineError& ResultState)
            {
                UE_LOG(LogElementusEpicOnlineServices_Internal, Log, TEXT("%s - User ID: %s; Result: %s"), *FString(__FUNCTION__), *UserNetIdRef->ToString(), *ResultState.ToLogString());
            }
        );

        StatsInterface->UpdateStats(UserNetIdRef, { UpdatedStats }, UpdateStatsDelegate);
    }
}

bool UPEEOSLibrary::ValidateOnlineSubsystem(IOnlineSubsystem*& OnlineSubsystem)
{
    if (!OnlineSubsystem)
    {
        OnlineSubsystem = FOnlineSubsystemEOS::Get(EOS_SUBSYSTEM);
    }

    return OnlineSubsystem != nullptr;
}

const FUniqueNetIdPtr UPEEOSLibrary::GetUniqueNetId(const int32 LocalUserNum, IOnlineSubsystem* OnlineSubsystem)
{
    if (const IOnlineIdentityPtr Interface = GetIdentityInterface(OnlineSubsystem))
    {
        return Interface->GetUniquePlayerId(LocalUserNum);
    }

    return nullptr;
}

const IOnlineIdentityPtr UPEEOSLibrary::GetIdentityInterface(IOnlineSubsystem* OnlineSubsystem)
{
    if (ValidateOnlineSubsystem(OnlineSubsystem))
    {
        return OnlineSubsystem->GetIdentityInterface();
    }

    return nullptr;
}

const IOnlineSessionPtr UPEEOSLibrary::GetSessionInterface(IOnlineSubsystem* OnlineSubsystem)
{
    if (ValidateOnlineSubsystem(OnlineSubsystem))
    {
        return OnlineSubsystem->GetSessionInterface();
    }

    return nullptr;
}

const IOnlinePresencePtr UPEEOSLibrary::GetPresenceInterface(IOnlineSubsystem* OnlineSubsystem)
{
    if (ValidateOnlineSubsystem(OnlineSubsystem))
    {
        return OnlineSubsystem->GetPresenceInterface();
    }

    return nullptr;
}

const IOnlineStatsPtr UPEEOSLibrary::GetStatsInterface(IOnlineSubsystem* OnlineSubsystem)
{
    if (ValidateOnlineSubsystem(OnlineSubsystem))
    {
        return OnlineSubsystem->GetStatsInterface();
    }

    return nullptr;
}