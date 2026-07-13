
#include "test_lf_queue_mpmc.h"
#include "test_ring_buffer.h"
#include "mini_test.h"

// ============================================================================
// main
// ============================================================================

int main(int argc, char** argv)
{
    // std::cout << " // ------------------------------- test_ring_buffer --------------------------------- // \n";
    // 
    // RUN_TEST(default_construction_is_empty);
    // RUN_TEST(initializer_list_construction_populates_in_order);
    // RUN_TEST(initializer_list_larger_than_capacity_overwrites_oldest);
    // RUN_TEST(capacity_is_a_compile_time_constant);
    // 
    // RUN_TEST(push_copy_grows_size_until_full);
    // RUN_TEST(push_when_full_overwrites_oldest_element);
    // RUN_TEST(push_move_stores_the_value);
    // 
    // RUN_TEST(try_pop_fails_on_empty_buffer);
    // RUN_TEST(try_pop_returns_elements_in_fifo_order);
    // 
    // RUN_TEST(blocking_pop_returns_immediately_when_not_empty);
    // RUN_TEST(blocking_pop_waits_until_another_thread_pushes);
    // RUN_TEST(blocking_pop_for_throws_when_it_times_out_on_empty_buffer);
    // RUN_TEST(blocking_pop_for_succeeds_when_value_arrives_before_timeout);
    // 
    // RUN_TEST(try_front_and_try_back_fail_on_empty_buffer);
    // RUN_TEST(try_front_and_try_back_peek_without_removing);
    // RUN_TEST(blocking_front_and_back_return_oldest_and_newest);
    // 
    // RUN_TEST(get_by_value_indexes_from_oldest_to_newest);
    // RUN_TEST(get_by_value_throws_out_of_range_for_bad_index);
    // RUN_TEST(get_or_returns_default_value_when_out_of_range);
    // 
    // RUN_TEST(clear_empties_the_buffer);
    // RUN_TEST(clear_then_push_reuses_indices_correctly);
    // RUN_TEST(empty_and_full_reflect_state_transitions);
    // 
    // RUN_TEST(available_and_utilization_track_fill_level);
    // 
    // RUN_TEST(concurrent_push_and_try_pop_preserve_every_element);
    // RUN_TEST(concurrent_push_overwrite_never_crashes_or_corrupts_size);
    // 
    // std::cout << " // ------------------------------- test_lf_queue_mpmc --------------------------------- // \n";
    // 
    // RUN_TEST(construction_accepts_valid_power_of_two_capacities);
    // RUN_TEST(construction_rejects_zero_capacity);
    // RUN_TEST(construction_rejects_non_power_of_two_capacities);
    // RUN_TEST(construction_rejects_capacity_of_one);
    // RUN_TEST(preserves_fifo_order);
    // RUN_TEST(pop_only_writes_result_on_success);
    // RUN_TEST(reports_full_at_capacity_and_accepts_again_after_a_pop);
    // RUN_TEST(reports_empty_on_a_fresh_queue);
    // RUN_TEST(capacity_two_edge_case);
    // RUN_TEST(survives_many_laps_around_a_small_ring);
    // RUN_TEST(survives_many_laps_with_partial_fill);
    // RUN_TEST(supports_move_only_types);
    // RUN_TEST(moved_from_source_is_left_in_moved_from_state);
    // RUN_TEST(destructor_destroys_remaining_elements_without_leaking);
    // RUN_TEST(approximate_stats_track_pushes_and_pops);
    // RUN_TEST(mpmc_stress_no_lost_or_duplicated_items);
    // 
    // std::cout << "// ------------------------------------------------------------------------------------ // \n";
    // 
    // 
    // return mini_test::summary();


    // Edit this to switch suites without touching Debugging > Command
    // Arguments: "" runs everything, or e.g. "ring_buffer" / "lf_queue_mpmc".
    // A command-line arg (if you pass one) always overrides this.
    const char* suite_to_run = "";

    const char* suite = (argc > 1) ? argv[1] : suite_to_run;

    if (suite && suite[0] != '\0')
    {
        if (!mini_test::run_suite(suite))
        {
            std::cerr << "No such test suite: " << suite << "\n";
            return 1;
        }
    }
    else
    {
        mini_test::run_all();
    }
    return mini_test::summary();
}

