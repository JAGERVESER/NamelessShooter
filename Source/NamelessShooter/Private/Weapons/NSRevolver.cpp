// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NSRevolver.h"

ANSRevolver::ANSRevolver()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(GetRootComponent());
}
