//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "CurrentMap.generated.h"
//
//UCLASS()
//class KIMYUMINDEMO_API ACurrentMap : public AActor
//{
//	GENERATED_BODY()
//
//
//	float TargetEmissiveValue;
//	float TargetOpacityValue;
//	float TargetDissolveValue;
//	float CurrentEmissiveValue;
//	float CurrentOpacityValue;
//	float CurrentDissolveValue;
//
//
//	bool IsChange;
//	bool IsOpenMap;
//
//	UMaterialInstanceDynamic* DMI_map;
//
//public:	
//	// Sets default values for this actor's properties
//	ACurrentMap();
//
//
//	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
//	class UStaticMeshComponent* MapMesh;
//
//
//	UPROPERTY(EditAnywhere)
//	class UMaterialInterface* MI_Map; //3D맵 모드 시 바뀔 머티리얼
//
//	class UMaterialInterface* MI_default; //기존 머티리얼 저장
//
//	UFUNCTION()
//	void ConvertMapMode();
//
//	UFUNCTION()
//	void ConvertDefaultMode();
//
//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//};
