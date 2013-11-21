/* 
 * File:   parameter_storage.h
 * Author: Matt
 *
 * Created on 15 November 2013, 21:26
 */

#ifndef PARAMETER_STORAGE_H
#define	PARAMETER_STORAGE_H


#ifdef	__cplusplus
extern "C" {
#endif


typedef enum
    {
    STORAGE_FLAG_ALL = 1,
    STORAGE_FLAG_LOAD_AT_STARTUP = 2,
    STORAGE_FLAG_LOAD_AT_REBOOT = 4,
    STORAGE_FLAG_SAVE_AFTER_CAL = 8,
    STORAGE_FLAG_STORE_CALIB = 16,
    } storage_flags_e;


    // Initialize storage
    void init_parameter_storage(void);

    // save parameters to storage
    void save_parameters(uint16_t flags);

    // load parameters from storage
    void load_parameters(uint16_t flags);


#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETER_STORAGE_H */
