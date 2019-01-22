/**
 * @file transaction-queue.h
 * @desc A circular queue for transaction descriptors
 * @author Samuel Dewan
 * @date 2019-01-03
 * Last Author:
 * Last Edited On:
 */

#ifndef transaction_queue_h
#define transaction_queue_h

struct transaction_queue_t {
    struct transaction_t {
        /** Transaction type specific state. */
        void *state;

        /** The identifier for this transaction. */
        uint8_t transaction_id;

        /** Flag which is set if this transaction slot is initilized. */
        uint8_t valid:1;
        /** Flag which is set if the transaction is currently in progress. */
        uint8_t active:1;
        /** Flag which is set when the transaction is complete. */
        uint8_t done:1;
    } *buffer;

    /** Number of elements in the queue. */
    uint16_t length;

    /** The last transaction in the queue to have been active. */
    uint16_t head;
    /** The next transaction ID to be assigned. */
    uint8_t next_id;
};


/**
 *  Initilize a transaction queue.
 *
 *  @param queue The queue to initialize.
 *  @param buffer The underlying buffer for the queue.
 *  @param length The number of entries in the queue.
 *  @param state_buffer The buffer for state information.
 *  @param state_length The number of bytes in each state object.
 */
static inline void init_transaction_queue(struct transaction_queue_t *queue,
                                          struct transaction_t *buffer,
                                          uint16_t length, void *state_buffer,
                                          uint8_t state_length)
{
    queue->buffer = buffer;
    queue->length = length;
    
    queue->head = 0;
    queue->next_id = 0;
    
    for (int i = 0; i < length; i++) {
        queue->buffer[i].state = (void*)((uint8_t*)state_buffer +
                                         (state_length * i));
        queue->buffer[i].valid = 0;
    }
}

/**
 *  Find the transaction in a queue with a given ID.
 *
 *  @param queue Queue which should be searched.
 *  @param id The id to be searched for.
 *
 *  @return The first entry in the queue with the given ID or NULL if no such
 *          entry exists.
 */
static inline struct transaction_t *transaction_queue_get(
                                        struct transaction_queue_t *queue,
                                        uint8_t id)
{
    for (uint8_t i = 0; i < queue->length; i++) {
        if (queue->buffer[i].valid && queue->buffer[i].transaction_id == id) {
            return queue->buffer + i;
        }
    }
    return NULL;
}

/**
 *  Find the next empty place in an SPI transaction queue.
 *
 *  @param queue The queue which should be searched.
 *
 *  @return The first place in the queue which is not populated or NULL if the
 *          queue is full;
 */
static inline struct transaction_t *transaction_queue_get_free(
                                            struct transaction_queue_t *queue)
{
    uint8_t i = queue->head + 1;
    do {
        if (!queue->buffer[i].valid) {
            return queue->buffer + i;
        }
        i = (i + 1) % queue->length;
    } while (i != queue->head + 1);

    return NULL;
}

/**
 *  Find the next transaction to be started and update the head.
 *
 *  @param queue The queue which should be searched.
 *
 *  @return The next transaction to be started or NULL if there are no pending
 *          transactions.
 */
static inline struct transaction_t *transaction_queue_next(
                                            struct transaction_queue_t *queue)
{
    uint8_t i = queue->head + 1;
    do {
        if (queue->buffer[i].valid && !queue->buffer[i].active
                                   && !queue->buffer[i].done) {
            queue->head = i;
            return queue->buffer + i;
        }
        i = (i + 1) % queue->length;
    } while (i != queue->head + 1);
    
    return NULL;
}

/**
 *  Check if the transaction at the head of a queue is active.
 *
 *  @param queue The queue which should be checked.
 *
 *  @return 1 if the transaction is active, 0 otherwise.
 */
static inline uint8_t transaction_queue_head_active(
                                            struct transaction_queue_t *queue)
{
    return queue->buffer[queue->head].active;
}

/**
 *  Get the current active transaction from a queue.
 *
 *  @param queue The queue for which the active transaction should be returned.
 *
 *  @return The active transaction from the queue or NULL if there is no active
 *          active transaction.
 */
static inline void *transaction_queue_get_active(
                                            struct transaction_queue_t *queue)
{
    if (transaction_queue_head_active(queue)) {
        return queue->buffer + queue->head;
    } else {
        return NULL;
    }
}

/**
 *  Find the next free transaction and initilize it.
 *
 *  @param queue The queue which should be searched.
 *
 *  @return The initialized transaction or NULL if the queue is full.
 */
static inline struct transaction_t *transaction_queue_add(
                                            struct transaction_queue_t *queue)
{
    struct transaction_t *t = transaction_queue_get_free(queue);
    
    if (t == NULL) {
        return NULL;
    }
    
    t->active = 0;
    t->done = 0;
    t->transaction_id = queue->next_id;
    queue->next_id++;
    
    return t;
}

/**
 *  Mark a transaction as valid.
 *
 *  @param trans The transaction to be marked as valid.
 */
static inline void transaction_queue_set_valid(struct transaction_t *trans)
{
    trans->valid = 1;
}

/**
 *  Mark a transaction as invalid so that it can be reused.
 *
 *  @param trans The transaction to be marked invalid.
 *
 *  @return 0 on success, 1 if the transaction is in progress and therefor
 *          cannot be invalidaed, 2 if the transaction is NULL.
 */
static inline uint8_t transaction_queue_invalidate(struct transaction_t *trans)
{
    if (trans == NULL) {
        return 2;
    } else if (trans->active) {
        return 1;
    }
    
    trans->valid = 0;
    return 0;
}

/**
 *  Determine if a transaction is done.
 *
 *  @param trans The transaction to be checked.
 *
 *  @return 1 if the transaction is done or does not exist, 0 otherwise.
 */
static inline uint8_t transaction_queue_is_done(struct transaction_t *trans)
{
    if (trans == NULL) {
        return 1;
    }
    return trans->done;
}

/**
 *  Mark a transaction as having been completed.
 *
 *  @param trans The transaction to be marked as complete.
 */
static inline void transaction_queue_set_done(struct transaction_t *trans)
{
    trans->done = 1;
}


#endif /* transaction_queue_h */
