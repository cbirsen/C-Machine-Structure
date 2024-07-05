/*
 *      emulator.c
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      December 3, 2023
 *      Optimized UM
 *
 *      This is the driver module for the um program. Expects a file with the
 *      .um extension to read and implement instructions using functions. 
 *      If the file cannot be opened properly or not supplied, returns
 *      with EXIT_FAILURE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* CS40 Libraries */
#include <bitpack.h>

/* Hanson Libraries */
#include <assert.h>
#include <stdbool.h>
#include <seq.h>
#include <mem.h>

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

/* Raised when the index of a word in a segment or register is out of bounds */
Except_T Word_Bounds = { "Index Out Of Bounds" };

/* Raised when the trying to unmap an unmapped segment or segment zero */
Except_T Faulty_Unmap = { "Refers to Unmapped Segment or Segment Zero" };

/* Helper Function Declarations */
static inline void initiate_program(FILE *fp);
static inline void word_interpreter(Seq_T mem, uint32_t *registers, Seq_T id_m);
static inline void conditional_move(int reg_a, int reg_b, int reg_c, uint32_t *reg);
static inline void segmented_load(int reg_a, int reg_b, int reg_c, Seq_T mem, uint32_t *reg);
static inline void segmented_store(int reg_a, int reg_b, int reg_c, Seq_T mem, uint32_t *reg);
static inline void addition(int reg_a, int reg_b, int reg_c, uint32_t *reg);
static inline void multiplication(int reg_a, int reg_b, int reg_c, uint32_t *reg);
static inline void division(int reg_a, int reg_b, int reg_c, uint32_t *reg);
static inline void bitwise_nand(int reg_a, int reg_b, int reg_c, uint32_t *reg);
static inline void map_segment(int reg_b, int reg_c, Seq_T mem, uint32_t *reg, Seq_T id_m);
static inline void unmap_segment(int reg_c, Seq_T mem, uint32_t *reg, Seq_T id_m);
static inline void output(int reg_c, uint32_t *reg);
static inline void input(int reg_c, uint32_t *reg);
static inline int load_program(int reg_b, int reg_c, Seq_T mem, uint32_t *reg);
static inline void load_value(int reg_a, int val, uint32_t *reg);
static inline FILE *open_or_die(int argc, char *argv[]);

/* segment struct representing each segment in memory and its length */
typedef struct segment_T *segment_T;
struct segment_T {
        int length;
        uint32_t *seg_arr;
};

int main(int argc, char *argv[]) {
        /* open input file */
        FILE *fp = open_or_die(argc, argv);
        if (fp == NULL) {
                return EXIT_FAILURE;
        }

        /* Calls operations module to implement the instructions */
        initiate_program(fp);

        fclose(fp);
        return 0;
}

static inline FILE *open_or_die(int argc, char *argv[]) {   
        /* check if correct number of arguments is supplied */
        if (argc != 2) {
                fprintf(stderr, "Incorrect number of arguments.\n");
                return NULL;
        }

        FILE *fp; 
        fp = fopen(argv[1], "r");
        
        return fp;
}

static inline void initiate_program(FILE *fp)
{
        assert(fp != NULL);

        /* a temp sequence is initiated to record words as they are read */
        Seq_T prog = Seq_new(10000);
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

        /* initialize segment 0 and transfer words from temp prog sequence to 
         * seg_0 segment_T struct
         */
        int proglen = Seq_length(prog);
        segment_T seg_0;
        NEW(seg_0);
        assert(seg_0 != NULL);

        seg_0->length = proglen;
        seg_0->seg_arr = malloc(proglen * sizeof(uint32_t));

        uint32_t *word_ptr = NULL;
        for (int i = 0; i < proglen; i++) {
                word_ptr = Seq_remlo(prog);
                seg_0->seg_arr[i] = *word_ptr;
                free(word_ptr);
        }

        /* memory is initialized with segment 0 filled with program */
        Seq_T mem = Seq_new(10000);
        assert(mem != NULL);

        Seq_addhi(mem, seg_0);

        /* registers and id manager for program is set up */
        Seq_T id_m = Seq_new(10000);
        assert(id_m != NULL);

        uint32_t *registers = malloc(8 * sizeof(uint32_t));
        assert(registers != NULL);

        /* initializes each register with value 0 */
        for (int i = 0; i < 8; i++) { 
                registers[i] = (uint32_t)0;
        }

        /* all program info is sent to helper function */
        word_interpreter(mem, registers, id_m);
        
        /* frees all allocated space */
        
        /* gets number of segments */
        int lengthmem = Seq_length(mem);
        
        /* goes through each segment */
        for (int i = 0; i < lengthmem; i++) {
                segment_T segment = Seq_remlo(mem);
                /* frees non-null segments */
                if (segment->length != 0) {
                        free(segment->seg_arr);  
                }
                FREE(segment);
        }

        Seq_free(&mem);
        free(registers);
        Seq_free(&prog);
        Seq_free(&id_m);
}

