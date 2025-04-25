// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickActor.generated.h"

UCLASS()
class KIMYUMINDEMO_API APickActor : public AActor
{
	GENERATED_BODY()

    // Components
    UPROPERTY(EditAnywhere)
    USceneComponent* Root;

    UPROPERTY(EditAnywhere)
    USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(EditAnywhere)
    class UPointLightComponent* PointLight;

    UPROPERTY(EditAnywhere)
    USkyLightComponent* SkyLight;

    UPROPERTY(EditAnywhere)
    class USceneCaptureComponent2D* SceneCapture2D;


private:
    FRotator OriginalRotate;
    FVector CameraOriginLocation;

    UFUNCTION(BlueprintCallable)
    void ChangeMesh(int num);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TArray<USkeletalMesh*> MeshList;

    UFUNCTION(BlueprintCallable)
    void StartRotate(FVector2D MouseLocation);

    FVector2D StartRotateLocation;

    bool CanRotate;

    UFUNCTION(BlueprintCallable)
    void EndRotate();

    UFUNCTION(BlueprintCallable)
    void Reset();

    UFUNCTION(BlueprintCallable)
    void MinusZoom();

    UFUNCTION(BlueprintCallable)
    void StartZoom();

    UFUNCTION(BlueprintCallable)
    void MouseMove(FVector2D MouseLocation);
	
public:	
	// Sets default values for this actor's properties
	APickActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
