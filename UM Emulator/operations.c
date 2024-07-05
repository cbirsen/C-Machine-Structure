/*
 *      operations.c
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Implementation for the operations.c file. Functions in this file are
 *      used to load in a program and perform the operation specified in the 
 *      program, such as addition, multiplication, division, nand, output, 
 *      input, and so on.
 */

/* CS40 Libraries */
#include <bitpack.h>

/* Hanson Libraries */
#include <assert.h>
#include <stdbool.h>

/* Custom .h files */
#include "operations.h"
#include "register.h"
#include "memory_management.h"

#define HINT 10000

/* Variable that specifies the type of operation to be performed */
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

/* Raised when the opcode does not represent a valid instruction */
Except_T Not_Recognized = { "Instruction Not Recognized" };

/* Raised when the program counter is out of the bounds for segment 0 */
Except_T Counter_Bounds = { "Counter Outside Bounds" };

/* Raised when the input or output value is out of the 0-255 range */
Except_T IO_Bounds = { "Need Value Between 0 and 255" };

/* Raised when a value is divided by 0 */
Except_T Division_Zero = { "Attempting Dividing by 0" };

/* Raised when the program refers to a value in an unmapped segment */
Except_T Segment_Unmapped = { "Refers to Unmapped Segment" };

/* Raised when the index of a word in a segment is out of bounds */
Except_T Word_Bounds = { "Word Index Out Of Bounds" };

/* Raised when the trying to unmap an unmapped segment or segment zero */
Except_T Faulty_Unmap = { "Refers to Unmapped Segment or Segment Zero" };

/* Helper Function Declarations */
void word_interpreter(Seq_T mem, UArray_T registers, Seq_T id_m);
void conditional_move(int reg_a, int reg_b, int reg_c, UArray_T reg);
void segmented_load(int reg_a, int reg_b, int reg_c, Seq_T mem, UArray_T reg);
void segmented_store(int reg_a, int reg_b, int reg_c, Seq_T mem, UArray_T reg);
void addition(int reg_a, int reg_b, int reg_c, UArray_T reg);
void multiplication(int reg_a, int reg_b, int reg_c, UArray_T reg);
void division(int reg_a, int reg_b, int reg_c, UArray_T reg);
void bitwise_nand(int reg_a, int reg_b, int reg_c, UArray_T reg);
void map_segment(int reg_b, int reg_c, Seq_T mem, UArray_T reg, Seq_T id_m);
void unmap_segment(int reg_c, Seq_T mem, UArray_T reg, Seq_T id_m);
void output(int reg_c, UArray_T reg);
void input(int reg_c, UArray_T reg);
int load_program(int reg_b, int reg_c, Seq_T mem, UArray_T reg);
void load_value(int reg_a, int val, UArray_T reg);
void segment_load_store_error(Seq_T mem, int segment_ind, int word_ind);

/****************************** initiate_program ******************************
 *
 * Creates registers, memory, and id manager that will be used during the
 * remainder of the program. Reads input from input file containing 
 * instructions to be executed, places them in memory and calls instruction 
 * executer.
 *
 * Inputs:
 *              FILE *fp: input file containing instructions
 * Return:
 *              n/a
 * Expects:
 *              File pointer to not be null
 * Notes:
 *              will CRE if any memory allocation fails to allocate space
 *              will CRE if 32-bit word left incomplete
 *              will CRE if input file null
 *
 *****************************************************************************/
