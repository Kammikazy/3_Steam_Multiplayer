the code to host server
#include "ServerGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Login.h"
#include "Widgets/LoginWidget.h"
#include "Engine/Engine.h"
#include "OnlineSessionSettings.h"
const static FName SESSION_NAME = TEXT("");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");
 
void AServerGameMode::BeginPlay()
{
 
    IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
    if (SubSystem != nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("Found Substem %s"), *SubSystem->GetSubsystemName().ToString());
        SessionInterface = SubSystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
       
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AServerGameMode::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &AServerGameMode::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &AServerGameMode::OnFindSessionComplete);
        //  SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &AServerGameMode::OnJoinSessionComplete);
       
        }
       
        Host("dsd");
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("Found no  Substem"));
    }
}
 
void AServerGameMode::OnFindSessionComplete(bool Sucess)
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
                //  UE_LOG(LogTemp, Warning, TEXT("Data Found Setting %s"), *TestSetting);
            }
            else
            {
                Data.Name = "Could Not Find the Server";
                //  UE_LOG(LogTemp, Warning, TEXT("Data Not Found Setting"));
            }
            ServerName.Add(Data);
        }
        loginMenu->SetServerList(ServerName);
    }
 
}
void AServerGameMode::OnCreateSessionComplete(FName SessionName, bool Sucess) {
 
    if (!Sucess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not Create Session"));
        return;
    }
    if (loginMenu != nullptr)
    {
        loginMenu->TearDown();
    }
    //UEngine* Engine = GetEngine();
    //if (!ensure(Engine != nullptr)) return;
//  Engine->AddOnScreenDebugMessage(0, 5, FColor::Red, TEXT("Hosting"));
    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;
    World->ServerTravel("/Game/Assets/Maps/Lobby?listen");
}
 
void AServerGameMode::OndestroySessionComplete(FName SessionName, bool Sucess)
{
    if (Sucess) {
        CreateSession();
    }
}
void AServerGameMode::Host(FString Servername)
{
    DiresedServername = Servername;
    if (SessionInterface.IsValid()) {
 
        auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(SESSION_NAME);
        }
        else
        {
            CreateSession();
           
        }
 
    }
 
}
 
void AServerGameMode::CreateSession()
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
 
        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
 
    }
}
