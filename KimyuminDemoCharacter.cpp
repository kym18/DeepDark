// Copyright Epic Games, Inc. All Rights Reserved.

#include "KimyuminDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Components/ChildActorComponent.h"

#include "MyGameInstance.h"
#include "Flare/Flare.h"
#include "Particles/ParticleSystem.h"

#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Mineral.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AKimyuminDemoCharacter

AKimyuminDemoCharacter::AKimyuminDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	// Grapple
	GrappleStartLocation = CreateDefaultSubobject<USceneComponent>(TEXT("GrappleStartLocation"));
	GrappleStartLocation->SetupAttachment(RootComponent);

	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleCable"));
	GrappleCable->SetupAttachment(GrappleStartLocation);


	 // 1. FirstPerson카메라
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

		// 2. LaserMesh
		LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
		LaserMesh->SetupAttachment(FirstPersonCamera);

			// 3. LaserArrow (LaserMesh 아래)
			LaserArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LaserArrow"));
			LaserArrow->SetupAttachment(LaserMesh);

				// 4. LaserArrow 아래 LaserSphere
				//LaserSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserSphere"));
				//LaserSphere->SetupAttachment(LaserArrow);

					// 5. LaserMesh 아래 다른 메시들
					DissolveLaser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DissolveLaser"));
					DissolveLaser->SetupAttachment(LaserArrow);
					DissolveLaser->SetVisibility(false);

					Laser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Laser"));
					Laser->SetupAttachment(LaserArrow);
					Laser->SetVisibility(false);

		// 6. RifleMesh (루트에 붙일 수도, FirstPersonCamera에 붙일 수도 있음)
		RifleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RifleMesh"));
		RifleMesh->SetupAttachment(FirstPersonCamera); // 또는 SetupAttachment(FirstPersonCamera);

	isInCave = false;
	CurrentCharacterIndex = 0;

	//모드
	modeNumber = 0;

	//조명
	flareNum = 4;
	flareSpeed = 1000.f;

	//대시
	isDash = false;

	//도감
	isPictorialOpen = false;

	//공격 관련
	grappleDistance = 3000.f;
}

void AKimyuminDemoCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetDefaultMode();

	//게임 인스턴스 저장
	UGameInstance* GameInstance = GetGameInstance();
	if (UMyGameInstance* my_game_instance = Cast<UMyGameInstance>(GameInstance)){
		myGameInstance = my_game_instance;
	}

	//위젯 생성
	if (WB_UIClass && WB_UIPictorialBook && WB_UIMapSelect && WB_UIStore) {
		if (UUserWidget* wb_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIClass)) {
			if (isInCave) {
				wb_ui->AddToViewport();
			}
		}

		if (UUserWidget* pictorial_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIPictorialBook)) {
			pictorialBook = pictorial_ui;
		}

		if (UUserWidget* map_select_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIMapSelect)) {
			wbMapSelect = map_select_ui;
		}

		if (UUserWidget* store_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIStore)) {
			wbStore = store_ui;
		}
	}

	//플레이어 카메라 조정
	APlayerController* player_controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (player_controller && player_controller->PlayerCameraManager) {
		player_controller->PlayerCameraManager->ViewPitchMin = -50.0f;
		player_controller->PlayerCameraManager->ViewPitchMax = 50.0f;
	}

	//산소 시스템
	//우주선 밖에서만 실행 되도록 수정 예정(임시)
	UE_LOG(LogTemp, Warning, TEXT("Oxygen Decreased OutOutOut InCave"));
	if (isInCave) {
		FTimerHandle OxygenTimerHandle;
		UE_LOG(LogTemp, Warning, TEXT("Oxygen Decreased Is InCave"));
		GetWorldTimerManager().SetTimer(
			OxygenTimerHandle,
			this,
			&AKimyuminDemoCharacter::DecreaseOxygen,
			1.0f,
			true
		);
	}

}

void AKimyuminDemoCharacter::DecreaseOxygen() 
{
	if (Oxygen <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("character die"));
		
		return;
	}
	Oxygen -= OxygenConsumptionRate;

}

void AKimyuminDemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaSeconds = DeltaTime;
}

void AKimyuminDemoCharacter::ModeChange()
{
	if (!isInCave) {
		SetDefaultMode();
	}
	else {
		if (modeNumber == 0) {
			SetDefaultMode();
		}
		else {
			SetWeaponMode();
		}
	}
}

void AKimyuminDemoCharacter::SetDefaultMode()
{
	modeNumber = 0;

	LaserMesh->SetVisibility(false);
	Laser->SetVisibility(false);
	DissolveLaser->SetVisibility(false);
	RifleMesh->SetVisibility(false);
	FirstPersonCamera->SetActive(false);
	FollowCamera->SetActive(true);
}

void AKimyuminDemoCharacter::SetWeaponMode()
{
	RifleMesh->SetVisibility(false);
	LaserMesh->SetVisibility(false);
	if (modeNumber == 1) {
		RifleMesh->SetVisibility(true);
	}
	else if (modeNumber == 2) {
		LaserMesh->SetVisibility(true);
	}
	FirstPersonCamera->SetActive(true);
	FollowCamera->SetActive(false);
}


void AKimyuminDemoCharacter::SpawnFlare()
{
	if (flareNum <= 0) {
		return;
	}

	flareNum--;

	if (!BP_Flare) return;

	// 1. 스폰 위치 및 회전
	FVector forward_vector = GetActorForwardVector();
	FVector spawn_location = GetActorLocation() + forward_vector * 40;

	// 2. 스폰 파라미터
	FActorSpawnParameters spawn_Params;
	spawn_Params.Owner = this;
	spawn_Params.Instigator = GetInstigator();

	// 3. 스폰 실행
	AActor* spawned_actor = GetWorld()->SpawnActor<AActor>(BP_Flare, spawn_location, GetActorRotation(), spawn_Params);
	if (!spawned_actor) return;

	AFlare* flare = Cast<AFlare>(spawned_actor);
	if (!flare) return;


	// 4. 물리 속도 적용
	FVector velocity = (forward_vector + FVector(0.f, 0.f, 1.f)) * flareSpeed;

	// 피융
	if (UStaticMeshComponent* flare_mesh = flare->FlareMesh){
		flare_mesh->SetSimulatePhysics(true);
		flare_mesh->SetPhysicsLinearVelocity(velocity);
	}

}

