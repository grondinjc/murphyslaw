// Fill out your copyright notice in the Description page of Project Settings.

#include "MurphysLaw.h"
#include "MurphysLawGameInstance.h"
#include "Network/MurphysLawPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Network/MurphysLawGameMode.h"

#define SETTING_SERVERNAME FName(TEXT("SERVERNAME"))

UMurphysLawGameInstance::UMurphysLawGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/** Bind function for CREATING a Session */
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMurphysLawGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UMurphysLawGameInstance::OnStartOnlineGameComplete);

	/** Bind function for FINDING a Session */
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMurphysLawGameInstance::OnFindSessionsComplete);

	/** Bind function for JOINING a Session */
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMurphysLawGameInstance::OnJoinSessionComplete);

	OnDestroySessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UMurphysLawGameInstance::OnDestroySessionComplete);
}

bool UMurphysLawGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the Session Settings that we want to use.

			There are more with SessionSettings.Set(...);
			For example the Map or the GameMode/Type.
			*/
			SessionSettings = MakeShareable(new FOnlineSessionSettings());

			SessionSettings->bIsLANMatch = true;
			SessionSettings->bUsesPresence = bIsPresence;
			SessionSettings->NumPublicConnections = GameSettings.NbPlayersPerTeam;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

			
			SessionSettings->Set(SETTING_SERVERNAME, FString(GameSettings.GameName), EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings->Set(SETTING_MAPNAME, FString("Default"), EOnlineDataAdvertisementType::ViaOnlineService);

			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(*UserId, GameSessionName, *SessionSettings);
		}
	}
	else
	{
		ShowError("No OnlineSubsytem found!");
	}

	return false;
}

void UMurphysLawGameInstance::OnCreateSessionComplete(FName GameSessionName, bool bWasSuccessful)
{
	ShowError(FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *GameSessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
			if (bWasSuccessful)
			{
				const FName URL(*("/Game/MurphysLaw/Visual/Landscape/Default" + GameSettings.Serialize()));
				UGameplayStatics::OpenLevel(GetWorld(), URL, true, "listen");
			}
		}
	}
}

void UMurphysLawGameInstance::StartSession()
{
	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Set the StartSession delegate handle
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

			// Our StartSessionComplete delegate should get called after this
			Sessions->StartSession(GameSessionName);
		}
	}
}

void UMurphysLawGameInstance::OnStartOnlineGameComplete(FName GameSessionName, bool bWasSuccessful)
{
	ShowError(FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *GameSessionName.ToString(), bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
			this->OnStartSessionCompleted();
		}
	}

	// tell non-local players to start online game
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMurphysLawPlayerController* PC = Cast<AMurphysLawPlayerController>(*It);
		if (PC && !PC->IsLocalPlayerController())
		{
			PC->ClientStartOnlineGame();
		}
		else
			PC->ChangeHUDVisibility(ESlateVisibility::Visible);

	}

	AMurphysLawGameMode* GameMode = Cast<AMurphysLawGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
		GameMode->StartMatch();
}

void UMurphysLawGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName GameSessionName, bool bIsLAN, bool bIsPresence)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get the SessionInterface from our OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
			*/
			SessionSearch = MakeShareable(new FOnlineSessionSearch());

			SessionSearch->bIsLanQuery = true;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->PingBucketSize = 50;

			// We only want to set this Query Setting if "bIsPresence" is true
			if (bIsPresence)
			{
				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
			}

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

			// Set the Delegate to the Delegate Handle of the FindSession function
			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			// Finally call the SessionInterface function. The Delegate gets called once this is finished
			Sessions->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else
	{
		// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
	}
}

void UMurphysLawGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the Delegate handle, since we finished this call
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{
				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					FString MapName;
					SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(SETTING_MAPNAME, MapName);
					// OwningUserName is just the GameSessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
					// This is something you can't do in Blueprint for example!
					// Default name: *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)
					
					const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SearchIdx];
					int32 CurrentPlayers = Result.Session.SessionSettings.NumPublicConnections
						+ Result.Session.SessionSettings.NumPrivateConnections
						- Result.Session.NumOpenPublicConnections
						- Result.Session.NumOpenPrivateConnections;

					if (CurrentPlayers != 0)
					{
						FServerEntry ServerEntry;
						FString ServerName;
						FString MapName;
						Result.Session.SessionSettings.Get(SETTING_SERVERNAME, ServerName);
						Result.Session.SessionSettings.Get(SETTING_MAPNAME, MapName);
						ServerEntry.ServerName = ServerName == "" ? Result.Session.OwningUserName : ServerName;
						ServerEntry.MapName = MapName;
						ServerEntry.GameType = "";
						ServerEntry.CurrentPlayers = FString::FromInt(CurrentPlayers);
						ServerEntry.MaxPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
							+ Result.Session.SessionSettings.NumPrivateConnections);
						ServerEntry.Ping = FString::FromInt(Result.PingInMs);
						ServerEntry.SearchResultsIndex = SearchIdx;
						ServerEntries.Add(ServerEntry);
					}
				}
			}
		}
	}
	this->OnSessionFindCompleted();
}

void UMurphysLawGameInstance::CancelFindOnlineGames()
{
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		}
	}
}

bool UMurphysLawGameInstance::JoinASession(TSharedPtr<const FUniqueNetId> UserId, FName GameSessionName, const FOnlineSessionSearchResult& SearchResult)
{
	// Return bool
	bool bSuccessful = false;

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			bSuccessful = Sessions->JoinSession(*UserId, GameSessionName, SearchResult);
		}
	}

	return bSuccessful;
}

void UMurphysLawGameInstance::OnJoinSessionComplete(FName GameSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the Delegate again
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController * const PlayerController = GetFirstLocalPlayerController();

			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the GameSessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;
			if (PlayerController && Sessions->GetResolvedConnectString(GameSessionName, TravelURL))
			{
				// Finally call the ClienTravel.
				ShowError(FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *GameSessionName.ToString(), static_cast<int32>(Result)));
				PlayerController->ClientTravel(TravelURL + GameSettings.Serialize(), ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMurphysLawGameInstance::OnDestroySessionComplete(FName GameSessionName, bool bWasSuccessful)
{
	ShowError(FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *GameSessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			// If it was successful, we just load another level (could be a MainMenu!)
			if (bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), "/Game/MurphysLaw/Visual/Map_Menu", true);
			}
		}
	}
}

void UMurphysLawGameInstance::StartOnlineGame(FString GameName, FString GameLength, int32 WinningScore, int32 NumPlayers, bool WarmupWanted)
{
	FString Left, Right;
	GameLength.Split(" ", &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	GameSettings.GameName = GameName;
	GameSettings.GameTime = FCString::Atoi(*Left) * 60;
	GameSettings.NbPointsForWin = WinningScore;
	GameSettings.NbPlayersPerTeam = NumPlayers;
	GameSettings.WarmupWanted = WarmupWanted;

	// Creating a local player where we can get the UserID from
	ULocalPlayer* const Player = GetFirstGamePlayer();

	// Call our custom HostSession function. GameSessionName is a GameInstance variable
	HostSession(Player->GetPreferredUniqueNetId(), true, true);
}

void UMurphysLawGameInstance::FindOnlineGames()
{
	ULocalPlayer* const Player = GetFirstGamePlayer();
	ServerEntries.Empty();
	FindSessions(Player->GetPreferredUniqueNetId(), GameSessionName, true, true);
}

void UMurphysLawGameInstance::JoinOnlineGame(int32 SessionIndex)
{
	ULocalPlayer* const Player = GetFirstGamePlayer();
	
	// Just a SearchResult where we can save the one we want to use, for the case we find more than one!
	FOnlineSessionSearchResult SearchResult;

	// If the Array is not empty, we can go through it
	if (SessionSearch->SearchResults.Num() > 0)
	{
		//for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		//{
			// To avoid something crazy, we filter sessions from ourself
			if (SessionSearch->SearchResults[SessionIndex].Session.OwningUserId != Player->GetPreferredUniqueNetId())
			{
				SearchResult = SessionSearch->SearchResults[SessionIndex];

				// Once we found sounce a Session that is not ours, just join it. Instead of using a for loop, you could
				// use a widget where you click on and have a reference for the GameSession it represents which you can use
				// here
				JoinASession(Player->GetPreferredUniqueNetId(), GameSessionName, SearchResult);
				//break;
			}
		//}
	}
}

void UMurphysLawGameInstance::DestroySessionAndLeaveGame()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
		}
	}
}

void UMurphysLawGameInstance::SetPlayerName(FString Name)
{
	GameSettings.CharacterName = Name;
}