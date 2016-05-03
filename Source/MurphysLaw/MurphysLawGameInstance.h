// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "Settings/MurphysLawGameSettings.h"
#include "MurphysLawGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString ServerName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString CurrentPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString MaxPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString GameType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString MapName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	FString Ping;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	int32 SearchResultsIndex;
};

/**
*
*/
UCLASS()
class MURPHYSLAW_API UMurphysLawGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UMurphysLawGameInstance(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ServerList")
	TArray<FServerEntry> ServerEntries;

	FString PlayerName;

private:
	MurphysLawGameSettings GameSettings;

#pragma region Pointers
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
#pragma endregion

#pragma region Delegates
	/* Delegate called when session created and started*/
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	/** Handle to registered delegate for searching a session */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	/** Handle to registered delegate for joining a session */
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
#pragma endregion

	/** Handle for efficient management of RespawnPickup timer */
	FTimerHandle TimerHandle_RespawnPickup;

private:

	/**
	*	Function to host a game!
	*
	*	@Param		UserID			User that started the request
	*	@Param		SessionName		Name of the Session
	*	@Param		bIsLAN			Is this is LAN Game?
	*	@Param		bIsPresence		"Is the Session to create a presence Session"
	*	@Param		MaxNumPlayers	        Number of Maximum allowed players on this "Session" (Server)
	*/
	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);

	/**
	*	Function fired when a session create request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session start request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Find an online session
	*
	*	@param UserId user that initiated the request
	*	@param SessionName name of session this search will generate
	*	@param bIsLAN are we searching LAN matches
	*	@param bIsPresence are we searching presence sessions
	*/
	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	/**
	*	Delegate fired when a session search query has completed
	*
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	*	Joins a session via a search result
	*
	*	@param SessionName name of session
	*	@param SearchResult Session to join
	*
	*	@return bool true if successful, false otherwise
	*/
	bool JoinASession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	/**
	*	Delegate fired when a session join request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	*	Delegate fired when a destroying an online session has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

public:

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartOnlineGame(FString GameName, FString GameLength, int32 WinningScore, int32 NumPlayers, bool WarmupWanted);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void FindOnlineGames();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void CancelFindOnlineGames();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinOnlineGame(int32 SessionIndex);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySessionAndLeaveGame();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetPlayerName(FString Name);

	UFUNCTION(BlueprintImplementableEvent, meta = (CallInEditor = "true"))
	void OnSessionFindCompleted();

	UFUNCTION(BlueprintImplementableEvent, meta = (CallInEditor = "true"))
	void OnStartSessionCompleted();

	UFUNCTION(BlueprintImplementableEvent, meta = (CallInEditor = "true"))
	void RemoveMenu();
};

