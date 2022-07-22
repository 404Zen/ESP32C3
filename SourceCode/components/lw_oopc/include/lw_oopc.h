/********************************************************************************************************
* @file     lw_oopc.h
* 
* @brief    lw_oopc header file
* 
* @author   MISOO
* 
* @date     2022-06-29  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LW_OOPC_H__
#define __LW_OOPC_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

/* Exported defines ----------------------------------------------------------*/
#define CLASS(type) \
typedef struct type type;   \
struct type

/* Constuructor */
#define CTOR(type)  \
void* type##New()   \
{   \
    struct type *t; \
    t = (struct type *)malloc(sizeof(struct type));

#define CTOR2(type, type2)  \
void* type2##New()  \
{   \
    struct type *t; \
    t = (struct type*)malloc(sizeof(struct type));

#define END_CTOR    \
    return (void*)t;    \
};


#define FUNCTION_SETTING(f1, f2)    t->f1 = f2;
#define IMPLEMENTS(type)            struct type type;
#define INTERFACE(type) \
typedef struct type type;   \
struct type

#endif /* __LW_OOPC_H__ */
/*********************************END OF FILE**********************************/
