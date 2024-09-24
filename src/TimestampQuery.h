#ifndef TIMESTAMPQUERY_H
#define TIMESTAMPQUERY_H

#include <graphics.h>

#include <string>
#include <utility>

namespace OM3D {

#define PROFILE_GPU(name_expr) auto CREATE_UNIQUE_NAME_WITH_PREFIX(gpu_prof) = ::OM3D::ScopeGuard([zone_id = ::OM3D::profile::begin_profile_zone(name_expr)] { ::OM3D::profile::end_profile_zone(zone_id); })


class TimestampQuery : NonCopyable {
    enum class State {
        None,
        Started,
        Ended,
        Resolved,
    };

    public:
        TimestampQuery() = default;
        ~TimestampQuery();

        TimestampQuery(TimestampQuery&& other);
        TimestampQuery& operator=(TimestampQuery&& other);

        void swap(TimestampQuery& other);

        static TimestampQuery create_and_begin();

        void begin();
        void end();

        Result<double> seconds(bool wait = false) const;

    private:
        GLHandle _begin;
        GLHandle _end;

        mutable double _time = 0.0;
        mutable State _state = State::None;
};



struct ProfileZone {
    std::string name;
    u32 contained_zones = 0;
    float cpu_time = 0.0f;
    float gpu_time = 0.0f;
};

Span<ProfileZone> retrieve_profile();
void process_profile_markers();


namespace profile {
    u32 begin_profile_zone(const char* name);
    void end_profile_zone(u32 zone_id);
}

}

#endif // TIMESTAMPQUERY_H
