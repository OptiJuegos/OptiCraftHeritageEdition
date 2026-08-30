#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;

// net.minecraft.src.Packet51MapChunk (Minecraft 1.2.5)
class Packet51MapChunk : public Packet
{
public:
    Packet51MapChunk();

    void readPacketData(std::istream &is) override;
    void writePacketData(std::ostream &os) override;
    void processPacket(NetHandler &nethandler) override;
    int_t getPacketSize() override;

    bool ensureDecompressed();
    std::vector<byte_t> takeCompressedData();
    std::vector<byte_t> copyCompressedData() const;

    int_t xCh = 0;
    int_t zCh = 0;
    int_t yChMin = 0;
    int_t yChMax = 0;
    bool includeInitialize = false;
    std::vector<byte_t> chunkData;

private:
    std::size_t expectedInflatedSize() const;

    int_t tempLength = 0;
    int_t field_48178_h = 0;
    std::vector<byte_t> compressedChunk;
};
