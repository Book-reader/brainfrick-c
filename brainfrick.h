void prog_exit(int status);
CELL_TYPE* get_prog_mem(long idx);
void change_prog_ptr(long* idx, Token* inst, short dir);

#ifdef largecells
#define CELL_TYPE uint16_t
const CELL_TYPE max_cell = UINT16_MAX;
#else
#define CELL_TYPE uint8_t
const CELL_TYPE max_cell = UINT8_MAX;
#endif

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

#define PROGRAM_MEM_BYTES 30000
