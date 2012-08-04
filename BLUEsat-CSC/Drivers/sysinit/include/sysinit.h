/**
 *  \file sysinit.h
 *
 *  \brief System initialisation
 */

#ifdef APPLICATION_H_
    #error "Applications should access drivers via services!"
#endif

#ifndef SYSINIT_H_
#define SYSINIT_H_

void sys_init(void);

#endif /* SYSINIT_H_ */
