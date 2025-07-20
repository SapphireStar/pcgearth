#include "WFCTileSet.h"

FWFCTileDefinition UWFCTileSet::GetTile(int32 Index) const
{
    if (Index >= 0 && Index < Tiles.Num())
    {
        return Tiles[Index];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WFCTileSet: Invalid tile index %d"), Index);
    return FWFCTileDefinition();
}

int32 UWFCTileSet::FindTileByName(const FString& TileName) const
{
    for (int32 i = 0; i < Tiles.Num(); i++)
    {
        if (Tiles[i].TileName.Equals(TileName, ESearchCase::IgnoreCase))
        {
            return i;
        }
    }
    return -1;
}

TArray<int32> UWFCTileSet::GetTilesByCategory(EWFCTileCategory Category) const
{
    TArray<int32> Result;
    for (int32 i = 0; i < Tiles.Num(); i++)
    {
        if (Tiles[i].Category == Category)
        {
            Result.Add(i);
        }
    }
    return Result;
}

bool UWFCTileSet::AreSocketsCompatible(const FString& Socket1, const FString& Socket2) const
{
    // 空Socket处理
    if (Socket1.IsEmpty() || Socket2.IsEmpty())
    {
        return Socket1.IsEmpty() && Socket2.IsEmpty();
    }

    // 查找Socket1的定义
    for (const FWFCSocket& SocketDef : SocketDefinitions)
    {
        if (SocketDef.SocketName.Equals(Socket1, ESearchCase::IgnoreCase))
        {
            return SocketDef.CompatibleSockets.Contains(Socket2);
        }
        if (SocketDef.SocketName.Equals(Socket2, ESearchCase::IgnoreCase))
        {
            return SocketDef.CompatibleSockets.Contains(Socket1);
        }
    }
    return false;
    //return Socket1.Equals(Socket2, ESearchCase::IgnoreCase);
}

FWFCSocket UWFCTileSet::GetSocketDefinition(const FString& SocketName) const
{
    for (const FWFCSocket& SocketDef : SocketDefinitions)
    {
        if (SocketDef.SocketName.Equals(SocketName, ESearchCase::IgnoreCase))
        {
            return SocketDef;
        }
    }
    
    return FWFCSocket(SocketName);
}

bool UWFCTileSet::ValidateTileSet(FString& OutErrorMessage) const
{
    TArray<FString> Errors;

    // 检查瓦片数量
    if (Tiles.Num() == 0)
    {
        Errors.Add(TEXT("No tiles defined in tile set"));
    }

    // 检查每个瓦片
    for (int32 i = 0; i < Tiles.Num(); i++)
    {
        const FWFCTileDefinition& Tile = Tiles[i];
        
        // 检查瓦片名称
        if (Tile.TileName.IsEmpty())
        {
            Errors.Add(FString::Printf(TEXT("Tile %d has empty name"), i));
        }

        // 检查Socket数量
        if (Tile.Sockets.Num() != 6)
        {
            Errors.Add(FString::Printf(TEXT("Tile %d (%s) has %d sockets, expected 6"), 
                i, *Tile.TileName, Tile.Sockets.Num()));
        }

        // 检查Mesh
        if (!Tile.Mesh)
        {
            Errors.Add(FString::Printf(TEXT("Tile %d (%s) has no mesh assigned"), i, *Tile.TileName));
        }
    }

    // 检查Socket定义
    TSet<FString> DefinedSockets;
    for (const FWFCSocket& SocketDef : SocketDefinitions)
    {
        if (SocketDef.SocketName.IsEmpty())
        {
            Errors.Add(TEXT("Found socket definition with empty name"));
            continue;
        }

        if (DefinedSockets.Contains(SocketDef.SocketName))
        {
            Errors.Add(FString::Printf(TEXT("Duplicate socket definition: %s"), *SocketDef.SocketName));
        }
        DefinedSockets.Add(SocketDef.SocketName);
    }

    // 检查Socket引用
    TSet<FString> UsedSockets;
    for (const FWFCTileDefinition& Tile : Tiles)
    {
        for (const FString& Socket : Tile.Sockets)
        {
            if (!Socket.IsEmpty())
            {
                UsedSockets.Add(Socket);
            }
        }
    }

    for (const FString& UsedSocket : UsedSockets)
    {
        if (!DefinedSockets.Contains(UsedSocket))
        {
            Errors.Add(FString::Printf(TEXT("Socket '%s' is used but not defined"), *UsedSocket));
        }
    }

    // 汇总错误信息
    if (Errors.Num() > 0)
    {
        OutErrorMessage = FString::Join(Errors, TEXT("\n"));
        return false;
    }

    OutErrorMessage = TEXT("Tile set validation passed");
    return true;
}

void UWFCTileSet::GenerateRotationVariants()
{
    TArray<FWFCTileDefinition> OriginalTiles = Tiles;
    
    for (const FWFCTileDefinition& OriginalTile : OriginalTiles)
    {
        if (!OriginalTile.bCanRotate) continue;

        // 生成90°, 180°, 270°旋转变体
        for (int32 RotationSteps = 1; RotationSteps < 4; RotationSteps++)
        {
            FWFCTileDefinition RotatedTile = OriginalTile;
            RotatedTile.TileName = FString::Printf(TEXT("%s_Rot%d"), 
                *OriginalTile.TileName, RotationSteps * 90);
            
            // 旋转Socket
            RotatedTile.Sockets = RotateSockets(OriginalTile.Sockets, RotationSteps);
            
            // 更新旋转信息
            RotatedTile.BaseRotation = FRotator(0, RotationSteps * 90.0f, 0);
            RotatedTile.bCanRotate = false; // 避免对变体再次生成旋转

            Tiles.Add(RotatedTile);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("WFCTileSet: Generated rotation variants, total tiles: %d"), Tiles.Num());
}

TArray<FString> UWFCTileSet::RotateSockets(const TArray<FString>& OriginalSockets, int32 RotationSteps) const
{
    if (OriginalSockets.Num() != 6) return OriginalSockets;

    TArray<FString> RotatedSockets;
    RotatedSockets.SetNum(6);

    // Z轴旋转不影响上下方向
    RotatedSockets[0] = OriginalSockets[0]; // Up
    RotatedSockets[1] = OriginalSockets[1]; // Down

    // 水平方向旋转映射：North->East->South->West->North
    int32 HorizontalMapping[4] = {2, 5, 3, 4}; // North, East, South, West
    
    for (int32 i = 0; i < 4; i++)
    {
        int32 OriginalIndex = HorizontalMapping[i];
        int32 RotatedIndex = HorizontalMapping[(i + RotationSteps) % 4];
        RotatedSockets[RotatedIndex] = OriginalSockets[OriginalIndex];
    }

    return RotatedSockets;
}