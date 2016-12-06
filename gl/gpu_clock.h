#pragma once

#include <glad/glad.h>

#include "platform/clock.h"

namespace pt
{
struct GpuClock
{
    GpuClock()
    {
        glGenQueries(1, &id);
    }
    ~GpuClock()
    {
        glDeleteQueries(1, &id);
    }
    TimePoint now() const
    {
        glQueryCounter(id, GL_TIMESTAMP);
        GLint available = 0;
        while (!available)
            glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE,
                               &available);
        uint64_t time = 0;
        glGetQueryObjectui64v(id, GL_QUERY_RESULT, &time);
        return TimePoint(boost::chrono::nanoseconds(time));
    }
private:
    GLuint id;
};

#define PTTIME_GPU(description) \
    pt::ScopedClock<GpuClock> scopedClock_(PTSRC(), description)

}  // namespace
