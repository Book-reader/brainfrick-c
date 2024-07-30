#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PROGRAM_MEM_BYTES 30000

enum TOKEN_TYPE {
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
  TOK_NONE,
};

typedef struct {
  enum TOKEN_TYPE type;
  int repeats;
} Token;

#ifdef verbose
#define INC_DYN_ARRAY(ARR, SIZE, IDX, TYPE, AMOUNT)						\
  while (IDX + AMOUNT >= SIZE) {										\
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
  IDX += AMOUNT;
#else
#define INC_DYN_ARRAY(ARR, SIZE, IDX, TYPE, AMOUNT)						\
  while (IDX + AMOUNT >= SIZE - AMOUNT) {								\
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
  IDX += AMOUNT;
#endif

#ifdef largecells
#define CELL_TYPE uint16_t
const CELL_TYPE max_cell = UINT16_MAX;
#else
#define CELL_TYPE uint8_t
const CELL_TYPE max_cell = UINT8_MAX;
#endif

void prog_exit(int status);
CELL_TYPE* get_prog_mem(long idx);
void change_prog_ptr(long* idx, Token* inst, short dir);

char* prog;
enum TOKEN_TYPE* instructions;
#ifdef growablemem
CELL_TYPE* prog_mem;
CELL_TYPE* prog_mem_neg;
unsigned int prog_mem_size;
unsigned int prog_mem_neg_size;
#else
CELL_TYPE prog_mem[PROGRAM_MEM_BYTES];
#endif
#ifdef verbose
CELL_TYPE* output;
#endif
CELL_TYPE* input_buf;
Token* inst_comp;

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

	INC_DYN_ARRAY(prog, prog_malloc, prog_size, char, 1);
  }

  fclose(bf);

  instructions = malloc(prog_size * sizeof(enum TOKEN_TYPE));

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
  free(prog);
  if (matching_brackets != 0) {
    printf("Incorrect number of brackets\n");
    prog_exit(1);
  }


  int inst_comp_size = 1;
  int inst_comp_idx = 0;
  inst_comp = malloc(inst_comp_size * sizeof(Token));

  for (int i = 0; i < prog_size; i++) {
	Token tmp_token = {instructions[i], 0};
	switch (tmp_token.type) {
	case TOK_NEXT:
	case TOK_PREV:
	case TOK_INC:
	case TOK_DEC:
	  while (1) {
		if (instructions[i] != tmp_token.type) {
		  i --;
		  break;
		}
		i++;
		tmp_token.repeats ++;
	  }

	  inst_comp[inst_comp_idx] = tmp_token;
	  #ifdef verbose
	  printf("type: %d, repeats: %d, idx: %d\n", tmp_token.type, tmp_token.repeats, i);
	  #endif
	  INC_DYN_ARRAY(inst_comp, inst_comp_size, inst_comp_idx, Token, 1);

	  break;
	case TOK_PRINT:
	case TOK_INPUT:
	case TOK_JMP_F:
	case TOK_JMP_B:
	  inst_comp[inst_comp_idx] = tmp_token;
	  #ifdef verbose
	  printf("type: %d, repeats: %d, idx: %d\n", tmp_token.type, tmp_token.repeats, i);
	  #endif
	  INC_DYN_ARRAY(inst_comp, inst_comp_size, inst_comp_idx, Token, 1);
	  break;
	default:
	  #ifdef verbose
	  printf("Ignored token of type: %d\n", tmp_token.type);
	  #endif
	  break;
	}
  }
  #ifdef verbose
  printf("Before: %d, %d\n", inst_comp_size, inst_comp_idx);
  #endif
  inst_comp_size = inst_comp_idx;
  #ifdef verbose
  printf("After: %d, %d\n", inst_comp_size, inst_comp_idx);
  #endif

  #ifdef verbose
  unsigned int output_pos = 0;
  unsigned int output_size = 1;
  output = malloc(output_size * sizeof(CELL_TYPE));
  #endif

  #ifdef growablemem
  prog_mem_size = 1;
  prog_mem = calloc(prog_mem_size, sizeof(CELL_TYPE));
  prog_mem_neg_size = 1;
  prog_mem_neg = calloc(prog_mem_neg_size, sizeof(CELL_TYPE));
  #endif

  long prog_ptr = 0;

  for (int i = 0; i < inst_comp_size; i++) {
    Token* inst = &inst_comp[i];
    CELL_TYPE* curr_ptr = get_prog_mem(prog_ptr);
    #ifdef verbose
	printf("Prog_ptr: %d\n", prog_ptr);
    printf("%d %d: %d, %c\n", i, prog_ptr, *curr_ptr, *curr_ptr);
    #endif
    switch (inst->type) {
    case TOK_NEXT:
	  #ifdef growablemem
	  change_prog_ptr(&prog_ptr, inst, 1);
	  #else
	  prog_ptr = (prog_ptr + inst->repeats) % PROGRAM_MEM_BYTES;
	  #endif
      break;
    case TOK_PREV:
	  change_prog_ptr(&prog_ptr, inst, -1);
      break;
    case TOK_INC:
	  #ifdef verbose
	  printf("Increasing %d by %d\n", *curr_ptr, inst->repeats);
	  #endif
	  (*curr_ptr) = ((*curr_ptr) + inst->repeats) % (max_cell + 1);
      #ifdef verbose
	  printf("Result: %d\n", *curr_ptr);
	  #endif
      break;
    case TOK_DEC:
	  #ifdef verbose
	  printf("Decreasing %d by %d\n", *curr_ptr, inst->repeats);
	  #endif
	  (*curr_ptr) = ((*curr_ptr) - inst->repeats) % max_cell;
	  #ifdef verbose
	  printf("Result: %d\n", *curr_ptr);
	  #endif
      break;
    case TOK_PRINT:
      #ifdef verbose
	  output[output_pos] = *curr_ptr;
	  INC_DYN_ARRAY(output, output_size, output_pos, CELL_TYPE, 1)
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
		  if (inst_comp[i].type == TOK_JMP_F)
			num_brackets ++;
		  if (inst_comp[i].type == TOK_JMP_B)
			num_brackets --;
		}
		#ifdef verbose
		printf("Idx: %d\n", i);
		#endif
      }
      break;
    case TOK_JMP_B:
      if (*curr_ptr != 0) {
		int num_brackets = 1;
		while (num_brackets != 0) {
		  i--;
		  if (inst_comp[i].type == TOK_JMP_B)
			num_brackets ++;
		  if (inst_comp[i].type == TOK_JMP_F)
			num_brackets --;
		}
      }
      break;
    default:
      printf("FATAL: Encounted unknown instruction (possible invalid memory): %d, %c\n", inst->type, inst->type);
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
  #ifdef growablemem
  // Hacky workaround, why dont the variables by themselves work?
  int l_min = -prog_mem_neg_size;
  int l_max = prog_mem_size;
  for (int i = l_min; i < l_max; i++) {
  #else
  for (int i = 0; i < PROGRAM_MEM_BYTES; i++) {
  #endif
    CELL_TYPE j = *get_prog_mem(i);
	if (j != 0)
      if (j != '\n')
		printf("{%d} = %c (%d)\n", i, j, j);
      else // dont print new lines
		printf("{%d} =   (%d)\n", i, j);
  }
  printf("pointer = %d\n", prog_ptr);

  printf("\nPROGRAM OUTPUT:\n");
  for (int i = 0; i < output_pos; i++) {
    printf("%c", output[i]);
  }
  #endif

  prog_exit(0);
}

CELL_TYPE* get_prog_mem(long idx) {
  #ifdef growablemem
  if (idx >= 0) {
	return &prog_mem[idx];
  }
  else {
	return &prog_mem_neg[-idx];
  }
  #else
  return &prog_mem[idx];
  #endif
}

void change_prog_ptr(long* idx, Token* inst, short dir) {
  #ifdef growablemem
  #ifdef verbose
  printf("Num: %d\n", inst->repeats);
  printf("Idx: %d, REP: %d\n", *idx, inst->repeats * dir);
  #endif
  if (*idx + (inst->repeats * dir) >= 0) {
	INC_DYN_ARRAY(prog_mem, prog_mem_size, (*idx), CELL_TYPE, (inst->repeats * dir));
  }
  else {
	long idx_ = 0;
	#ifdef verbose
	printf("Idx_: %d, %d\n", idx_, inst->repeats);
	#endif
	INC_DYN_ARRAY(prog_mem_neg, prog_mem_neg_size, idx_, CELL_TYPE, ((*idx + (inst->repeats * dir)) * dir));
	#ifdef verbose
	printf("Idx2: %d, REP: %d, %d\n", *idx, idx_, idx_*dir);
	#endif
	(*idx) = idx_ * dir;
  }
  #else
  (*idx) += inst->repeats * dir;
  #endif
}

void prog_exit(int status) {
  free(instructions);
  free(input_buf);
  #ifdef growablemem
  free(prog_mem);
  free(prog_mem_neg);
  #endif
  #ifdef verbose
  free(output);
  #endif
  free(inst_comp);
  exit(status);
}
