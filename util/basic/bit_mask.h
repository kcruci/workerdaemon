// Copyright (C) 2013 - Inc.
// Author: davidluan
// CreateTime: 2013-10-08

#ifndef _COMM_BASE_BASIC_INCLUDE_BIT_MASK_H
#define _COMM_BASE_BASIC_INCLUDE_BIT_MASK_H


#define BIT_NOT_SETED 0

//获取某一位的值
#define GET_BIT_VALUE(value, bit_num) ((value) & (1<<(bit_num)))
//设置
#define SET_BIT_VALUE(value, bit_num) ((value) |= (1<<(bit_num)))
//清除指定bit
#define CLEAR_BIT_VALUE(value, bit_num) ((value) &= (~(1<<(bit_num))))
//测试指定bit的是否为1
//没有置位 true,已经置位false
#define TEST_BIT_VALUE(value, bit_num) (((value) & (1<<(bit_num))) == BIT_NOT_SETED)

//没有置位 true,已经置位false
#define TEST_BIT_NOT_SET(value, bit_num) (((value) & (1<<(bit_num))) == BIT_NOT_SETED)

#define TEST_BIT_SETED(value,bit_num) !(((value) & (1<<(bit_num))) == BIT_NOT_SETED)




#endif // _COMM_BASE_BASIC_INCLUDE_BIT_MASK_H
