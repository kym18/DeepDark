// Fill out your copyright notice in the Description page of Project Settings.


#include "PickActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"




// Sets default values
APickActor::APickActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Root component
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    // Skeletal Mesh
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootComponent);
    SkeletalMesh->SetRelativeLocation(FVector(0, -70.f, 0.f));

    // Point Light
    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLight->SetupAttachment(SkeletalMesh);
    PointLight->SetRelativeLocation(FVector(0, 0.f, 317.f));

    // Sky Light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(PointLight);
    SkyLight->SetRelativeLocation(FVector(0, 0.f, -317.f));

    // Scene Capture Component 2D
    SceneCapture2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture2D"));
    SceneCapture2D->SetupAttachment(RootComponent);

    CanRotate = false;
}

// Called when the game starts or when spawned
void APickActor::BeginPlay()
{
	Super::BeginPlay();

    // 1. SceneCapture2D�� �ڱ� �ڽŸ� ���̵��� ����
    if (SceneCapture2D) {
        SceneCapture2D->ShowOnlyActorComponents(this); 
        CameraOriginLocation = SceneCapture2D->GetComponentLocation();
    }

    // 2. SkeletalMesh�� ���� ȸ�� ����
    if (SkeletalMesh) {
        OriginalRotate = SkeletalMesh->GetComponentRotation();
    }
}

void APickActor::ChangeMesh(int num)
{
    if (!SkeletalMesh || !MeshList.IsValidIndex(num)) {
        UE_LOG(LogTemp, Warning, TEXT("Invalid SkeletalMesh or MeshList index"));
        return;
    }

    // ���� Ȯ��: 9 <= num <= 11
    if (num >= 9 && num <= 11) {
        // Scale 0.4, ��ġ ����
        SkeletalMesh->SetWorldScale3D(FVector(0.4f));
        SkeletalMesh->SetRelativeLocation(FVector(0.f, -50.f, -80.f));
    }
    else
    {
        // Scale 1.0, ��ġ �ʱ�ȭ
        SkeletalMesh->SetWorldScale3D(FVector(1.0f));
        SkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    }

    // �޽� ����
    SkeletalMesh->SetSkeletalMesh(MeshList[num]);
}

void APickActor::StartRotate(FVector2D MouseLocation)
{
    StartRotateLocation = MouseLocation;
    CanRotate = true;
}

void APickActor::EndRotate()
{
    CanRotate = false;
}

void APickActor::Reset()
{
    SkeletalMesh->SetWorldRotation(OriginalRotate);
    SceneCapture2D->SetWorldLocation(CameraOriginLocation);
}

void APickActor::MinusZoom()
{
    SceneCapture2D->AddWorldOffset(FVector(0.f, 50.f, 0.f));
}

void APickActor::StartZoom()
{
    float temp_location = SceneCapture2D->GetComponentLocation().Y - SkeletalMesh->GetComponentLocation().Y;

    if (temp_location > 50.f) {
        SceneCapture2D->AddWorldOffset(FVector(0.f, -50.f, 0.f));
    }
}

void APickActor::MouseMove(FVector2D MouseLocation)
{
    if (!CanRotate || !SkeletalMesh) return;

    // ���콺 �̵��� ���
    FVector2D MouseDelta = MouseLocation - StartRotateLocation;

    // ȸ�� ���� (Yaw�� ����, X�� �̵� �� Yaw)
    FRotator DeltaRotation = FRotator(0.f, 0.f, -1.0f * MouseDelta.X);  // �������Ʈ �� -1 ����

    // ȸ�� ����
    SkeletalMesh->AddWorldRotation(DeltaRotation);

    // ȸ�� ���� ��ġ ����
    StartRotateLocation = MouseLocation;
}


// Called every frame
void APickActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

