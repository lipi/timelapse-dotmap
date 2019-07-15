#ifndef __FRAME_H__
#define __FRAME_H__

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "update.h"

class FrameProvider  {

public:
    explicit FrameProvider(const char* filename);
    std::vector<uint32_t> GetTimestamps();
    size_t GetFrameSize();
    size_t GetFrameSizeBytes();
    size_t GetSnapshot(uint32_t timestamp, glm::vec2* frame, size_t size);
    size_t FillDelta(uint32_t timestamp, glm::vec2 *frame, size_t numLocations);
    void Next(glm::vec2 *frame);

private:
    size_t m_FrameSize;
    std::vector<uint32_t> m_Timestamps;
    uint m_TimeIndex;
    SQLite::Database m_Db;
    SQLite::Statement m_TimestampsQuery;
    SQLite::Statement m_SnapshotQuery;
    SQLite::Statement m_DeltaQuery;

};

#endif // __FRAME_H__

