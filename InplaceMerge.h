#pragma once

/*************************************************************************
 * File: InplaceMerge.hh
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * An implementation of the fast inplace merge algorithm described by Huang
 * and Langston in "Practical In-Place Merging," Communications of the ACM,
 * Volume 31, Issue 3, March 1988, pages 348-352.
 *
 * The algorithm works by splitting the input into sqrt(n) blocks of size
 * sqrt(n), choosing one block to use as a buffer, and then merging the
 * other blocks together with the buffer as scratch space.  The main complexity
 * of the algorithm comes from the case where the input is not a perfect
 * square in length.  When this happens, the input needs to be heavily
 * preprocessed into a sequence of blocks that are of suitable size.
 *
 * A summary of the algorithm is as follows.  First, we compute the size of
 * each block to be s = ceil(sqrt(n)).  Next, we locate the largest s elements
 * of the input, which are some combination of the last elements of the two
 * sequences to be merged.  The paper refers to the group of largest elements
 * in the first sequence as A and the group in the second sequence as B.
 * Next, a group of elements C is chosen which directly precedes A and has
 * size equal to B, and a group of elements D is chosen which directly precedes
 * B and has size K - |B| mod s, where K is the length of either list.  The
 * reason for this choice is that the second list, with B and D removed, has
 * a perfect multiple of s elements.  A group F is then chosen at the front
 * of the first list with size K mod s, again to make the first list have
 * a size multiple of s.  The input now looks like this:
 *
 * F s s s ... s s C A s s s ... s D B
 *
 * Here, each s refers to an arbitrary block of size s.  We ned swap regions
 * C and B, which have the same size.  This puts B and A next to each other,
 * and since they were chosen to be the largest elements of the entire
 * sequence, we will treat them together as a buffer block, referred to by
 * Z.  This is seen here:
 *
 * F s s s ... s s Z s s s ... s D B
 *
 * Everything here except for F, D, and B are in their proper place.  Without
 * any optimizations, we could now correctly sort all of these elements
 * in-place in O(n) as follows.  First we apply the subroutine described
 * below to sort all of the s blocks together, giving us the following
 * sequence:
 *
 * F [all s blocks in sorted order] Z D B
 *
 * Since |F| < s and |Z| = s, we could then merge F and the sorted sequence
 * using Z as a buffer in O(n) time, then repeat this process to merge
 * C and B into the sequence as well.  However, the paper details several
 * optimizations which yield an O(n) algorithm with much smaller runtime
 * as follows.  Recall that the elements are ordered as follows:
 *
 * F s s s ... s s Z s s s ... s D B
 *
 * We begin by merging together the elements of D and B together using Z as
 * a temporary buffer.  This produces a new group E, as seen here:
 *
 * F s s s ... s s Z s s s ... s E
 *
 * At this point we slightly diverge from the original text.  Let the first
 * block of the second range be G, and then swap that block with the first
 * actual block of the first list (H):
 *
 * F G s s ... s s Z H s ... s E
 *
 * Notice that F and G have the smallest elements from the range, so if we
 * merge the two of them together, the first |F| elements of the result are
 * the smallest elements of the entire range.  We merge them together
 * using the buffer as scratch space, yielding two new groups F' and G',
 * as seen here:
 *
 * F' G' s s ... s s Z H s ... s E
 *
 * Finally, exchange G' and H to get
 *
 * F' H s s ... s s Z G' s ... s E
 *
 * Now, the only parts of this setup that aren't perfect multiples of s are
 * F', which we don't need to touch (it's already in the right place), and
 * E.  Once we've run the subroutine described below, we can merge E into the
 * sequence quite easily; we'll see how later.
 *
 * The core of the algorithm is a block merging step in which all of the
 * size-s blocks are merged into sorted order, using a special buffer block
 * as scratch space.  This buffer must consist of the largest elements of the
 * sequence for reasons that will become clearer later on.  Thus the input to
 * this subroutine looks like this:
 *
 * s s s ... s Z s ... s s
 *
 * Where Z is the buffer block.  Note that because the input was initially two
 * sorted sequences, each of these blocks is sorted in ascending order.
 *
 * The first step of this algorithm is to swap the buffer to the front of the
 * sequence in O(s) time, as seen here:
 *
 * Z s s s ... s s
 *
 * Next, we sort the remaining blocks in ascending order by their last element.
 * To do so, we use naive selection sort.  Selection sorting O(s) blocks will
 * make O(s^2) = O(n) comparisons, and will perform a total of O(s) block
 * swaps, each of which takes O(s) time, for a total of O(s^2) = O(n) swap
 * time.
 *
 * Finally, we begin the merge.  Starting at the leftmost block, we scan for
 * the largest increasing sequence we can find by picking a string of blocks
 * where each block's last element is less than the next block's first element.
 * This gives us an O(s) way of locating increasing sequences, since each
 * block is internally sorted.  Once we have located this sequence, we merge
 * it in with the next block, using the buffer as temporary space.  We stop
 * the merge as soon as the last element of the first sequence is moved into
 * the correct place.  Since the blocks are stored in sorted ascending order,
 * the merge will terminate with the buffer elements moved up past the first
 * sequence and before some remaining portion of the second sequence.  This
 * process terminates when the buffer comes directly before a nondecreasing
 * sequence.  When this happens, we use the rotate algorithm to push the
 * buffer past the last elements.  This gives us a sequence of sorted blocks
 * followed by an unsorted block containing the largest elements of the
 * sequence, which are all in the correct region of the input.  We can then
 * sort the buffer to finish the merge.
 *
 * Once we've finished this step, we have everything in place except for the
 * elements in E.  We can fix this easily as follows.  First, swap |E| of the
 * smallest elements from the sorted sequence and E.  We now have this setup:
 *
 * E {...sorted sequence...} {|E| of the smallest elements}
 *
 * Now, do a backwards merge of E and the sorted sequence in a fashion that
 * moves the last |E| elements to the front of the array, then sort the first
 * E elements in-place.  Voila!  Everything is sorted!
 */
