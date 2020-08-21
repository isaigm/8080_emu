#ifndef CPU_H
#define CPU_H
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>
#include <SFML/Graphics.hpp>
class CPU
{
public:
    CPU(const std::string &rom);
    void run();

private:
    long romSize;
    uint8_t ports[9];
    uint8_t RAM[0x10000];
    uint16_t pc;                 // Program counter
    uint16_t sp;                 // Stack pointer
    uint8_t A, B, C, D, E, H, L; // Registros
    bool S = false, Z = false, P = false, CY = false, AC = false;
    bool interrupt_enabled = false;
    // Flags cy -> bit de acarreo, s -> signo, z -> bit que indica si alguna operacion da resultado cero
    //P -> bit de paridad -> el numero de bits a uno son contados, y si el total es un numero par, se pone a uno, si no se resetea a 0
    //AC -> bit de acarreo auxiliar
    int disassemble(uint8_t opcode);
    int count_bits(uint8_t op1);
    uint16_t get_word(uint8_t op1, uint8_t op2);
    void update_all_flags(uint16_t res);
    void update_zsp(uint16_t res);
    void generate_interrupt(uint16_t addr);
    void lxi(uint8_t &op1, uint8_t &op2); //carga en los operandos los dos siguientes bytes a partir del valor actual de pc
    int mov(uint8_t &op1, uint8_t &op2);  // carga en el registro r1 o posicion de memoria lo que hay en en r2, que puede ser otro registro o bien una posicion de memoria
    void jmp();                           //obtiene la siguiente palabra y salta el pc ahí
    int mvi(uint8_t &op1);                //carga en el registro r1 el segundo byte a partir del pc actual
    void inr(uint8_t &op1);               //incrementa en uno el registro o posicion de memoria
    void dcr(uint8_t &op1);               //decrementa en uno el registro o posicion de memoria
    void cma();                           //saca el complemento a uno del registro A y lo guarda en el mismo
    void stax(uint8_t op1, uint8_t op2);  //guarda en RAM[BC] o RAM[DE] lo que hay en A
    void ldax(uint8_t op1, uint8_t op2);  //A = RAM[BC] o A = RAM[DE]
    void add(uint8_t op1);                // A += op1
    void adc(uint8_t op1);                // A += op1 + CY
    void sub(uint8_t op1);                // A -= op1
    void sbb(uint8_t op1);                // A = A - op1 - CY
    void ana(uint8_t op1);                // A = A & op1
    void xra(uint8_t op1);                // A = A ^ op1
    void ora(uint8_t op1);                // A = A | op1
    void cmp(uint8_t op1);                // hace A - op1 y actualiza los flags
    void rlc();                           // A = (A << 1) | MSB, luego el carry toma el valor del MSB del acumulador
    void rrc();                           // LSB << 7 | (temp >> 1), luego carry = LSB
    void ral();
    void rar();
    void pop(uint8_t &op1, uint8_t &op2); //toma dos bytes de la pila y los coloca en registros
    void pop_psw();
    void push_psw();
    void push(uint8_t op1, uint8_t op2);
    void dad(uint16_t op1);               // HL = HL + BC/DE
    void inx(uint8_t &op1, uint8_t &op2); //incrementa en uno los pares de registros
    void dcx(uint8_t &op1, uint8_t &op2); //decrementa en uno los pares de registros
    void xchg();                          //intercambia H con D y E y con L
    void xthl();                          //intercambia H con [sp + 1] y L con [sp]
    void adi();                           //A += byte siguiente
    void aci();                           // A = A + byte + carry
    void sbi();                           // A = A - byte - carry
    void ani();                           // A = A & byte
    void xri();                           // A = A ^ byte
    void sui();                           // A = A - byte
    void ori();                           // A = A | byte
    void cpi();                           // A - byte
    void shld();                          //(adr) <-L; (adr+1)<-H
    void lhld();                          //L <- (adr); H<-(adr+1)
    void pchl();                          //	PC.hi <- H; PC.lo <- L
    void jc();                            //jmp if carry
    void jnc();                           //jmp if not carry
    void jz();                            //jmp if zero
    void jnz();                           //jmp if not zero
    void jm();                            //jmp if the sign bit is one
    void jp();                            //jmp if the sign bit is zero
    void jpe();                           //jmp if the parity bit is one
    void jpo();                           //jmp if the parity bit is zero
    void call();                          //(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP-2;PC=adr
    void cc(int &opbytes);                //call if carry
    void cnc(int &cycles);                //call if not carry
    void cz(int &cycles);                 //call if zero
    void cnz(int &cycles);                //call if not zero
    void cm(int &opbytes);                //call if the sign bit is one
    void cp(int &opbytes);                //call if the sign bit is zero
    void cpe(int &opbytes);               //call if the parity bit is one
    void cpo(int &opbytes);               //call if the parity bit is zero
    void ret();                           //PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
    void rc(int &cycles);                 //return if carry
    void rnc(int &cycles);                //return if not carry
    void rz(int &cycles);                 //return if zero
    void rnz(int &cycles);                //return if not zero
    void rm(int &opbytes);                //return if sign bit is one
    void rp(int &opbytes);                //return if sign bit is zero
    void rpe(int &opbytes);               //return if parity bit is one
    void rpo(int &opbytes);               //return if parity bit is zero
    void debug(const std::string &msg);
    void handle_input();
    static int HandleResize(void *, SDL_Event *);
    void cpu_run(long cycles);
    void render();
    sf::RenderWindow *window;
    sf::Uint8 *pixels;
    sf::Image img;
    sf::Sprite sprite;

};
#endif

