/** TRACCC library, part of the ACTS project (R&D line)
 *
 * (c) 2022 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Local include(s).
#include "traccc/io/event_map2.hpp"

#include "traccc/io/csv/make_hit_reader.hpp"
#include "traccc/io/csv/make_measurement_hit_id_reader.hpp"
#include "traccc/io/csv/make_measurement_reader.hpp"
#include "traccc/io/csv/make_particle_reader.hpp"
#include "traccc/io/utils.hpp"
namespace traccc {

event_map2::event_map2(std::size_t event, const std::string& measurement_dir,
                       const std::string& hit_dir,
                       const std::string particle_dir) {

    std::string io_measurement_hit_id_file =
        io::data_directory() + hit_dir +
        io::get_event_filename(event, "-measurement-simhit-map.csv");

    std::string io_particle_file =
        io::data_directory() + particle_dir +
        io::get_event_filename(event, "-particles.csv");

    std::string io_hit_file = io::data_directory() + hit_dir +
                              io::get_event_filename(event, "-hits.csv");

    std::string io_measurement_file =
        io::data_directory() + measurement_dir +
        io::get_event_filename(event, "-measurements.csv");

    auto mreader = io::csv::make_measurement_reader(io_measurement_file);

    auto hreader = io::csv::make_hit_reader(io_hit_file);

    auto preader = io::csv::make_particle_reader(io_particle_file);

    auto mhid_reader =
        io::csv::make_measurement_hit_id_reader(io_measurement_hit_id_file);

    std::vector<traccc::io::csv::measurement_hit_id> measurement_hit_ids;
    std::vector<traccc::io::csv::particle> particles;
    std::vector<traccc::io::csv::hit> hits;
    std::vector<traccc::io::csv::measurement> measurements;

    traccc::io::csv::measurement_hit_id io_mh_id;
    while (mhid_reader.read(io_mh_id)) {
        measurement_hit_ids.push_back(io_mh_id);
    }

    traccc::io::csv::particle io_particle;
    while (preader.read(io_particle)) {
        point3 pos{io_particle.vx, io_particle.vy, io_particle.vz};
        vector3 mom{io_particle.px, io_particle.py, io_particle.pz};

        particles.push_back(io_particle);
        ptc_map[io_particle.particle_id] =
            particle{io_particle.particle_id, io_particle.particle_type,
                     io_particle.process,     pos,
                     io_particle.vt,          mom,
                     io_particle.m,           io_particle.q};
    }

    traccc::io::csv::hit io_hit;
    while (hreader.read(io_hit)) {
        hits.push_back(io_hit);
    }

    traccc::io::csv::measurement io_measurement;
    while (mreader.read(io_measurement)) {
        measurements.push_back(io_measurement);
    }

    for (const auto& csv_meas : measurements) {

        // Hit index
        const auto h_id = measurement_hit_ids[csv_meas.measurement_id].hit_id;

        // Make spacepoint
        const auto csv_hit = hits[h_id];
        point3 global_pos{csv_hit.tx, csv_hit.ty, csv_hit.tz};
        point3 global_mom{csv_hit.tpx, csv_hit.tpy, csv_hit.tpz};

        // Make particle
        const auto csv_ptc = particles[csv_hit.particle_id];
        point3 pos{csv_ptc.vx, csv_ptc.vy, csv_ptc.vz};
        vector3 mom{csv_ptc.px, csv_ptc.py, csv_ptc.pz};
        particle ptc{csv_ptc.particle_id, csv_ptc.particle_type,
                     csv_ptc.process,     pos,
                     csv_ptc.vt,          mom,
                     csv_ptc.m,           csv_ptc.q};

        // Construct the measurement object.
        traccc::measurement meas;
        std::array<typename transform3::size_type, 2u> indices{0u, 0u};
        meas.meas_dim = 0u;

        // Local key is a 8 bit char and first and last bit are dummy value. 2 -
        // 7th bits are for 6 bound track parameters.
        // Ex1) 0000010 or 2 -> meas dim = 1 and [loc0] active -> strip or wire
        // Ex2) 0000110 or 6 -> meas dim = 2 and [loc0, loc1] active -> pixel
        // Ex3) 0000100 or 4 -> meas dim = 1 and [loc1] active -> annulus
    auto handle_measurement = [&](unsigned int ipar) {
    switch (ipar) {
        case e_bound_loc0: {
            meas.local[0] = csv_meas.local0;
            meas.variance[0] = csv_meas.var_local0;
            indices[meas.meas_dim++] = ipar;
        }; break;
        case e_bound_loc1: {
            meas.local[1] = csv_meas.local1;
            meas.variance[1] = csv_meas.var_local1;
            indices[meas.meas_dim++] = ipar;
        }; break;
        default: {
            // Handle default case here
        }; break;
    }
};

// Iterate over the parameters and apply the lambda function
for (unsigned int ipar = 0; ipar < 2u; ++ipar) {
    if (((csv_meas.local_key) & (1 << (ipar + 1))) != 0) {
        handle_measurement(ipar);
    }
}

        meas.subs.set_indices(indices);
        meas.surface_link = detray::geometry::barcode{csv_meas.geometry_id};

        // Fill measurement to truth global position and momentum map
        meas_xp_map[meas] = std::make_pair(global_pos, global_mom);

        // Fill particle to measurement map
        ptc_meas_map[ptc].push_back(meas);

        // Fill measurement to particle map
        auto& contributing_particles = meas_ptc_map[meas];
        contributing_particles[ptc]++;
    }
}

}  // namespace traccc
