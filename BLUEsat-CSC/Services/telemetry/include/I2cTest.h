 /**
 *  \file i2cTesh.h
 *
 *  \brief A small service to test the I2C communication and driver
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef I2CTEST_H_
#define I2CTEST_H_

/**
 * \brief Initialise I2C Test Service
 *
 * \param[in] uxPriority Priority for I2C Test Service.
 */
void vI2CTest_Init(unsigned portBASE_TYPE uxPriority);



#endif /* I2CTEST_H_ */
