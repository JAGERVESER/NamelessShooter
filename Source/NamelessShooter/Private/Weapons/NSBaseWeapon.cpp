// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NSBaseWeapon.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Player/NSBaseCharacter.h"
#include "Engine/World.h"
#include "Weapons/Components/NSWeaponFXComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANSBaseWeapon::ANSBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>("SceneRootComponent");
	SetRootComponent(SceneRootComponent);

	WeaponFXComponent = CreateDefaultSubobject<UNSWeaponFXComponent>("WeaponFXComponent");
}

void ANSBaseWeapon::BeginPlay()
{

	Super::BeginPlay();
	CurrentBulletsNum = BulletsNum;
	Reload();
}

void ANSBaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ���������� ����� ������������
void ANSBaseWeapon::Shot()
{
	if (!GetOwner() || !GetWorld() || CurrentBulletInClipNum == 0) return;
	FHitResult HitResult;
	FVector TraceStart;
	FVector TraceEnd;

	GetTraceData(TraceStart, TraceEnd);

	GetHitResult(HitResult, TraceStart, TraceEnd);
	
	MakeHit(HitResult, TraceStart, TraceEnd);

	if (WeaponFXComponent) 
	{
		if (WeaponFXComponent->bPlayImpactFX || WeaponFXComponent->bSpawnDecal) WeaponFXComponent->PlayImpactFX(HitResult);
		if (WeaponFXComponent->bPlayMuzzleFX) WeaponFXComponent->PlayMuzzleFX(GetMesh(), MuzzleSocketName);
		if (WeaponFXComponent->bPlayTraceFX) WeaponFXComponent->PlayTraceFX(TraceStart, HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd);
	}

	DecreaseAmmo();
}

// ������ player controller
APlayerController* ANSBaseWeapon::GetPlayerController() const
{
	//if (!GetOwner() || !GetWorld()) return &APlayerController();
	const auto Player = Cast<ACharacter>(GetOwner());
	if (!Player) return nullptr;

	return Player->GetController<APlayerController>();
}

// �������� ���������� ���� ������
FVector ANSBaseWeapon::GetMuzzleWorldLocation() const
{
	//if (!GetOwner() || !GetWorld()) return;
	return WeaponMesh->GetSocketLocation(MuzzleSocketName);
}

void ANSBaseWeapon::DecreaseAmmo()
{
	CurrentBulletInClipNum--;
	if (CurrentBulletInClipNum == 0 && MayReload())
	{
		auto Character = Cast<ANSBaseCharacter>(GetOwner());
		Character->WeaponReload();
	}
}

// ����� ��������� ���� ��� ������ (������ / �����)
void ANSBaseWeapon::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{	
	if (!GetOwner() || !GetWorld()) return;

	TraceStart = GetMuzzleWorldLocation();
	const FVector ShootDirection = GetOwner()->GetActorForwardVector();
	TraceEnd = GetOwner()->GetActorLocation() + ShootDirection * TraceMaxDistance;
}

// ������ line trace � �������� ���� � ���������, � ����������� �� ���� ���� ������ ������� ����.
void ANSBaseWeapon::GetHitResult(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd)
{
	if (!GetOwner() || !GetWorld()) return;

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	CollisionQueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionQueryParams);
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);
}

void ANSBaseWeapon::MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd)
{
	if (!GetOwner() || !GetWorld()) return;
	if (HitResult.bBlockingHit)
	{
		if (HitResult.GetActor()->IsA<ANSBaseCharacter>())
		{
			auto Character = Cast<ANSBaseCharacter>(HitResult.GetActor());

			Character->TakeDamage(DamageGiven, FPointDamageEvent(), GetPlayerController(), GetOwner());
			if(Character->IsDead()) Character->GetMesh()->AddImpulse((TraceEnd - TraceStart).GetSafeNormal() * 5000, "Pelvis", true);
			else Character->GetCharacterMovement()->AddImpulse((TraceEnd - TraceStart).GetSafeNormal() * 1000, true);
		}
	}
}

// ����������� ������
void ANSBaseWeapon::Reload()
{
	if (!MayReload()) return;

	if (CurrentBulletsNum - (MaxBulletsInClipNum - CurrentBulletInClipNum) >= 0)
	{	
		CurrentBulletsNum = CurrentBulletsNum - (MaxBulletsInClipNum - CurrentBulletInClipNum);
		CurrentBulletInClipNum = MaxBulletsInClipNum;
	}
	else 
	{
		CurrentBulletInClipNum = CurrentBulletsNum;
		CurrentBulletsNum = 0;
	}
}

bool ANSBaseWeapon::MayReload()
{
	return !(!GetWorld() || !GetOwner() || CurrentBulletsNum == 0 || CurrentBulletInClipNum == MaxBulletsInClipNum);
}

