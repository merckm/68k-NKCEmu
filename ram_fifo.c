/* $Id: ram_fifo.c $ */

/**
 *
 * \file
 * \brief  Implements an ringbuffer based RAM-FIFO
 * \author Andreas Voggeneder
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

//#include <stddef.h>
//#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ram_fifo.h"

/******************************************************************************/
/*------------------------- defines-------------------------------------------*/
/******************************************************************************/
//#define USE_SEM   // uncomment if semaphores are needed -> when threads are used

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
#ifdef USE_SEM
sem_t fifo_mutex;
#endif

/******************************************************************************/
/*-------------------------private Function Prototypes -----------------------*/
/******************************************************************************/


/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
// FIFO is empty when ALL bits of both pointers matches

bool fifo_empty(volatile t_ram_fifo const * const p_ram_fifo)
{
    bool ret_value = false;
#ifdef USE_SEM
    sem_wait(&fifo_mutex);
#endif
    const uint32_t write_pointer = p_ram_fifo->write_pointer;
    if (p_ram_fifo->read_pointer == write_pointer) {
        ret_value = true;
    }
#ifdef USE_SEM
    sem_post(&fifo_mutex);
#endif
    return ret_value;
}

// FIFO is full when Pointer matches and msb bit of pointer does not match
bool fifo_full(volatile t_ram_fifo const * const p_ram_fifo)
{
    bool ret_value = false;
#ifdef USE_SEM
    sem_wait(&fifo_mutex);
#endif
    const uint32_t write_pointer = (uint32_t)p_ram_fifo->write_pointer ^ FIFO_MASK_MSB;
    if (p_ram_fifo->read_pointer == write_pointer) {
        ret_value = true;
    }
#ifdef USE_SEM
    sem_post(&fifo_mutex);
#endif
    return ret_value;
}

void fifo_destroy(volatile t_ram_fifo const * const p_ram_fifo) {
    (void)p_ram_fifo;
#ifdef USE_SEM
    sem_destroy(&fifo_mutex);
#endif
}

char read_fifo_value(volatile t_ram_fifo * const p_ram_fifo)
{
    //assert(!fifo_empty(p_ram_fifo));
    if (fifo_empty(p_ram_fifo)) {
        return 0;
    }
#ifdef USE_SEM
    sem_wait(&fifo_mutex);
#endif
    const uint32_t index = p_ram_fifo->read_pointer & FIFO_MASK;
    const char value = p_ram_fifo->fifo_buffer[index];
    p_ram_fifo->read_pointer = (p_ram_fifo->read_pointer + 1u) & (FIFO_MASK | FIFO_MASK_MSB);
#ifdef USE_SEM
    sem_post(&fifo_mutex);
#endif
    //printf("Read data 0x%X from FIFO\n", value);

    return value;
}

bool write_fifo_value(volatile t_ram_fifo * const p_ram_fifo, const char value)
{
    //assert(!fifo_full(p_ram_fifo));
    if (fifo_full(p_ram_fifo)){
        return true;
    }
#ifdef USE_SEM
    sem_wait(&fifo_mutex);
#endif
    const uint32_t wr_ptr           = p_ram_fifo->write_pointer & FIFO_MASK;
    p_ram_fifo->fifo_buffer[wr_ptr] = value;
    p_ram_fifo->write_pointer       = (p_ram_fifo->write_pointer + 1u) & (FIFO_MASK | FIFO_MASK_MSB);
#ifdef USE_SEM
    sem_post(&fifo_mutex);
#endif
    return false;
}

void init_fifo(volatile t_ram_fifo * const p_ram_fifo)
{
    memset((void*)p_ram_fifo->fifo_buffer, 0, sizeof(p_ram_fifo->fifo_buffer));
#ifdef USE_SEM
    sem_init(&fifo_mutex, 0, 1);
#endif
    p_ram_fifo->write_pointer = 0u;
    p_ram_fifo->read_pointer  = 0u;
}


