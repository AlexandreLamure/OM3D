#ifndef TIMESTAMPQUERY_H
#define TIMESTAMPQUERY_H

#include <graphics.h>

#include <string_view>
#include <utility>

namespace OM3D {

#define PROFILE_GPU(name_expr) auto CREATE_UNIQUE_NAME_WITH_PREFIX(gpu_prof) = ::OM3D::ScopeGuard([query = ::OM3D::TimestampQuery::create_and_begin(), name = name_expr]() mutable { ::OM3D::push_marker(name, std::move(query)); })

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


void push_marker(std::string_view name, TimestampQuery query);
void process_profile_markers();
Span<std::pair<std::string_view, double>> previous_profile();


}

#endif // TIMESTAMPQUERY_H