#ifndef InplaceMerge_Included
#define InplaceMerge_Included

#include <algorithm>  // For rotate, swap_ranges, make_heap, sort_heap, iter_swap
#include <iterator>   // For iterator_traits, distance
#include <stdexcept>  // For invalid_argument
#include <cmath>      // For ceil, sqrt
#include <utility>    // For pair
#include <functional> // For less

 /**
  * Function: InplaceMerge(RandomIterator start, RandomIterator end);
  * -------------------------------------------------------------------
  * Given the sequence [start, end) such that the first half of the
  * elements form an ascending sequence and the second half of the
  * elements is also an ascending sequences, sorts the entire range
  * in ascending order in O(n) time and O(1) auxiliary storage space.
  */
template <typename RandomIterator>
void InplaceMerge(RandomIterator start, RandomIterator end);

/**
 * Function: InplaceMerge(RandomIterator start, RandomIterator end,
 *                        Comparator comp);
 * -------------------------------------------------------------------
 * Given the sequence [start, end) such that the first half of the
 * elements form an ascending sequence and the second half of the
 * elements is also an ascending sequences, sorts the entire range
 * in ascending order in O(n) time and O(1) auxiliary storage space.
 * The ranges are assumed to be sorted in ascending order according
 * to comp.
 */
template <typename RandomIterator, typename Comparator>
void InplaceMerge(RandomIterator start, RandomIterator end,
    Comparator comp);

/* * * * * Implementation Below This Point * * * * */
namespace inplacemerge_detail {
    /**
     * Function: pair<RandomIterator, RandomIterator>
     *              LocateMaxElems(RandomIterator begin, RandomIterator mid,
     *                             RandomIterator end, size_t s,
     *                             Comparator comp);
     * ---------------------------------------------------------------------
     * Given two sorted ranges [begin, mid) and [mid, end) which are sorted
     * according to comp and a parameter s, returns iterators (A, B) such
     * that [A, mid) and [B, end) are the s largest elements of the two
     * sorted ranges.
     */
    template <typename RandomIterator, typename Comparator>
    std::pair<RandomIterator, RandomIterator>
        LocateMaxElems(RandomIterator begin, RandomIterator mid,
            RandomIterator end, size_t s, Comparator comp) {
        /* This operation is essentially a merge step in which we don't move
         * anything around.  We keep two pointers, one to the end of each range,
         * then continuously move them inward by skipping past the larger of
         * the two at each step.  Once we've visited s elements, we're done.
         */
        std::pair<RandomIterator, RandomIterator> result(mid, end);

        /* Locate the largest s elements one at a time. */
        for (size_t i = 0; i < s; ++i) {
            /* The elements we're interested in comparing are one step before
             * the current iterator positions.  This is because iterators define
             * half-open ranges.
             */
            if (comp(*(result.first - 1), *(result.second - 1))) // Second > First
                --result.second;
            else // First >= Second
                --result.first;
        }

        return result;
    }