static inline void word_interpreter(Seq_T mem, uint32_t *reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);

        /* get the segment 0 for program */
        if (0 >= Seq_length(mem)) {
                RAISE(Word_Bounds);
        }

        segment_T program_T = Seq_get(mem, 0);
        uint32_t *program = program_T->seg_arr;

        /* get number of words in running program, segment 0 */
        uint32_t length_p = program_T->length;

        /* halt being called at the end of the program to be checked later */
        bool halt_called = false;

        /* go through each word in segment 0 to execute instructions */
        for (uint32_t p_counter = 0; p_counter < length_p; p_counter++) {
                uint32_t curr_word = program[p_counter];

                /* creates the mask */
                uint32_t mask = ~((uint32_t)0);
                mask = mask >> 28 << 28;

                /* gets the word with only the extracted value field */
                int op = (curr_word & mask) >> 28;
                mask = ~((uint32_t)0);

                int ra = -1;
                int rb = -1;
                int rc = -1;
                int value = -1;

                /* populates related fields depending on operation code */
                if (op == 13) {
                        mask = mask >> 4;
                        
                        uint32_t raandvalue = curr_word & mask;
                        mask = ~((uint32_t)0);

                        mask = mask >> 29 << 25;
                        ra = (raandvalue & mask) >> 25;
                        
                        mask = ~((uint32_t)0);
                        mask = mask >> 7;
                        value = raandvalue & mask;
                } else {
                        mask = mask >> 23;

                        uint32_t rarbrc = curr_word & mask;
                        mask = ~((uint32_t)0);
                        mask = mask >> 29;
                        rc = rarbrc & mask;

                        mask = mask << 3;
                        rb = (rarbrc & mask) >> 3;

                        mask = mask << 3;
                        ra = (rarbrc & mask) >> 6;
                }

                /* calls function for opcode in the instruction */
                switch (op) {
                        case 0:
                                conditional_move(ra, rb, rc, reg);
                                break;
                        case 1:
                                segmented_load(ra, rb, rc, mem, reg);
                                break;
                        case 2:
                                segmented_store(ra, rb, rc, mem, reg);
                                break;
                        case 3:
                                addition(ra, rb, rc, reg);
                                break;
                        case 4:
                                multiplication(ra, rb, rc, reg);
                                break;
                        case 5:
                                division(ra, rb, rc, reg);
                                break;
                        case 6:
                                bitwise_nand(ra, rb, rc, reg);
                                break;
                        case 7:
                                /* jumps to the end of the program */
                                p_counter = length_p;
                                halt_called = true;
                                break;
                        case 8:
                                map_segment(rb, rc, mem, reg, id_m);
                                break;
                        case 9:
                                unmap_segment(rc, mem, reg, id_m);
                                break;
                        case 10:
                                output(rc, reg);
                                break;
                        case 11:
                                input(rc, reg);
                                break;
                        case 12:
                                /* update counter, program, and prog length */
                                p_counter = load_program(rb, rc, mem, reg) - 1;
                                program_T = Seq_get(mem, 0);
                                program = program_T->seg_arr;
                                length_p = program_T->length;
                                break;
                        case 13:
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

static inline void conditional_move(int reg_a, int reg_b, int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        if (reg[reg_c] != 0) {
                reg[reg_a] = reg[reg_b];
        }
}

static inline void segmented_load(int reg_a, int reg_b, int reg_c, Seq_T mem, uint32_t *reg)
{
        assert(mem != NULL);
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        /* Retrieve the segment and word indices */
        uint32_t segment_id = reg[reg_b];
        int word_id = reg[reg_c];

        /* check for requirements */
        if (segment_id >= (uint32_t)Seq_length(mem)) {
                RAISE(Word_Bounds);
        }

        segment_T seg_T = Seq_get(mem, segment_id);
        uint32_t *seg = seg_T->seg_arr;

        /* check for requirements */
        if (seg == NULL) {
                RAISE(Segment_Unmapped);
        }
        if (word_id >= seg_T->length) {
                RAISE(Word_Bounds);
        }

        reg[reg_a] = seg[word_id];
}

static inline void segmented_store(int reg_a, int reg_b, int reg_c, Seq_T mem, uint32_t *reg)
{
        assert(mem != NULL);
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }
        
        /* Retrieve the segment and word indices */
        uint32_t segment_ind = reg[reg_a];
        int word_ind = reg[reg_b];

        /* check for requirements */
        if (segment_ind >= (uint32_t)Seq_length(mem)) {
                RAISE(Word_Bounds);
        }

        segment_T seg_T = Seq_get(mem, segment_ind);
        uint32_t *seg = seg_T->seg_arr;

        /* check for requirements */
        if (seg == NULL) {
                RAISE(Segment_Unmapped);
        }

        if (word_ind >= seg_T->length) {
                RAISE(Word_Bounds);
        }

        /* Update value in memory to be equal to the word at reg index reg_c */
        seg[word_ind] =  reg[reg_c];
}

static inline void addition(int reg_a, int reg_b, int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        reg[reg_a] = reg[reg_b] + reg[reg_c];
}

static inline void multiplication(int reg_a, int reg_b, int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        reg[reg_a] = reg[reg_b] * reg[reg_c];
}

static inline void division(int reg_a, int reg_b, int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        uint32_t right = reg[reg_c];
        
        /* URE if divisor is 0 */
        if (right == 0) {
                RAISE(Division_Zero);
        }
        
        reg[reg_a] = reg[reg_b] / right;
}

static inline void bitwise_nand(int reg_a, int reg_b, int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0 || reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }

        reg[reg_a] = ~(reg[reg_b] & reg[reg_c]);
}

