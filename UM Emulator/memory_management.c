/*
 *      memory_management.c
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Implementation for the memory_management.h file. Functions in this file
 *      are used to created a segmented memory structure, add new segment, 
 *      access to a specific segment or to a word at a given offset in a 
 *      specific segment, add a word to a given segment, get the length of 
 *      a segment or the total number of segments in the memory (mapped or 
 *      unmapped) and free the memory allocated for this module.
 */


/* Hanson Libraries */
#include <assert.h>
#include <stdint.h>
#include <except.h>

/* Custom .h files */
#include "memory_management.h"

/* Hint value definition for memory initiation */
#define HINT 10000

/* Exception for when segment or word to be retrieved is out of bounds */
Except_T Bad_Bounds_Mem = { "Index out of bounds" };

/******************************** memory_new *********************************
 *
 * Creates and returns a sequence representing segmented memory where each
 * element is a uarray of uint32_t's. Initializes the first element of the 
 * memory sequence to be the uarray of words representing the program.
 *
 * Inputs:
 *              UArray_T program: UArray of 32-bit words representing the 
 *                                program being run
 * Return:
 *              sequence of uarrays of 32-bit words representing
 *                    segmented memory
 * Expects:
 *              program to not be null and memory to be successfully
 *              instantiated
 * Notes:
 *              will CRE if expectations fail
 *              it's the user's responsibility to call memory_free to free 
 *              the memory sequence and the segment sequences stored in it
 *
 *****************************************************************************/
Seq_T memory_new(UArray_T program)
{
        assert(program != NULL);
        
        /* Initialize memory sequence */
        Seq_T mem = Seq_new(HINT);
        assert(mem != NULL);

        /* Push uarray representing program as the first element of memory */
        Seq_addhi(mem, program);
        return mem;
}


/******************************** segment_put *********************************
 *
 * puts given segment to memory in the specific id given
 *
 * Inputs:
 *              Seq_T memory: Sequence of sequences of 32-bit words 
 *                            representing a segmented memory
 *              int segment_id: which index the segment will be placed at
 *              void *value: segment to be placed into memory when dereferenced
 * Return:
 *              none
 * Expects:
 *              memory to not be null
 *              index to be in bounds of memory
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void segment_put(Seq_T memory, int segment_id, void *value)
{
        /* check requirements */
        assert(memory != NULL);

        /* check id bounds */
        if (segment_id >= Seq_length(memory) || segment_id < 0) {
                RAISE(Bad_Bounds_Mem);
        }

        /* push value to specific segment in memory */
        Seq_put(memory, segment_id, value);
}

/******************************** segment_add *********************************
 *
 * Adds the given segment as the last element of the given memory sequence 
 *
 * Inputs:
 *              Seq_T memory: Sequence of uarrays that the segment will be
 *                            added onto, representing memory
 *              void *elem: uarray of uint32_t words that represents a segment
 * Return:
 *              none
 * Expects:
 *              memory and segment to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void segment_add(Seq_T memory, void *segment)
{
        /* check for requirements */
        assert(memory != NULL);
        assert(segment != NULL);
        
        /* push new instruction to segment */
        Seq_addhi(memory, segment);
}


/******************************** segment_at *********************************
 *
 * Returns a pointer to the segment sequence at the given index of the memory 
 *
 * Inputs:
 *              Seq_T memory: Sequence of uarrays that holds segments, 
 *                            representing memory
 *              int seg_id  : The index where the desired segment is at in the
 *                            memory
 * Return:
 *              pointer to the segment at the given index in memory
 * Expects:
 *              memory to not be null and index to not be out of memory bounds
 * Notes:
 *              will CRE if memory is null, and will cause Unchecked Runtime 
 *              Error by raising Bad_Bounds_Mem exception if index is out of 
 *              bounds
 *
 *****************************************************************************/
void *segment_at(Seq_T memory, int seg_id)
{
        /* check for requirements */
        assert(memory != NULL);

        /* bounds checking for memory */
        if (seg_id >= Seq_length(memory) || seg_id < 0) {
                RAISE(Bad_Bounds_Mem);
        }
        
        /* returns the segment at specified seg_id */
        return Seq_get(memory, seg_id);
}

/******************************** word_at *********************************
 *
 * Returns a pointer to the word in the given offset of the segment
 *
 * Inputs:
 *              UArray_T segment: uarray of uint32_t words, representing
 *                                a segment in memory
 *              int offset      : the index where the desired word is at
 *                                in the segment
 * Return:
 *              pointer to the word in the given offset of the segment
 * Expects:
 *              segment to not be null and offset to not be
 *              out of bounds for the segment
 * Notes:
 *              will CRE if segment is null, and will cause Unchecked Runtime 
 *              Error by raising Bad_Bounds_Mem exception if offset is out of 
 *              bounds
 *
 *****************************************************************************/
void *word_at(UArray_T segment, int offset)
{
        /* checks for requirements */
        assert(segment != NULL);

        /* bounds checking for segment */
        if (offset >= UArray_length(segment) || offset < 0) {
                RAISE(Bad_Bounds_Mem);
        }

        /* returns instruction at specified offset */
        return UArray_at(segment, offset);
}

