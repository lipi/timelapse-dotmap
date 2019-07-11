
#include <cstdlib>

#include <spdlog/spdlog.h>

#include "frame.h"

FrameProvider::FrameProvider(const char* filename)
    : m_Db(filename),
      m_TimestampsQuery(m_Db, "SELECT timestamp FROM delta WHERE timestamp > 0"),
      m_SnapshotQuery(m_Db, "SELECT frame FROM snapshot WHERE timestamp <= :timestamp ORDER BY timestamp DESC limit 1"),
      m_DeltaQuery(m_Db, "SELECT frame FROM delta WHERE timestamp = :timestamp")
{
    spdlog::info("Opened {}, found {} frames", filename, (uint32_t)GetTimestamps().size());
}

std::vector<uint32_t> FrameProvider::GetTimestamps() {
    std::vector<uint32_t> timestamps;
    
    // Loop to execute the query step by step, to get one a row of results at a time
    while (m_TimestampsQuery.executeStep()) {
        timestamps.push_back(m_TimestampsQuery.getColumn(0));
    }
    m_TimestampsQuery.reset();
    return timestamps;
}

size_t FrameProvider::GetSnapshot(uint32_t timestamp, glm::vec2* buffer, size_t numLocations) {
    const void* blobData = NULL;
    size_t blobSize;
    size_t locationSize = sizeof(float) * 2;
    
    m_SnapshotQuery.bind(":timestamp", timestamp);
    if (m_SnapshotQuery.executeStep()) {
        SQLite::Column colBlob = m_SnapshotQuery.getColumn(0);
        blobData = colBlob.getBlob();
        blobSize = colBlob.getBytes();
        if (blobSize / locationSize < numLocations) {
            numLocations = blobSize / locationSize;
        }

        float* floatData = (float*)blobData;
        spdlog::debug("converting {} floats ({} locations)", 2 * numLocations, numLocations);
        for (size_t i = 0; i < numLocations; i++) { // TODO: in-situ blob placement by sqlite?
            buffer[i] = glm::vec2(floatData[2 * i+1], floatData[2 * i]);
        }
    }
    m_SnapshotQuery.clearBindings();
    m_SnapshotQuery.reset();

    return numLocations;
}

size_t FrameProvider::FillDelta(uint32_t timestamp, glm::vec2 *buffer, size_t numLocations) {
    const void* blobData = NULL;
    size_t blobSize;
    size_t locationSize = sizeof(update_t);

    m_DeltaQuery.bind(":timestamp", timestamp);
    if (m_DeltaQuery.executeStep()) {
        SQLite::Column colBlob = m_DeltaQuery.getColumn(0);
        blobData = colBlob.getBlob();
        blobSize = colBlob.getBytes();
        if (blobSize / locationSize < numLocations) {
            numLocations = blobSize / locationSize;
        }

        update_t* items = (update_t*)blobData;
        spdlog::debug("converting {} floats ({} locations)", 2 * numLocations, numLocations);
        for (size_t i = 0; i < numLocations; i++) {
            uint32_t index = items[i].index;
            buffer[index] = glm::vec2(items[i].lon, items[i].lat);
        }
    }
    m_DeltaQuery.clearBindings();
    m_DeltaQuery.reset();

    return numLocations;
}