    /**
     * Function: BufferedInplaceMerge(RandomIterator begin, RandomIterator mid,
     *                                RandomIterator end, RandomIterator bStart,
     *                                RandomIterator bEnd, Comparator comp);
     * ----------------------------------------------------------------------
     * Utility function which, given adjacent ranges [begin, mid) and [mid, end)
     * to be merged, does a standard merge using the buffer [bStart, bEnd) for
     * auxiliary storage space.  The input ranges are assumed to be sorted
     * using comp and will ultimately be sorted by comp.  It is assumed that
     * the range [begin, mid) fits in the buffer, though [mid, end) does not
     * need to.
     */
    template <typename RandomIterator, typename Comparator>
    void BufferedInplaceMerge(RandomIterator begin, RandomIterator mid,
        RandomIterator end, RandomIterator bStart,
        RandomIterator bEnd, Comparator comp) {
        /* The algorithm works as follows.  We begin by swapping the first range
         * into the buffer.  Next, we maintain two pointers to the start points
         * of the ranges, and use a standard merge technique to select the
         * smaller of the two, which is then swapped into the proper position.
         * If the second range is exhausted, what's left of the first range
         * is swapped into place.  If the first range is exhausted, then we do
         * not need to take any action because the second range is already
         * at the end of the destination.
         */

         /* Move the first elements into the buffer. */
        std::swap_ranges(begin, mid, bStart);

        /* Maintain pointers to the first elements of both ranges and the next
         * write position.
         */
        RandomIterator one = bStart, two = mid, nextWrite = begin;

        /* Cache the end point of the first range. */
        const RandomIterator oneEnd = bStart + (mid - begin);

        /* While neither range has been exhausted, do the standard merge step. */
        while (one != oneEnd && two != end) {
            /* Determine which element comes next. */
            if (comp(*one, *two)) { // First element is smaller
                std::iter_swap(one, nextWrite);
                ++one;
            }
            else { // Second element is smaller
                std::iter_swap(two, nextWrite);
                ++two;
            }

            /* Bump the next write position. */
            ++nextWrite;
        }

        /* If we ran out of elements from the first range, we're done. */
        if (one == oneEnd) return;

        /* Otherwise, we need to exchange the rest of the first range with
         * the accumulated buffer elements at the end of the merge range.
         */
        std::swap_ranges(one, oneEnd, nextWrite);
    }

    /**
     * Function: SortBlocks(RandomIterator begin, RandomIterator end,
     *                      size_t blockSize, Comparator comp);
     * ----------------------------------------------------------------------
     * Given a range of blocks [begin, end) of size blockSize, sorts them
     * in ascending order by comp.  This step uses naive selection sort to
     * ensure that the sorting takes time O(n).
     */
    template <typename RandomIterator, typename Comparator>
    void SortBlocks(RandomIterator begin, RandomIterator end,
        size_t blockSize, Comparator comp) {
        for (RandomIterator itr = begin; itr != end; itr += blockSize) {
            /* Scan over all the blocks from here forward, finding the smallest. */
            RandomIterator minBlock = itr;
            for (RandomIterator curr = itr + blockSize; curr != end; curr += blockSize) {
                /* If this is outright smaller, pick it. */
                if (comp(*(curr + blockSize - 1), *(minBlock + blockSize - 1)))
                    minBlock = curr;
                /* Otherwise if it's equal but has a smaller first element, pick it. */
                else if (!comp(*(minBlock + blockSize - 1), *(curr + blockSize - 1)) &&
                    comp(*curr, *minBlock))
                    minBlock = curr;
            }

            /* If the element is in the right place, do nothing.  Otherwise, swap
             * the current block and the best block.
             */
            if (minBlock != itr)
                std::swap_ranges(itr, itr + blockSize, minBlock);
        }
    }

    /**
     * Function: FindEndOfRange(RandomIterator begin, RandomIterator end,
     *                          size_t blockSize, Comparator comp);
     * ---------------------------------------------------------------------
     * Given a range of blocks [begin, end) of size blockSize, returns an
     * iterator to the start of the first block whose head is smaller than
     * its predecessor's tail.  If no such block exists, end is returned.
     */
    template <typename RandomIterator, typename Comparator>
    RandomIterator FindEndOfRange(RandomIterator begin, RandomIterator end,
        size_t blockSize, Comparator comp) {
        /* Scan over the blocks after the first, checking whether the previous
         * block end is greater than the current block start.
         */
        for (RandomIterator itr = begin + blockSize; itr != end; itr += blockSize)
            if (comp(*itr, *(itr - 1)))
                return itr;

        /* All sorted; hand back end as a sentinel. */
        return end;
    }

