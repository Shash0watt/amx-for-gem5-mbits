#include <stdio.h>
#include <unistd.h>
#include <immintrin.h> // Required for __m256d and AVX functions

#include <gem5/m5ops.h> // for annotation in gem5

// complile by running 'make' in terminal


uint32_t get_float_bits(float f) {
    union {
        float f;
        uint32_t i;
    } converter;
    
    converter.f = f;
    return converter.i;
}

int main(void) {
    
    // 1. Setup input data (four 64-bit doubles = 256 bits)
    float test_float0 = 3.14f;
    float test_float1 = 3.14f;
    uint32_t test_data0 = get_float_bits(test_float0);
    uint32_t test_data1 = get_float_bits(test_float1);

    // 2. Load data into AVX types (using m5ops, and eventually actually)
    m5_work_begin(0,0);
    // note: this will use XMM0 and XMM1 to emulaet YMM0
    m5_avx_load(2,test_data0); // NOW is AMX TILE LOAD (MAKE A NEW FILE FOR THIS)
    // note: this will use XMM2 and XMM3 to emulate YMM1
    m5_avx_load(4,test_data1);

    // 3. Inline assembly
    /* Creating raw bytes for: vaddps %ymm4, %ymm2, %ymm0 // YMM2 + YMM4 = YMM0
     Byte breakdown of (VEX Encoding):

     * 0xC5 - 2-byte VEX prefix

     [R][VVVV][L][PP]
        - IF R == 0: add 8 to destination register (YMM0 means R=1)
        - VVVV is the inverted src1 register number. 
          YMM2 is 0010 -> bitwise inverted -> 1101
        - IF L = 1 then YMM regs ELSE XMM regs
        - PP / precision
            00, usually PS (packed single, multiple 32-bit floats)
            01, usually PD (packed double, multiple 64-bit floats)
            10, usually SS (scalar single, only using the first 32-bits) 
            11, usually SD (scalar double, only using the first 64-bits)

     1 (R) | 1101 (VVVV=YMM2) | 1 (L)  | 00 (PP=PS)
     * 0xEC - Payload byte (1110 1100)

     * 0x58 - ADDPS Opcode

     [Mode (2 bits)] [Reg (3 bits)] [Reg/Mem (3 bits)]
      11 (accs regs)| 000 (YMM 0)  | 100 (YMM 4) => 11 000 100
     * 0xC4 - ModR/M byte 

     */

     // note: the gem5 decoder is hardcoded to blindly strip VEX prefixes and fall 
     // back to whatever legacy opcode happens to share the same base byte.
     // ie this add will become 0xEC, 0x58, 0xC4 
     // (FAKE FIX IMPLEMENTED IN GEM5!)
    asm volatile(
        ".byte 0xC5, 0xEC, 0x58, 0xC4\n" // YMM0 = YMM2 + YMM4 (VADDPS)
    );

    // 4. Store the result back to memory (using SSE
    // Since YMM0 is just XMM0 and XMM1
    float result[8] = {0};

    asm volatile (
        "movups %%xmm0, %0\n" // Extract lower 128 bits (4 floats)
        "movups %%xmm1, %1\n" // Extract upper 128 bits (4 floats)
        : "=m" (result[0]), "=m" (result[4])
        : // no inputs
        : "memory"
    );

    m5_work_end(0,0);



    // m5_work_begin(0,1);
    // 5. Print the results
    printf("--- AVX VADDPS RESULT (YMM0) ---\n");
    printf("Lower 128-bits (XMM0): %f, %f, %f, %f\n", result[0], result[1], result[2], result[3]);
    printf("Upper 128-bits (XMM1): %f, %f, %f, %f\n", result[4], result[5], result[6], result[7]);
    // m5_work_end(0,0);
    return 0;
}
