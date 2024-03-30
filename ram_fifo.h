/* $Id: ram_fifo.h $ */
/**
 *
 * \file
 * \brief  Implements an ringbuffer based RAM-FIFO
 * \author Andreas Voggeneder
 */

#ifndef RAM_FIFO_H
#define RAM_FIFO_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <errno.h>

/******************************************************************************/
/*------------------------- defines-------------------------------------------*/
/******************************************************************************/

// RAM_FIFO has to be a power of two
#define FIFO_BITS     10uL
#define FIFO_SIZE     (1uL<<FIFO_BITS)
#define FIFO_MASK     ((1uL<<FIFO_BITS)-1u)
#define FIFO_MASK_MSB (1uL<<FIFO_BITS)
//#define FIFO_MASK_MSB 0x10u

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef unsigned int uint32_t;
typedef struct {
    volatile uint32_t write_pointer;
    volatile uint32_t read_pointer;
    volatile char     fifo_buffer[FIFO_SIZE];
} t_ram_fifo;

extern sem_t fifo_mutex;

/******************************************************************************/
/*-------------------------Global Function Prototypes-------------------------*/
/******************************************************************************/

//void init_ram_fifo(void);
void init_fifo(volatile t_ram_fifo * const p_ram_fifo);
void fifo_destroy(volatile t_ram_fifo const * const p_ram_fifo);
char read_fifo_value(volatile t_ram_fifo * const p_ram_fifo);
bool write_fifo_value(volatile t_ram_fifo * const p_ram_fifo, const char value);
bool fifo_empty(volatile t_ram_fifo const * const p_ram_fifo);
bool fifo_full(volatile t_ram_fifo const * const p_ram_fifo);

/******************************************************************************/
/*-------------------------Inline Function Prototypes-------------------------*/
/******************************************************************************/


/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/

#endif
