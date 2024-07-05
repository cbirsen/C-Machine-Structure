/*
 *      memory_management.h
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Interface for the memory_management.h file. Functions in this file
 *      are used to created a segmented memory structure, add new segment, 
 *      access to a specific segment or to a word at a given offset in a 
 *      specific segment, add a word to a given segment, get the length of 
 *      a segment or the total number of segments in the memory (mapped or 
 *      unmapped) and free the memory allocated for this module.
 */


#ifndef MEMORY_MANAGEMENT_INCLUDED
#define MEMORY_MANAGEMENT_INCLUDED

/* Standard C Libraries */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* Hanson Libraries */
#include <seq.h>
#include <uarray.h>

extern Seq_T memory_new(UArray_T program);
extern void segment_add(Seq_T memory, void *segment);
extern void *segment_at(Seq_T memory, int seg_id);
extern void *word_at(UArray_T segment, int offset);
extern void segment_put(Seq_T memory, int segment_id, void *value);
extern void free_seg(Seq_T seg);
extern void memory_free(Seq_T *memory);
extern int get_length(Seq_T mem);
extern int get_length_seg(UArray_T seg);
extern Seq_T id_m_new();
extern void id_m_add(Seq_T id_m, uint32_t id);
extern uint32_t id_m_pop(Seq_T id_m);
extern void id_m_free(Seq_T *id_m);

#endif