void initiate_program(FILE *fp)
{
        assert(fp != NULL);

        /* a temp sequence is initiated to record to words as they are read */
        Seq_T prog = Seq_new(HINT);
        assert(prog != NULL);

        /* populating prog sequence by reading instructions from input file 
         * until end of input is reached
         */
        int c = getc(fp);
        while (c != EOF) {
                /* instruction initialized as 0 */
                uint32_t *word = malloc(sizeof(uint32_t));
                *word = (uint32_t)0;
                
                /* 32 bit word built */
                for(int i = 3; i >= 0; i--) {
                        if (i != 3) {
                                c = getc(fp);
                        }
                        assert(c != EOF);
                        *word = Bitpack_newu(*word, 8, i * 8, (uint32_t)c);
                }

                /* instruction added to program sequence */
                Seq_addhi(prog, word);
                c = getc(fp);
        }

        /* initialize segment 0 and transfer qords from temp prog sequence to 
         * seg_0 UArray
         */
        int proglen = Seq_length(prog);
        UArray_T seg_0 = UArray_new(proglen, sizeof(uint32_t));
        for (int i = 0; i < proglen; i++) {
                *(uint32_t *)word_at(seg_0, i) = *(uint32_t *)Seq_get(prog, i);
        }

        /* memory is initialized with segment 0 filled with program */
        Seq_T mem = memory_new(seg_0);

        /* registers and id manager for program is set up */
        Seq_T id_m = id_m_new();
        UArray_T registers = register_new();

        /* all program info is sent to helper function */
        word_interpreter(mem, registers, id_m);
        
        /* frees all allocated space */
        memory_free(&mem);
        register_free(&registers);
        free_seg(prog);

        //MODULARITY NOTE: BUNUN ICIN FUNCTION GEREKIR MI?
        // Seq_free(&id_m);
        id_m_free(&id_m);
}

/****************************** word_interpreter ******************************
 *
 * Goes through each instruction provided and calls appropriate operations
 * with necessary parameters depending on opcode provided in the word. Uses
 * bitpack to unpack words and acquire necessary fields.
 *
 * Inputs:
 *              Seq_T mem: sequence of UArrays of words representing memory
 *              UArray_T reg: UArray representing reg
 *              Seq_T id_m: id manager keeping track of free segments
 * Return:
 *              n/a
 * Expects:
 *              memory, reg and id manager to not be null
 * Notes:
 *              will CRE if expectations fail
 *              will URE if program does not end with halt or counter is out of
 *                      bounds
 *              will URE if the instruction is not known
 *
 *****************************************************************************/
void word_interpreter(Seq_T mem, UArray_T reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);

        /* get the segment 0 for program */
        UArray_T program = segment_at(mem, 0);

        /* gets number of words in running program, segment 0 */
        uint32_t length_p = get_length_seg(program);

        /* halt being called at the end of the program will be checked later */
        bool halt_called = false;

        /* goes through each word in segment 0 to execute instructions */
        for (uint32_t p_counter = 0; p_counter < length_p; p_counter++) {
                uint32_t curr_word = *(uint32_t *)word_at(program, p_counter);
                Um_opcode op = Bitpack_getu(curr_word, 4, 28);
                int ra = -1;
                int rb = -1;
                int rc = -1;
                int value = -1;

                /* populates related fields depending on operation code */
                if (op == LV) {
                        ra = Bitpack_getu(curr_word, 3, 25);
                        value = Bitpack_getu(curr_word, 25, 0);
                } else {
                        ra = Bitpack_getu(curr_word, 3, 6);
                        rb = Bitpack_getu(curr_word, 3, 3);
                        rc = Bitpack_getu(curr_word, 3, 0);
                }

                /* calls function for opcode in the instruction */
                switch (op) {
                        case CMOV:
                                conditional_move(ra, rb, rc, reg);
                                break;
                        case SLOAD:
                                segmented_load(ra, rb, rc, mem, reg);
                                break;
                        case SSTORE:
                                segmented_store(ra, rb, rc, mem, reg);
                                break;
                        case ADD:
                                addition(ra, rb, rc, reg);
                                break;
                        case MUL:
                                multiplication(ra, rb, rc, reg);
                                break;
                        case DIV:
                                division(ra, rb, rc, reg);
                                break;
                        case NAND:
                                bitwise_nand(ra, rb, rc, reg);
                                break;
                        case HALT:
                                /* jumps to the end of the program */
                                p_counter = length_p;
                                halt_called = true;
                                break;
                        case ACTIVATE:
                                map_segment(rb, rc, mem, reg, id_m);
                                break;
                        case INACTIVATE:
                                unmap_segment(rc, mem, reg, id_m);
                                break;
                        case OUT:
                                output(rc, reg);
                                break;
                        case IN:
                                input(rc, reg);
                                break;
                        case LOADP:
                                /* update counter, program, and prog lenght */
                                p_counter = load_program(rb, rc, mem, reg) - 1;
                                program = segment_at(mem, 0);
                                length_p = get_length_seg(program);
                                break;
                        case LV:
                                load_value(ra, value, reg);
                                break;
                        default:
                                /* unrecognized instruction */
                                RAISE(Not_Recognized);
                }
        }

        /* raise exception if halt was not called */
        if (!halt_called) {
                RAISE(Counter_Bounds);
        }
}

