#ifndef STATS_H
#define STATS_H
#include "patches.h"

/**
 * Strategy for updating patch statistics
 */
typedef void (*stats_updater_t)(void** stats, const val_t z);

/**
 * Strategy for printing node stats
 */
typedef void (*stats_printer_t)(const void* stats);

/*
 * Gather patch statistics from an image, given a template
 *
 * @param pnoisy Noisy (input) image
 * @param patch_t template. This one is used to determine the patch in which each pixel occurs
 * @param M Alphabet size.
 * @param[out] tree This tree is populated with the patches that actually occur, their counts, and their center pixel histogram
 */
patch_node_t* gather_patch_stats(const image_t* pnoisy,
                                  const image_t* pctx,
                                  const template_t* ptpl,
                                  patch_mapper_t mapper,
                                  stats_updater_t update_stats,
                                  patch_node_t* ptree);


void* get_patch_stats(const patch_node_t* ptree, const patch_t* pctx);

/**
 * Print a patch tree with its counts
 */
void print_patch_stats(patch_node_t* pnode, char* prefix, stats_printer_t print_stats);

#endif