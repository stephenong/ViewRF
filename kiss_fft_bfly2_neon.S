/*
* Copyright (C) 2012, Code Aurora Forum. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Code Aurora Forum, Inc. nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

@ NEON optimized assembly routine of kf_bfly2()

    .text
    .fpu neon
    .align 4
    .global     kf_bfly2
    .func       kf_bfly2

kf_bfly2:
    stmdb            sp!, {r4, r5, r6, r7, r8, r9, sl, fp, lr}
@    vstmdb           sp!, {d8-d15}
                                                @ r0 - Fout| r1 - fstride | r2 - st | r3 - m
    pld             [r0, #0]
    mov             r8, r3, asl #3              @ convert m into bytes count (m*8)
    add             r5, r0, r8                  @ Fout2 = Fout + m;
    add             r6, r2, #264                @ tw1 = st->twiddles
    pld             [r6, #0]
    mov             r1, r1, asl #3              @ convert fstride into bytes count (fstride*8)
                                                @ float32x4x2_t *Fout;     q0, q1 (d0-d3)
                                                @ float32x4x2_t tmp;         q2, q3 (d4-d7)
                                                @ float32x4x2_t *Fout2;    q10, q11 (d20-d23)
                                                @ float32x4x2_t *tw1;      q12, q13 (d24-d27)
                                                @ float32x4x2_t t;      q8, q9 (d16-d19)
    asrs            r4, r3, #2                  @ size_t k=m/4;
    beq             .bfly2_do_while1
    mov             r7, r1, asl #2              @ convert fstride into bytes count (fstride*8*4 /*4 samples*/)

.bfly2_do_while4:                               @ do { //process 4 samples per iteration
    vld2.32         {d20-d23}, [r5]             @ load *Fout2;
    vld2.32         {d16-d19}, [r6], r7         @ load *tw1; tw1 += (fstride*4);
    pld             [r6, #0]                    @ preload next tw1
    vmul.f32        q2, q10, q12                @ C_MUL (t,  *Fout2 , *tw1);
    vmul.f32        q3, q11, q13
    vsub.f32        q8, q2, q3
    vmul.f32        q2, q10, q13
    vmul.f32        q3, q11, q12
    vadd.f32        q9, q2, q3

    vld2.32         {d0-d3}, [r0]               @ load *Fout;
    vsub.f32        q10, q0, q8                 @ C_SUB( *Fout2 ,  *Fout , t );
    vsub.f32        q11, q1, q9
    vst2.32         {d20-d23}, [r5]!            @ store *Fout2; Fout2+=4
    pld             [r5, #0]                    @ preload next Fout2

    vadd.f32        q0, q0, q8                  @ C_ADDTO( *Fout ,  t );
    vadd.f32        q1, q1, q9
    vst2.32         {d0-d3}, [r0]!              @ store *Fout; Fout+=4
    pld             [r0, #0]                    @ preload next Fout

    subs            r4, r4, #1                  @ }while(--k);
    bne             .bfly2_do_while4

@.kf_bfly2_process_remaining:
    asr             r8, r3, #31
    lsr             r7, r8, #30
    add             r4, r7, r3
    ands            r3, r4, #3                  @ if (k % 4 == 0)
    beq             .kf_bfly2_done
                                                @ float32x4x2_t *Fout;      d0 {s0,s1}
                                                @ float32x4x2_t tmp;        d1 {s2,s3}
                                                @ float32x4x2_t *Fout2;     d2 {s4,s5}
                                                @ float32x4x2_t *tw1;       d3 {s6,s7}
                                                @ float32x4x2_t t;          d4 {s8,s9}


.bfly2_do_while1:                               @ do { //process 1 sample per iteration
    vld1.32         {d2}, [r5]                  @ load *Fout2;{s16,s17}
    vld1.32         {d3}, [r6], r1              @ load *tw1; tw1 += (fstride);{s24,s25}
    pld             [r6, #0]                    @ preload next tw1
    vmul.f32        d1, d2, d3                  @ @ C_MUL (t,  *Fout2 , *tw1);
    vsub.f32        s8, s2, s3
    vmul.f32        s2, s4, s7
    vmul.f32        s3, s5, s6
    vadd.f32        s9, s2, s3

    vld1.32         {d0}, [r0]                  @ load *Fout;
    vsub.f32        d5, d0, d4                  @ C_SUB( *Fout2 ,  *Fout , t );
    vst1.32         {d5}, [r5]!                 @ store *Fout2; ++Fout2
    pld             [r5, #0]                    @ preload next Fout2

    vadd.f32        d0, d0, d4                  @ C_ADDTO( *Fout ,  t );
    vst1.32         {d0}, [r0]!                 @ store *Fout; ++Fout
    pld             [r0, #0]                    @ preload next Fout

    subs            r3, r3, #1                  @ }while(--k);
    bne             .bfly2_do_while1

.kf_bfly2_done:
@    vldmia           sp!, {d8-d15}
    ldmia            sp!, {r4, r5, r6, r7, r8, r9, sl, fp, pc}
    nop

    .endfunc
    .end
