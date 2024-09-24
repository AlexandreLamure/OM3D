#include "TimestampQuery.h"

#include <string>
#include <deque>
#include <vector>

#include <glad/gl.h>

namespace OM3D {

namespace profile {
    struct Marker {
        std::string name;
        u32 contained_zones;
        double cpu_time;
        TimestampQuery query;
    };

    static std::vector<Marker> current_frame;

    static std::deque<std::vector<Marker>> queued_frames;
    static std::vector<ProfileZone> ready;

    u32 begin_profile_zone(const char* name) {
        const u32 index = u32(current_frame.size());

        // Either you forgot to call process_profile_markers every frame, or you have too many marker
        // In the later case, you can just remove this assert
        ALWAYS_ASSERT(index < 65536, "Too many profile markers");

        Marker& marker = current_frame.emplace_back();
        marker.name = name;
        marker.cpu_time = program_time();
        marker.query.begin();

        return index;
    }

    void end_profile_zone(u32 zone_id) {
        Marker& marker = current_frame[zone_id];
        marker.cpu_time = program_time() - marker.cpu_time;
        marker.contained_zones = u32(current_frame.size()) - zone_id - 1;
        marker.query.end();
    }
}




void process_profile_markers() {
    profile::queued_frames.emplace_back().swap(profile::current_frame);
    DEBUG_ASSERT(profile::current_frame.empty());

    bool any_profile_ready = false;
    std::vector<profile::Marker> ready_frame;
    while(!profile::queued_frames.empty()) {
        auto& frame = profile::queued_frames.front();

        bool ready = true;
        for(auto& marker : frame) {
            if(!marker.query.seconds().is_ok) {
                ready = false;
                break;
            }
        }

        if(ready) {
            any_profile_ready = true;
            ready_frame = std::move(frame);
            profile::queued_frames.pop_front();
        } else {
            break;
        }
    }

    if(any_profile_ready) {
        profile::ready.clear();
        for(auto& marker : ready_frame) {
            ProfileZone& zone = profile::ready.emplace_back();
            zone.name = std::move(marker.name);
            zone.contained_zones = marker.contained_zones;
            zone.cpu_time = float(marker.cpu_time);
            zone.gpu_time = float(marker.query.seconds(true).value);
        }
    }
}

Span<ProfileZone> retrieve_profile() {
    return profile::ready;
}






static auto create_query_handles() {
    std::array<GLuint, 2> handles = {};
    glGenQueries(u32(handles.size()), handles.data());
    return handles;
}

TimestampQuery::~TimestampQuery() {
    DEBUG_ASSERT(_state ==  State::Resolved || _state == State::Ended || _state == State::None);

    if(auto handle = _begin.get()) {
        glDeleteQueries(1, &handle);
    }
    if(auto handle = _end.get()) {
        glDeleteQueries(1, &handle);
    }
}

TimestampQuery::TimestampQuery(TimestampQuery&& other) {
    swap(other);
}

TimestampQuery& TimestampQuery::operator=(TimestampQuery&& other) {
    swap(other);
    return *this;
}

void TimestampQuery::swap(TimestampQuery& other) {
    _begin.swap(other._begin);
    _end.swap(other._end);
    std::swap(_time, other._time);
    std::swap(_state, other._state);
}

TimestampQuery TimestampQuery::create_and_begin() {
    TimestampQuery ts;
    ts.begin();
    return ts;
}

void TimestampQuery::begin() {
    DEBUG_ASSERT(_state == State::None);
    _state = State::Started;

    if(!_begin.is_valid()) {
        DEBUG_ASSERT(!_end.is_valid());

        const auto handles = create_query_handles();
        _begin = GLHandle(handles[0]);
        _end = GLHandle(handles[1]);
    }

    glQueryCounter(_begin.get(), GL_TIMESTAMP);
}

void TimestampQuery::end() {
    DEBUG_ASSERT(_state == State::Started);
    _state = State::Ended;
    glQueryCounter(_end.get(), GL_TIMESTAMP);
}

Result<double> TimestampQuery::seconds(bool wait) const {
    if(_state == State::Resolved) {
        return {true, _time};
    }

    DEBUG_ASSERT(_state == State::Ended);

    const u64 invalid_timestamp = u64(-1);
    const GLenum pname = wait ? GL_QUERY_RESULT : GL_QUERY_RESULT_NO_WAIT;

    u64 end_ns = invalid_timestamp;
    glGetQueryObjectui64v(_end.get(), pname, &end_ns);
    if(end_ns == invalid_timestamp) {
        return {false, {}};
    }

    u64 begin_ns = invalid_timestamp;
    glGetQueryObjectui64v(_begin.get(), pname, &begin_ns);
    if(begin_ns == invalid_timestamp) {
        return {false, {}};
    }

    _time = double(end_ns - begin_ns) / 1'000'000'000.0;
    _state = State::Resolved;

    return {true, _time};
}

}
