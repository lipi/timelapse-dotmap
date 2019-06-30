#ifndef __FRAME_H__
#define __FRAME_H__

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

class Frame  {
    
    private:
        SQLite::Database m_Db;
        SQLite::Statement m_TimestampsQuery;
        SQLite::Statement m_SnapshotQuery;
        SQLite::Statement m_DeltaQuery;
        
    public:
        Frame(const char* filename);
        std::vector<uint32_t> GetTimestamps();
        size_t GetSnapshot(uint32_t timestamp, glm::vec3* buffer, size_t size);
        size_t GetDelta(uint32_t timestamp, glm::vec3* buffer, size_t size);
};

#endif // __FRAME_H__