    /**
     * Function: BufferMovingMerge(RandomIterator begin, RandomIterator mid,
     *                             RandomIterator end, RandomIterator buffer,
     *                             Comparator comp);
     * ----------------------------------------------------------------------
     * Given a range of elements [buffer, begin), [begin, mid), [mid, end),
     * merges the elements into a sorted range such that the resulting
     * sequence looks like
     *
     * [-merged elements-] [buffer] [unconsumed second sequence]
     *
     * This function then returns an iterator to the new start position of
     * the buffer.
     */
    template <typename RandomIterator, typename Comparator>
    RandomIterator BufferMovingMerge(RandomIterator begin, RandomIterator mid,
        RandomIterator end, RandomIterator buffer,
        Comparator comp) {
        /* This function works by doing a standard merge operation to consume
         * [begin, mid).  We maintain iterators to the start of the buffer,
         * the start of the first sequence, and the start of the second sequence.
         * We then continously swap the smaller of the two sequence elements
         * with the next buffer element and bump the pointers forward.
         */
        RandomIterator one = begin, two = mid, output = buffer;

        /* Keep writing until we exhaust the first sequence.  We are guaranteed
         * that we won't exhaust the second because its last element is bigger
         * than the last element of the first sequence.
         */
        while (one != mid) {
            /* Move the smaller element forward. */
            if (!comp(*two, *one)) { // First element is <=
                std::iter_swap(one, output);
                ++one;
            }
            else { // Second element is smaller
                std::iter_swap(two, output);
                ++two;
            }

            /* Advance the output pointer to the next spot. */
            ++output;
        }

        /* Return the start of the new buffer. */
        return output;
    }

    /**
     * Function: MergeBlocks(RandomIterator begin, RandomIterator end,
     *                       size_t blockSize, Comparator comp)
     * ----------------------------------------------------------------------
     * Given a range of blocks of size blockSize that evenly partition
     * [begin, end) into a sequence of blocks, does a merge step to convert the
     * range into a sorted range.  It is assumed that the elements
     * [begin, begin + blockSize) are buffer elements, and these elements will
     * end up occupying the last block of the sequence.
     */
    template <typename RandomIterator, typename Comparator>
    void MergeBlocks(RandomIterator begin, RandomIterator end,
        size_t blockSize, Comparator comp) {
        /* Begin by sorting all of the blocks in ascending order by their last
         * elements.
         */
        SortBlocks(begin + blockSize, end, blockSize, comp);

        /* Now, we need to start scanning over the blocks, looking for increasing
         * sequences that can be merged.  We'll do this by maintaining a pointer
         * to the start of the first block to merge, where in that block to
         * pick up from, and where the buffer is.
         */
        RandomIterator buffer = begin, currBlock = begin + blockSize,
            currPos = begin + blockSize;
        while (true) {
            /* Find the first block past the end of the current sequence. */
            RandomIterator rangeEnd = FindEndOfRange(currBlock, end,
                blockSize,
                comp);

            /* If we didn't find anything, we're done. */
            if (rangeEnd == end) break;

            /* Otherwise, do a buffer-moving merge of the range up through the
             * start of the last block with the last block, and remember where
             * the buffer ended up getting repositioned.
             */
            buffer = BufferMovingMerge(currPos, rangeEnd, rangeEnd + blockSize,
                buffer, comp);

            /* Update the current block to be the end of the range since we
             * didn't finish moving everything out of it.
             */
            currBlock = rangeEnd;

            /* Update the write position to be one block past the buffer start. */
            currPos = buffer + blockSize;
        }

        /* When we get here, we know that the range [currPos, end) is sorted.
         * To finish up this step, we sort the buffer using an in-place sort
         * (here, heapsort), then rotate it through the rest of the range to
         * finish up the sequence.
         */
        std::make_heap(buffer, buffer + blockSize, comp);
        std::sort_heap(buffer, buffer + blockSize, comp);
        std::rotate(buffer, buffer + blockSize, end);
    }
}