/****************************** conditional_move ******************************
 *
 * Updates the register with index reg_a to hold the same value as the register
 * with reg_b if the value in the register with reg_c is not zero
 * 
 * Inputs:
 *              int reg_a: register index to update
 *              int reg_b: register index to retrieve the value from
 *              int reg_c: register index to check if zero
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register array to not be null
 * Notes:
 *              will CRE if reg is null
 *
 *****************************************************************************/
void conditional_move(int reg_a, int reg_b, int reg_c, UArray_T reg)
{
        assert(reg != NULL);
        if (*(uint32_t *)value_at(reg, reg_c) != 0) {
                *(uint32_t *)value_at(reg, reg_a) = *(uint32_t *)value_at(reg, 
                                                                        reg_b);
        }
}

/****************************** segmented_load ******************************
 *
 * Updates the register with index reg_a to hold the word at offset reg_c of 
 * segment reg_b
 * 
 * Inputs:
 *              int reg_a: register index to update
 *              int reg_b: segment index
 *              int reg_c: offset index
 *              UArray_T reg: UArray representing 8 32-bit registers
 *              Seq_T mem: sequence of UArrays of words representing memory
 * Return:
 *              n/a
 * Expects:
 *              memory and register to not be null
 * Notes:
 *              will CRE if expectations fail
 *              will URE if the value in reg_b refers to an unmapped segment
 *              will URE if the value in reg_c refers to a location outside the
 *                      bounds of a mapped segment
 *
 *****************************************************************************/
void segmented_load(int reg_a, int reg_b, int reg_c, Seq_T mem, UArray_T reg)
{
        assert(mem != NULL);
        assert(reg != NULL);

        /* Retrieve the segment and word indices */
        uint32_t segment_id = *(uint32_t *)value_at(reg, reg_b);
        int word_id = *(uint32_t *)value_at(reg, reg_c);

        /* Check that the segment is mapped and word index is in bounds */
        segment_load_store_error(mem, segment_id, word_id);
        
        /* Update value at reg index reg_a to be equal to the word in memory */
        UArray_T seg = segment_at(mem, segment_id);
        *(uint32_t *)value_at(reg, reg_a) = *(uint32_t *)word_at(seg, word_id);
}

/****************************** segmented_store ******************************
 *
 * Updates the memory with segment reg_a and offset reg_b to hold the value in
 * the register with index reg_c
 * 
 * Inputs:
 *              int reg_a: segment index 
 *              int reg_b: offset index
 *              int reg_c: register index to update
 *              UArray_T reg: UArray representing 8 32-bit registers
 *              Seq_T mem: sequence of UArrays of words representing memory
 * Return:
 *              n/a
 * Expects:
 *              memory and register to not be null
 * Notes:
 *              will CRE if expectations fail
 *              will URE if the value in reg_a refers to an unmapped segment
 *              will URE if the value in reg_b refers to a location outside the
 *                      bounds of a mapped segment
 *
 *****************************************************************************/
void segmented_store(int reg_a, int reg_b, int reg_c, Seq_T mem, UArray_T reg)
{
        assert(mem != NULL);
        assert(reg != NULL);
        
        /* Retrieve the segment and word indices */
        uint32_t segment_ind = *(uint32_t *)value_at(reg, reg_a);
        int word_ind = *(uint32_t *)value_at(reg, reg_b);

        /* Check that the segment is mapped and word index is in bounds */
        segment_load_store_error(mem, segment_ind, word_ind);

        /* Update value in memory to be equal to the word at reg index reg_c */
        UArray_T seg = segment_at(mem, segment_ind);
        uint32_t *new_word = word_at(seg, word_ind);
        *new_word =  *(uint32_t *)value_at(reg, reg_c);
}