static inline void map_segment(int reg_b, int reg_c, Seq_T mem, uint32_t *reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0) {
                RAISE(Word_Bounds);
        }

        /* initialize the new segment with 0's as words */
        int num_words = reg[reg_c];
        uint32_t *new_seg = malloc(num_words * sizeof(uint32_t));
        assert(new_seg != NULL);

        for (int i = 0; i < num_words; i++) {
                new_seg[i] = (uint32_t)0;
        }
        
        uint32_t segment_id = 0;
        
        /* if there is no space available in the current memory, add segment to
         * the end. if there is, place the segment to an unmapped location 
         * retrieved from id_m. update new segment_id
         */
        if (Seq_length(id_m) == 0) {
                segment_T new_seg_T;
                NEW(new_seg_T);
                assert(new_seg_T != NULL);
                new_seg_T->seg_arr = new_seg;
                new_seg_T->length = num_words;

                Seq_addhi(mem, new_seg_T);
                segment_id = Seq_length(mem) - 1;
        } else {
                segment_id = (uint32_t)(uintptr_t)Seq_remlo(id_m);
                segment_T seg_exist = Seq_get(mem, segment_id);
                seg_exist->seg_arr = new_seg;
                seg_exist->length = num_words;
        }

        /* store new segment id in register with index reg_b */
        reg[reg_b] = (uint32_t)segment_id;
}

