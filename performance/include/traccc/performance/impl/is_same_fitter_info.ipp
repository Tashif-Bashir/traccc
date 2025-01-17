/** TRACCC library, part of the ACTS project (R&D line)
 *
 * (c) 2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

// Project include(s).
#include "traccc/edm/track_state.hpp"

namespace traccc::details {

/// @c traccc::is_same_object specialisation for @c traccc::fitter_info
template <>
class is_same_object<fitter_info<transform3>> {

    public:
    /// Constructor with a reference object, and an allowed uncertainty
    is_same_object(const fitter_info<transform3>& ref,
                   scalar unc = float_epsilon);

    /// Specialised implementation for @c traccc::measurement
    bool operator()(const fitter_info<transform3>& obj) const;

    private:
    /// The reference object
    std::reference_wrapper<const fitter_info<transform3>> m_ref;
    /// The uncertainty
    scalar m_unc;

};  // class is_same_object<fitter_info<transform3>>

}  // namespace traccc::details
