
#include "test_lf_queue_mpmc.h"
#include "mini_test.h"

// ============================================================================
// main
// ============================================================================

int main()
{
    // ------------------------------- test_lf_queue_mpmc --------------------------------- //
    RUN_TEST(construction_accepts_valid_power_of_two_capacities);
    RUN_TEST(construction_rejects_zero_capacity);
    RUN_TEST(construction_rejects_non_power_of_two_capacities);
    RUN_TEST(construction_rejects_capacity_of_one);
    RUN_TEST(preserves_fifo_order);
    RUN_TEST(pop_only_writes_result_on_success);
    RUN_TEST(reports_full_at_capacity_and_accepts_again_after_a_pop);
    RUN_TEST(reports_empty_on_a_fresh_queue);
    RUN_TEST(capacity_two_edge_case);
    RUN_TEST(survives_many_laps_around_a_small_ring);
    RUN_TEST(survives_many_laps_with_partial_fill);
    RUN_TEST(supports_move_only_types);
    RUN_TEST(moved_from_source_is_left_in_moved_from_state);
    RUN_TEST(destructor_destroys_remaining_elements_without_leaking);
    RUN_TEST(approximate_stats_track_pushes_and_pops);
    RUN_TEST(mpmc_stress_no_lost_or_duplicated_items);
    // ------------------------------------------------------------------------------------ //


    return mini_test::summary();
}

