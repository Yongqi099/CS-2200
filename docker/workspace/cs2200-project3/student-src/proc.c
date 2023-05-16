#include "proc.h"
#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 3 --------------------------------------
 * Checkout PDF section 4 for this problem
 * 
 * This function gets called every time a new process is created.
 * You will need to allocate a frame for the process's page table using the
 * free_frame function. Then, you will need update both the frame table and
 * the process's PCB. 
 * 
 * @param proc pointer to process that is being initialized 
 * 
 * HINTS:
 *      - pcb_t: struct defined in pagesim.h that is a process's PCB.
 *      - You are not guaranteed that the memory returned by the free frame allocator
 *      is empty - an existing frame could have been evicted for our new page table.
 * ----------------------------------------------------------------------------------
 */
void proc_init(pcb_t *proc) {
    // TODO: initialize proc's page table.
    pfn_t page_table = free_frame();                     //allocate frame for process table
    
    // creates a pointer points to an entry in the frame table corresponding to the frame number stored in pageTable
    fte_t * frame = (frame_table + page_table);
    frame->protected = 1;
    frame->mapped = 1;
    frame->process = proc;

    memset(mem + page_table * PAGE_SIZE, 0, PAGE_SIZE);  //clear space for process
    
    //set the saved_ptbr field of the pcb_t struct pointed to by proc to the value of pageTable
    proc->saved_ptbr = page_table;
}

/**
 * --------------------------------- PROBLEM 4 --------------------------------------
 * Checkout PDF section 5 for this problem
 * 
 * Switches the currently running process to the process referenced by the proc 
 * argument.
 * 
 * Every process has its own page table, as you allocated in proc_init. You will
 * need to tell the processor to use the new process's page table.
 * 
 * @param proc pointer to process to become the currently running process.
 * 
 * HINTS:
 *      - Look at the global variables defined in pagesim.h. You may be interested in
 *      the definition of pcb_t as well.
 * ----------------------------------------------------------------------------------
 */
void context_switch(pcb_t *proc) {
    // TODO: update any global vars and proc's PCB to match the context_switch.
    PTBR = proc->saved_ptbr;
}

/**
 * --------------------------------- PROBLEM 8 --------------------------------------
 * Checkout PDF section 8 for this problem
 * 
 * When a process exits, you need to free any pages previously occupied by the
 * process.
 * 
 * HINTS:
 *      - If the process has swapped any pages to disk, you must call
 *      swap_free() using the page table entry pointer as a parameter.
 *      - If you free any protected pages, you must also clear their"protected" bits.
 * ----------------------------------------------------------------------------------
 */
void proc_cleanup(pcb_t *proc) {
    // TODO: Iterate the proc's page table and clean up each valid page

    // Iterate through the process's page table
    for (size_t i = 0; i < NUM_PAGES; i++) {
        // Get a pointer to the current page table entry
        pte_t *pte_ptr = (pte_t *)(mem + proc->saved_ptbr * PAGE_SIZE) + i;
        // If the page is valid, clear the valid and mapped bit
        if (pte_ptr->valid) {
            frame_table[pte_ptr->pfn].mapped = 0;            
            pte_ptr->valid = 0;
        }
        // If the page has been swapped to disk free the swap space associated with this page
        if (swap_exists(pte_ptr)) swap_free(pte_ptr);
    }

    // Get a pointer to the frame table entry for this process's page table, clear its bits
    fte_t *cur_fte = frame_table + proc->saved_ptbr;
    cur_fte->protected = 0;
    cur_fte->mapped = 0;   
}

#pragma GCC diagnostic pop
