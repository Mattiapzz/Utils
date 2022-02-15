/** @file threadpool/parallel_transform.h
 *
 * Threadpool for C++11, parallel transform
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.1 $
 * $Date: 2014/05/16 08:24:36 $
 */
#ifndef PARALLEL_TRANSFORM_H
#define PARALLEL_TRANSFORM_H

namespace threadpool {

  /**
   * A thread pool to be used as base for container processing.
   *
   * @tparam InputIterator
   *		The iterator type to be used to get input values.
   *
   * @tparam OutputIterator
   *		The iterator type to be used to store the results.
   *
   * @tparam Function
   *		Type of the function to be called with
   *		successive elements from the input
   *		iterator. The function must return a result
   *		which is stored through the result iterator.
   */
  template<class InputIterator, class Last,class OutputIterator, class Function>
  class TransformThreadPoolImpl {
    typedef Transform_Queue<
      InputIterator,
      Last,
      OutputIterator,
      Function,
      is_forward_iterator<InputIterator>::value &&
      is_forward_iterator<OutputIterator>::value
    > Queue;
    Queue                 queue;
    GenericThreadPoolTmpl pool;

  public:

    /**
     * Run a function on all objects in an input iterator range,
     * store the return values through an output iterator.
     *
     * @param first
     *			The start of the range to be processed.
     *
     * @param last
     *			One past the end of the range to be processed.
     *
     * @param result
     *			The iterator receiving the results.
     *
     * @param fun
     *			The function to apply to all objects
     *			in the container.
     *
     * @param thread_count
     *			The number of threads to spawn. If the
     *			default value of -1 is specified, the
     *			thread count is determined based on
     *			the number of available hardware
     *			threads. A value of 1 selects the
     *			single-threaded algorithm.
     *
     * @param maxpart
     *			The maximum part of the remaining
     *			input range that one thread is allowed
     *			to take.  If maxpart is for example 5
     *			and 100 elements remain to be
     *			processed, then a task will take 100 /
     *			5 = 20 elements and process them. If a
     *			large value is chosen for maxpart,
     *			each thread will take small chunks of
     *			work and will look for more work
     *			frequently, causing increased
     *			synchronization overhead. If a small
     *			value is chosen for maxpart, each
     *			thread will take huge chunks of work,
     *			possibly leaving the remaining threads
     *			out of work at the end. A good value
     *			might be four times the number of
     *			threads. A value of 0 enforces
     *			single-object processing.
     */
    TransformThreadPoolImpl(
      InputIterator  & first,
      Last const     & last,
      OutputIterator & result,
      Function       & fun,
      int              thread_count,
      std::size_t      maxpart
    )
    : queue(first, last, result, fun, maxpart)
    , pool(queue, thread_count)
    { }

    /**
     * Collect threads, throw any pending exceptions.
     *
     * Use this function if waiting in the desctructor or throwing
     * exceptions from the destructor is undesirable.  After
     * join() returned, the thread pool can be destroyed without
     * delay and without throwing.
     */
    void
    join() { pool.join(); }
  };

  namespace parallel {

