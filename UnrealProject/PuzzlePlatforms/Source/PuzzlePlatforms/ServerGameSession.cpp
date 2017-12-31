
#include "ServerGameSession.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Login.h"
#include "Widgets/LoginWidget.h"
#include "Engine/Engine.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = TEXT("");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

void AServerGameSession::RegisterServer()
{

	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
	if (SubSystem != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Found Substem %s"), *SubSystem->GetSubsystemName().ToString());
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AServerGameSession::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &AServerGameSession::OnFindSessionComplete);
		//	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &AServerGameSession::OnCreateSessionComplete);
		

		}
		Host("dsd");

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Found no  Substem"));
	}
}


void AServerGameSession::OnFindSessionComplete(bool Sucess)
{


	if (Sucess && SessionSearch.IsValid() && loginMenu != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"));

		TArray<FServerData> ServerName;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults) {
			UE_LOG(LogTemp, Warning, TEXT("Found Session Name %s"), *SearchResult.GetSessionIdStr());
			FServerData Data;

			Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			Data.HostUserName = SearchResult.Session.OwningUserName;
			FString ServerNames;
			if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerNames))
			{
				Data.Name = ServerNames;
			
			}
			else
			{
				Data.Name = "Could Not Find the Server";
				
			}
			ServerName.Add(Data);
		}
		loginMenu->SetServerList(ServerName);
	}

}
void AServerGameSession::OnCreateSessionComplete(FName SessionName, bool Sucess) {

	if (!Sucess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not Create Session"));
		return;
	}
	if (loginMenu != nullptr)
	{
		loginMenu->TearDown();
	}
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;
	World->ServerTravel("/Game/Assets/Maps/Lobby");
}

void AServerGameSession::OndestroySessionComplete(FName SessionName, bool Sucess)
{
	if (Sucess) {
		CreateSession();
	}
}
void AServerGameSession::Host(FString Servername)
{
	DiresedServername = Servername;
	if (SessionInterface.IsValid()) {

		auto ExistingSession = SessionInterface->GetNamedSession(GameSessionName);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(GameSessionName);
		}
		else
		{
			CreateSession();

		}

	}

}

void AServerGameSession::CreateSession()
{
	if (SessionInterface.IsValid()) {
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL") {
			SessionSettings.bIsLANMatch = true;
		}
		else {
			SessionSettings.bIsLANMatch = false;
		}

		SessionSettings.NumPublicConnections = 2;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = false;
		SessionSettings.bIsDedicated = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DiresedServername, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, GameSessionName, SessionSettings);

	}
}
