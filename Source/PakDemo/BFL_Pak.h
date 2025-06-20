// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_Pak.generated.h"


UCLASS()
class PAKDEMO_API UBFL_Pak : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	
	UFUNCTION(BlueprintCallable, Category = "PAK")
	static void MountAndRegisterPak(FString PakFilePath,bool& bIsMountSuccessful);

	UFUNCTION(BlueprintCallable, Category = "PAK")
	static bool MountPakFile(const FString& PakFilePath, const FString& PakMountPoint);

	UFUNCTION(BlueprintCallable, Category = "PAK")
	static bool UnmountPakFile(const FString& PakFilePath);

	UFUNCTION(BlueprintCallable, Category = "PAK")
	static void RegisterMountPoint(const FString& RootPath, const FString& ContentPath);

	UFUNCTION(BlueprintCallable, Category = "PAK")
	static void UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "PAK")
	static FString const GetPakMountPoint(const FString& PakFilePath);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "PAK")
	static TArray<FString> GetPakContent(const FString& PakFilePath, bool bOnlyCooked = true);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "PAK")
	static FString GetPakMountContentPath(const FString& PakFilePath);

	/*
	 *	查找蓝图类的UClass, 末尾带_C
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PAK")
	static UClass* LoadBlueprintClass(FString PakContentPath);

	/*
	 * 获取虚幻资源路径
	 *  例：
	 *		输入 xxxxxx/Content/Path/MyAsset.umap
	 *		得到 /Game/Path/MyAsset.MyAsset 
	 */
	static FString GetUnrealPath(const FString& Filename);

	/*
	 * 获取虚幻路径和资产基础名称
	 */
	static void GetUnrealPathAndBaseName(const FString& Filename, FString& OutUnrealPath, FString& OutBaseName);

	/*
	 * 打开pak中的第一个找到的场景
	 */
	UFUNCTION(BlueprintCallable, Category = "PAK")
	static void LoadFirstLevel(const FString& PakFilePath, UObject* InContextObject);

	/*
	 * 流式加载pak中第一个找到场景
	 */
	UFUNCTION(BlueprintCallable, Category = "PAK")
	static void StreamFirstLevel(const FString& PakFilePath, UObject* InContextObject);
};