    /**
     * Run a function on all objects in a range of iterators, store
     * the return values through an output iterator.
     *
     * @param first
     *		The start of the range to be processed.
     *
     * @param last
     *		One past the end of the range to be processed.
     *
     * @param result
     *		The iterator receiving the return values
     *
     * @param fun
     *		The function to apply to all objects in the
     *		range.
     *
     * @returns
     *		The final value of the result iterator
     *
     * @tparam thread_count
     *		The number of threads to spawn. If the default
     *		value of -1 is specified, the thread count is
     *		determined based on the number of available
     *		hardware threads. A value of 1 selects the
     *		single-threaded algorithm.
     *
     * @tparam maxpart The maximum part of the remaining input
     *		range that one thread is allowed to take.  If
     *		maxpart is for example 5 and 100 elements
     *		remain to be processed, then a task will take
     *		100 / 5 = 20 elements and process them. If a
     *		large value is chosen for maxpart, each thread
     *		will take small chunks of work and will look
     *		for more work frequently, causing increased
     *		synchronization overhead. If a small value is
     *		chosen for maxpart, each thread will take huge
     *		chunks of work, possibly leaving the remaining
     *		threads out of work at the end. A good value
     *		might be three times the number of
     *		threads. The default value of 1 lets the
     *		system determine a value, which is three times
     *		the number of threads. A value of 0 enforces
     *		single-object processing.
     */
    template<
      int thread_count    = -1,
      std::size_t maxpart = 1,
      class InputIterator,
      class Last,
      class OutputIterator,
      class Function,
      class = typename std::enable_if<std::is_same<InputIterator,Last>::value ||
              !std::is_integral<InputIterator>::value ||
              !std::is_integral<Last>::value
              >::type
    >
    typename std::decay<OutputIterator>::type
    transform(
      InputIterator  first,
      Last const &   last,
      OutputIterator result,
      Function&&     fun
    ) {
      unsigned tc = GenericThreadPoolTmpl::determine_thread_count(thread_count);
      if (tc <= 1) {
        return std::transform(first, last, result, fun);
      } else {
        TransformThreadPoolImpl<InputIterator, Last, OutputIterator, Function>(
          first, last, result, fun, thread_count,
          maxpart != 1 ? maxpart : 3 * (tc + 1)
        );
        return result;
      }
    }

    /**
     * Run a function with each of a range of integral values, store
     * the return values through an output iterator.
     *
     * @param first
     *		The start of the range to be processed.
     *
     * @param last
     *		One past the end of the range to be processed.
     *
     * @param result
     *		The iterator receiving the return values
     *
     * @param fun
     *		The function to call with all numbers in the
     *		range.
     *
     * @returns
     *		The final value of the result iterator
     *
     * @tparam thread_count
     *		The number of threads to spawn. If the default
     *		value of -1 is specified, the thread count is
     *		determined based on the number of available
     *		hardware threads. A value of 1 selects the
     *		single-threaded algorithm.
     *
     * @tparam maxpart The maximum part of the remaining input
     *		range that one thread is allowed to take.  If
     *		maxpart is for example 5 and 100 elements
     *		remain to be processed, then a task will take
     *		100 / 5 = 20 elements and process them. If a
     *		large value is chosen for maxpart, each thread
     *		will take small chunks of work and will look
     *		for more work frequently, causing increased
     *		synchronization overhead. If a small value is
     *		chosen for maxpart, each thread will take huge
     *		chunks of work, possibly leaving the remaining
     *		threads out of work at the end. A good value
     *		might be three times the number of
     *		threads. The default value of 1 lets the
     *		system determine a value, which is three times
     *		the number of threads. A value of 0 enforces
     *		single-object processing.
     */
    template<
      int thread_count =-1,
      unsigned maxpart = 1,
      class InputIterator, class Last, class OutputIterator, class Function,
      class = typename std::enable_if<!std::is_same<InputIterator,Last>::value &&
              std::is_integral<InputIterator>::value &&
              std::is_integral<Last>::value
              >::type
    >
    typename std::decay<OutputIterator>::type
    transform(
      InputIterator  && first,
      Last const      & last,
      OutputIterator && result,
      Function&& fun
    ) {
      /*
        We can not use the generic function because the user
        might specify `first` as 0 which makes type
        `InputIterator' become `int`, and `last` as something of
        type `std::size_t` not representable in an `int`. This
        loop would run forever. Just extend type `InputIterator`.
       */
      typedef typename std::common_type<InputIterator, Last>::type common_type;
      typedef IntegralIterator<common_type> CommonIterator;

      return transform<thread_count, maxpart>(
        CommonIterator(std::forward<InputIterator>(first)),
        CommonIterator(last),
        std::forward<OutputIterator>(result),
        std::forward<Function>(fun));
    }

