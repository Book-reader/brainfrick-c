#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

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

int main (int argc, char *argv[]) {
  if (!argv[1]) {
    printf("Please pass a brainf**k file");
    return 1;
  }
  FILE *bf = fopen(argv[1], "r");
  if (!bf) {
    printf("Unable to open file %s", argv[1]);
    return 1;
  }
  
  unsigned int prog_malloc = 1;
  unsigned int prog_size = 0;
  char* prog = malloc(prog_malloc * sizeof(char));
  char prog_char;
  while (!feof(bf)) {
    if (prog_size >= prog_malloc) {
      prog_malloc *= 2;
      char* new_prog = realloc(prog, prog_malloc * sizeof(char));
      if (!new_prog) {
	printf("Unable to realloc file array!");
	free(prog);
	return 1;
      }
      prog = new_prog;
    }
    prog_char = fgetc(bf);

    if (prog_char == -1) break;
    
    prog[prog_size] = prog_char;
    prog_size++;
  }
  /* prog_size -= 1; */

  fclose(bf);
  
  char prog_mem[PROGRAM_MEM_BYTES] = {0};

  unsigned int prog_ptr = 0;


  enum TOKEN instructions[prog_size];

  int matching_brackets = 0;

  for (int i = 0; i < prog_size; i++) {
    const char c = prog[i];
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
  char* output = malloc(output_size * sizeof(char));
  #endif

  if (matching_brackets != 0) {
    printf("Incorrect number of brackets\n");
    return 1;
  }

  for (int i = 0; i < prog_size; i++) {
    const enum TOKEN* inst = &instructions[i];
    char* curr_ptr = &prog_mem[prog_ptr];
    #ifdef verbose
    printf("%d %d: %d, %c\n", i, prog_ptr, *curr_ptr, *curr_ptr);
    #endif
    switch (*inst) {
    case TOK_NEXT:
      if  (prog_ptr < PROGRAM_MEM_BYTES - 1)
	prog_ptr ++;
	else {
	  #ifdef wraparound
	  prog_ptr = 0;
	  #else
	  printf("Stack overflow at %d\n", i);
	  return 1;
	  #endif
	}
      break;
    case TOK_PREV:
      if (prog_ptr > 0)
	prog_ptr --;
      else {
	#ifdef wraparound
	prog_ptr = PROGRAM_MEM_BYTES - 1;
	#else
	printf("Stack underflow at %d\n", i);
	return 1;
	#endif
      }
      break;
    case TOK_INC:
      if (*curr_ptr < CHAR_MAX) {
	(*curr_ptr) ++;
      }
      else {
	#ifdef wraparound
	(*curr_ptr) = 0;
	#else
	printf("Integer overflow at %d\n", i);
	return 1;
	#endif
      }
      break;
    case TOK_DEC:
      if (*curr_ptr > 0)
	(*curr_ptr) --;
      else {
	#ifdef wraparound
	(*curr_ptr) = CHAR_MAX - 1;
	#else
	printf("Integer underflow at %d\n", i);
	return 1;
	#endif
      }
      /* (*curr_ptr) --; */
      break;
    case TOK_PRINT:
      #ifdef verbose
      if (output_pos >= output_size) {
	output_size *= 2;
	char* new_output = realloc(output, output_size * sizeof(char));
	printf("Resizing output buffer to %d chars\n", output_size);
	if (!new_output) {
	  printf("Unable to realloc output array!\n");
	  free(output);
	  return 1;
	}
	output = new_output;
      }
      output[output_pos] = *curr_ptr;
      output_pos++;
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
      printf("Encounted non-bf char (%c, %d) may be part of a comment\n", *inst, *inst);
      break;
    default:
      printf("FATAL: Encounted unknown instruction (possible invalid memory): %d, %c\n", *inst, *inst);
      #ifdef verbose
      printf("\nPROGRAM OUTPUT:\n");
      for (int i = 0; i < output_pos; i++) {
	printf("%c", output[i]);
      }
      free(output);
      #endif
       return 1;
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
  free(output);
  #endif

  free(prog);
  return 0;
}
