#pragma once

void default_construction_is_empty();
void initializer_list_construction_populates_in_order();
void initializer_list_larger_than_capacity_overwrites_oldest();
void capacity_is_a_compile_time_constant();

void push_copy_grows_size_until_full();
void push_when_full_overwrites_oldest_element();
void push_move_stores_the_value();
 
void try_pop_fails_on_empty_buffer();
void try_pop_returns_elements_in_fifo_order();

void blocking_pop_returns_immediately_when_not_empty();
void blocking_pop_waits_until_another_thread_pushes();
void blocking_pop_for_throws_when_it_times_out_on_empty_buffer();
void blocking_pop_for_succeeds_when_value_arrives_before_timeout();

void try_front_and_try_back_fail_on_empty_buffer();
void try_front_and_try_back_peek_without_removing();
void blocking_front_and_back_return_oldest_and_newest();

void get_by_value_indexes_from_oldest_to_newest();
void get_by_value_throws_out_of_range_for_bad_index();
void get_or_returns_default_value_when_out_of_range();

void clear_empties_the_buffer();
void clear_then_push_reuses_indices_correctly();
void empty_and_full_reflect_state_transitions();

void available_and_utilization_track_fill_level();

void concurrent_push_and_try_pop_preserve_every_element();
void concurrent_push_overwrite_never_crashes_or_corrupts_size();