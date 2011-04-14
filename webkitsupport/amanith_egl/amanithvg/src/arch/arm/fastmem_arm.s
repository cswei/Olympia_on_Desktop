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
	\file fastmem_arm.S
	\brief Fast memory routines for ARM processors, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if (defined(__thumb__) || defined(__thumb2__) || defined(__arm__)) && !defined(AM_EXT_LIBC)

	#if (defined(__GNUC__) && !defined(__ARMCC_VERSION)) || (defined(__SYMBIAN32__) && defined (__GCC32__))

		.text
		.global amMemset32_ARM9

		#if !defined(_WIN32_WCE)
			@ needed by ELF format
			.type amMemset32_ARM9,%function
		#endif
		.align 4

		#if defined(__thumb__) || defined(__thumb2__)
			.thumb
			amMemset32_ARM9:
				push {r4, r5, lr}
				movs r5, r2      @ r5 = count
				cmp r2, $8       @ less than 8 32bit values to write?
				blt 2f
				movs r2, r1      @ r2 = data
				movs r3, r1      @ r3 = data
				movs r4, r1      @ r4 = data
				lsrs r5, r2, $3   @ r5 = count / 8
				beq 1f
			0:
				subs r5, r5, $1
				stmia r0!, {r1, r2, r3, r4}
				stmia r0!, {r1, r2, r3, r4}
				bne 0b
			1:
				movs r5, $7
				ands r5, r5, r2
				beq 3f
			2:
				subs r5, r5, $1
				stmia r0!, {r1}
				bne 2b
			3:
				pop {r4, r5, pc}
		#else
			.arm
			amMemset32_ARM9:
				stmfd sp!, {r4, lr}
				cmp r2, $8
				blt 2f
				mov r3, r1
				mov r4, r1
				mov ip, r1
				@ each iteration copies eight 32bit values
			1:
				subs r2, r2, $8
				stmgeia r0!, {r1, r3, r4, ip}
				stmgeia r0!, {r1, r3, r4, ip}
				bge 1b
				add r2, r2, $8
				@ take care of possible left (up to 7) 32bit values
			2:
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				subs r2, r2, $1
				strge r1, [r0], $4
				ldmfd sp!, {r4, pc}
		#endif

		#if !defined(_WIN32_WCE)
			@ needed by ELF format
			.size amMemset32_ARM9,.-amMemset32_ARM9
		#endif

	#elif defined(__ARMCC_VERSION) || (defined(__SYMBIAN32__) && defined(__ARMCC_2__))
		AREA |.text|, CODE, READONLY
		EXPORT amMemset32_ARM9
amMemset32_ARM9
		#if defined(__thumb__) || defined(__thumb2__)
			THUMB
				push {r4, r5, lr}
				movs r5, r2      ; r5 = count
				cmp r2, #8       ; less than 8 32bit values to write?
				blt copy_left_words
				movs r2, r1      ; r2 = data
				movs r3, r1      ; r3 = data
				movs r4, r1      ; r4 = data
				lsrs r5, r2, #3   ; r5 = count / 8
				beq check_left_words
copy_8_words
				subs r5, r5, #1
				stmia r0!, {r1, r2, r3, r4}
				stmia r0!, {r1, r2, r3, r4}
				bne copy_8_words
check_left_words
				movs r5, #7
				ands r5, r5, r2
				beq exit_from_func
copy_left_words
				subs r5, r5, #1
				stmia r0!, {r1}
				bne copy_left_words
exit_from_func
				pop {r4, r5, pc}
		#else
			ARM
				stmfd sp!, {r4, lr}
				cmp r2, #8
				blt copy_left_words
				mov r3, r1
				mov r4, r1
				mov ip, r1
				; each iteration copies eight 32bit values
copy_8_words
				subs r2, r2, #8
				stmgeia r0!, {r1, r3, r4, ip}
				stmgeia r0!, {r1, r3, r4, ip}
				bge copy_8_words
				add r2, r2, #8
				; take care of possible left (up to 7) 32bit values
copy_left_words
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				subs r2, r2, #1
				strge r1, [r0], #4
				ldmfd sp!, {r4, pc}
		#endif
		END
	#else
	#endif
#else

	#if defined(__ARMCC_VERSION) || (defined(__SYMBIAN32__) && defined(__ARMCC_2__))
		END
	#endif
#endif
