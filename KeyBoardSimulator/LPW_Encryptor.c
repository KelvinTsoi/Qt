#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "LPW_Encryptor.h"

#define DIGIT_NUM 6
#define LOOP_NUM 4

const int S_box_Dictionary[10] = {6, 8, 4, 9, 7, 2, 1, 0, 3, 5};

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int TransIntToIntArray(int SrcValue, int* DstValueArray)
{
    int i = 0;
    int s = 1, d, m, b;
    if (SrcValue >= pow(10, DIGIT_NUM + 1))
    {
        return -1;
    }
    else
    {
        for (i = 0; i < DIGIT_NUM; i++)
        {
            DstValueArray[DIGIT_NUM - 1 - i] = SrcValue % 10;
            SrcValue = SrcValue / 10;
        }
        return 0;
    }
}

int S_box_Displace(int* numArray)
{
    int i;
    for (i = 0; i < DIGIT_NUM; i++)
    {
        int a = numArray[i];
        numArray[i] = S_box_Dictionary[a];
    }
}

int S_box_Restore(int* numArray)
{
    int i;
    int j;
    for (i = 0; i < DIGIT_NUM; i++)
    {
        j = 0;
        while (numArray[i] != S_box_Dictionary[j] && j < 10)
        {
            j++;
        }
        numArray[i] = j;
    }
}

int ContentReplace(int* numArray)
{
    int tmpNum;
    if (DIGIT_NUM >= 6)
    {
        tmpNum = numArray[1];
        numArray[1] = numArray[3];
        numArray[3] = tmpNum;

        tmpNum = numArray[2];
        numArray[2] = numArray[5];
        numArray[5] = tmpNum;
    }
}

int ContentRestore(int* numArray)
{
    int tmpNum;
    if (DIGIT_NUM >= 6)
    {
        tmpNum = numArray[3];
        numArray[3] = numArray[1];
        numArray[1] = tmpNum;

        tmpNum = numArray[5];
        numArray[5] = numArray[2];
        numArray[2] = tmpNum;
    }
}

int EncryptedByKey(int* DataArray, int* KeyArray)
{
    int i;
    int d;
    for (i = 0; i < DIGIT_NUM; i++)
    {
        d = DataArray[i];
        DataArray[i] = (d + KeyArray[i]) % 10;
    }
}

int DecryptedByKey(int* DataArray, int* KeyArray)
{
    int i;
    int d;
    for (i = 0; i < DIGIT_NUM; i++)
    {
        if (DataArray[i] < KeyArray[i])
        {
            d = DataArray[i] + 10;
        }
        else
        {
            d = DataArray[i];
        }
        DataArray[i] = d - KeyArray[i];
    }
}

int LPW_Encrypt(int plaintext, int key)
{
    int i;
    int tmp = 0;

    int dataArray[DIGIT_NUM] = {0x00};
    int keyArray[DIGIT_NUM] = {0x00};
    if (TransIntToIntArray(plaintext, dataArray) != 0)
    {
        return -1;
    }
    if (TransIntToIntArray(key, keyArray) != 0)
    {
        return -2;
    }

    //printf("dataIn is %d \r\n",dataIn);

    //for(i = 0; i < DIGIT_NUM; i++) {
    //	printf("%d ",dataArray[i]);
    //}

    //printf("\r\nKey is %d \r\n",Key);

    //for(i = 0; i < DIGIT_NUM; i++) {
    //	printf("%d ",keyArray[i]);
    //}

    for (i = 0; i < LOOP_NUM; i++)
    {
        S_box_Displace(dataArray);
        ContentReplace(dataArray);
        EncryptedByKey(dataArray, keyArray);
    }

    //for(i = 0; i < DIGIT_NUM; i++) {
    //	printf("\r\n code is %d ",dataArray[i]);
    //}

    tmp += dataArray[0];
    for (i = 1; i <= DIGIT_NUM - 1; i++)
    {
        tmp *= 10;
        tmp += dataArray[i];
    }
    //*dataOut = tmp;

    return tmp;

    //printf("\r\n dataout is %d \r\n",*dataOut);
}

int LPW_Decrypt(int ciphertext, int key)
{
    int i;
    int tmp = 0;

    int dataArray[DIGIT_NUM] = {0x00};
    int keyArray[DIGIT_NUM] = {0x00};
    if (TransIntToIntArray(ciphertext, dataArray) != 0)
    {
        return -1;
    }
    if (TransIntToIntArray(key, keyArray) != 0)
    {
        return -2;
    }

    for (i = 0; i < LOOP_NUM; i++)
    {
        DecryptedByKey(dataArray, keyArray);
        ContentRestore(dataArray);
        S_box_Restore(dataArray);
    }

    tmp += dataArray[0];
    for (i = 1; i <= DIGIT_NUM - 1; i++)
    {
        tmp *= 10;
        tmp += dataArray[i];
    }
    //*dataOut = tmp;

    return tmp;
    //printf("\r\n dataout is %d \r\n",*dataOut);
}



