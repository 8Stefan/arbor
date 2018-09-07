#pragma once

#include <pybind11/pybind11.h>

#include <arbor/common_types.hpp>
#include <arbor/event_generator.hpp>
#include <arbor/schedule.hpp>

namespace arb {
namespace py {

// A Python shim that holds the information that describes an
// arb::regular_schedule. This is wrapped in pybind11, and users constructing
// a regular_schedule in python are manipulating this type. This is converted to
// an arb::regular_schedule when a C++ recipe is created from a Python recipe.
struct regular_schedule_shim {
    time_type tstart = arb::terminal_time;
    time_type dt = 0;
    time_type tstop = arb::terminal_time;

    regular_schedule_shim() = default;

    regular_schedule_shim(time_type t0, time_type dt, time_type t1):
        tstart(t0),
        dt(dt),
        tstop(t1)
    {}

    arb::schedule schedule() const {
        return regular_schedule(tstart, dt, tstop);
    }
};

// A Python shim that holds the information that describes an
// arb::explicit_schedule. This is wrapped in pybind11, and users constructing
// a explicit_schedule in python are manipulating this type. This is converted to
// an arb::explicit_schedule when a C++ recipe is created from a Python recipe.
struct explicit_schedule_shim {
    pybind11::list py_times;

    explicit_schedule_shim() = default;

    arb::schedule schedule() const {
        std::vector<time_type> times;

        times.reserve(py_times.size());
        for (auto& t: py_times) {
            times.push_back(pybind11::cast<time_type>(t));
        }

        // Sort the times in ascending order if necessary
        if (!std::is_sorted(times.begin(), times.end())) {
            std::sort(times.begin(), times.end());
        }

        return explicit_schedule(times);
    }
};

struct event_generator {
    arb::cell_lid_type lid;
    double weight;
    arb::schedule time_seq;

    event_generator(arb::cell_lid_type lid, double weight, arb::schedule seq):
        lid(lid),
        weight(weight),
        time_seq(std::move(seq))
    {}
};

} // namespace arb
} // namespace py
