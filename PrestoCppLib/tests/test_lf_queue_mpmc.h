#pragma once

void construction_accepts_valid_power_of_two_capacities();
void construction_rejects_zero_capacity();
void construction_rejects_non_power_of_two_capacities();
void construction_rejects_capacity_of_one();
void preserves_fifo_order();
void pop_only_writes_result_on_success();
void reports_full_at_capacity_and_accepts_again_after_a_pop();
void reports_empty_on_a_fresh_queue();
void capacity_two_edge_case();
void survives_many_laps_around_a_small_ring();
void survives_many_laps_with_partial_fill();
void supports_move_only_types();
void moved_from_source_is_left_in_moved_from_state();
void destructor_destroys_remaining_elements_without_leaking();
void approximate_stats_track_pushes_and_pops();
void mpmc_stress_no_lost_or_duplicated_items();

// ============================================================================
// Compile-time guard checks (NOT built as part of this binary)
//
// lf::Queue::try_push/try_pop use static_assert to refuse, at compile
// time, any T whose relevant constructor/move-assignment could throw.
// To confirm those guards actually fire, put this in a separate .cpp and
// try to compile it -- the build should FAIL with a static_assert error,
// not succeed:
//
//   struct Thrower {
//       int x;
//       explicit Thrower(int v) noexcept : x(v) {}
//       Thrower(const Thrower& o) : x(o.x) { if (x == 999) throw 1; }
//   };
//   int main() {
//       lf::Queue<Thrower> q(4);
//       Thrower t(999);
//       q.try_push(t); // <-- should fail to compile
//   }
// ============================================================================