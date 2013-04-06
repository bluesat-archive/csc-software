#include "commsBuffer.h"
#include "debug.h"
// TaskToken globtoken;

/*
 *  Bit Stuffing Functions
 * ---------------------
 * */
UnivRetCode stuffBufMSBtoLSB (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   UnivRetCode result = URC_FAIL;
   char temp;
   buffer input;
   if (inputBuff==NULL || outputBuff == NULL ||input_size==0)
   {
         return result;
   }
   if (initBuffer(&input, inputBuff, input_size) == URC_FAIL)return result;
   while (bitPopMSBtoLSB (&input, &temp, sizeof (char))==URC_SUCCESS)
      {
         outputBuff->connectedOnes = (temp==0)?0: outputBuff->connectedOnes+1;
         if ( outputBuff->connectedOnes > PatternLimit)
            {
               outputBuff->connectedOnes = 1; // Take into account the 1 to be added after this if block
               if (bitPush (outputBuff, 0)== URC_FAIL)return result;
            }
         if (bitPush (outputBuff, temp)== URC_FAIL)return result;
      }
   return URC_SUCCESS;
}

UnivRetCode stuffBufLSBtoMSB (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   UnivRetCode result = URC_FAIL;
   char temp;
   buffer input;
   if (inputBuff==NULL || outputBuff == NULL ||input_size==0)
   {
         return result;
   }
   if (initBuffer(&input, inputBuff, input_size) == URC_FAIL)return result;
   while (bitPopLSBtoMSB (&input, &temp, sizeof (char))==URC_SUCCESS)
      {
         outputBuff->connectedOnes = (temp==0)?0: outputBuff->connectedOnes+1;
         if ( outputBuff->connectedOnes > PatternLimit)
            {
               outputBuff->connectedOnes = 1; // Take into account the 1 to be added after this if block
               if (bitPush (outputBuff, 0)== URC_FAIL)return result;
            }
         if (bitPush (outputBuff, temp)== URC_FAIL)return result;
      }
   return URC_SUCCESS;
}

UnivRetCode pushBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   UnivRetCode result = URC_FAIL;
   char temp;
   buffer input;
   if (inputBuff==NULL || outputBuff == NULL ||input_size==0)
   {
         return result;
   }
   if (initBuffer(&input, inputBuff, input_size) == URC_FAIL)return result;
   while (bitPopLSBtoMSB (&input, &temp, sizeof (char))==URC_SUCCESS)
      {
         if (bitPush (outputBuff, temp)== URC_FAIL)return result;
      }
   return URC_SUCCESS;
}

UnivRetCode initBuffer(buffer * input, char * buff, unsigned int size)
{
   if (input == NULL || buff == NULL ||size == 0) return URC_FAIL;
   input->buff = buff;
   input->buff_size = size;
   input->byte_pos = 0;
   input->index = 0;
   input->connectedOnes=0;
   return URC_SUCCESS;
}

UnivRetCode bitPopLSBtoMSB (buffer* buff, char * out, unsigned int size)
{
   UnivRetCode result = URC_FAIL;
   char temp  = 0;
   if (size == 0||out ==NULL ||buff==NULL)
      {
         return result;
      }
   if (buff->index>=buff->buff_size)
      {
         return result;
      }
   temp = buff->buff[buff->index];
   *out = (temp& (LSB_bit_mask<<buff->byte_pos++))?1:0;

   if (buff->byte_pos%8==0)
      {
         ++buff->index;
         buff->byte_pos = 0;
      }
   return URC_SUCCESS;
}

UnivRetCode bitPopMSBtoLSB (buffer* buff, char * out, unsigned int size)
{
   UnivRetCode result = URC_FAIL;
   char temp  = 0;
   if (size == 0||out ==NULL ||buff==NULL)
      {
         return result;
      }
   if (buff->index>=buff->buff_size)
      {
         return result;
      }
   temp = buff->buff[buff->index];
   *out = (temp& (MSB_bit_mask>>buff->byte_pos++))?1:0;

   if (buff->byte_pos%8==0)
      {
         ++buff->index;
         buff->byte_pos = 0;
      }
   return URC_SUCCESS;
}

UnivRetCode bitPush (buffer* buff, char in)
{
   UnivRetCode result = URC_FAIL;
   unsigned int  temp; // Use unsigned int due to left shifting later on
   if (buff==NULL)
      {
         return result;
      }
   if (buff->index>=buff->buff_size)
      {
         return result;
      }
   temp = (in == 0)?0:LSB_bit_mask;
   buff->buff[buff->index] = (buff->buff[buff->index]| (temp<<buff->byte_pos++));
   if (buff->byte_pos%8==0)
      {
         ++buff->index;
         buff->byte_pos = 0;
      }
   return URC_SUCCESS;
}
