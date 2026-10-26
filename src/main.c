#include <coreinit/memdefaultheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/memheap.h>
#include <coreinit/screen.h>
#include <nsysnet/_socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <vpad/input.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

static void init_logging(void) {
    socket_lib_init();
    WHBLogModuleInit();
    WHBLogCafeInit();
    WHBLogUdpInit();
}

static void print_line(const char *str, uint32_t row) {
    OSScreenPutFontEx(SCREEN_TV, 0, row, str);
    OSScreenPutFontEx(SCREEN_DRC, 0, row, str);
}

static void draw_menu(void) {
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);
    print_line("Wii U Crash Tester v0.1.0 by JerrySM64", 0);
    print_line("Press a button to trigger a crash:", 2);
    print_line("A: null write (DSI)", 3);
    print_line("X: jump to 0 (ISI)", 4);
    print_line("Y: trap (Program)", 5);
    print_line("B: misaligned stfdu (Alignment)", 6);
    print_line("R: illegal instruction (Program)", 7);
    print_line("L: crash with messed up stack on r1", 8);
    print_line("Press HOME to quit without crashing.", 10);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

#define CRASH_ADDR 0x90000000u

__attribute__((noinline)) static void crash_dsi(void) {
    *(volatile uint32_t *) CRASH_ADDR = 0;
}

__attribute__((noinline)) static void crash_isi(void) {
    ((void (*)(void)) CRASH_ADDR)();
}

__attribute__((noinline)) static void crash_program_trap(void) {
    __asm__ volatile("trap");
}

// 0xDEADC0DE is "stfdu f21, -0x3F22(r13)" triggering an alignment exception
__attribute__((noinline)) static void crash_alignment_stfdu(void) {
    __asm__ volatile(".int 0xDEADC0DE\n");
}

// 0x00000000 is an illegal instruction - self-explanatory
__attribute__((noinline)) static void crash_program_illegal(void) {
    __asm__ volatile(".int 0x00000000\n");
}

// Crash with a messed up stack on r1
__attribute__((noinline)) static void crash_broken_stack(void) {
    __asm__ volatile(
        "lis   %%r1, 0x9000\n"
        "ori   %%r1, %%r1, 0xFF00\n"
        "li    %%r3, 0\n"
        "stw   %%r3, 0(%%r1)\n"
        :
        :
        : "r3", "memory");
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    init_logging();
    WHBLogPrintf("crashtest: main entered");

    WHBProcInit();
    WHBLogPrintf("crashtest: proc init done");

    OSScreenInit();
    uint32_t tvBufSize  = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    WHBLogPrintf("crashtest: screen init done, tv %08X drc %08X", tvBufSize, drcBufSize);

    MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    void *tvBuf        = MEMAllocFromFrmHeapEx(heap, tvBufSize, 4);
    void *drcBuf       = MEMAllocFromFrmHeapEx(heap, drcBufSize, 4);
    WHBLogPrintf("crashtest: buffers %08X %08X", (uint32_t) (uintptr_t) tvBuf,
                 (uint32_t) (uintptr_t) drcBuf);
    if (tvBuf == NULL || drcBuf == NULL) {
        WHBLogPrintf("crashtest: buffer alloc failed");
        WHBProcShutdown();
        return 1;
    }

    OSScreenSetBufferEx(SCREEN_TV, tvBuf);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuf);
    OSScreenEnableEx(SCREEN_TV, true);
    OSScreenEnableEx(SCREEN_DRC, true);
    WHBLogPrintf("crashtest: screen configured");

    VPADInit();
    WHBLogPrintf("crashtest: vpad init done");

    WHBLogPrintf("crashtest: ready");
    WHBLogPrintf("crash_dsi             at %08X", (uint32_t) (uintptr_t) &crash_dsi);
    WHBLogPrintf("crash_isi             at %08X", (uint32_t) (uintptr_t) &crash_isi);
    WHBLogPrintf("crash_program_trap    at %08X", (uint32_t) (uintptr_t) &crash_program_trap);
    WHBLogPrintf("crash_alignment_stfdu at %08X", (uint32_t) (uintptr_t) &crash_alignment_stfdu);
    WHBLogPrintf("crash_program_illegal at %08X", (uint32_t) (uintptr_t) &crash_program_illegal);
    WHBLogPrintf("crash_broken_stack    at %08X", (uint32_t) (uintptr_t) &crash_broken_stack);

    VPADStatus status;
    VPADReadError error;
    memset(&status, 0, sizeof(status));

    while (WHBProcIsRunning()) {
        VPADRead(VPAD_CHAN_0, &status, 1, &error);
        if (error == VPAD_READ_SUCCESS) {
            uint32_t triggered = status.trigger;
            if (triggered & VPAD_BUTTON_A) {
                WHBLogPrintf("triggering DSI");
                crash_dsi();
            } else if (triggered & VPAD_BUTTON_X) {
                WHBLogPrintf("triggering ISI");
                crash_isi();
            } else if (triggered & VPAD_BUTTON_Y) {
                WHBLogPrintf("triggering Program (trap)");
                crash_program_trap();
            } else if (triggered & VPAD_BUTTON_B) {
                WHBLogPrintf("triggering Alignment (misaligned stfdu)");
                crash_alignment_stfdu();
            } else if (triggered & VPAD_BUTTON_R) {
                WHBLogPrintf("triggering Program (illegal instruction)");
                crash_program_illegal();
            } else if (triggered & VPAD_BUTTON_L) {
                WHBLogPrintf("triggering broken-stack crash (guard test)");
                crash_broken_stack();
            }
        }
        draw_menu();
    }

    WHBLogPrintf("crashtest: exiting");
    WHBLogModuleDeinit();
    WHBLogCafeDeinit();
    WHBLogUdpDeinit();
    WHBProcShutdown();
    VPADShutdown();
    return 0;
}