/************************** segment_load_store_error **************************
 *
 * Checks that the segment being referred to is mapped and the word being 
 * referred to is in bounds
 * 
 * Inputs:
 *              Seq_T mem: sequence of UArrays of words representing memory
 *              int segment_ind: segment id in the memory being referred to
 *              int word_ind: word offset in the segment being referred to
 * Return:
 *              n/a
 * Expects:
 *              memory to not be null
 * Notes:
 *              will CRE if expectations fail
 *              will URE if segment_ind refers to an unmapped segment
 *              will URE if word_ind refers to a location outside the bounds
 *                       of a mapped segment
 *
 *****************************************************************************/
void segment_load_store_error(Seq_T mem, int segment_ind, int word_ind)
{
        assert(mem != NULL);
        if (segment_at(mem, segment_ind) == NULL) {
                RAISE(Segment_Unmapped);
        }
        if (word_ind >= get_length_seg(segment_at(mem, segment_ind))) {
                RAISE(Word_Bounds);
        }

}

/****************************** addition ******************************
 *
 * Adds values at reg_b and reg_c together and stores sum in reg_a
 * 
 * Inputs:
 *              int reg_a: register index to store sum in
 *              int reg_b: register index to retrieve the value from
 *              int reg_c: register index to retrieve the value from
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register array to not be null
 * Notes:
 *              will CRE if reg is null
 *
 *****************************************************************************/
void addition(int reg_a, int reg_b, int reg_c, UArray_T reg)
{
        assert(reg != NULL);

        /* acquires the two values to get sum of */
        uint32_t left = *(uint32_t *)value_at(reg, reg_b);
        uint32_t right = *(uint32_t *)value_at(reg, reg_c);

        /* acquires address and stores sum in register */
        uint32_t *sum = (uint32_t *)value_at(reg, reg_a);
        *sum = (left + right);
}

/****************************** multiplication ******************************
 *
 * Multiplies values at reg_b and reg_c and stores product in reg_a
 * 
 * Inputs:
 *              int reg_a: register index to store product in
 *              int reg_b: register index to retrieve the value from
 *              int reg_c: register index to retrieve the value from
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register array to not be null
 * Notes:
 *              will CRE if reg is null
 *
 *****************************************************************************/
void multiplication(int reg_a, int reg_b, int reg_c, UArray_T reg)
{
        assert(reg != NULL);

        /* acquires the two values to get product of */
        uint32_t left = *(uint32_t *)value_at(reg, reg_b);
        uint32_t right = *(uint32_t *)value_at(reg, reg_c);
        
        /* acquires address and stores product in register */
        uint32_t *product = (uint32_t *)value_at(reg, reg_a);
        *product = (left * right);
}

/****************************** division ******************************
 *
 * Multiplies values at reg_b and reg_c and stores product in reg_a
 * 
 * Inputs:
 *              int reg_a: register index to store product in
 *              int reg_b: register index to retrieve the value from
 *              int reg_c: register index to retrieve the value from
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register array to not be null
 * Notes:
 *              will CRE if reg is null
 *              will URE if divisor is 0
 *
 *****************************************************************************/
void division(int reg_a, int reg_b, int reg_c, UArray_T reg)
{
        assert(reg != NULL);

        /* acquires dividend and divisor */
        uint32_t left = *(uint32_t *)value_at(reg, reg_b);
        uint32_t right = *(uint32_t *)value_at(reg, reg_c);
        
        /* URE if divisor is 0 */
        if (right == 0) {
                RAISE(Division_Zero);
        }
        
        /* acquires and stores quotient in register */
        uint32_t *quotient = (uint32_t *)value_at(reg, reg_a);
        *quotient = left / right;
}

/****************************** bitwise_nand ********************************
 *
 * Performs bitwise and on reg_b and reg_c and stores the complement in reg_a
 * 
 * Inputs:
 *              int reg_a: register index to store result in
 *              int reg_b: register index to retrieve the value from
 *              int reg_c: register index to retrieve the value from
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register array to not be null
 * Notes:
 *              will CRE if reg is null
 *
 *****************************************************************************/