    /**
     * Run a function on all objects in a container, store the return
     * values through an output iterator.
     *
     * Version for lvalue containers
     *
     * @param container
     *		The container.
     *
     * @param result
     *		The iterator receiving the return values
     *
     * @param fun
     *		The function to apply to all objects in the
     *		container.
     *
     * @returns
     *		The final value of the result iterator
     *
     * @tparam thread_count
     *		The number of threads to spawn. If the default
     *		value of -1 is specified, the thread count is
     *		determined based on the number of available
     *		hardware threads. A value of 1 selects the
     *		single-threaded algorithm.
     *
     * @tparam maxpart The maximum part of the remaining input
     *		range that one thread is allowed to take.  If
     *		maxpart is for example 5 and 100 elements
     *		remain to be processed, then a task will take
     *		100 / 5 = 20 elements and process them. If a
     *		large value is chosen for maxpart, each thread
     *		will take small chunks of work and will look
     *		for more work frequently, causing increased
     *		synchronization overhead. If a small value is
     *		chosen for maxpart, each thread will take huge
     *		chunks of work, possibly leaving the remaining
     *		threads out of work at the end. A good value
     *		might be three times the number of
     *		threads. The default value of 1 lets the
     *		system determine a value, which is three times
     *		the number of threads. A value of 0 enforces
     *		single-object processing.
     */
    template<
      int         thread_count = -1,
      std::size_t maxpart      = 1,
      class Container,
      class OutputIterator,
      class Function
    >
    typename std::decay<OutputIterator>::type
    transform(
      Container      &  container,
      OutputIterator && result,
      Function       && fun
    ) {
      return transform<thread_count, maxpart>(
        std::begin(container),
        std::end(container),
        std::forward<OutputIterator>(result),
        std::forward<Function>(fun)
      );
    }

    /**
     * Run a function on all objects in a container, store the return
     * values through an output iterator.
     *
     * Version for rvalue containers
     *
     * @param container
     *		The container.
     *
     * @param result
     *		The iterator receiving the return values
     *
     * @param fun
     *		The function to apply to all objects in the
     *		container.
     *
     * @returns
     *		The final value of the result iterator
     *
     * @tparam thread_count
     *		The number of threads to spawn. If the default
     *		value of -1 is specified, the thread count is
     *		determined based on the number of available
     *		hardware threads. A value of 1 selects the
     *		single-threaded algorithm.
     *
     * @tparam maxpart The maximum part of the remaining input
     *		range that one thread is allowed to take.  If
     *		maxpart is for example 5 and 100 elements
     *		remain to be processed, then a task will take
     *		100 / 5 = 20 elements and process them. If a
     *		large value is chosen for maxpart, each thread
     *		will take small chunks of work and will look
     *		for more work frequently, causing increased
     *		synchronization overhead. If a small value is
     *		chosen for maxpart, each thread will take huge
     *		chunks of work, possibly leaving the remaining
     *		threads out of work at the end. A good value
     *		might be three times the number of
     *		threads. The default value of 1 lets the
     *		system determine a value, which is three times
     *		the number of threads. A value of 0 enforces
     *		single-object processing.
     */
    template<
      int         thread_count = -1,
      std::size_t maxpart      = 1,
      class Container,
      class OutputIterator,
      class Function,
      class = typename std::enable_if<!std::is_lvalue_reference<Container>::value>::type
    >
    typename std::decay<OutputIterator>::type
    transform(
      Container      && container,
      OutputIterator && result,
      Function       && fun
    ) {
      return transform<thread_count, maxpart>(
        std::make_move_iterator(std::begin(container)),
        std::make_move_iterator(std::end(container)),
        std::forward<OutputIterator>(result),
        std::forward<Function>(fun)
      );
    }

  } // End of namespace parallel

} // End of namespace threadpool

#endif // !defined(PARALLEL_TRANSFORM_H)
