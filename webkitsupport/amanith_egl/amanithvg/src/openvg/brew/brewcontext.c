/****************************************************************************
** Copyright (C) 2004-2009 Mazatech S.r.l. All rights reserved.
**
** This file is part of AmanithVG software, an OpenVG implementation.
** This file is strictly confidential under the signed Mazatech Software
** Non-disclosure agreement and it's provided according to the signed
** Mazatech Software licensing agreement.
**
** Khronos and OpenVG are trademarks of The Khronos Group Inc.
** OpenGL is a registered trademark and OpenGL ES is a trademark of
** Silicon Graphics, Inc.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** For any information, please contact info@mazatech.com
**
****************************************************************************/

/*!
	\file brewcontext.c
	\brief BREW specific context related functions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif


#include "vgcontext.h"

#if defined(AM_OS_BREW) || defined(AM_DOCUMENTATION_GENERATE)

#include <AEEStdLib.h>

/*!
	\brief BREW current context associated with the thread.
*/
typedef struct _AMBrewContext {
	//! The current context.
	AMContext currentContext;
} AMBrewContext;

/*!
	\brief Get the current context, using TLS mechanism.
	\return pointer to the current context.
*/
void *amBrewCurrentContextGet(void) {

	AMBrewContext* d = NULL;
	
	GETTLS(d);
	
	// DBGPRINTF("amBrewCurrentContextGet GETTLS: %d\n", d);

	if (d == NULL) {
		d = (AMBrewContext *)MALLOC(sizeof(AMBrewContext));
		if (!d)
			DBGPRINTF("Error allocating Tls data!");
		else {
			d->currentContext.initialized = VG_FALSE;
			DBGPRINTF("AMBrewContext allocated! %d\n", d);
		}
			
		SETTLS(d);
	}
	return (void *)&d->currentContext;
}

/*!
	\brief Destroy the current context, using TLS mechanism.
*/
void amBrewCurrentContextDestroy(void) {

	AMBrewContext* d = NULL;
	
	GETTLS(d);
	
	DBGPRINTF("Destroying context GETTLS: %d\n", d);
	
	if (d != NULL) {
		FREE(d);
		SETTLS(NULL);
		GETTLS(d);
		DBGPRINTF("GETTLS after destroying: %d\n", d);
	}
}

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif



