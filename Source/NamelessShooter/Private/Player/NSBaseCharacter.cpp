// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NSBaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/NSHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/NSWeaponComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/NSBaseWeapon.h"

ANSBaseCharacter::ANSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->TargetArmLength = 0.0f;
	SpringArmComponent->SocketOffset.Z = 1000.0f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetWorldRotation(FRotator(0.0f, 0.0f, -90.0f));

	HealthComponent = CreateDefaultSubobject<UNSHealthComponent>("HealthComponent");
	WeaponComponent = CreateDefaultSubobject<UNSWeaponComponent>("WeaponComponent");

	bUseControllerRotationYaw = false;
}

void ANSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(WeaponComponent);
	check(HealthComponent);
	
	HealthComponent->OnDeath.AddUObject(this, &ANSBaseCharacter::OnDeath);
	HealthComponent->OnHealthChanged.AddUObject(this, &ANSBaseCharacter::OnHealthChanged);

}

void ANSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetPawnRotationToMouse();

}

// ������� ������
void ANSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveAlong", this, &ANSBaseCharacter::MoveAlong);
	PlayerInputComponent->BindAxis("MoveAcross", this, &ANSBaseCharacter::MoveAcross);
	PlayerInputComponent->BindAxis("LookAlong", this, &ANSBaseCharacter::LookAlong);
	PlayerInputComponent->BindAxis("LookAcross", this, &ANSBaseCharacter::LookAcross);
	PlayerInputComponent->BindAction("WantsToLookAround", IE_Pressed, this, &ANSBaseCharacter::StartLookingAround);
	PlayerInputComponent->BindAction("WantsToLookAround", IE_Released, this, &ANSBaseCharacter::StopLookingAround);
	PlayerInputComponent->BindAction("Shot", IE_Pressed, this, &ANSBaseCharacter::Shot);
	PlayerInputComponent->BindAction("WeaponReload", IE_Pressed, this, &ANSBaseCharacter::WeaponReload);
}

// ���� ��� ������� �� �������� ����� (������ W � S)
void ANSBaseCharacter::MoveAlong(float Amount)
{
	if (Amount == 0.0f) return;
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Amount);
}

// ���� ��� ������� �� �������� ������� (������ A � D)
void ANSBaseCharacter::MoveAcross(float Amount)
{
	if (Amount == 0.0f) return;
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Amount);
}

// ���������� ���� ����� ��������� � forward vector ���������
float ANSBaseCharacter::GetMovementDirection()
{ 
	const auto VelocityNormal = GetVelocity().GetSafeNormal();
	const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
	const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
	const auto Degreese = FMath::RadiansToDegrees(AngleBetween);
	return CrossProduct.IsZero() ? Degreese : Degreese * FMath::Sign(CrossProduct.Z);
}

// ���� ��� ������� �� ������ ��������� (����� �����)
void ANSBaseCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);

	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	WeaponComponent->OnOwnerDeath();

	OnCharacterDeath();
}

// ���� ��� ������� �� ��������� �������� ���������
void ANSBaseCharacter::OnHealthChanged(float Health, float DeltaHealth)
{
	if (DeltaHealth < 0) OnTakeDamage();
}
// ������� ����� � ����� 
void ANSBaseCharacter::SetPawnRotationToMouse()
{
	if (!GetWorld()) return;

	const auto MouseLocationByCharacter = GetMouseLocationByCharacter();
	const auto ActorXYVec = FVector(GetActorLocation().X, GetActorLocation().Y, 0.0f);
	const auto MouseXYVec = FVector(MouseLocationByCharacter.X, MouseLocationByCharacter.Y, 0.0f);
	//(���� ����� ����� 120 �� � ����� �� ����������)
	if ((MouseXYVec - ActorXYVec).Size() > 70.0f)
	{
		NeedToRotating = UKismetMathLibrary::FindLookAtRotation(ActorXYVec, MouseXYVec);
	}

	SetActorRotation(FRotator(GetActorRotation().Pitch, NeedToRotating.Yaw, GetActorRotation().Roll));
	//DrawDebugLine(GetWorld(), GetActorLocation(), MouseLocationByCharacter, FColor::Red, false, 0.2f);
}
// ������� player controller
APlayerController* ANSBaseCharacter::GetPlayerController()
{
	return Cast<APlayerController>(Controller);

}
// �������� ����� ����� � ���� ������������ ��������� (����� ���������)
FVector ANSBaseCharacter::GetMouseLocationByCharacter()
{
	const auto PlayerController = GetPlayerController();
	if (!PlayerController) return FVector();

	FVector MouseLocation;
	FVector MouseDirection;

	PlayerController->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
	const auto CurrentCos = FVector::DotProduct(FVector(0.0f, 0.0f,-1.0f), MouseDirection);
	const auto CurrentMouseWorldLocation = MouseLocation + MouseDirection * ((SpringArmComponent->SocketOffset.Z - 20.0f) / CurrentCos);

	return CurrentMouseWorldLocation;
}
// ���� ��� �� ������ ������ ���������
void ANSBaseCharacter::StartLookingAround()
{	
	bWantsLookAround = true;

	//if (bReloadAnimMontageInProgress) return;
	WeaponComponent->StartAim();
}

// ���� ��� �� ����� ������ ���������
void ANSBaseCharacter::StopLookingAround()
{
	bWantsLookAround = false;
	SpringArmComponent->SocketOffset.Y = 0;
	SpringArmComponent->SocketOffset.X = 0;

	//if (bReloadAnimMontageInProgress) return;
	WeaponComponent->StopAim();
}

// ����� �� ��� y
void ANSBaseCharacter::LookAlong(float Amount)
{
	if (!bWantsLookAround || Amount == 0) return;

	// ���� ������ �������� ��������� �� �������
	if (FMath::Abs(SpringArmComponent->SocketOffset.Y + CameraLookoutVelocity * Amount) > MaxCameraLookoutDistance)
	{
		return;
	}
	SpringArmComponent->SocketOffset.Y += CameraLookoutVelocity * Amount;
}

// ����� �� ��� x
void ANSBaseCharacter::LookAcross(float Amount)
{
	if (!bWantsLookAround || Amount == 0) return;

	// ���� ������ �������� ��������� �� �������
	if (FMath::Abs(SpringArmComponent->SocketOffset.X + CameraLookoutVelocity * Amount) > MaxCameraLookoutDistance)
	{
		return;
	}
	SpringArmComponent->SocketOffset.X += CameraLookoutVelocity * Amount;
}
// �� �������
void ANSBaseCharacter::Shot()
{
	if (bReloadAnimMontageInProgress) return;
	WeaponComponent->Shot();
}

// �����������
void ANSBaseCharacter::WeaponReload()
{
	if (bReloadAnimMontageInProgress) return;
	WeaponComponent->Reload();
}
bool ANSBaseCharacter::IsDead()
{
	return HealthComponent->IsDead();
}
// ������ ������
ANSBaseWeapon* ANSBaseCharacter::GetCurrentWeapon() const
{
	return WeaponComponent->GetCurrentWeapon(); 
}
