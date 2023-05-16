#include "types.h"
#include "pagesim.h"
#include "mmu.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);

pfn_t last_evicted = 0;         // Initialize a static variable to keep track of the victim frame

/**
 * --------------------------------- PROBLEM 7 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Make a free frame for the system to use. You call the select_victim_frame() method
 * to identify an "available" frame in the system (already given). You will need to 
 * check to see if this frame is already mapped in, and if it is, you need to evict it.
 * 
 * @return victim_pfn: a phycial frame number to a free frame be used by other functions.
 * 
 * HINTS:
 *      - When evicting pages, remember what you checked for to trigger page faults 
 *      in mem_access
 *      - If the page table entry has been written to before, you will need to use
 *      swap_write() to save the contents to the swap queue.
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void) {
    // Select a victim frame using select_victim_frame()
    pfn_t victim = select_victim_frame();
    
    // Check if the victim frame is already mapped
    if (frame_table[victim].mapped) {
        // Get the page table entry for the victim frame
        pfn_t victim_ptbr = (frame_table[victim].process)->saved_ptbr;
        vpn_t victim_vpn = frame_table[victim].vpn;

        pte_t *pg_entry = (pte_t *) (mem + victim_ptbr * PAGE_SIZE) + victim_vpn;
        
        // If the page table entry is dirty
        if (pg_entry->dirty) {
            // Write contents to swap queue using swap_write()
            swap_write(pg_entry, mem + pg_entry->pfn * PAGE_SIZE);
            stats.writebacks++;
        }
        pg_entry->valid = 0;                    // Set valid flag in page table entry to 0        
        frame_table[victim].mapped = 0;   // Set mapped flag in frame table entry to 0
    }
    
    return victim;
}


/**
 * --------------------------------- PROBLEM 9 --------------------------------------
 * Checkout PDF section 10, 10.1, and 10.2 for this problem
 * 
 * MAKE SURE YOU COMPLETE daemon_update() BELOW BEFORE DOING APPROX_LRU
 * 
 * Finds a free physical frame. If none are available, uses either a
 * randomized, approximate LRU, or clocksweep algorithm to find a used frame for
 * eviction.
 * 
 * @return The physical frame number of a victim frame.
 * 
 * HINTS: 
 *      - Use the global variables MEM_SIZE and PAGE_SIZE to calculate
 *      the number of entries in the frame table.
 *      - Use the global last_evicted to keep track of the pointer into the frame table
 * ----------------------------------------------------------------------------------
 */
pfn_t select_victim_frame() {

    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped) {
            return i;
        }
    }

    // RANDOM implemented for you.
    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t unprotected_found = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                unprotected_found = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (unprotected_found < NUM_FRAMES) {
            return unprotected_found;
        }

    }  else if (replacement == CLOCKSWEEP) {
        // TODO: Implement the clocksweep page replacement algorithm here
        for (pfn_t i = last_evicted; i < NUM_FRAMES; i++) { // Iterate through all frames in the frame table starting from the current value of victim
            if (!frame_table[i].protected) { // If the frame is not protected
                if (frame_table[i].referenced) frame_table[i].referenced = 0; // If the frame has been referenced, reset its reference bit to 0
                else {
                    // Update the value of victim to be equal to i + 1 if we are not at the last frame in memory yet. Otherwise, set it equal to zero.
                    if (i != NUM_FRAMES - 1) last_evicted = i + 1;
                    else last_evicted = 0;
                    
                    return i; // Return this frame as a victim for replacement
                }
            }
            if (i == NUM_FRAMES - 1) i = 0; // If we have reached the last frame in memory, wrap around back to zero
        }

    }   else if (replacement == APPROX_LRU) {
        // TODO: Implement the Approximate LRU algorithm here (COMPLETE daemon_update() BEFORE DOING THIS)
        uint8_t min = 255; // Initialize a variable to keep track of the minimum reference counter value
        for (pfn_t i = 0; i < NUM_FRAMES; i++) { // Iterate through all frames in the frame table
            if (!frame_table[i].protected && frame_table[i].ref_count < min) { 
                // If the frame is not protected and If the frame's reference counter value is less than the current minimum value
                min = frame_table[i].ref_count; // Update the minimum value to be this frame's reference counter value
                last_evicted = i; // Set this frame as a potential victim for replacement
            }
        }
        return last_evicted;
    }

    /* If every frame is protected, give up. This should never happen
       on the traces we provide you. */
    panic("System ran out of memory\n");
    exit(1);
}

/**
 * --------------------------------- PROBLEM 9 --------------------------------------
 * Checkout PDF for this problem
 * 
 * Updates the associated variables for the Approximate LRU,
 * called every time the simulator daemon wakes up.
 * 
 * ----------------------------------------------------------------------------------
*/
void daemon_update(void) {
    // TODO
    for (size_t i = 0; i < NUM_FRAMES; i++) {
        frame_table[i].ref_count >>= 1; // Right-shift the reference counter value by 1 bit
        if (frame_table[i].referenced) { // If the frame has been referenced
            frame_table[i].ref_count |= (1 << 7); // Set the MSB of the reference counter
            frame_table[i].referenced = 0; // Reset its reference bit to 0
        }
    }
}

