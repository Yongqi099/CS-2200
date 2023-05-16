#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* The frame table pointer. You will set this up in system_init. */
fte_t *frame_table;

/**
 * --------------------------------- PROBLEM 2 --------------------------------------
 * Checkout PDF sections 4 for this problem
 * 
 * In this problem, you will initialize the frame_table pointer. The frame table will
 * be located at physical address 0 in our simulated memory. You should zero out the 
 * entries in the frame table, in case for any reason physical memory is not clean.
 * 
 * HINTS:
 *      - mem: Simulated physical memory already allocated for you.
 *      - PAGE_SIZE: The size of one page
 * ----------------------------------------------------------------------------------
 */
void system_init(void) {
    // TODO: initialize the frame_table pointer.
    memset(mem, 0, PAGE_SIZE);      //pass in mem pointer, at frame 0, with size PAGE_SIZE
    frame_table = (fte_t * ) mem;   //sets the frame_table pointer to point to the start of the mem array
    frame_table->protected = 1;   //mark the first entry as protected
}

/**
 * --------------------------------- PROBLEM 5 --------------------------------------
 * Checkout PDF section 6 for this problem
 * 
 * Takes an input virtual address and performs a memory operation.
 * 
 * @param addr virtual address to be translated
 * @param access 'r' if the access is a read, 'w' if a write
 * @param data If the access is a write, one byte of data to written to our memory.
 *             Otherwise NULL for read accesses.
 * 
 * HINTS:
 *      - Remember that not all the entry in the process's page table are mapped in. 
 *      Check what in the pte_t struct signals that the entry is mapped in memory.
 * ----------------------------------------------------------------------------------
 */
uint8_t mem_access(vaddr_t addr, char access, uint8_t data) {
    // Get the page table entry for the given virtual address
    pte_t * pt_entry = (pte_t * )(mem + PTBR * PAGE_SIZE) + vaddr_vpn(addr);

    // Check if the page table entry is valid
    if (!pt_entry->valid) page_fault(addr);     // If not valid, call the page_fault function to handle it

    // Mark the frame as referenced in the frame table
    int index = pt_entry->pfn;
    fte_t *frame = &frame_table[index];
    frame->referenced = 1;

    // Calculate the physical frame number by accessing the pfn field of the pt_entry structure
    int phys_frame_num = pt_entry->pfn;
    // Shift the physical frame number left by OFFSET_LEN bits to get the physical address of the frame
    paddr_t phys_frame_addr = (paddr_t)(phys_frame_num << OFFSET_LEN);
    // Combine the physical frame address and offset within the page to get the final physical address
    paddr_t phys_addr = phys_frame_addr | vaddr_offset(addr);


    if (access == 'w') {        // if write, write data, if read just return data at address
        pt_entry->dirty = 1;    // entry is modified while in main memory
        mem[phys_addr] = data;   // Write data to memory at physical address
    }
    
    stats.accesses++;           // Increment stats.accesses counter
    return mem[phys_addr];      // both r and w need to return data
}
