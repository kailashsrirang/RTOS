.syntax unified
.cpu cortex-m4
.thumb

.extern osCurrentTask
.extern osNextTask
.extern _tcbs

.global PendSV_Handler
.thumb_func
PendSV_Handler:
    /* ---- Save current task ---- */
    MRS     R0, PSP              /* R0 = current task's PSP */
    STMDB   R0!, {R4-R11}        /* Push R4-R11 onto task stack, update R0 */

    /* Save updated PSP into current TCB's stackPtr (first member) */
    LDR     R1, =osCurrentTask
    LDR     R2, [R1]             /* R2 = osCurrentTask index */
    LDR     R3, =_tcbs
    
    MOV     R4, #12
    MUL     R2, R2, R4           /* offset = index * 12 (TCB size =  bytes) */
    
    ADD     R3, R3, R2           /* R3 = &_tcbs[osCurrentTask] */
    STR     R0, [R3]             /* _tcbs[osCurrentTask].stackPtr = PSP */

    /* ---- Switch to next task ---- */
    LDR     R4, =osNextTask
    LDR     R4, [R4]             /* R4 = osNextTask index */
    STR     R4, [R1]             /* osCurrentTask = osNextTask */

    /* Load next task's PSP from its TCB */
    LDR     R3, =_tcbs

    MOV     R1, #12
    MUL     R4, R4, R1          

    ADD     R3, R3, R4           /* R3 = &_tcbs[osNextTask] */
    LDR     R0, [R3]             /* R0 = next task's stackPtr */

    /* ---- Restore next task ---- */
    LDMIA   R0!, {R4-R11}        /* Pop R4-R11 from next task's stack */
    MSR     PSP, R0              /* Update PSP to new top of stack */

    /* Return to thread mode using PSP */
    LDR     LR, =0xFFFFFFFD
    BX      LR