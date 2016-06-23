/********************************************************************************

 **** Copyright (C), 2015, BinHong tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : list.h
 * Author        : chenxu
 * Date          : 2015-12-27
 * Version       : 1.0
 * Function List :
 * Description   : listÏà¹Ø½Ó¿Ú
 * Record        :
 * 1.Date        : 2015-12-27
 *   Author      : chenxu
 *   Modification: Created file

 *******************************************************************************/
#ifndef __LIST_H__
#define __LIST_H__
/*------------------------------------------------------------------------------
Section: Includes
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Macro Definitions
------------------------------------------------------------------------------*/

#define LIST_INIT(plist) \
	{\
	    (plist)->head = NULL;\
	    (plist)->tail  = NULL;\
	    (plist)->count = 0;\
	}

/*����ɾ��*/
#define LIST_DEL(plist, pnode) \
{\
    if (pnode->prev == NULL)\
		(plist)->head = pnode->next;\
    else\
		pnode->prev->next = pnode->next;\
    if (pnode->next == NULL)\
		(plist)->tail = pnode->prev;\
    else\
		pnode->next->prev = pnode->prev;\
    (plist)->count--;/* update node count */\
}

/*�豸�������*/
#define DEV_LIST_INSERT(plist, pnode, temp)\
	{\
	    for (temp = plist->tail; temp != NULL; temp = temp->prev) {\
	        if ((temp->dev_p.dev_type_id == pnode->dev_p.dev_type_id)\
				&& (temp->dev_p.dev_id < pnode->dev_p.dev_id)){\
	            break;\
	        }else if(temp->dev_p.dev_type_id < pnode->dev_p.dev_type_id)\
	        	break;\
	    }\
		pnode->prev = temp;/* ����temp�ĺ��� */\ 
		if (temp != NULL) {\
			if (temp->next != NULL) {  /* ����β�ڵ� */\
				temp->next->prev = pnode;\
			} else {  /* ��β�ڵ� */\
				plist->tail = pnode;\
			}\
			pnode->next = temp->next;\
			temp->next = pnode;\
		} else {\
			if (plist->head != NULL) {  /* �б�Ϊ�� */\
				plist->head->prev = pnode;\
			} else {  /* �б�Ϊ�� */\
				plist->tail = pnode;\
			}\
			pnode->next = plist->head;\
			plist->head = pnode;\
	    }\		
		plist->count++;/* �����еĽ������1 */\
	}

/*�����������*/
#define VAR_LIST_INSERT(plist, vnode, temp, member) \
	{\
	    temp = plist->tail;\
	    while(temp != NULL){\
			if(temp->var_p.period <= vnode->var_p.period){\
				if(temp->var_p.period == vnode->var_p.period){\
					while(temp != NULL){\
					    if(((temp->var_p.period == vnode->var_p.period)\
							&& (temp->var_p.member <= vnode->var_p.member))\
							|| (temp->var_p.period != vnode->var_p.period))\
							break;\
						temp = temp->prev;\
					}\
				}\
				break;\
			}\
			temp = temp->prev;\
	    }\
		vnode->prev = temp;\
		if (temp != NULL) {\
			if (temp->next != NULL) {  /* ����β�ڵ� */\
				temp->next->prev = vnode;\
			} else {  /* ��β�ڵ� */\
				plist->tail = vnode;\
			}\
			vnode->next = temp->next;\
			temp->next = vnode;\
		} else {\
			if (plist->head != NULL) {  /* �б�Ϊ�� */\
				plist->head->prev = vnode;\
			} else {  /* �б�Ϊ�� */\
				plist->tail = vnode;\
			}\
			vnode->next = plist->head;\
			plist->head = vnode;\
		}\
		plist->count++;/* �����еĽ������1 */\
	}

/*------------------------------------------------------------------------------
Section: Type Definitions
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Globals
------------------------------------------------------------------------------*/
/* None */
/*------------------------------------------------------------------------------
Section: Function Prototypes
------------------------------------------------------------------------------*/
/* None */
#endif

