// Copyright Epic Games, Inc. All Rights Reserved.

#include "KimyuminDemoCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Components/ChildActorComponent.h"

#include "MyGameInstance.h"
#include "Flare/Flare.h"

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


	isInCave = false;

	//조명
	flareNum = 4;
	flareSpeed = 1000.f;

	//대시
	isDash = false;

	//도감
	isPictorialOpen = false;
}

void AKimyuminDemoCharacter::BeginPlay()
{
	Super::BeginPlay();

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
			UE_LOG(LogTemp, Warning, TEXT("map_select_ui"));
			wbMapSelect = map_select_ui;
		}

		if (UUserWidget* store_ui = CreateWidget<UUserWidget>(GetWorld(), WB_UIStore)) {
			UE_LOG(LogTemp, Warning, TEXT("store_ui"));
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
	/*FTimerHandle OxygenTimerHandle;
	GetWorldTimerManager().SetTimer(
		OxygenTimerHandle,                  
		this,                                
		&AKimyuminDemoCharacter::DecreaseOxygen,            
		1.0f,                               
		true
	);*/

}

//void AKimyuminDemoCharacter::DecreaseOxygen() 
//{
//	//임시
//	UE_LOG(LogTemp, Warning, TEXT("Oxygen Decreased"));
//}

void AKimyuminDemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaSeconds = DeltaTime;
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
	FVector velocity = forward_vector * flareSpeed;

	// 피융
	if (UStaticMeshComponent* flare_mesh = flare->FlareMesh){
		flare_mesh->SetSimulatePhysics(true);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *velocity.ToString());
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