void AKimyuminDemoCharacter::DashFlipFlop()
{
	if (!isDash) {
	//걷기 -> 대시 중
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	else {
	//대시 중 -> 걷기
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
	isDash = !isDash;
}

void AKimyuminDemoCharacter::PictorialFlipFlop()
{
	if (!pictorialBook) return;
	if (!isInCave) return;

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!pictorialBook->IsInViewport()) {
		pictorialBook->AddToViewport();

		// 마우스 커서 ON
		controller->bShowMouseCursor = true;

		// 입력 모드: 게임 + UI
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(pictorialBook->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		controller->SetInputMode(InputMode);
	}
	else {
		pictorialBook->RemoveFromParent();

		// 마우스 커서 OFF
		controller->bShowMouseCursor = false;

		// 입력 모드: 게임
		FInputModeGameOnly InputMode;
		controller->SetInputMode(InputMode);
	}
}

//비효율적임. 바꿔야함
void AKimyuminDemoCharacter::MapAndStoreFlipFlop()
{
	//맵 또는 상점 둘중에 하나라도 없으면 return
	if (!wbMapSelect || !wbStore) return;

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	//맵 선택 UI 오픈
	if(isOverlapMapSelectZone){
		if (!wbMapSelect->IsInViewport()) {
			wbMapSelect->AddToViewport();
			// 마우스 커서 ON
			controller->bShowMouseCursor = true;

			// 입력 모드: 게임 + UI
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(wbMapSelect->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(true);
			controller->SetInputMode(InputMode);
		}
		else {
			wbMapSelect->RemoveFromParent();

			// 마우스 커서 OFF
			controller->bShowMouseCursor = false;

			// 입력 모드: 게임
			FInputModeGameOnly InputMode;
			controller->SetInputMode(InputMode);
		}
	}

	//상점 UI 오픈
	if (isOverlapStore) {
		if (!wbStore->IsInViewport()) {
			wbStore->AddToViewport();
			// 마우스 커서 ON
			controller->bShowMouseCursor = true;

			// 입력 모드: 게임 + UI
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(wbStore->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(true);
			controller->SetInputMode(InputMode);
		}
		else {
			wbStore->RemoveFromParent();

			// 마우스 커서 OFF
			controller->bShowMouseCursor = false;

			// 입력 모드: 게임
			FInputModeGameOnly InputMode;
			controller->SetInputMode(InputMode);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void AKimyuminDemoCharacter::LeftMouseBtnPressed()
{
	if (modeNumber == 1) { //총
		// 1. 반동 적용
		RifleMesh->AddLocalRotation(FRotator(-2.0f, 0.f, 0.f)); // 위로 반동

		// 2. 라인 트레이스 계산
		FVector Start = FirstPersonCamera->GetComponentLocation();
		FVector End = Start + FirstPersonCamera->GetForwardVector() * 100000.0f;

		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		TraceParams.bReturnPhysicalMaterial = true;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_Visibility,
			TraceParams
		);

		// 3. 피격 처리
		if (bHit){
			// 이펙트 스폰
			if (ExplosionFX){
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ExplosionFX,
					Hit.ImpactPoint,
					FRotator::ZeroRotator,
					FVector(1.0f) // Scale
				);
			}

			// 데미지 적용
			UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				10.0f,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}

		// 4. 반동 복귀
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this](){
			RifleMesh->AddLocalRotation(FRotator(2.0f, 0.f, 0.f)); // 원위치
		}, 0.05f, false);
	}
	else if(modeNumber == 2) { //레이저
		Laser->SetVisibility(true);

		StartFiringLaser();
	}
	else if (modeNumber == 3) { //특수 능력
		//0은 탐험가, 1은 전투 특화
		if (CurrentCharacterIndex == 0) {
			IsHoldingF = true;
			// 1. 라인트레이스 시작과 끝 위치
			FVector Start = FirstPersonCamera->GetComponentLocation();
			FVector Direction = FirstPersonCamera->GetForwardVector();
			FVector End = Start + Direction * grappleDistance;

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this); // 자기 자신 무시
			Params.bReturnPhysicalMaterial = false;

			// 2. 라인트레이스
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				Params
			);

			// 3. 히트한 경우 Dissolve 액터 스폰
			if (bHit) {
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(Hit.ImpactPoint);
				SpawnTransform.SetRotation(FQuat::Identity);
				SpawnTransform.SetScale3D(FVector(1.0f));

				AActor* Spawned = GetWorld()->SpawnActor<AActor>(DissolveClass, SpawnTransform);
				currentHole = Spawned;
			}
		}
		else if (CurrentCharacterIndex == 1) {
			// 1. 상태 설정
			isGrappling = true;
			FRotator CameraRotate = FRotator::ZeroRotator; // 이후 회전 처리에 사용
			GetCharacterMovement()->GravityScale = 1.0f;

			// 2. 라인트레이스 정보 계산
			FVector Start = FirstPersonCamera->GetComponentLocation();
			FVector Forward = FirstPersonCamera->GetForwardVector();
			FVector End = Start + (Forward * grappleDistance);

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.bReturnPhysicalMaterial = false;

			// 3. 트레이스 실행
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				Params
			);

			// 4. 히트 성공 시 회전 계산
			if (bHit)
			{
				FVector TraceDir = (Hit.ImpactPoint - Start).GetSafeNormal();
				CameraRotate = TraceDir.ToOrientationRotator();

				FVector grapple_location = Hit.Location;

				// 1. 액터 스폰
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(grapple_location);
				SpawnTransform.SetRotation(GetActorRotation().Quaternion());
				SpawnTransform.SetScale3D(FVector(1.0f));
				Hook = GetWorld()->SpawnActor<AActor>(GrappleHookClass, SpawnTransform);

				GrappleCable->SetVisibility(true);

				GrappleCable->SetWorldLocation(grapple_location);

				FVector CurrentLocation = GetActorLocation();
				FVector Direction = (grapple_location - CurrentLocation).GetSafeNormal();
				FVector LaunchVelocity = Direction * 3000.f;

				//날기!!!
				LaunchCharacter(LaunchVelocity, false, false);
				FTimerHandle GrappleResetTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(GrappleResetTimerHandle, this, &AKimyuminDemoCharacter::ResetGrappleHook, 0.45f, false);
			}
			else {
				ResetGrappleHook();
			}
		}
	}
}

void AKimyuminDemoCharacter::ResetGrappleHook()
{

	// 1. 케이블 숨기기
	if (GrappleCable) {
		GrappleCable->SetVisibility(false);
	}

	// 2. 훅 제거
	if (IsValid(Hook)) {
		Hook->Destroy();
		Hook = nullptr;
	}

	// 3. 중력 스케일 복원
	GetCharacterMovement()->GravityScale = 1.0f;

	// 4. 상태 초기화
	isGrappling = false;

	// 5. 딜레이 후 CanGrapple = true
	FTimerHandle GrappleCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(
		GrappleCooldownTimer,
		this,
		&AKimyuminDemoCharacter::EnableGrapple,
		2.0,
		false
	);
}

