#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PROGRAM_MEM_BYTES 30000

enum TOKEN {
  TOK_NEXT, // >
  TOK_PREV, // <
  TOK_INC, // +
  TOK_DEC, // -
  TOK_PRINT, // .
  TOK_INPUT, // ,
  TOK_JMP_F, // [
  TOK_JMP_B, // ]
  TOK_NULL, // a-z, A-Z, 1-9...
  TOK_WS, // ' '
};

#ifdef verbose
#define INC_DYN_ARRAY(ARR, SIZE, IDX, TYPE)								\
  if (IDX >= SIZE - 1) {												\
	unsigned int new_ ## ARR ## _size = SIZE * 2;						\
    TYPE* new_ ## ARR = realloc(ARR, new_ ## ARR ## _size * sizeof(TYPE)); \
	printf("Resizing %s memory to %d bytes\n", #ARR, new_ ## ARR ## _size); \
	if (!new_ ## ARR) {													\
	  printf("Unable to resize %s memory!", #ARR);						\
	  prog_exit(1);														\
	}																	\
	memset(new_ ## ARR + SIZE, 0, SIZE);								\
    SIZE = new_ ## ARR ## _size;										\
    ARR = new_ ## ARR;													\
  }																		\
  IDX ++;
#else
#define INC_DYN_ARRAY(ARR, SIZE, IDX, TYPE)								\
  if (IDX >= SIZE - 1) {												\
	unsigned int new_ ## ARR ## _size = SIZE * 2;						\
    TYPE* new_ ## ARR = realloc(ARR, new_ ## ARR ## _size * sizeof(TYPE)); \
	if (!new_ ## ARR) {													\
	  printf("Unable to resize %s memory!", #ARR);						\
	  prog_exit(1);														\
	}																	\
	memset(new_ ## ARR + SIZE, 0, SIZE);								\
    SIZE = new_ ## ARR ## _size;										\
    ARR = new_ ## ARR;													\
  }																		\
  IDX ++;
#endif


#ifdef largecells
#define CELL_TYPE uint16_t
const CELL_TYPE max_cell = UINT16_MAX;
#else
#define CELL_TYPE uint8_t
const CELL_TYPE max_cell = UINT8_MAX;
#endif

void prog_exit(int status);

char* prog;
enum TOKEN* instructions;
#ifdef growablemem
CELL_TYPE* prog_mem;
#endif
#ifdef verbose
CELL_TYPE* output;
#endif
CELL_TYPE* input_buf;

int main (int argc, char *argv[]) {
  #ifdef largecells
  printf("WARNING: %s was compiled with large (16 bit) cells, normal brainf**k programs may not work\n", argv[0]);
  #endif

  if (!argv[1]) {
    printf("Please pass a brainf**k file");
    prog_exit(1);
  }
  FILE *bf = fopen(argv[1], "r");
  if (!bf) {
    printf("Unable to open file %s", argv[1]);
    prog_exit(1);
  }
  
  unsigned int prog_malloc = 1;
  unsigned int prog_size = 0;
  prog = malloc(prog_malloc * sizeof(char));
  char prog_char;
  while (!feof(bf)) {
    prog_char = fgetc(bf);

    if (prog_char == -1) break;
    
    prog[prog_size] = prog_char;

	INC_DYN_ARRAY(prog, prog_malloc, prog_size, char);
  }

  fclose(bf);

  #ifdef growablemem
  unsigned int prog_mem_size = 1;
  prog_mem = calloc(prog_mem_size, sizeof(CELL_TYPE));
  #else
  CELL_TYPE prog_mem[PROGRAM_MEM_BYTES] = {0};
  #endif

  unsigned int prog_ptr = 0;


  instructions = malloc(prog_size * sizeof(enum TOKEN));

  int matching_brackets = 0;

  for (int i = 0; i < prog_size; i++) {
    const CELL_TYPE c = prog[i];
    switch (c) {
    case '>':
      instructions[i] = TOK_NEXT;
      break;
    case '<':
      instructions[i] = TOK_PREV;
      break;
    case '+':
      instructions[i] = TOK_INC;
      break;
    case '-':
      instructions[i] = TOK_DEC;
      break;
    case '.':
      instructions[i] = TOK_PRINT;
      break;
    case ',':
      instructions[i] = TOK_INPUT;
      break;
    case '[':
      instructions[i] = TOK_JMP_F;
      matching_brackets ++;
      break;
    case ']':
      instructions[i] = TOK_JMP_B;
      matching_brackets --;
      break;
    case '\b':
    case ' ':
    case '\n':
    case '\t':
      instructions[i] = TOK_WS;
      break;
    default:
      instructions[i] = TOK_NULL;
      break;
    }
  }

  #ifdef verbose
  unsigned int output_pos = 0;
  unsigned int output_size = 1;
  output = malloc(output_size * sizeof(CELL_TYPE));
  #endif

  if (matching_brackets != 0) {
    printf("Incorrect number of brackets\n");
    prog_exit(1);
  }

  for (int i = 0; i < prog_size; i++) {
    const enum TOKEN* inst = &instructions[i];
    CELL_TYPE* curr_ptr = &prog_mem[prog_ptr];
    #ifdef verbose
    printf("%d %d: %d, %c\n", i, prog_ptr, *curr_ptr, *curr_ptr);
    #endif
    switch (*inst) {
    case TOK_NEXT:
	  #ifdef growablemem
	  INC_DYN_ARRAY(prog_mem, prog_mem_size, prog_ptr, CELL_TYPE);
	  #else
      if  (prog_ptr < PROGRAM_MEM_BYTES - 1)
		prog_ptr ++;
      else {
        #ifndef nowraparound
		prog_ptr = 0;
        #else
		printf("Stack overflow at %d\n", i);
	    prog_exit(1);
        #endif
      }
	  #endif
      break;
    case TOK_PREV:
      if (prog_ptr > 0)
		prog_ptr --;
      else {
        #ifndef nowraparound
		prog_ptr = PROGRAM_MEM_BYTES - 1;
        #else
		printf("Stack underflow at %d\n", i);
		prog_exit(1);
        #endif
      }
      break;
    case TOK_INC:
      if (*curr_ptr < max_cell) {
		(*curr_ptr) ++;
      }
      else {
        #ifndef nowraparound
		(*curr_ptr) = 0;
        #else
		printf("Integer overflow at %d\n", i);
	    prog_exit(1);
        #endif
      }
      break;
    case TOK_DEC:
      if (*curr_ptr > 0)
		(*curr_ptr) --;
      else {
        #ifndef nowraparound
		(*curr_ptr) = max_cell;
        #else
		printf("Integer underflow at %d\n", i);
		prog_exit(1);
        #endif
      }
      break;
    case TOK_PRINT:
      #ifdef verbose
	  output[output_pos] = *curr_ptr;
	  INC_DYN_ARRAY(output, output_size, output_pos, CELL_TYPE)
      #else
      printf("%c", *curr_ptr);
      #endif
      break;
    case TOK_INPUT:
      scanf("%c",curr_ptr);
	  break;
    case TOK_JMP_F:
      if (*curr_ptr == 0) {
		int num_brackets = 1;
		while (num_brackets != 0) {
		  i++;
		  if (instructions[i] == TOK_JMP_F)
			num_brackets ++;
		  if (instructions[i] == TOK_JMP_B)
			num_brackets --;
		}
      }
      break;
    case TOK_JMP_B:
      if (*curr_ptr != 0) {
		int num_brackets = 1;
		while (num_brackets != 0) {
		  i--;
		  if (instructions[i] == TOK_JMP_B)
			num_brackets ++;
		  if (instructions[i] == TOK_JMP_F)
			num_brackets --;
		}
      }
      break;
    case TOK_WS:
      break;
    case TOK_NULL:
      #ifdef verbose
      printf("Encounted non-bf char (%c, %d) may be part of a comment\n", *inst, *inst);
      #endif
      break;
    default:
      printf("FATAL: Encounted unknown instruction (possible invalid memory): %d, %c\n", *inst, *inst);
      #ifdef verbose
      printf("\nPROGRAM OUTPUT:\n");
      for (int i = 0; i < output_pos; i++) {
		printf("%c", output[i]);
      }
	  #endif
      prog_exit(1);
	}
  }

  #ifdef verbose
  printf("\nMEMORY STATE:\n");
  for (int i = 0; i < PROGRAM_MEM_BYTES; i++)
    if (prog_mem[i] != 0)
      if (prog_mem[i] != '\n')
		printf("{%d} = %c (%d)\n", i, prog_mem[i], prog_mem[i]);
      else // dont print new lines
		printf("{%d} =   (%d)\n", i, prog_mem[i]);

  printf("pointer = %d\n", prog_ptr);

  printf("\nPROGRAM OUTPUT:\n");
  for (int i = 0; i < output_pos; i++) {
    printf("%c", output[i]);
  }
  #endif

  prog_exit(0);
}

void prog_exit(int status) {
  free(prog);
  free(instructions);
  free(input_buf);
  #ifdef growablemem
  free(prog_mem);
  #endif
  #ifdef verbose
  free(output);
  #endif
  exit(status);
}
