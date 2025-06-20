// Fill out your copyright notice in the Description page of Project Settings.


#include "BFL_Pak.h"
#include "IPlatformFilePak.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"


void UBFL_Pak::MountAndRegisterPak(FString PakFilePath, bool& bIsMountSuccessful)
{
	FString PakRootPath = "/Game/";
	if (!PakFilePath.IsEmpty())
	{
		if (MountPakFile(PakFilePath, FString()))
		{
			bIsMountSuccessful = true;
			const FString MountPoint = GetPakMountContentPath(PakFilePath);
			RegisterMountPoint(PakRootPath, MountPoint);
		}
	}
}

bool UBFL_Pak::MountPakFile(const FString& PakFilePath, const FString& PakMountPoint)
{
	bool bIsMounted = false;

	FString InputPath = PakFilePath;
	FPaths::MakePlatformFilename(InputPath);

	GEngine->AddOnScreenDebugMessage(-1, 300.f, FColor::Red, FString::Printf(TEXT("Input Path: %s"), *InputPath));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(
		TEXT("PakFile")));
	if (!PakPlatformFile)
	{
		PakPlatformFile = new FPakPlatformFile();
		PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
	}

	if (!PakMountPoint.IsEmpty())
	{
		bIsMounted = PakPlatformFile->Mount(*InputPath, 0, *PakMountPoint);
	}
	else
	{
		bIsMounted = PakPlatformFile->Mount(*InputPath, 0);
	}

	GEngine->AddOnScreenDebugMessage(-1, 300.f, FColor::Red,
	                                 FString::Printf(TEXT("PakMountPoint: %s"), *PakMountPoint));

	return bIsMounted;
}

bool UBFL_Pak::UnmountPakFile(const FString& PakFilePath)
{
	FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(
		TEXT("PakFile")));
	if (PakPlatformFile)
	{
		return PakPlatformFile->Unmount(*PakFilePath);
	}
	return false;
}

void UBFL_Pak::RegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::RegisterMountPoint(RootPath, ContentPath);
	FString _RootPath = RootPath;
	FString _ContentPath = ContentPath;
}

void UBFL_Pak::UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
}

FString const UBFL_Pak::GetPakMountPoint(const FString& PakFilePath)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")),
	                                              *PakFilePath, false);
	Pak = PakFile.GetReference();
	if (Pak->IsValid())
	{
		return Pak->GetMountPoint();
	}
	return FString();
}

TArray<FString> UBFL_Pak::GetPakContent(const FString& PakFilePath, bool bOnlyCooked /*= true*/)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")),
	                                              *PakFilePath, false);
	Pak = PakFile.GetReference();
	TArray<FString> PakContent;

	if (Pak->IsValid())
	{
		FString ContentPath, PakAppendPath;
		FString MountPoint = GetPakMountPoint(PakFilePath);
		//GEngine->AddOnScreenDebugMessage(-1,300.f,FColor::Yellow,MountPoint);
		MountPoint.Split("/Content/", &ContentPath, &PakAppendPath);

		TArray<FPakFile::FFilenameIterator> Records;
		for (FPakFile::FFilenameIterator It(*Pak, false); It; ++It)
		{
			Records.Add(It);
		}

		for (auto& It : Records)
		{
			if (bOnlyCooked)
			{
				if (FPaths::GetExtension(It.Filename()) == TEXT("uasset"))
				{
					PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
				}
			}
			else
			{
				PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
			}
		}
	}
	return PakContent;
}