void AKimyuminDemoCharacter::EnableGrapple(){
	isGrappling = true;
}

void AKimyuminDemoCharacter::StartFiringLaser()
{
	GetWorld()->GetTimerManager().SetTimer(LaserTimerHandle, this, &AKimyuminDemoCharacter::FireLaserTick, 0.05f, true);
}

void AKimyuminDemoCharacter::FireLaserTick()
{
	if (!LaserArrow || !Laser) return;

	FVector Start = LaserArrow->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * 100000.0f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit) {
		float DistanceToWall = Hit.Distance;

		// 1. 메시 길이 조정
		FVector NewScale = FVector(DistanceToWall * 1.0f, 0.17f, 0.17f); // Scale은 약간 보정 필요
		Laser->SetWorldScale3D(NewScale);

		// 2. 메시 회전 보정
		//FRotator RotationAdjust = FRotator(0.f, 0.f, 5.f); // 블루프린트에 따라 각도 조정
		//Laser->AddLocalRotation(RotationAdjust);

		 // 방향 벡터 계산 (벽면을 바라보게)
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Start, End);
		Laser->SetWorldRotation(LookAtRot);

		// 3. 충돌 이펙트 및 데미지
		if (LaserImpactFX){
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaserImpactFX, Hit.ImpactPoint);
		}

		UGameplayStatics::ApplyDamage(
			Hit.GetActor(),
			10.0f,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// 4. 충돌 액터 스폰 (BP_LaserCollision)
		if (LaserCollisionClass) {
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Hit.ImpactPoint);
			GetWorld()->SpawnActor<AActor>(LaserCollisionClass, SpawnTransform);
		}

		// 5. 광물 캐스팅 시도
		if (AMineral* Mineral = Cast<AMineral>(Hit.GetActor())){
			// TODO: 원하는 처리 추가
		}
	}
}


void AKimyuminDemoCharacter::LeftMouseBtnReleased()
{
	if (modeNumber == 2) { //레이저
		UE_LOG(LogTemp, Warning, TEXT("modeNumber==2"));
		Laser->SetVisibility(false);
		Laser->SetRelativeScale3D(FVector(2, 2, 2));
		GetWorld()->GetTimerManager().ClearTimer(LaserTimerHandle);
	}
	else if (modeNumber == 3) {
		if (!IsHoldingF) return;
		if (currentHole) {
			currentHole->Destroy();
		}
		IsHoldingF = false;
	}
}

void AKimyuminDemoCharacter::RightMouseBtnPressed()
{
	if (modeNumber != 2) return;

	// 1. DissolveLaser 활성화
	DissolveLaser->SetVisibility(true);

	// 2. 레이저 업데이트용 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(DissolveTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			FVector Start = LaserArrow->GetComponentLocation();
			FVector Direction = LaserArrow->GetForwardVector();
			FVector End = Start + Direction * 100000.0f;

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

			if (bHit){
				float DistanceToWall = Hit.Distance;

				// 3. DissolveLaser 크기 조정
				DissolveLaser->SetRelativeScale3D(FVector(DistanceToWall, 2.f, 2.f)); // 거리 보정
				DissolveLaser->AddLocalRotation(FRotator(5.f, 0.f, 0.f));

				// 4. 히트 지점에 Dissolve 이펙트 스폰
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(Hit.ImpactPoint);
				SpawnTransform.SetRotation(FQuat::Identity);
				SpawnTransform.SetScale3D(FVector(1.f));

				GetWorld()->SpawnActor<AActor>(DissolveCircleClass, SpawnTransform);
			}

		}), 0.05f, true);
}

void AKimyuminDemoCharacter::RightMouseBtnReleased()
{
	if (modeNumber != 2) return;

	DissolveLaser->SetVisibility(false);
	DissolveLaser->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
	
	GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);
}

void AKimyuminDemoCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AKimyuminDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKimyuminDemoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKimyuminDemoCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AKimyuminDemoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AKimyuminDemoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// 그리고 이것은 광물 획득 시 퍼센티지 올리는 거시다...
void AKimyuminDemoCharacter::UpdateMineral(FName MineralName)
{
	if (double* Existing = MineralList.Find(MineralName))
	{
		*Existing += 0.1;
	}

	else
	{
		MineralList.Add(MineralName, 0.1f);
	}
}
