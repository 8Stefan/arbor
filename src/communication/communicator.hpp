#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

#include <algorithms.hpp>
#include <connection.hpp>
#include <event_queue.hpp>
#include <spike.hpp>
#include <threading/threading.hpp>
#include <util/debug.hpp>

namespace nest {
namespace mc {
namespace communication {

// When the communicator is constructed the number of target groups and targets
// is specified, along with a mapping between local cell id and local
// target id.
//
// The user can add connections to an existing communicator object, where
// each connection is between any global cell and any local target.
//
// Once all connections have been specified, the construct() method can be used
// to build the data structures required for efficient spike communication and
// event generation.
template <typename Time, typename CommunicationPolicy>
class communicator {
public:
    using id_type = cell_gid_type;
    using time_type = Time;
    using communication_policy_type = CommunicationPolicy;

    using spike_type = spike<cell_member_type, time_type>;

    communicator() = default;

    // for now, still assuming one-to-one association cells <-> groups,
    // so that 'group' gids as represented by their first cell gid are
    // contiguous.
    communicator(id_type cell_from, id_type cell_to):
        cell_gid_from_(cell_from), cell_gid_to_(cell_to)
    {
        auto num_groups_local_ = cell_gid_to_-cell_gid_from_;

        // create an event queue for each target group
        events_.resize(num_groups_local_);
    }


    void add_connection(connection<time_type> con) {
        EXPECTS(is_local_cell(con.destination().gid));
        connections_.push_back(con);
    }

    bool is_local_cell(id_type gid) const {
        return gid>=cell_gid_from_ && gid<cell_gid_to_;
    }

    // builds the optimized data structure
    void construct() {
        if (!std::is_sorted(connections_.begin(), connections_.end())) {
            std::sort(connections_.begin(), connections_.end());
        }
    }

    time_type min_delay() {
        auto local_min = std::numeric_limits<time_type>::max();
        for (auto& con : connections_) {
            local_min = std::min(local_min, con.delay());
        }

        return communication_policy_.min(local_min);
    }

    void add_spike(spike_type s) {
        thread_spikes().push_back(s);
    }

    void add_spikes(const std::vector<spike_type>& s) {
        auto& v = thread_spikes();
        v.insert(v.end(), s.begin(), s.end());
    }

    std::vector<spike_type>& thread_spikes() {
        return thread_spikes_.local();
    }

    void exchange() {
        // global all-to-all to gather a local copy of the global spike list
        // on each node
        auto global_spikes = communication_policy_.gather_spikes(local_spikes());
        num_spikes_ += global_spikes.size();
        clear_thread_spike_buffers();

        for (auto& q : events_) {
            q.clear();
        }

        // check all global spikes to see if they will generate local events
        for (auto spike : global_spikes) {
            // search for targets
            auto targets =
                std::equal_range(
                    connections_.begin(), connections_.end(), spike.source
                );

            // generate an event for each target
            for (auto it=targets.first; it!=targets.second; ++it) {
                auto gidx = cell_group_index(it->destination().gid);
                events_[gidx].push_back(it->make_event(spike));
            }
        }
    }

    uint64_t num_spikes() const { return num_spikes_; }

    const std::vector<postsynaptic_spike_event<time_type>>& queue(int i) const {
        return events_[i];
    }

    const std::vector<connection<time_type>>& connections() const {
        return connections_;
    }

    communication_policy_type communication_policy() const {
        return communication_policy_;
    }

    std::vector<spike_type> local_spikes() {
        std::vector<spike_type> spikes;
        for (auto& v : thread_spikes_) {
            spikes.insert(spikes.end(), v.begin(), v.end());
        }
        return spikes;
    }

    void clear_thread_spike_buffers() {
        for (auto& v : thread_spikes_) {
            v.clear();
        }
    }

    void reset() {
        // remove all in-flight spikes/events
        clear_thread_spike_buffers();
        for (auto& evbuf: events_) {
            evbuf.clear();
        }
    }

private:
    std::size_t cell_group_index(cell_gid_type cell_gid) const {
        // this will be more elaborate when there is more than one cell per cell group
        EXPECTS(cell_gid>=cell_gid_from_ && cell_gid<cell_gid_to_);
        return cell_gid-cell_gid_from_;
    }

    //
    //  both of these can be fixed with double buffering
    //
    // FIXME : race condition on the thread_spikes_ buffers when exchange() modifies/access them
    //         ... other threads will be pushing to them simultaneously
    // FIXME : race condition on the group-specific event queues when exchange pushes to them
    //         ... other threads will be accessing them to update their event queues

    // thread private storage for accumulating spikes
    using local_spike_store_type =
        nest::mc::threading::enumerable_thread_specific<std::vector<spike_type>>;
    local_spike_store_type thread_spikes_;

    std::vector<connection<time_type>> connections_;
    std::vector<std::vector<postsynaptic_spike_event<time_type>>> events_;

    // for keeping track of how time is spent where
    //util::Profiler profiler_;

    communication_policy_type communication_policy_;

    uint64_t num_spikes_ = 0u;
    id_type cell_gid_from_;
    id_type cell_gid_to_;
};

} // namespace communication
} // namespace mc
} // namespace nest