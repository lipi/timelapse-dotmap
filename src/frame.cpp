
#include <cstdlib>

#include <spdlog/spdlog.h>

#include "frame.h"

FrameProvider::FrameProvider(const char* filename)
    : m_TimeIndex(0),
      m_Db(filename),
      m_TimestampsQuery(m_Db, "SELECT timestamp FROM timestamps WHERE timestamp > 0"),
      m_SnapshotQuery(m_Db, "SELECT frame FROM snapshot WHERE timestamp = :timestamp"),
      m_DeltaQuery(m_Db, "SELECT frame FROM delta WHERE timestamp = :timestamp")
{
    m_FrameSize = 70000; // TODO: read it from DB
    spdlog::info("Opening {}...", filename);
    m_Timestamps = GetTimestamps();
    spdlog::info("Opened {}, found {} frames", filename, (uint32_t)GetTimestamps().size());
}

size_t FrameProvider::GetFrameSize() {
    return m_FrameSize;
}

size_t FrameProvider::GetFrameSizeBytes() {
    return m_FrameSize * sizeof(glm::vec2);
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

size_t FrameProvider::FillDelta(uint32_t timestamp, glm::vec2 *frame, size_t numLocations) {
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
            frame[index] = glm::vec2(items[i].lon, items[i].lat);
        }
    }
    m_DeltaQuery.clearBindings();
    m_DeltaQuery.reset();

    return numLocations;
}

void FrameProvider::Next(glm::vec2* frame) {
    uint timestamp = m_Timestamps[m_TimeIndex];
    spdlog::debug("Loading frame {} at {} {}",
                  m_TimeIndex, timestamp, (void*)frame);
    FillDelta(timestamp, frame, m_FrameSize);
    m_TimeIndex++;
    m_TimeIndex %= m_Timestamps.size();
}

uint32_t FrameProvider::CurrentTimestamp() {
    return m_Timestamps[m_TimeIndex];;
}
