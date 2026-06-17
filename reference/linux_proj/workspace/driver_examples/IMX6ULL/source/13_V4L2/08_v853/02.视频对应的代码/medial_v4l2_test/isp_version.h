/*
* Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : isp_version.h
* Description :
* History :
* Author  : zhaowei <zhaowei@allwinnertech.com>
* Date    : 2018/02/08
*
*/

#ifndef _ISP_VERSION_H_
#define _ISP_VERSION_H_

//#include "include/isp_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISP_VERSION 600
#define ISP_REPO_TAG "isp-500-520-v2.00"
#define ISP_REPO_BRANCH "libisp-dev"
#define ISP_REPO_COMMIT "c3bfa4e23fb037123e65a04eaaa39f7876cfe1a6"
#define ISP_REPO_DATE "Mon Apr 10 10:37:15 2023 +0800"
#define ISP_RELEASE_AUTHOR "<zhenghanjie@allwinnertech.com>"

static inline void isp_version_info(void)
{
	printf(">>>>>>>>>>>>>>>>>>>> ISP VERSION INFO <<<<<<<<<<<<<<<<<<<\n"
		"IPCORE: ISP%d\n"
		"branch: %s\n"
		"commit: %s\n"
		"date  : %s\n"
		"author: %s\n"
		"--------------------------------------------------------\n\n",
		ISP_VERSION, ISP_REPO_BRANCH, ISP_REPO_COMMIT, ISP_REPO_DATE, ISP_RELEASE_AUTHOR);
}

#ifdef __cplusplus
}
#endif

#endif

