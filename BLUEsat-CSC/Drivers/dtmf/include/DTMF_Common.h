/*!
 * \file DTMF_Common.h
 *
 * \brief Common header file containing the structures used by
 *        both the ISR and the DTMF Task
 *
 *  $Author: Colin Tan $
 *  \version 1.0
 *  $Date: 2010-02-07 02:01:06 +1100 (Sun, 07 Feb 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */


#ifndef DTMF_COMMON_H_
#define DTMF_COMMON_H_

/**
 * /brief Possible DTMF Decoder options
 */
#define MAXDECODERS 2

typedef enum
{
   decoder1   = 0,       /**< First Decoder*/
   decoder2   = 1        /**< Second Decoder*/
}Decoders;

/**
 * /brief Tones as they are decoded from the decoder
 * */
typedef enum _Tones
{
   Tone_1 = 1,
   Tone_2 = 2,
   Tone_3 = 3,
   Tone_4 = 4,
   Tone_5 = 5,
   Tone_6 = 6,
   Tone_7 = 7,
   Tone_8 = 8,
   Tone_9 = 9,
   Tone_0 = 10,
   Tone_STAR = 11,
   Tone_HASH = 12,
   Tone_A = 13,
   Tone_B = 14,
   Tone_C = 15,
   Tone_D = 16
}Tones;

/**
 * /brief Message carrier containing the DTMF tone decoded and
 *        the decoder which decoded the tone.
 */
typedef struct _DtmfTone
{
   Tones    tone:5;     /**< Tone received */
   Decoders decoder:3;  /**< Decoder received on */
}DtmfTone;



#endif /* DTMF_COMMON_H_ */
