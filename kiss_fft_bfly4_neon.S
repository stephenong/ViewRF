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

@ NEON optimized assembly routine of kf_bfly4()

    .text
    .fpu neon
    .align 4
    .global     kf_bfly4
    .func       kf_bfly4

kf_bfly4:
    stmdb           sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
@   vstmdb          sp!, {d8-d15}
                                                @ r0 - Fout| r1 - fstride | r2 - st | r3 - m
    pld             [r0, #0]
    mov             r5, r3
    mov             r3, r3, asl #3              @ convert m into bytes count (m*8)
    add             r6, r2, #264                @ tw1 = st->twiddles
    pld             [r6, #0]
    mov             r7, r6                      @ tw2 = st->twiddles
    mov             r8, r7                      @ tw3 = st->twiddles
    ldr             r2, [r2, #4]                @ st->inverse
    mov             r1, r1, asl #3              @ convert fstride into bytes count (fstride*8)
    mov             r9, r1, asl #1              @ fstride*2
    add             r10, r1, r9                 @ fstride*3
                                                @ float32x4x2_t rfout;       q0, q1 (d0-d3)
                                                @ float32x4x2_t tmp;         q2, q3 (d4-d7)
                                                @ float32x4x2_t scratch0;    q12, q13 (d24-d27)
                                                @ float32x4x2_t scratch1;    q14, q15 (d28-d31)
                                                @ float32x4x2_t scratch2;    q8, q9 (d16-d19)
                                                @ float32x4x2_t scratch3;    q10, q11 (d20-d23)
    asrs            r4, r5, #2                  @ size_t k=m/4;
    beq             .kf_bfly4_do_while1         @ if(k==0)

.kf_bfly4_do_while4:                            @ do { //process 4 samples per iteration
    add             r11, r0, r3                 @ fom = Fout+m;
    mov             r12, r11
    pld             [r7, #0]
    vld1.32         {d20}, [r6], r1             @ rtwd1 = vld1_f32((const float32_t*)tw1); tw1 += fstride;
    vld1.32         {d21}, [r6], r1             @ rtwd2 = vld1_f32((const float32_t*)tw1); tw1 += fstride;
    vld1.32         {d22}, [r6], r1             @ rtwd3 = vld1_f32((const float32_t*)tw1); tw1 += fstride;
    vld1.32         {d23}, [r6], r1             @ rtwd4 = vld1_f32((const float32_t*)tw1); tw1 += fstride;
    vuzp.32         q10, q11                    @ scratch3 = vuzpq_f32(vcombine_f32(rtwd1, rtwd2), vcombine_f32(rtwd3, rtwd4));
    vld2.32         {d0-d3}, [r11], r3          @ rfout = vld2q_f32((const float32_t*)(fom1)); fom2 = Fout+m2;
    vmul.f32        q2, q0, q10                 @ C_MUL_NEON(scratch0, rfout, scratch3);
    vmul.f32        q3, q1, q11
    vsub.f32        q12, q2, q3
    vmul.f32        q2, q0, q11
    vmul.f32        q3, q1, q10
    vadd.f32        q13, q2, q3

    pld             [r8, #0]
    vld1.32         {d20}, [r7], r9             @ rtwd1 = vld1_f32((const float32_t*)tw2); tw2 += fstride*2;
    vld1.32         {d21}, [r7], r9             @ rtwd2 = vld1_f32((const float32_t*)tw2); tw2 += fstride*2;
    vld1.32         {d22}, [r7], r9             @ rtwd3 = vld1_f32((const float32_t*)tw2); tw2 += fstride*2;
    vld1.32         {d23}, [r7], r9             @ trtwd4 = vld1_f32((const float32_t*)tw2); tw2 += fstride*2;
    vuzp.32         q10, q11                    @ scratch3 = vuzpq_f32(vcombine_f32(rtwd1, rtwd2), vcombine_f32(rtwd3, rtwd4));
    vld2.32         {d0-d3}, [r11], r3          @ rfout = vld2q_f32((const float32_t*)(fom2)); fom3 = Fout+m3;
    vmul.f32        q2, q0, q10                 @ C_MUL_NEON(scratch1, rfout, scratch3);
    vmul.f32        q3, q1, q11
    vsub.f32        q14, q2, q3
    vmul.f32        q2, q0, q11
    vmul.f32        q3, q1, q10
    vadd.f32        q15, q2, q3

    pld             [r0, #0]
    vld1.32         {d20}, [r8], r10            @ rtwd1 = vld1_f32((const float32_t*)tw3); tw3 += fstride*3;
    vld1.32         {d21}, [r8], r10            @ rtwd2 = vld1_f32((const float32_t*)tw3); tw3 += fstride*3;
    vld1.32         {d22}, [r8], r10            @ rtwd3 = vld1_f32((const float32_t*)tw3); tw3 += fstride*3;
    vld1.32         {d23}, [r8], r10            @ rtwd4 = vld1_f32((const float32_t*)tw3); tw3 += fstride*3;
    vuzp.32         q10, q11                    @ scratch3 = vuzpq_f32(vcombine_f32(rtwd1, rtwd2), vcombine_f32(rtwd3, rtwd4));
    vld2.32         {d0-d3}, [r11]              @ rfout = vld2q_f32((const float32_t*)(fom3));
    vmul.f32        q2, q0, q10                 @ C_MUL_NEON(scratch2, rfout, scratch3);
    vmul.f32        q3, q1, q11
    vsub.f32        q8, q2, q3
    vmul.f32        q2, q0, q11
    vmul.f32        q3, q1, q10
    vadd.f32        q9, q2, q3

    vld2.32         {d0-d3}, [r0]               @ rfout = vld2q_f32((const float32_t*)(Fout));
    vsub.f32        q2, q0, q14                 @ C_SUB_NEON(tmp, rfout, scratch1 );
    vsub.f32        q3, q1, q15

    vadd.f32        q0, q0, q14                 @ C_ADD_NEON(rfout, rfout, scratch1);
    vadd.f32        q1, q1, q15

    vadd.f32        q10, q12, q8                @ C_ADD_NEON(scratch3, scratch0, scratch2);
    vadd.f32        q11, q13, q9

    vsub.f32        q12, q12, q8                @ C_SUB_NEON(scratch0, scratch0, scratch2);
    vsub.f32        q13, q13, q9

    vsub.f32        q8, q0, q10                 @ C_SUB_NEON(scratch2, rfout, scratch3);
    vsub.f32        q9, q1, q11

    vadd.f32        q0, q0, q10                 @ C_ADD_NEON(rfout, rfout, scratch3);
    vadd.f32        q1, q1, q11
    vst2.32         {d0-d3}, [r0]!              @ vst2q_f32((float32_t*)Fout, rfout); Fout+=4;;

    cmp             r2, #0
    beq             .not_inverse4               @ if(st->inverse) {
    vsub.f32        q10, q2, q13                @ scratch3.val[0] = vsubq_f32(tmp.val[0], scratch0.val[1]);
    vadd.f32        q11, q3, q12                @ scratch3.val[1] = vaddq_f32(tmp.val[1], scratch0.val[0]);
    vadd.f32        q14, q2, q13                @ scratch1.val[0] = vaddq_f32(tmp.val[0], scratch0.val[1]);
    vsub.f32        q15, q3, q12                @ scratch1.val[1] = vsubq_f32(tmp.val[1], scratch0.val[0]);
    b               .c_end4
.not_inverse4:                                  @ } else {
    vadd.f32        q10, q2, q13                @ scratch3.val[0] = vaddq_f32(tmp.val[0], scratch0.val[1]);
    vsub.f32        q11, q3, q12                @ scratch3.val[1] = vsubq_f32(tmp.val[1], scratch0.val[0]);
    vsub.f32        q14, q2, q13                @ scratch1.val[0] = vsubq_f32(tmp.val[0], scratch0.val[1]);
    vadd.f32        q15, q3, q12                @ scratch1.val[1] = vaddq_f32(tmp.val[1], scratch0.val[0]);
                                                @ }
.c_end4:
    vst2.32         {d20-d23}, [r12], r3        @ vst2q_f32((float32_t*)(fom), scratch3); fom2 = Fout+m2;
    vst2.32         {d16-d19}, [r12], r3        @ vst2q_f32((float32_t*)fom2, scratch2); fom3 = Fout+m3;
    vst2.32         {d28-d31}, [r12]            @ vst2q_f32((float32_t*)(fom3), scratch1);

    pld             [r6, #0]

    subs            r4, r4, #1                  @ }while(--k);
    bne             .kf_bfly4_do_while4

@.kf_bfly4_process_singles:
    asr             r4, r5, #31
    lsr             r4, r4, #30
    add             r4, r4, r5
    ands            r5, r4, #3                  @ if (k%4 == 0)
    beq             .kf_bfly4_done

.kf_bfly4_do_while1:                            @ do { //process 1 sample per iteration
    pld             [r7, #0]
    vld1.32         {d18}, [r6], r1             @ rtwd1 = vld1_f32((const float32_t*)tw1); tw1 += fstride;
    vuzp.32         d18, d19                    @ scratch3 = vuzp_f32(rtwd1, rtwd2); //d11 is empty
    add             r12, r0, r3                 @ fom = Fout+m;
    vld1.32         {d0}, [r12], r3             @ rfout = vld2_f32((const float32_t*)(fom1)); fom2 = Fout+m2;
    vuzp.32         d0, d1                      @ d1 is empty
    vmul.f32        q1, q0, q9                  @ C_MUL_NEON(scratch0, rfout, scratch3);
    vsub.f32        d4, d2, d3
    vmul.f32        d2, d0, d19
    vmul.f32        d3, d1, d18
    vadd.f32        d5, d2, d3

    pld             [r8, #0]
    vld1.32         {d18}, [r7], r9             @ rtwd1 = vld1_f32((const float32_t*)tw2); tw2+= fstride*2;
    vuzp.32         d18, d19                    @ scratch3 = vuzp_f32(rtwd1, rtwd2); //d11 is empty
    vld1.32         {d0}, [r12], r3             @ rfout = vld2_f32((const float32_t*)(fom2)); fom3 = Fout+m3;
    vuzp.32         d0, d1                      @ d1 is empty
    vmul.f32        q1, q0, q9                  @ C_MUL_NEON(scratch1, rfout, scratch3);
    vsub.f32        d6, d2, d3
    vmul.f32        d2, d0, d19
    vmul.f32        d3, d1, d18
    vadd.f32        d7, d2, d3

    pld             [r0, #0]
    vld1.32         {d18}, [r8], r10            @ rtwd1 = vld1_f32((const float32_t*)tw3); tw3 += fstride*3;
    vuzp.32         d18, d19                    @ scratch3 = vuzp_f32(rtwd1, rtwd2); //d11 is empty
    vld1.32         {d0}, [r12]                 @ rfout = vld2_f32((const float32_t*)(fom3));
    vuzp.32         d0, d1                      @ d1 is empty
    vmul.f32        q1, q0, q9                  @ C_MUL_NEON(scratch2, rfout, scratch3);
    vsub.f32        d16, d2, d3
    vmul.f32        d2, d0, d19
    vmul.f32        d3, d1, d18
    vadd.f32        d17, d2, d3

    vld1.32         {d0}, [r0]                  @ rfout = vld2_f32((const float32_t*)(Fout));
    vuzp.32         d0, d1
    vsub.f32        q1, q0, q3                  @ C_SUB_NEON(tmp, rfout, scratch1 );

    vadd.f32        q0, q0, q3                  @ C_ADD_NEON(rfout, rfout, scratch1);

    vadd.f32        q9, q2, q8                  @ C_ADD_NEON(scratch3, scratch0, scratch2);

    vsub.f32        q2, q2, q8                  @ C_SUB_NEON(scratch0, scratch0, scratch2);

    vsub.f32        q8, q0, q9                  @ C_SUB_NEON(scratch2, rfout, scratch3);

    vadd.f32        q0, q0, q9                  @ C_ADD_NEON(rfout, rfout, scratch3);

    cmp             r2, #0
    beq             .not_inverse1               @ if(st->inverse) {
    vsub.f32        d18, d2, d5                 @ scratch3.val[0] = vsub_f32(tmp.val[0], scratch0.val[1]);
    vadd.f32        d19, d3, d4                 @ scratch3.val[1] = vadd_f32(tmp.val[1], scratch0.val[0]);
    vadd.f32        d6, d2, d5                  @ scratch1.val[0] = vadd_f32(tmp.val[0], scratch0.val[1]);
    vsub.f32        d7, d3, d4                  @ scratch1.val[1] = vsub_f32(tmp.val[1], scratch0.val[0]);
    b               .c_end1
.not_inverse1:                                  @ } else {
    vadd.f32        d18, d2, d5                 @ scratch3.val[0] = vadd_f32(tmp.val[0], scratch0.val[1]);
    vsub.f32        d19, d3, d4                 @ scratch3.val[1] = vsub_f32(tmp.val[1], scratch0.val[0]);
    vsub.f32        d6, d2, d5                  @ scratch1.val[0] = vsub_f32(tmp.val[0], scratch0.val[1]);
    vadd.f32        d7, d3, d4                  @ scratch1.val[1] = vadd_f32(tmp.val[1], scratch0.val[0]);
                                                @ }
.c_end1:
    mov             r12, r0
    vzip.32         d0, d1
    vst1.32         {d0}, [r12], r3             @ vst2_f32((float32_t*)Fout, rfout); fom = Fout+m;

    vzip.32         d18, d19
    vst1.32         {d18}, [r12], r3            @ vst2_f32((float32_t*)(fom), scratch3); fom2 = Fout+m2;

    vzip.32         d16, d17
    vst1.32         {d16}, [r12], r3            @ vst2_f32((float32_t*)fom2, scratch2);  fom3 = Fout+m3;

    vzip.32         d6, d7
    vst1.32         {d6}, [r12]                 @ vst2_f32((float32_t*)(fom3), scratch1);

    add             r0, r0, #8                  @ Fout+=1;
    pld             [r6, #0]

    subs            r5, r5, #1                  @ }while(--k);
    bne             .kf_bfly4_do_while1

.kf_bfly4_done:
@   vldmia          sp!, {d8-d15}
    ldmia           sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
    nop

    .endfunc
    .end

