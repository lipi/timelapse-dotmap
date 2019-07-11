#ifndef __FRAME_H__
#define __FRAME_H__

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "update.h"

class FrameProvider  {
    
    private:
        SQLite::Database m_Db;
        SQLite::Statement m_TimestampsQuery;
        SQLite::Statement m_SnapshotQuery;
        SQLite::Statement m_DeltaQuery;
        
    public:
        FrameProvider(const char* filename);
        std::vector<uint32_t> GetTimestamps();
        size_t GetSnapshot(uint32_t timestamp, glm::vec2* buffer, size_t size);
        size_t FillDelta(uint32_t timestamp, glm::vec2 *buffer, size_t numLocations);
};

#endif // __FRAME_H__