void bitwise_nand(int reg_a, int reg_b, int reg_c, UArray_T reg)
{
        assert(reg != NULL);

        /* acquires pointers to registers */
        uint32_t *val_a = (uint32_t *)value_at(reg, reg_a);
        uint32_t *val_b = (uint32_t *)value_at(reg, reg_b);
        uint32_t *val_c = (uint32_t *)value_at(reg, reg_c);

        /* performs bitwise and and stores its complement in reg_a */
        *val_a = ~(*val_b & *val_c);
}

/****************************** map_segment ********************************
 *
 * A new segment is created with a number of words equal to the value in 
 * register with index reg_c. Each word in the new segment is initialized to 0.
 * A bit pattern that is not all zeroes and that does not identify any 
 * currently mapped segment is placed in register with index reg_b. The new 
 * segment is mapped as $m[$r[B]].
 * 
 * Inputs:
 *              int reg_b: register index to store the segment value in
 *              int reg_c: register index to retrieve the number of words to 
 *                         map
 *              UArray_T reg: UArray representing 8 32-bit registers
 *              Seq_T mem: sequence of UArrays of words representing memory
 *              Seq_T id_m: sequence of segment ids available to be mapped
 * Return:
 *              n/a
 * Expects:
 *              register, memory, id_m to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void map_segment(int reg_b, int reg_c, Seq_T mem, UArray_T reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);

        /* initialize the new segment with 0's as words */
        int num_words = *(uint32_t *)value_at(reg, reg_c);
        UArray_T new_seg = UArray_new(num_words, sizeof(uint32_t));
        for (int i = 0; i < num_words; i++) {
                *(uint32_t *)word_at(new_seg, i) = (uint32_t)0;
        }
        
        uint32_t segment_id = 0;
        
        /* if there is no space available in the current memory, add segment to
         * the end. if there is, place the segment to an unmapped location 
         * retrieved from id_m. update new segment_id
         */
        if (get_length(id_m) == 0) {
                segment_add(mem, new_seg);
                segment_id = get_length(mem) - 1;
        } else {
                segment_id = id_m_pop(id_m);
                segment_put(mem, segment_id, new_seg);
        }

        /* store new segment id in register with index reg_b */
        *(uint32_t *)value_at(reg, reg_b) = (uint32_t)segment_id;
}

        

/****************************** unmap_segment ********************************
 *
 * The segment with the id stored in register with index reg_c is unmapped
 * 
 * Inputs:
 *              int reg_c: register index to retrieve the segment id to be 
 *                         freed
 *              UArray_T reg: UArray representing 8 32-bit registers
 *              Seq_T mem: sequence of UArrays of words representing memory
 *              Seq_T id_m: sequence of segment ids available to be mapped
 * Return:
 *              n/a
 * Expects:
 *              register, memory, id_m to not be null
 * Notes:
 *              will CRE if expectations fail
 *              will URE if segment 0 or an already unmapped segment are tried 
 *                      to be unmapped
 *
 *****************************************************************************/
void unmap_segment(int reg_c, Seq_T mem, UArray_T reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);
        
        /* retrieve the segment id to be unmapped and add it to free ids seq */
        uint32_t segment_ind = *(uint32_t *)value_at(reg, reg_c);
        id_m_add(id_m, segment_ind);

        /* raise an error if the user tries to unmap segment 0 */
        if (segment_ind == 0) {
                RAISE(Faulty_Unmap);
        }
        
        /* retrieve the segment to be unmapped */
        UArray_T unmap_seg = segment_at(mem, segment_ind);
        
        /* raise an error if the user tries to unmap a not mapped segment */
        if (unmap_seg == NULL) {
                RAISE(Faulty_Unmap);
        }
        
        /* frees segment to be unmapped and places NULL in its place */
        UArray_free(&unmap_seg);
        segment_put(mem, segment_ind, NULL);
}


/****************************** output ******************************
 *
 * Writes value at the register with the given index to the I/O device
 *
 * Inputs:
 *              int reg_c: register index to retrieve the value from
 *              UArray_T reg: UArray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register UArray to not be null and for the input value to be 
 *              in the range 0-255
 * Notes:
 *              will CRE if reg is null
 *              will URE with IO_Bounds if the output is not in bounds
 *
 *****************************************************************************/
