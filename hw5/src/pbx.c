/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include <sys/socket.h>
#include <semaphore.h>

// - maintain registry of connected clients
// - manage tu objs associated w clients
// - map each ext to associated tu obj
// - provide appropriate sync using semaphores
// - shutdown network connections to all registered clients using shutdown(2)
// - wait for all client service threads to unregister tus before returning

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */

typedef struct pbx {
    TU *tus[PBX_MAX_EXTENSIONS];
    sem_t mutex;
}PBX;


PBX *pbx_init() {
    PBX *pbx;
    pbx = malloc(sizeof(struct pbx));
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) { pbx->tus[i] = NULL; }
    sem_init(&pbx->mutex, 0, 1);
    debug("%s", "Initialized TU");
    return pbx;
}

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */

void pbx_shutdown(PBX *pbx) {
    sem_wait(&pbx->mutex);
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        if(pbx->tus[i] != NULL) {
            shutdown(tu_fileno(pbx->tus[i]), SHUT_RDWR);
            pbx_unregister(pbx, pbx->tus[i]);
        }
    }
    sem_post(&pbx->mutex);
    free(pbx);
}

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */

int pbx_register(PBX *pbx, TU *tu, int ext) {
    sem_wait(&pbx->mutex);
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        if (pbx->tus[i] == NULL) {
            pbx->tus[i] = tu;
            tu_set_extension(tu, ext);
            tu_ref(tu, "Registered the TU");
            sem_post(&pbx->mutex);
            return 0;
        }
    }
    sem_post(&pbx->mutex);
    return -1;
}

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */

int pbx_unregister(PBX *pbx, TU *tu) {
    sem_wait(&pbx->mutex);
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        if (pbx->tus[i] == tu) {
            pbx->tus[i] = NULL;
            tu_hangup(tu);
            tu_unref(tu, "Unregistered the TU.");
            sem_post(&pbx->mutex);
            return 0;
        }
    }
    sem_post(&pbx->mutex);
    return -1;
}

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */

int pbx_dial(PBX *pbx, TU *tu, int ext) {
    sem_wait(&pbx->mutex);
    for (int i = 0; i < PBX_MAX_EXTENSIONS; i++) {
        if (tu_extension(pbx->tus[i]) == ext) {
            tu_dial(tu, pbx->tus[i]);
            debug("%s %d", "Dialing TU #", ext);
            sem_post(&pbx->mutex);
            return 0;
        }
    }
    sem_post(&pbx->mutex);
    return -1;
}

