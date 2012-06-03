/**
 *  \file gpio.h
 *
 *  \brief THE GPIO driver, used to read and write to gpio pins
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-05-12 16:38:58 +1100 (Sat, 12 May 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note TODO consider adding function for enable gpio interrupt
 *        TODO consider adding function for gpio pull up and pull down resistor
 */

#ifdef APPLICATION_H_
	#error "Applications should access drivers via services!"
#endif

#ifndef GPIO_H_
#define GPIO_H_


#define INPUT 0
#define OUTPUT 1

/**
 * \brief Set the output of a GPIO
 *
 * \param[in] portNO port number
 * \param[in] pinNO pin number
 * \param[in] newValue the value to be set, 0 or 1
 *
 */
void setGPIO(unsigned char portNo, unsigned char pinNo, unsigned char newValue);

/**
 * \brief Read the value of a GPIO
 *
 * \param[in] portNO port number
 * \param[in] pinNO pin number
 *
 * \returns value at the GPIO
 */
int getGPIO(unsigned char portNo, unsigned char pinNo);

/**
 * \brief Set the direction of a GPIO to be input or output
 *
 * \param[in] portNO port number
 * \param[in] pinNO pin number
 * \param[in] direction the value to be set, 0 or 1
 *
 */
void setGPIOdir(unsigned char portNo, unsigned char pinNo, unsigned char direction);

/**
 * \brief Set the function of a GPIO
 *
 * \param[in] portNO port number
 * \param[in] pinNO pin number
 * \param[in] func, 00,01,10,11, the four functions indicated on datasheet
 *
 */
void set_Gpio_func(unsigned char portNo, unsigned char pinNo, unsigned char func);

/**
 * \brief Anything that is needed to set up GPIO
 *
 *
 */
void Gpio_Init(void);

#endif /* GPIO_H_*/