void output(int reg_c, UArray_T reg)
{
        assert(reg != NULL);
        
        /* Retrieve value from registers */
        uint32_t val = *(uint32_t *)value_at(reg, reg_c);
        
        /* Check for value range and print if there is no problem */
        if (val > 255) {
                RAISE(IO_Bounds);
        }
        
        printf("%c", val);
}

/****************************** input ******************************
 *
 * Expects input from the I/O device, and when the input arrives, loads it in 
 * the register with the index reg_c.  If the end of input has been signaled,
 * then the register with index reg_c is loaded with a full 32-bit word in
 * which every bit is 1.
 *
 * Inputs:
 *              int reg_c: register index to place the input at
 *              UArray_T reg: uarray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register uarray to not be null and for the input value to be 
 *              in the range 0-255
 * Notes:
 *              will CRE if reg is null
 *              will URE with IO_Bounds if the input is not in bounds
 *
 *****************************************************************************/
void input(int reg_c, UArray_T reg)
{
        assert(reg != NULL);

        /* Get input */
        int c;
        c = getc(stdin);

        /* Retrieve pointer to the register to place the input at */
        uint32_t *ind = (uint32_t *)value_at(reg, reg_c);

        /* Check if EOF is signaled or if input is in bounds or place the value
        in the register */
        if (c == EOF) {
                uint32_t zero = (uint32_t)0;
                uint32_t max_val = ~zero;
                *ind = max_val;
        } else if (c > 255 || c < 0) {
                RAISE(IO_Bounds);
        } else {
                *ind = c;
        }
}

/****************************** load_program ******************************
 *
 * Duplicates segment with the index as the value reg_b stores, replacing
 * segment zero. The program counter is then set to the value reg_c stores,
 * jumping to the instruction at the given index.
 *
 * Inputs:
 *              int reg_b: segment index to replace segment zero
 *              int reg_c: program counter to jump to a word in a segment
 *              Seq_T mem: sequence of UArrays of words representing memory
 *              UArray_T reg: uarray representing 8 32-bit registers
 * Return:
 *              int representing new program counter value
 * Expects:
 *              register and mem to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
int load_program(int reg_b, int reg_c, Seq_T mem, UArray_T reg)
{
        /* checks for requirements */
        assert(mem != NULL);
        assert(reg != NULL);
        
        /* gets index of segment to replace segment 0 */
        uint32_t value_b = *(uint32_t *)value_at(reg, reg_b);
        
        /* if segment to replace segment 0 is not segment 0 itself: */
        if (value_b != 0) {
                /* segment at 0 is unmapped and freed */
                UArray_T unmap_seg = segment_at(mem, 0);                
                
                UArray_free(&unmap_seg);
                segment_put(mem, 0, NULL);

                /* duplicates the segmnt to be put at segment 0 */
                UArray_T dup = segment_at(mem, value_b);
                if (dup == NULL) {
                        RAISE(Segment_Unmapped);
                }
                int num_words = get_length_seg(dup);

                /* initiates a new segment with number of words defined */
                UArray_T new_seg = UArray_new(num_words, sizeof(uint32_t));

                /* adds words from the original segment to the duplicate */
                for (int i = 0; i < num_words; i++) {
                        *(uint32_t *)word_at(new_seg, i) = 
                                                  *(uint32_t *)word_at(dup, i);
                }
                
                /* segment is duplicated and put into segment 0 */
                segment_put(mem, 0, new_seg);
        }

        /* new program counter is returned */
        return *(uint32_t *)value_at(reg, reg_c);
}

/****************************** load_value ******************************
 *
 * Updates register with the index reg_a such that it holds a 32-bit word 
 * representing value
 *
 * Inputs:
 *              int reg_a: register index to place the value at
 *              int val: the value to move into the register
 *              UArray_T reg: uarray representing 8 32-bit registers
 * Return:
 *              n/a
 * Expects:
 *              register uarray to not be null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void load_value(int reg_a, int val, UArray_T reg)
{
        assert(reg != NULL);
        
        uint32_t *ind = (uint32_t *)value_at(reg, reg_a);
        *ind = (uint32_t)val;
}

#undef HINT