/* Actual implementation of InplaceMerge */
template <typename RandomIterator, typename Comparator>
void InplaceMerge(RandomIterator begin, RandomIterator end,
    Comparator comp) {
    /* Grant access to the utility functions we've written. */
    using namespace inplacemerge_detail;

    /* Confirm that the input has an even number of elements. */
    if (std::distance(begin, end) % 2 != 0)
        throw std::invalid_argument("Range to merge must have an even number of elements.");

    /* In order to guarantee that the input range has sufficiently many elements
     * that the named blocks exist, there should be at least 50 elements in
     * the range.  Otherwise, each half is not guaranteed to have at least
     * three blocks.  If there are fewer than 50 elements, we'll just use an
     * in-place heap sort to do the work.  This doesn't affect the overall
     * asymptotic runtime of the algorithm, since big-O only considers behavior
     * in the limit.
     */
    if (std::distance(begin, end) < 50) {
        std::make_heap(begin, end, comp);
        std::sort_heap(begin, end, comp);
        return;
    }

    /* Compute s, the block size.  The casts are necessary to resolve which
     * overload to use.
     */
    const size_t s = (size_t)std::ceil(std::sqrt(double(end - begin)));

    /* Cache the number of elements in each subrange. */
    const size_t listSize = (end - begin) / 2;

    /* Compute the midpoint, which is where the second list starts and
     * the first list stops.
     */
    RandomIterator mid = begin + listSize;

    /* Get back iterators to the start of blocks A and B. */
    std::pair<RandomIterator, RandomIterator> maxElems =
        LocateMaxElems(begin, mid, end, s, comp);

    /* Compute the groups C and D.  Group C is formed from the group of elements
     * of size |B| that directly precede A.
     */
    RandomIterator cStart = maxElems.first - std::distance(maxElems.second, end);

    /* Group D is formed by taking the K - |B| % s elements that precede B. */
    RandomIterator dStart = maxElems.second - ((listSize - std::distance(maxElems.second, end)) % s);

    /* Exchange C and B.  This makes the range [cStart, mid) the buffer. */
    std::swap_ranges(cStart, maxElems.first, maxElems.second);

    /* Now, we need to merge D and B together using the buffer as the temporary
     * storage space.  We don't want to move the buffer when doing this, so we'll
     * do an in-place merge using the buffer as a temporary.
     */
    BufferedInplaceMerge(dStart, maxElems.second, end, // D B
        cStart, mid,                  // Buffer location
        comp);

    /* Next, we need to fix up the leftover elements from the front of the first
     * list.  The first list is as it originally was, except that the last block
     * has been swapped with the buffer.  This means that the number of elements
     * that are beyond what's necessary is K mod s.  Let's see what this is.
     */
    const size_t firstSlack = listSize % s;

    /* There are two cases to consider.  First, if s == 0, then there is no
     * leftover slack and we can just run the main algorithm.  Otherwise,
     * we need to do a merge of these elements with the smallest elements from
     * the second list.
     */
    if (firstSlack != 0) {
        /* Swap the first block from the second list and the second block from
         * the first list.  This puts the smallest elements from the two lists
         * adjacent to one another.
         */
        std::swap_ranges(begin + firstSlack, begin + firstSlack + s, mid);

        /* Merge the slack space and this block using the buffer for scratch. */
        BufferedInplaceMerge(begin, begin + firstSlack, begin + firstSlack + s,
            cStart, mid, comp);

        /* Move the blocks back. */
        std::swap_ranges(begin + firstSlack, begin + firstSlack + s, mid);
    }

    /* Okay!  At this point we're ready to begin the main algorithm.  We do
     * this by swapping the buffer and the first actual block.
     */
    std::swap_ranges(begin + firstSlack, begin + firstSlack + s, cStart);

    /* Run the block-merging step to eat everything from the first block up to
     * the start of E.
     */
    MergeBlocks(begin + firstSlack, dStart, s, comp);

    /* At this point, all that's left to do is merge E into this resulting
     * sequence.  We do this by swapping E and the smallest elements of the
     * range, doing an in-place merge, then rotating the buffer back and
     * sorting it.
     */
    std::swap_ranges(dStart, end, begin);
    BufferedInplaceMerge(begin, begin + distance(dStart, end), dStart,
        dStart, end, comp);

    /* Sort the buffer using heapsort. */
    std::make_heap(dStart, end, comp);
    std::sort_heap(dStart, end, comp);

    /* Rotate the buffer to the front. */
    std::rotate(begin, dStart, end);
}

/* Non-comparator version calls comparator version. */
template <typename RandomIterator>
void InplaceMerge(RandomIterator begin, RandomIterator end) {
    InplaceMerge(begin, end,
        std::less<typename std::iterator_traits<RandomIterator>::value_type>());
}

#endif