static inline void unmap_segment(int reg_c, Seq_T mem, uint32_t *reg, Seq_T id_m)
{
        assert(mem != NULL);
        assert(reg != NULL);
        assert(id_m != NULL);

        if (reg_c >=  8 || reg_c < 0) {
                RAISE(Word_Bounds);
        }
        
        /* retrieve the segment id to be unmapped and add it to free ids seq */
        uint32_t segment_ind = reg[reg_c];
        Seq_addlo(id_m, (void *)(uintptr_t)segment_ind);

        /* raise an error if the user tries to unmap segment 0 */
        if (segment_ind == 0) {
                RAISE(Faulty_Unmap);
        }
        
        /* retrieve the segment to be unmapped */
        if (segment_ind >= (uint32_t)Seq_length(mem)) {
                RAISE(Word_Bounds);
        }
        
        segment_T unmap_seg_T = Seq_get(mem, segment_ind);
        uint32_t *unmap_seg = unmap_seg_T->seg_arr;
        
        /* raise an error if the user tries to unmap a not mapped segment */
        if (unmap_seg == NULL) {
                RAISE(Faulty_Unmap);
        }
        
        /* frees segment to be unmapped and places NULL in its place */
        free(unmap_seg);
        unmap_seg = NULL;
        unmap_seg_T->length = 0;
}

static inline void output(int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0) {
                RAISE(Word_Bounds);
        }
        
        /* Retrieve value from register */
        uint32_t val = reg[reg_c];
        
        /* Check for value range and print if there is no problem */
        if (val > 255) {
                RAISE(IO_Bounds);
        }
        
        printf("%c", val);
}

static inline void input(int reg_c, uint32_t *reg)
{
        assert(reg != NULL);

        /* Get input */
        int c;
        c = getc(stdin);

        if (reg_c >=  8 || reg_c < 0) {
                RAISE(Word_Bounds);
        }

        /* Check if EOF is signaled or if input is in bounds or place the value
        in the register */
        if (c == EOF) {
                uint32_t zero = (uint32_t)0;
                uint32_t max_val = ~zero;
                reg[reg_c] = max_val;
        } else if (c > 255 || c < 0) {
                RAISE(IO_Bounds);
        } else {
                reg[reg_c] = c;
        }
}

static inline int load_program(int reg_b, int reg_c, Seq_T mem, uint32_t *reg)
{
        /* checks for requirements */
        assert(mem != NULL);
        assert(reg != NULL);

        if (reg_c >=  8 || reg_c < 0 || reg_b >=  8 || reg_b < 0) {
                RAISE(Word_Bounds);
        }
        
        /* gets index of segment to replace segment 0 */
        uint32_t value_b = reg[reg_b];
        
        /* if segment to replace segment 0 is not segment 0 itself: */
        if (value_b != 0) {
                /* segment at 0 is unmapped and freed */
                segment_T unmap_seg_T = Seq_get(mem, 0);     
                uint32_t *unmap_seg = unmap_seg_T->seg_arr;

                free(unmap_seg);
                unmap_seg_T->length = 0;

                /* duplicates the segmnt to be put at segment 0 */
                if (value_b >= (uint32_t)Seq_length(mem)) {
                        RAISE(Word_Bounds);
                }
                
                segment_T dup_T = Seq_get(mem, value_b);     
                uint32_t *dup = dup_T->seg_arr;
                if (dup == NULL) {
                        RAISE(Segment_Unmapped);
                }
                
                int num_words = dup_T->length;

                /* initiates a new segment with number of words defined */
                uint32_t *new_seg = malloc(num_words * sizeof(uint32_t));

                /* adds words from the original segment to the duplicate */
                for (int i = 0; i < num_words; i++) {
                        new_seg[i] = dup[i];
                }
                
                /* segment is duplicated and put into segment 0 */
                unmap_seg_T->seg_arr = new_seg;
                unmap_seg_T->length = num_words;
        }

        /* new program counter is returned */
        return reg[reg_c];
}

static inline void load_value(int reg_a, int val, uint32_t *reg)
{
        assert(reg != NULL);

        if (reg_a >=  8 || reg_a < 0) {
                RAISE(Word_Bounds);
        }
        
        reg[reg_a] = (uint32_t)val;
}