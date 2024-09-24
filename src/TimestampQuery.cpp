#include "TimestampQuery.h"

#include <deque>
#include <vector>
#include <algorithm>
#include <iterator>

#include <glad/gl.h>

namespace OM3D {

using FrameMarkers = std::vector<std::pair<std::string_view, TimestampQuery>>;

static FrameMarkers marker_queries;
static std::deque<FrameMarkers> waiting_profiles;
static std::vector<std::pair<std::string_view, double>> last_profile;

void push_marker(std::string_view name, TimestampQuery query) {
    query.end();
    if(auto it = std::find_if(marker_queries.begin(), marker_queries.end(), [&](const auto& marker) { return marker.first == name; }); it != marker_queries.end()) {
        it->second = std::move(query);
    } else {
        marker_queries.emplace_back(name, std::move(query));
    }
}

void process_profile_markers() {
    std::swap(waiting_profiles.emplace_back(), marker_queries);
    DEBUG_ASSERT(marker_queries.empty());

    FrameMarkers last_ready;
    while(!waiting_profiles.empty()) {
        bool ready = true;
        for(auto& [name, query] : waiting_profiles.front()) {
            if(!query.seconds().is_ok) {
                ready = false;
                break;
            }
        }

        if(ready) {
            last_ready = std::move(waiting_profiles.front());
            waiting_profiles.pop_front();
        }
    }

    last_profile.clear();
    std::transform(last_ready.begin(), last_ready.end(), std::back_inserter(last_profile), [](const auto& marker) { return std::make_pair(marker.first, marker.second.seconds(true).value); });
}

Span<std::pair<std::string_view, double>> previous_profile() {
    return last_profile;
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
