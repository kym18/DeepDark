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

    // 1. SceneCapture2D에 자기 자신만 보이도록 설정
    if (SceneCapture2D) {
        SceneCapture2D->ShowOnlyActorComponents(this); 
        CameraOriginLocation = SceneCapture2D->GetComponentLocation();
    }

    // 2. SkeletalMesh의 월드 회전 저장
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

    // 조건 확인: 9 <= num <= 11
    if (num >= 9 && num <= 11) {
        // Scale 0.4, 위치 조정
        SkeletalMesh->SetWorldScale3D(FVector(0.4f));
        SkeletalMesh->SetRelativeLocation(FVector(0.f, -50.f, -80.f));
    }
    else
    {
        // Scale 1.0, 위치 초기화
        SkeletalMesh->SetWorldScale3D(FVector(1.0f));
        SkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    }

    // 메시 변경
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

    // 마우스 이동량 계산
    FVector2D MouseDelta = MouseLocation - StartRotateLocation;

    // 회전 생성 (Yaw만 적용, X축 이동 → Yaw)
    FRotator DeltaRotation = FRotator(0.f, 0.f, -1.0f * MouseDelta.X);  // 블루프린트 상 -1 곱함

    // 회전 적용
    SkeletalMesh->AddWorldRotation(DeltaRotation);

    // 회전 시작 위치 갱신
    StartRotateLocation = MouseLocation;
}


// Called every frame
void APickActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

