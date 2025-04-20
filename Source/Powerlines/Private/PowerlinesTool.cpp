// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerlinesTool.h"
#include "Drawing/ScriptableToolLineSet.h"
#include "Drawing/ScriptableToolLine.h"
#include "Drawing/ScriptableToolPointSet.h"
#include "Drawing/ScriptableToolPoint.h"
#include "Kismet/KismetMathLibrary.h"
#include"UObject/UnrealType.h"

UPowerlinesTool::UPowerlinesTool()
{

}

void UPowerlinesTool::Setup()
{
    UScriptableModularBehaviorTool::Setup();
    
    int capturePriority =0;

     CanBeginClickSequence.BindDynamic(this, &UPowerlinesTool::CanBeginSequence);
    
     OnBeginSequencePreview.BindDynamic(this, &UPowerlinesTool::ShowLinePreview);
     OnNextSequencePreview.BindDynamic(this, &UPowerlinesTool::ShowLinePreview);

     OnBeginClickSequence.BindDynamic(this, &UPowerlinesTool::ShowLineAndUpdatePowerlines);
     OnNextSequenceClick.BindDynamic(this, &UPowerlinesTool::ShowLineAndCreateOrUpdatePowerlines);
     OnTerminateClickSequence.BindDynamic(this, &UPowerlinesTool::RemoveLinesAndPoints);
    
    AddMultiClickSequenceBehavior(OnBeginSequencePreview,  
    CanBeginClickSequence,  
    OnBeginClickSequence,  
    OnNextSequencePreview,  
    OnNextSequenceClick,  
    OnTerminateClickSequence,  
    RequestAbortClickSequence,  
     CaptureCheck,  
    HoverCaptureCheck,  
    capturePriority,  
    EScriptableToolMouseButton::LeftButton);

    LineSet = AddLineSet();
    PointSet = AddPointSet();

}

bool UPowerlinesTool::CanBeginSequence(FInputDeviceRay ClickPos, EScriptableToolMouseButton MouseButton)
{
    return true;
}

void UPowerlinesTool::ShowLinePreview(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers,
                                      EScriptableToolMouseButton MouseButton)
{
    FVector cursorLocation = GetLocationUnderCursor(ClickPos);
    
    if (CurrentLinePreview)
    {
        CurrentLinePreview->SetLineEnd(cursorLocation);
        CurrentLinePreview->SetLineColor(LinePreviewColor);
    }

}

void UPowerlinesTool::ShowLineAndUpdatePowerlines(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers,
    EScriptableToolMouseButton MouseButton)
{
    FVector cursorLocation = GetLocationUnderCursor(ClickPos);
    AddLineSegment(cursorLocation);

    FVector lineStart, lineEnd;
    GetLastTwoRoutePoints(lineStart, lineEnd);
    if (SpawnedPowerlines)
    {
        SpawnedPowerlines->AddSplinePoints(lineStart, lineEnd);
        SpawnedPowerlines->UpdatePowerlines();
    }
}

bool UPowerlinesTool::ShowLineAndCreateOrUpdatePowerlines(FInputDeviceRay ClickPos,
    FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
    FVector cursorLocation = GetLocationUnderCursor(ClickPos);
    AddLineSegment(cursorLocation);
    
    FVector lineStart, lineEnd;
    GetLastTwoRoutePoints(lineStart, lineEnd);
    if (SpawnedPowerlines)
    {
        SpawnedPowerlines->AddSplinePoints(lineStart, lineEnd);
        SpawnedPowerlines->UpdatePowerlines();
    }
    else
    {
        SpawnedPowerlines = SpawnPowerlines(lineStart, lineEnd);
        SpawnedPowerlines->UpdatePowerlines();
    }

    return true;
}

void UPowerlinesTool::Shutdown(EToolShutdownType ShutdownType)
{
    UScriptableModularBehaviorTool::Shutdown(ShutdownType);
}


void UPowerlinesTool::RemoveLinesAndPoints(FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
    LineSet->RemoveAllLines();
    PointSet->RemoveAllPoints();
}

void UPowerlinesTool::AddLineSegment(const FVector& PositionToAdd)
{
    SplinePoints.Add(PositionToAdd);
    UScriptableToolPoint * point = PointSet->AddPoint();
    
    point->SetPointSize(10.0f);
    point->SetPointPosition(PositionToAdd);

    FVector offsetPosition = PositionToAdd + FVector(0.0f, 0.0f, 10.0f);
    if (CurrentLinePreview)
    {
        CurrentLinePreview->SetLineEnd(offsetPosition);
        CurrentLinePreview->SetLineColor(LineLockedColor);
    }

    CurrentLinePreview = LineSet->AddLine();
    CurrentLinePreview->SetLineEndPoints(offsetPosition,offsetPosition);
    CurrentLinePreview->SetLineColor(LinePreviewColor);
    CurrentLinePreview->SetLineThickness(5.0f);

}

FVector UPowerlinesTool::GetLocationUnderCursor(FInputDeviceRay Ray) const
{
    UWorld* world = GetWorld();
    if (!world) return  FVector::ZeroVector;

    float rayDistance = 1000000.0f;
    FVector start = Ray.WorldRay.Origin;
    FVector end = start + Ray.WorldRay.Direction * rayDistance;
    FHitResult hitResult;
    
    bool bHitL = world->LineTraceSingleByChannel(
        hitResult,  
        start,      
        end,        
        ECC_Visibility, 
        FCollisionQueryParams::DefaultQueryParam, 
        FCollisionResponseParams::DefaultResponseParam 
    );
	
    if (bHitL)
    {
        return hitResult.Location;
    }
    return FVector::ZeroVector;
}

APowergrid * UPowerlinesTool::SpawnPowerlines(const FVector &SpawnLocation, const FVector &DestinationLocation) const
{
    float lineLength = FVector::Dist(SpawnLocation, DestinationLocation);
    int poles = FMath::Floor(lineLength * (1/CableLength)) + 1;
    
    FActorSpawnParameters spawnParams;
    spawnParams.bDeferConstruction = true;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, DestinationLocation);
    
    if (lineLength > CableLength)
    {
        APowergrid * spawnedPowergrid = GetWorld()->SpawnActor<APowergrid>(APowergrid::StaticClass(),SpawnLocation, spawnRotation, spawnParams);

        if (spawnedPowergrid)
        {
            spawnedPowergrid->InitializePowerlineParams(PowerlinesMesh, poles, CableLength);
            spawnedPowergrid->FinishSpawning(FTransform(spawnRotation, SpawnLocation));
        }
        return spawnedPowergrid;
    }
    
    return nullptr;
}

void UPowerlinesTool::GetLastTwoRoutePoints(FVector& SecondToLast, FVector& Last) const
{
    if (SplinePoints.Num() > 1)
    {
        Last = SplinePoints[SplinePoints.Num() - 1];
        SecondToLast = SplinePoints[SplinePoints.Num() - 2];
    }
}