/******************************** memory_free *********************************
 *
 * Frees the uarrays representing segments within the sequence representing
 * memory, and frees the spine of memory
 *
 * Inputs:
 *              Seq_T *memory: Sequence of uarrays of 32-bit words 
 *                             representing a segmented memory
 * Return:
 *              none
 * Expects:
 *              pointer to memory and memory to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void memory_free(Seq_T *memory)
{
        /* checks for requirements */
        assert(memory != NULL);
        assert(*memory != NULL);
        
        /* gets number of segments */
        int lengthmem = Seq_length(*memory);
        
        /* goes through each segment */
        for (int i = 0; i < lengthmem; i++) {
                UArray_T segment = Seq_remlo(*memory);
                /* frees non-null segments */
                if (segment != NULL) {
                        UArray_free(&segment);
                }
        }

        /* frees spine of memory structure */
        Seq_free(memory);
}

/******************************** free_seg *********************************
 *
 * Frees given segment. This function will only be called when initiating
 * the program to memory. No other segment will be represented as a sequence.
 *
 * Inputs:
 *              Seq_T seg: Sequence of uint32_t words to initiate program
 * Return:
 *              none
 * Expects:
 *              provided segment to not be null
 * Notes:
 *              will CRE if expectation fails
 *
 *****************************************************************************/
void free_seg(Seq_T seg)
{
        /* check for requirements */
        assert(seg != NULL);

        /* get length of segment */
        int lengthseg = Seq_length(seg);
        
        /* frees each word in segment */
        for (int j = 0; j < lengthseg; j++) {
                free(Seq_remlo(seg));
        }

        /* frees segment */
        Seq_free(&seg);
}

/******************************** get_length *********************************
 *
 * Returns number of segments within memory or number of indices in id manager
 * depending on which sequence it is called on
 *
 * Inputs:
 *              Seq_T seq: sequence to calculate the length of
 * Return:
 *              the number of elements of the given sequence
 * Expects:
 *              seq to not be null
 * Notes:
 *              will CRE if expectation fails
 *
 *****************************************************************************/
int get_length(Seq_T seq)
{
        assert(seq != NULL);
        return Seq_length(seq);
}

/******************************** get_length_seg ******************************
 *
 * Returns number of words within a segment
 *
 * Inputs:
 *              UArray_T seg: segment to calculate the length of
 * Return:
 *              the number of elements of the given segment
 * Expects:
 *              seg to not be null
 * Notes:
 *              will CRE if expectation fails
 *
 *****************************************************************************/
int get_length_seg(UArray_T seg)
{
        assert(seg != NULL);
        return UArray_length(seg);
}

/******************************** id_m_new *********************************
 *
 * Creates and returns a sequence representing the free ids within a memory
 * representation.
 *
 * Inputs:
 *              n/a
 * Return:
 *              sequence of 32-bit words representing free segments that can
 *              be mapped to in memory
 * Expects:
 *              id manager to be successfully instantiated
 * Notes:
 *              will CRE if expectations fail
 *              it's the user's responsibility to free the id manager by 
 *              calling id_m_free function
 *
 *****************************************************************************/
Seq_T id_m_new()
{
        Seq_T id_m = Seq_new(HINT);
        assert(id_m != NULL);

        return id_m;
}

/******************************** id_m_add *********************************
 *
 * Creates and returns a sequence representing the free ids within a memory
 * representation.
 *
 * Inputs:
 *              Seq_T id_m: sequence of 32-bit words representing free segments
 *                          that can be mapped to in memory
 *              uint32_t id: 32-bit word representing a free segment's id
 * Return:
 *              n/a
 * Expects:
 *              id_m to not be null
 * Notes:
 *              will CRE if expectations fail
 *****************************************************************************/
void id_m_add(Seq_T id_m, uint32_t id)
{
        assert(id_m != NULL);
        
        /* push new free id to id manager */
        Seq_addlo(id_m, (void *)(uintptr_t)id);
}

/******************************** id_m_pop *********************************
 *
 * Returns a free id in memory to map a segment into.
 *
 * Inputs:
 *              Seq_T id_m: sequence of 32-bit words representing free segments
 *                          that can be mapped to in memory
 * Return:
 *              uint32_t id representing a free segment's id
 * Expects:
 *              id_m to not be null
 * Notes:
 *              will CRE if expectations fail
 *****************************************************************************/
uint32_t id_m_pop(Seq_T id_m)
{
        assert(id_m != NULL);
        
        /* returns a free id to be mapped to */
        return (uint32_t)(uintptr_t)Seq_remlo(id_m);
}

/******************************** id_m_free *********************************
 *
 * Frees the id manager representing the free indices where segments can be
 * mapped to in memory
 *
 * Inputs:
 *              Seq_T *id_m: Sequence of uint32_t words representing free
 *                           segment indices to be freed
 * Return:
 *              none
 * Expects:
 *              pointer to id_m and id_m to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void id_m_free(Seq_T *id_m)
{
        /* checks for requirements */
        assert(id_m != NULL);
        assert(*id_m != NULL);
        
        /* frees id_m structure */
        Seq_free(id_m);
}

#undef HINT