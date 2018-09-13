#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include <arbor/common_types.hpp>
#include <arbor/context.hpp>
#include <arbor/domain_decomposition.hpp>
#include <arbor/recipe.hpp>
#include <arbor/sampling.hpp>
#include <arbor/schedule.hpp>
#include <arbor/spike.hpp>
#include <arbor/util/handle_set.hpp>

namespace arb {

using spike_export_function = std::function<void(const std::vector<spike>&)>;
using external_spike_function = std::function<std::vector<spike>(time_type tfrom)>;

// simulation_state comprises private implentation for simulation class.
class simulation_state;

class simulation {
public:
    simulation(const recipe& rec, const domain_decomposition& decomp, const context& ctx);

    void reset();

    time_type run(time_type tfinal, time_type dt);

    // Note: sampler functions may be invoked from a different thread than that
    // which called the `run` method.

    sampler_association_handle add_sampler(cell_member_predicate probe_ids,
        schedule sched, sampler_function f, sampling_policy policy = sampling_policy::lax);

    void remove_sampler(sampler_association_handle);

    void remove_all_samplers();

    std::size_t num_spikes() const;

    // Set event binning policy on all our groups.
    void set_binning_policy(binning_kind policy, time_type bin_interval);

    // Register a callback that will perform a export of the global
    // spike vector.
    void set_global_spike_callback(spike_export_function = spike_export_function{});

    // Register a callback that will perform a export of the rank local
    // spike vector.
    void set_local_spike_callback(spike_export_function = spike_export_function{});

    // Register a callback for getting spikes from an external source,
    // typically another simulator.
    void set_external_spike_callback(external_spike_function);

    // Query the minimum delay of the network.
    time_type min_delay() const;

    // Set the min_delay of the network to the minimum of t
    // and the shortest connection in the network.
    time_type min_delay(time_type t);

    // Add events directly to targets.
    // Must be called before calling simulation::run, and must contain events that
    // are to be delivered at or after the current simulation time.
    void inject_events(const pse_vector& events);

    ~simulation();

private:
    std::unique_ptr<simulation_state> impl_;
};

} // namespace arb
