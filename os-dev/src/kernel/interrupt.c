

typedef struct {
    uint32_t irq_basic_pending;
    uint32_t irq_gpu_pending1;
    uint32_t irq_gpu_pending2;
    uint32_t fiq_control;
    uint32_t irq_gpu_enable1;
    uint32_t irq_gpu_enable2;
    uint32_t irq_basic_enable;
    uint32_t irq_gpu_disable1;
    uint32_t irq_gpu_disable2;
    uint32_t irq_basic_disable;
} interrupt_registers_t;

void irq_handler(void) {
    printf("IRQ HANDLER\n");
    while(1);
}

void __attribute__((interrupt("ABORT"))) reset_handler(void) {
    printf("RESET HANDLER\n");
    while(1);
}

void __attribute__ ((interrupt("ABORT"))) prefetch_abort_handler(void) {
    printf("PREFETCH ABORT HANDLER\n");
    while(1);
}

void __attribute__ ((interrupt("ABORT"))) data_abort_handler(void) {
    printf("DATA ABORT HANDLER\n");
    while(1);
}

void __attribute__ ((interrupt("UNDEF"))) undefined_instruction_handler(void) {
    printf("UNDEFINED INSTRUCTION HANDLER\n");
    while(1);
}

void __attribute__ ((interrupt("SWI"))) software_interrupt_handler(void) {
    printf("SWI HANDLER\n");
    while(1);


void __attribute__ ((interrupt("FIQ"))) fast_irq_handler(void) {
    printf("FIQ HANDLER\n");
    while(1);
}


// this is a pointer to a function which has no parameters or return value
typedef void (*interrupt_handler_f)(void);

                /*  
            an array of the `interrupt_handler_f` since there are three 4-byte words representing 
            possible interrupts, but the last 3 bytes of the basic interrupts are repeated of others, 
            there are 72 different interrupts that could be handled, so we declare and array of 72 handlers
                */
static interrupt_handler_f handlers[72];


// In the Raspberry Pi an interrupt pending flag cannot be cleared using the interrupt peripheral.
// It must be cleared using whatever hardware peripheral that triggered the interrupt. 
// For this reason, we will also want users to register a specialized clearer function for that particular interrupt:

typedef void (*interrupt_clearer_f)(void);

static interrupt_clearer_f clearers[72];

// to register a handler we use this function

void register_irq_handler(irq_number_t irq_num, interrupt_handler_f handler, interrupt_clearer_f clearer){
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)){
        irq_pos = irq_num - 64;
        handlers[irq_num] = handler;
        clearers[irq_num] = clearer;
        interrupt_regs->irq_basic_enable |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = handler;
        clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable2 |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = handler;
        clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable1 |= (1 << irq_pos);
    }
}


// In order to check which IRQs have been triggered and execute the handler, we must check the enabled bits 
// of the IRQ peripheral, and execute the correct handler accordingly

// this code iterates over all irq pending flags, if an interrupt is pending and there is a corresponding handler,
// the clearer is called, then interrupts are enabled so to allow for "nesting" of interrupts,
// the handler is called and interrupts are disabled to finish off the interrupt.

void irq_handler(void) {
    int j;
    for(j=0; j<NUM_IRQS; j++){
        // If the interrupt is pending and there is a handler, run the handler
        if(IRQ_IS_PENDING(interrupt_regs, j) && (handlers[j] != 0)){
            clearers[j]();
            ENABLE_INTERRUPTS();
            handlers[j]();
            DISABLE_INTERRUPTS();
            return;
        }
    }
}


// initializing the interrupts

static interrupt_registers_t* interrupt_regs;

extern void move_exception_vectors(void);


void interrupts_init(void) {
    interrupt_regs = (interrupt_registers_t *)INTERRUPTS_PENDING;
    bzero(handlers, sizeof(interrupt_handler_f) * NUM_IRQS);
    bzero(clearers, sizeof(interrupt_clearer_f) * NUM_IRQS);)
    interrupt_regs->irq_basic_disable = 0xfffffff; // disable all interrupts
    interrupt_regs->irq_gpu_disable1 = 0xfffffff;
    interrupt_regs->irq_gpu_disable2 = 0xfffffff;
    move_exception_vector();
    ENABLE_INTERRUPTS();
}


// ENABLE_INTERRUPTS AND DISABLE_INTERRUPTS are inline functions that will be called frequently

__inline__ int INTERRUPTS_ENABLED(void) {
    int res;
    __asm__ __volatile__("mrs %[res], CPSR": [res] "=r" (res)::);
    return ((res >> 7) & 1) == 0;
}

__inline__ void ENABLE_INTERRUPTS(void) {
    if (!INTERRUPTS_ENABLED()) {
        __asm__ __volatile__("cpsie i");
    }
}



/// System timer peripheral

typedef struct {
    uint8_t timer0_matched: 1;
    uint8_t timer1_matched: 1;
    uint8_t timer2_matched: 1;
    uint8_t timer3_matched: 1;
    uint32_t reserved: 28;
} timer_control_reg_t;

typedef struct {
    timer_control_reg_t control;
} timer_registers_t;