FString UBFL_Pak::GetPakMountContentPath(const FString& PakFilePath)
{
	FString ContentPath, RightStr;
	bool bIsContent;
	FString MountPoint = GetPakMountPoint(PakFilePath);
	bIsContent = MountPoint.Split("/Content/", &ContentPath, &RightStr);
	if (bIsContent)
	{
		return FString::Printf(TEXT("%s/Content/"), *ContentPath);
	}
	// Check Pak Content for /Content/ Path
	else
	{
		TArray<FString> Content = GetPakContent(PakFilePath);
		if (Content.Num() > 0)
		{
			FString FullPath = FString::Printf(TEXT("%s%s"), *MountPoint, *Content[0]);
			bIsContent = FullPath.Split("/Content/", &ContentPath, &RightStr);
			if (bIsContent)
			{
				return FString::Printf(TEXT("%s/Content/"), *ContentPath);
			}
		}
	}
	//Failed to Find Content
	return FString("");
}

UClass* UBFL_Pak::LoadBlueprintClass(FString PakContentPath)
{
	FString PakRootPath = "/Game/";
	FString PackagePath = PakRootPath + FPaths::GetBaseFilename(PakContentPath, false);
	FString AssetName = FPaths::GetBaseFilename(PakContentPath, true);
	FString ClassReference = FString::Printf(TEXT("%s.%s_C"), *PackagePath, *AssetName);
	return StaticLoadClass(UObject::StaticClass(), nullptr, *ClassReference);
}


FString UBFL_Pak::GetUnrealPath(const FString& Filename)
{
	FString BaseFileName = FPaths::GetBaseFilename(Filename, true);
	FString RootPath;
	FString RelativePath;
	Filename.Split(TEXT("/Content/"), &RootPath, &RelativePath);

	// 去掉后缀名 (例如.umap, .uasset)
	FString RelativeRemoveExt = FPaths::GetBaseFilename(RelativePath, false);

	return TEXT("/Game/") + RelativeRemoveExt + TEXT(".") + BaseFileName;
}



void UBFL_Pak::LoadFirstLevel(const FString& PakFilePath, UObject* InContextObject)
{
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")),
	                                              *PakFilePath, false);

	TArray<FString> FoundFileNames;
	PakFile->FindPrunedFilesAtPath(FoundFileNames, *PakFile->GetMountPoint(), true, false, true);

	for (auto Filename : FoundFileNames)
	{
		if (Filename.EndsWith(TEXT(".umap")))
		{
			FString BaseFileName = FPaths::GetBaseFilename(Filename);

			FString UnrealPath = GetUnrealPath(Filename);
			// 加载 Object
			UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), NULL, *UnrealPath);		

			// 直接打开关卡
			UGameplayStatics::OpenLevel(InContextObject->GetWorld(), FName(BaseFileName));			

			break;
		}
	}
}

void UBFL_Pak::StreamFirstLevel(const FString& PakFilePath, UObject* InContextObject)
{
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")),
												  *PakFilePath, false);

	TArray<FString> FoundFileNames;
	PakFile->FindPrunedFilesAtPath(FoundFileNames, *PakFile->GetMountPoint(), true, false, true);

	for (auto Filename : FoundFileNames)
	{
		if (Filename.EndsWith(TEXT(".umap")))
		{
			FString BaseFileName = FPaths::GetBaseFilename(Filename);

			FString UnrealPath = GetUnrealPath(Filename);

			UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), NULL, *UnrealPath);	


			FString LevelPath = FPaths::GetBaseFilename(UnrealPath, false);			

			// 动态流式加载关卡
			ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(
				InContextObject->GetWorld(), ULevelStreamingDynamic::StaticClass(), NAME_None, RF_Transient
			);

			if (StreamingLevel)
			{
				// 设置关卡资源路径（如 "/Game/Path/LevelName"）
				StreamingLevel->SetWorldAssetByPackageName(FName(*LevelPath));
                
				// 添加到世界并触发加载
				InContextObject->GetWorld()->AddStreamingLevel(StreamingLevel);
				StreamingLevel->SetShouldBeLoaded(true);
				StreamingLevel->SetShouldBeVisible(true);

				// 可选：绑定加载完成回调
				// StreamingLevel->OnLevelLoaded.AddDynamic(this, &UBFL_Pak::OnLevelStreamed);
			}

			break;
		}
	}
}

