#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 6 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Page fault handler.
 * 
 * When the CPU encounters an invalid address mapping in a page table, it invokes the 
 * OS via this handler. Your job is to put a mapping in place so that the translation 
 * can succeed.
 * 
 * @param addr virtual address in the page that needs to be mapped into main memory.
 * 
 * HINTS:
 *      - You will need to use the global variable current_process when
 *      altering the frame table entry.
 *      - Use swap_exists() and swap_read() to update the data in the 
 *      frame as it is mapped in.
 * ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t addr) {
   // TODO: Get a new frame, then correctly update the page table and frame table

    vpn_t vpn = vaddr_vpn(addr);    // Get the virtual page number from the virtual address
    // Get the page table entry for the faulting virtual address
    pte_t *pt_entry = (pte_t *)(mem + PTBR * PAGE_SIZE) + vpn;
    pfn_t new_frame = free_frame();   // Get a new frame using free_frame()
    
    // Update the mapping from VPN to the new PFN in the current process’ page table
    pt_entry->pfn = new_frame;
    pt_entry->valid = 1;
    pt_entry->dirty = 0;
    
    // Check if there is an existing swap entry for the faulting page table entry and read in saved frame into new frame if it exists, otherwise clear new frame 
    uint8_t *swap_entry = mem + new_frame * PAGE_SIZE;   
    if (swap_exists(pt_entry)) swap_read(pt_entry, swap_entry);
    else memset(swap_entry, 0, PAGE_SIZE);

    // Update frame table entry flags, entry process and VPN fields
    fte_t *ft = frame_table + new_frame;
    ft->protected = 0;
    ft->mapped = 1;
    ft->referenced = 1;
    ft->process = current_process;
    ft->vpn = vpn;

   stats.page_faults++;        // Increment page faults counter
}

#pragma GCC diagnostic pop
