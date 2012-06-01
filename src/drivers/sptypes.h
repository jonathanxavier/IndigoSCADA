/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/*
*Header For: sample point types
*Purpose:
*/   
#ifndef include_sptypes_h 
#define include_sptypes_h 

#define TYPE_M_SP_NA_1 "M_SP_NA_1"
#define TYPE_M_SP_TA_1 "M_SP_TA_1"
#define TYPE_M_DP_NA_1 "M_DP_NA_1"
#define TYPE_M_DP_TA_1 "M_DP_TA_1"
#define TYPE_M_ST_NA_1 "M_ST_NA_1"
#define TYPE_M_ST_TA_1 "M_ST_TA_1"
#define TYPE_M_BO_NA_1 "M_BO_NA_1"
#define TYPE_M_BO_TA_1 "M_BO_TA_1"
#define TYPE_M_ME_NA_1 "M_ME_NA_1"
#define TYPE_M_ME_TA_1 "M_ME_TA_1"
#define TYPE_M_ME_NB_1 "M_ME_NB_1"
#define TYPE_M_ME_TB_1 "M_ME_TB_1"
#define TYPE_M_ME_NC_1 "M_ME_NC_1"
#define TYPE_M_ME_TC_1 "M_ME_TC_1"
#define TYPE_M_IT_NA_1 "M_IT_NA_1"
#define TYPE_M_IT_TA_1 "M_IT_TA_1"
#define TYPE_M_EP_TA_1 "M_EP_TA_1"
#define TYPE_M_EP_TB_1 "M_EP_TB_1"
#define TYPE_M_EP_TC_1 "M_EP_TC_1"
#define TYPE_M_PS_NA_1 "M_PS_NA_1"
#define TYPE_M_ME_ND_1 "M_ME_ND_1"

#define TYPE_M_SP_TB_1 "M_SP_TB_1"
#define TYPE_M_DP_TB_1 "M_DP_TB_1"
#define TYPE_M_ST_TB_1 "M_ST_TB_1"
#define TYPE_M_BO_TB_1 "M_ST_TB_1"
#define TYPE_M_ME_TD_1 "M_ME_TD_1"
#define TYPE_M_ME_TE_1 "M_ME_TE_1"
#define TYPE_M_ME_TF_1 "M_ME_TF_1"
#define TYPE_M_IT_TB_1 "M_IT_TB_1"
#define TYPE_M_EP_TD_1 "M_EP_TD_1"
#define TYPE_M_EP_TE_1 "M_EP_TE_1"
#define TYPE_M_EP_TF_1 "M_EP_TF_1"

#define TYPE_C_SC_NA_1 "C_SC_NA_1"
#define TYPE_C_DC_NA_1 "C_DC_NA_1"
#define TYPE_C_RC_NA_1 "C_RC_NA_1"
#define TYPE_C_SE_NA_1 "C_SE_NA_1"
#define TYPE_C_SE_NB_1 "C_SE_NB_1"
#define TYPE_C_SE_NC_1 "C_SE_NC_1"
#define TYPE_C_BO_NA_1 "C_BO_NA_1"

#define TYPE_M_EI_NA_1 ""

#define TYPE_C_IC_NA_1 "C_IC_NA_1"
#define TYPE_C_CI_NA_1 "C_CI_NA_1"
#define TYPE_C_RD_NA_1 "C_RD_NA_1"
#define TYPE_C_CS_NA_1 "C_CS_NA_1"
#define TYPE_C_TS_NA_1 "C_TS_NA_1"
#define TYPE_C_RP_NA_1 "C_RP_NA_1"
#define TYPE_C_CD_NA_1 "C_CD_NA_1"

#define TYPE_P_ME_NA_1 "P_ME_NA_1"
#define TYPE_P_ME_NB_1 "P_ME_NB_1"
#define TYPE_P_ME_NC_1 "P_ME_NC_1"
#define TYPE_P_AC_NA_1 "P_AC_NA_1"

#define TYPE_F_FR_NA_1 "F_FR_NA_1"
#define TYPE_F_SR_NA_1 "F_SR_NA_1"
#define TYPE_F_SC_NA_1 "F_SC_NA_1"
#define TYPE_F_LS_NA_1 "F_LS_NA_1"
#define TYPE_F_AF_NA_1 "F_AF_NA_1"
#define TYPE_F_SG_NA_1 "F_SG_NA_1"
#define TYPE_F_DR_TA_1 "F_DR_TA_1"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//IndigoSCADA types: Enscada custom defined types

#define TYPE_M_ME_TN_1 "M_ME_TN_1"//<150> := measured value, long floating point number with time tag CP56Time2a
#define TYPE_M_ME_TO_1 "M_ME_TO_1"//<151> := measured value, 32 bit unsigned int number with time tag CP56Time2a
#define TYPE_M_ME_TP_1 "M_ME_TP_1"//<152> := measured value, 32 bit signed int number with time tag CP56Time2a
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Common tags used, capital because they became column names in database
#define VALUE_TAG "VALUE"
#define BIT_TAG   "BIT"
#define WORD_TAG  "WORD"
#define MIN_TAG "MINIMUM"
#define MAX_TAG "MAXIMUM"
#define STATUS_TAG "STATUS"
//
#define DEFAULT_TAG_VALS ",0,0,0,0,0,0,0,0,'(default)',1,0,'',''"
#define TAG_VALS ",0,0,0,0,0,0,0,0,"

#endif
