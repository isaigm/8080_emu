#include "cpu.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#define TIC (1000.0 / 60.0)
#define CYCLES_PER_MS 2000
#define CYCLES_PER_TIC (CYCLES_PER_MS * TIC)
#define HEIGHT 256
#define WIDTH 224
void CPU::debug(const std::string &msg)
{
    std::cout << msg << std::endl;
}
CPU::~CPU()
{
    delete window;
    delete [] pixels;
}
CPU::CPU(const std::string &rom)
{
    std::fstream fs(rom, std::ios_base::in | std::ios_base::binary);
    if (!fs.is_open())
    {
        std::cout << "No se puede abrir el archivo\n";
        std::cout << "Saliendo...\n";
        exit(1);
    }
    else
    {
        fs.seekg(0, fs.end);
        romSize = fs.tellg();
        fs.seekg(0, fs.beg);
        fs.read((char *)(RAM), romSize);
        pc = 0;
        sp = 0;
        A = B = C = D = E = H = L = 0;
        window = new sf::RenderWindow(sf::VideoMode(2 * WIDTH, 2 * HEIGHT), "Space Invaders");
        pixels = new sf::Uint8[WIDTH * HEIGHT * 4];
        window->setPosition(sf::Vector2i((sf::VideoMode::getDesktopMode().width - 2 * WIDTH)/2,(sf::VideoMode::getDesktopMode().height - 2 * HEIGHT)/2));
        texture.create(WIDTH, HEIGHT);
        sprite.setScale(2, 2);
        window->setVerticalSyncEnabled(true);
        debug("ROM CARGADA");
    }
}
uint16_t CPU::get_word(uint8_t op1, uint8_t op2)
{
    return uint16_t(op1 << 8) | uint16_t(op2);
}
int CPU::count_bits(uint8_t op1)
{
    int bits = 0;
    while (op1 > 0)
    {
        bits += op1 & 0x1;
        op1 >>= 1;
    }
    return bits;
}
void CPU::update_all_flags(uint16_t res)
{
    Z = (res & 0xFF) == 0;
    S = (res & 0x80) != 0;
    P = ((count_bits(res & 0xFF)) % 2) == 0;
    CY = res > 0xFF;
}
void CPU::update_zsp(uint16_t res)
{
    Z = (res & 0xFF) == 0;
    S = (res & 0x80) != 0;
    P = ((count_bits(res & 0xFF)) % 2) == 0;
}
void CPU::lxi(uint8_t &op1, uint8_t &op2)
{
    op1 = RAM[pc + 1];
    op2 = RAM[pc];
}
void CPU::inr(uint8_t &op1)
{
    uint16_t res = uint16_t(op1) + 1;
    update_zsp(res);
    op1 = res & 0xFF;
}
void CPU::dcr(uint8_t &op1)
{
    uint16_t res = uint16_t(op1) - 1;
    update_zsp(res);
    op1 = res & 0xFF;
}
void CPU::cma()
{
    A = ~A;
}
int CPU::mov(uint8_t &op1, uint8_t &op2)
{
    op1 = op2;
    return 5;
}
int CPU::mvi(uint8_t &op1)
{
    return mov(op1, RAM[pc]) + 2;
}
void CPU::stax(uint8_t op1, uint8_t op2)
{
    RAM[get_word(op1, op2)] = A;
}
void CPU::ldax(uint8_t op1, uint8_t op2)
{
    A = RAM[get_word(op1, op2)];
}
void CPU::add(uint8_t op1)
{
    uint16_t res = uint16_t(A) + uint16_t(op1);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::adc(uint8_t op1)
{
    uint16_t res = uint16_t(A) + uint16_t(op1) + uint16_t(CY);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::sub(uint8_t op1)
{
    uint16_t res = uint16_t(A) - uint16_t(op1);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::sbb(uint8_t op1)
{
    uint16_t res = uint16_t(A) - uint16_t(op1) - uint16_t(CY);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::ana(uint8_t op1)
{
    uint16_t res = uint16_t(A) & uint16_t(op1);
    update_all_flags(res);
    A = res & 0xFf;
}
void CPU::xra(uint8_t op1)
{
    uint16_t res = uint16_t(A) ^ uint16_t(op1);
    update_all_flags(res);
    A = res & 0xFf;
}
void CPU::ora(uint8_t op1)
{
    uint16_t res = uint16_t(A) | uint16_t(op1);
    update_all_flags(res);
    A = res & 0xFf;
}
void CPU::cmp(uint8_t op1)
{
    uint16_t res = uint16_t(A) - uint16_t(op1);
    update_all_flags(res);
}
void CPU::rlc()
{
    uint8_t temp = A;
    A = (temp << 1) | ((temp & 0x80) >> 7);
    CY = (temp & 0x80) >> 7;
}
void CPU::rrc()
{
    uint8_t temp = A;
    A = ((temp & 1) << 7) | (temp >> 1);
    CY = (temp & 1);
}
void CPU::ral()
{
    uint8_t temp = A;
    A = (temp << 1) | (CY << 7);
    CY = (temp & 0x80) >> 7;
}
void CPU::rar()
{
    uint8_t temp = A;
    A = (CY << 7) | (temp >> 1);
    CY = (temp & 1);
}
void CPU::pop(uint8_t &op1, uint8_t &op2)
{
    op1 = RAM[sp + 1];
    op2 = RAM[sp];
    sp += 2;
}
void CPU::pop_psw()
{
    A = RAM[sp + 1];
    uint8_t psw = RAM[sp];
    S = (psw >> 7) & 1;
    AC = (psw >> 4) & 1;
    Z = (psw >> 6) & 1;
    P = (psw >> 2) & 1;
    CY = (psw >> 0) & 1;
    sp += 2;
}
void CPU::push_psw()
{
    RAM[sp - 1] = A;
    uint8_t psw = 0;
    psw |= S << 7;
    psw |= Z << 6;
    psw |= AC << 4;
    psw |= P << 2;
    psw |= 1 << 1; // bit 1 is always 1
    psw |= CY << 0;
    RAM[sp - 2] = psw;
    sp -= 2;
}
void CPU::push(uint8_t op1, uint8_t op2)
{
    RAM[sp - 1] = op1;
    RAM[sp - 2] = op2;
    sp -= 2;
}
void CPU::dad(uint16_t op1)
{
    int16_t hl = get_word(H, L);
    uint32_t res = hl + op1;
    uint16_t temp = res & 0xFFFF;
    H = temp >> 8;
    L = temp & 0xFF;
    CY = res > 0xFFFF;
}
void CPU::inx(uint8_t &op1, uint8_t &op2)
{
    uint16_t res = get_word(op1, op2) + 1;
    op1 = res >> 8;
    op2 = res & 0xFF;
}
void CPU::dcx(uint8_t &op1, uint8_t &op2)
{
    uint16_t res = get_word(op1, op2) - 1;
    op1 = res >> 8;
    op2 = res & 0xFF;
}
void CPU::xchg()
{
    std::swap(H, D);
    std::swap(L, E);
}
void CPU::xthl()
{
    std::swap(H, RAM[sp + 1]);
    std::swap(L, RAM[sp]);
}
void CPU::adi()
{
    uint16_t res = uint16_t(A) + uint16_t(RAM[pc]);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::aci()
{
    uint16_t res = uint16_t(A) + uint16_t(RAM[pc]) + CY;
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::sbi()
{
    uint16_t res = uint16_t(A) - uint16_t(RAM[pc]) - CY;
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::ani()
{
    uint16_t res = uint16_t(A) & uint16_t(RAM[pc]);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::xri()
{
    uint16_t res = uint16_t(A) ^ uint16_t(RAM[pc]);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::ori()
{
    uint16_t res = uint16_t(A) | uint16_t(RAM[pc]);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::sui()
{
    uint16_t res = uint16_t(A) - uint16_t(RAM[pc]);
    update_all_flags(res);
    A = res & 0xFF;
}
void CPU::cpi()
{
    uint16_t res = uint16_t(A) - uint16_t(RAM[pc]);
    update_all_flags(res);
}
void CPU::shld()
{
    uint16_t addr = get_word(RAM[pc + 1], RAM[pc]);
    RAM[addr + 1] = H;
    RAM[addr] = L;
}
void CPU::lhld()
{
    uint16_t addr = get_word(RAM[pc + 1], RAM[pc]);
    H = RAM[addr + 1];
    L = RAM[addr];
}
void CPU::jmp()
{
    pc = get_word(RAM[pc + 1], RAM[pc]);
}
void CPU::pchl()
{
    pc = get_word(H, L);
}
void CPU::jc()
{
    if (CY)
    {
        jmp();
    }
    else
        pc += 2;
}
void CPU::jnc()
{
    if (!CY)
    {
        jmp();
    }
    else
        pc += 2;
}
void CPU::jz()
{
    if (Z)
    {
        jmp();
    }
    else
        pc += 2;
}
void CPU::jnz()
{
    if (!Z)
    {
        jmp();
    }
    else
        pc += 2;
}
void CPU::jm()
{
    if (S)
    {
        jmp();
    }
    else
        pc += 2;
}
void CPU::jp()
{
    if (!S)
    {
        jmp();
    }
}
void CPU::jpe()
{
    if (P)
    {
        jmp();
    }
}
void CPU::jpo()
{
    if (!P)
    {
        jmp();
    }
}
void CPU::call()
{
    int r = pc + 2;
    RAM[sp - 1] = r >> 8;
    RAM[sp - 2] = r & 0xFF;
    pc = get_word(RAM[pc + 1], RAM[pc]);
    sp -= 2;
}
void CPU::cc(int &opbytes)
{
    if (CY)
    {
        call();
        opbytes = 0;
    }
    else
        opbytes = 3;
}
void CPU::cnc(int &cycles)
{
    if (!CY)
    {
        call();
        cycles = 17;
    }
    else
    {
        pc += 2;
        cycles = 11;
    }
}
void CPU::cz(int &cycles)
{
    if (Z)
    {
        call();
        cycles = 17;
    }
    else
    {
        pc += 2;
        cycles = 11;
    }
}
void CPU::cnz(int &cycles)
{
    if (!Z)
    {
        call();
        cycles = 17;
    }
    else
    {
        pc += 2;
        cycles = 11;
    }
}
void CPU::cm(int &opbytes)
{
    if (S)
    {
        call();
        opbytes = 0;
    }
    else
        opbytes = 3;
}
void CPU::cp(int &opbytes)
{
    if (!S)
    {
        call();
        opbytes = 0;
    }
    else
        opbytes = 3;
}
void CPU::cpe(int &opbytes)
{
    if (P)
    {
        call();
        opbytes = 0;
    }
    else
        opbytes = 3;
}
void CPU::cpo(int &opbytes)
{
    if (!P)
    {
        call();
        opbytes = 0;
    }
    else
        opbytes = 3;
}
void CPU::ret()
{
    pc = get_word(RAM[sp + 1], RAM[sp]);
    sp += 2;
}
void CPU::rc(int &cycles)
{
    if (CY)
    {
        ret();
        cycles = 11;
    }
    else
    {
        cycles = 5;
    }
}
void CPU::rnc(int &cycles)
{
    if (!CY)
    {
        ret();
        cycles = 11;
    }
    else
    {
        cycles = 5;
    }
}
void CPU::rz(int &cycles)
{
    if (Z)
    {
        ret();
        cycles = 11;
    }
    else
    {
        cycles = 5;
    }
}
void CPU::rnz(int &cycles)
{
    if (!Z)
    {
        ret();
        cycles = 11;
    }
    else
    {
        cycles = 5;
    }
}
void CPU::rm(int &opbytes)
{
    if (S)
    {
        ret();
        opbytes = 0;
    }
    else
        opbytes = 1;
}
void CPU::rp(int &opbytes)
{
    if (!S)
    {
        ret();
        opbytes = 0;
    }
    else
        opbytes = 1;
}
void CPU::rpe(int &opbytes)
{
    if (P)
    {
        ret();
        opbytes = 0;
    }
    else
        opbytes = 1;
}
void CPU::rpo(int &opbytes)
{
    if (!P)
    {
        ret();
        opbytes = 0;
    }
    else
        opbytes = 1;
}
void CPU::generate_interrupt(uint16_t addr)
{
    push(pc >> 8, pc & 0xFF);
    pc = addr;
    interrupt_enabled = false;
}
void CPU::handle_input()
{
    sf::Event ev;
    while (window->pollEvent(ev))
    {
        switch (ev.type)
        {
        case sf::Event::KeyPressed:
            switch (ev.key.code)
            {
            case sf::Keyboard::C: // Insert coin
                ports[1] |= 1;
                break;
            case sf::Keyboard::S: // P1 Start
                ports[1] |= 1 << 2;
                break;
            case sf::Keyboard::W: // P1 Shoot
                ports[1] |= 1 << 4;
                break;
            case sf::Keyboard::A: // P1 Move Left
                ports[1] |= 1 << 5;
                break;
            case sf::Keyboard::D: // P1 Move Right
                ports[1] |= 1 << 6;
                break;
            case sf::Keyboard::Left: // P2 Move Left
                ports[2] |= 1 << 5;
                break;
            case sf::Keyboard::Right: // P2 Move Right
                ports[2] |= 1 << 6;
                break;
            case sf::Keyboard::Enter: // P2 Start
                ports[1] |= 1 << 1;
                break;
            case sf::Keyboard::Up: // P2 Shoot
                ports[2] |= 1 << 4;
                break;
            default:
                break;
            }
            break;

        case sf::Event::KeyReleased:
            switch (ev.key.code)
            {
            case sf::Keyboard::C: // Insert coin
                ports[1] &= ~1;
                break;
            case sf::Keyboard::S: // P1 Start
                ports[1] &= ~(1 << 2);
                break;
            case sf::Keyboard::W: // P1 shoot
                ports[1] &= ~(1 << 4);
                break;
            case sf::Keyboard::A: // P1 Move left
                ports[1] &= ~(1 << 5);
                break;
            case sf::Keyboard::D: // P1 Move Right
                ports[1] &= ~(1 << 6);
                break;
            case sf::Keyboard::Left: // P2 Move Left
                ports[2] &= ~(1 << 5);
                break;
            case sf::Keyboard::Right: // P2 Move Right
                ports[2] &= ~(1 << 6);
                break;
            case sf::Keyboard::Enter: // P2 Start
                ports[1] &= ~(1 << 1);
                break;
            case sf::Keyboard::Up: // P2 Shoot
                ports[2] &= ~(1 << 4);
                break;

            case sf::Keyboard::Q: // Quit
                window->close();
                break;
            default:
                break;
            }
            break;

        case sf::Event::Closed:
            window->close();
            break;
        default:
            break;
        }
    }
}
int CPU::disassemble(uint8_t opcode)
{
    int cycles = 0;
    switch (opcode)
    {
    case 0X0:
        cycles = 4;
        break;
    case 0X1:
        lxi(B, C);
        pc += 2;
        cycles = 10;
        break;
    case 0x2:
        stax(B, C);
        cycles = 7;
        break;
    case 0x3:
        inx(B, C);
        cycles = 5;
        break;
    case 0x4:
        inr(B);
        cycles = 5;
        break;
    case 0X5:
        dcr(B);
        cycles = 5;
        break;
    case 0X6:
        cycles = mvi(B);
        pc++;
        break;
    case 0x7:
        rlc();
        cycles = 4;
        break;
    case 0X9:
        dad(get_word(B, C));
        cycles = 10;
        break;
    case 0xA:
        ldax(B, C);
        cycles = 7;
        break;
    case 0xC:
        inr(C);
        cycles = 5;
        break;
    case 0XE:
        cycles = mvi(C);
        pc++;
        break;
    case 0xF:
        rrc();
        cycles = 4;
        break;
    case 0x14:
        inr(D);
        cycles = 5;
        break;
    case 0x1B:
        dcx(D, E);
        cycles = 5;
        break;
    case 0x1C:
        inr(E);
        cycles = 5;
        break;
    case 0XD:
        dcr(C);
        cycles = 5;
        break;
    case 0x1F:
        rar();
        cycles = 4;
        break;
    case 0xD2:
        jnc();
        cycles = 10;
        break;
    case 0xDA:
        jc();
        cycles = 10;
        break;
    case 0X11:
        lxi(D, E);
        pc += 2;
        cycles = 10;
        break;
    case 0x12:
        stax(D, E);
        cycles = 7;
        break;
    case 0X13:
        inx(D, E);
        cycles = 5;
        break;
    case 0x15:
        dcr(D);
        cycles = 5;
        break;
    case 0x16:
        cycles = mvi(D);
        pc++;
        break;
    case 0X19:
        dad(get_word(D, E));
        cycles = 10;
        break;
    case 0X1A:
        ldax(D, E);
        cycles = 7;
        break;
    case 0x1E:
        cycles = mvi(E);
        pc++;
        break;
    case 0X21:
        lxi(H, L);
        pc += 2;
        cycles = 10;
        break;
    case 0x22:
        shld();
        pc += 2;
        cycles = 16;
        break;
    case 0X23:
        inx(H, L);
        cycles = 5;
        break;
    case 0x24:
        inr(H);
        cycles = 5;
        break;
    case 0x25:
        dcr(H);
        cycles = 5;
        break;
    case 0X26:
        cycles = mvi(H);
        pc++;
        break;
    case 0x27:
        cycles = 4;
        break;
    case 0X29:
        dad(get_word(H, L));
        cycles = 10;
        break;
    case 0x2A:
        lhld();
        pc += 2;
        cycles = 16;
        break;
    case 0x2B:
        dcx(H, L);
        cycles = 5;
        break;
    case 0x2C:
        inr(L);
        cycles = 5;
        break;
    case 0x2E:
        cycles = mvi(L);
        pc++;
        break;
    case 0x2F:
        cma();
        cycles = 4;
        break;
    case 0X31:
        sp = get_word(RAM[pc + 1], RAM[pc]);
        pc += 2;
        cycles = 10;
        break;
    case 0X32:
        RAM[get_word(RAM[pc + 1], RAM[pc])] = A;
        pc += 2;
        cycles = 13;
        break;
    case 0x34:
        inr(RAM[get_word(H, L)]);
        cycles = 10;
        break;
    case 0x35:
        dcr(RAM[get_word(H, L)]);
        cycles = 10;
        break;
    case 0X36:
        cycles = mvi(RAM[get_word(H, L)]) + 3;
        pc++;
        break;
    case 0x37:
        CY = true;
        cycles = 4;
        break;
    case 0X3A:
        A = RAM[get_word(RAM[pc + 1], RAM[pc])];
        pc += 2;
        cycles = 13;
        break;
    case 0x3C:
        inr(A);
        cycles = 5;
        break;
    case 0x3D:
        dcr(A);
        cycles = 5;
        break;
    case 0X3E:
        cycles = mvi(A);
        pc++;
        break;
    case 0x40:
        cycles = mov(B, B);
        break;
    case 0x41:
        cycles = mov(B, C);
        break;
    case 0x42:
        cycles = mov(B, D);
        break;
    case 0x43:
        cycles = mov(B, E);
        break;
    case 0x44:
        cycles = mov(B, H);
        break;
    case 0x46:
        mov(B, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x47:
        cycles = mov(B, A);
        break;
    case 0x48:
        cycles = mov(C, B);
        break;
    case 0x4E:
        mov(C, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x4F:
        cycles = mov(C, A);
        break;
    case 0x56:
        mov(D, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x57:
        cycles = mov(D, A);
        break;
    case 0x5e:
        mov(E, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x5f:
        cycles = mov(E, A);
        break;
    case 0x61:
        cycles = mov(H, C);
        break;
    case 0x64:
        cycles = mov(H, H);
        break;
    case 0x65:
        cycles = mov(H, L);
        break;
    case 0x66:
        mov(H, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x67:
        cycles = mov(H, A);
        break;
    case 0x68:
        cycles = mov(L, B);
        break;
    case 0x69:
        cycles = mov(L, C);
        break;
    case 0x6f:
        cycles = mov(L, A);
        break;
    case 0x70:
        mov(RAM[get_word(H, L)], B);
        cycles = 7;
        break;
    case 0x71:
        mov(RAM[get_word(H, L)], C);
        cycles = 7;
        break;
    case 0x72:
        mov(RAM[get_word(H, L)], D);
        cycles = 7;
        break;
    case 0x73:
        mov(RAM[get_word(H, L)], E);
        cycles = 7;
        break;
    case 0X77:
        mov(RAM[get_word(H, L)], A);
        cycles = 7;
        break;
    case 0x78:
        cycles = mov(A, B);
        break;
    case 0x79:
        cycles = mov(A, C);
        break;
    case 0x7a:
        cycles = mov(A, D);
        break;
    case 0x7b:
        cycles = mov(A, E);
        break;
    case 0x7c:
        cycles = mov(A, H);
        break;
    case 0x7D:
        cycles = mov(A, L);
        break;
    case 0x7e:
        mov(A, RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x80:
        add(B);
        cycles = 4;
        break;
    case 0x81:
        add(C);
        cycles = 4;
        break;
    case 0x82:
        add(D);
        cycles = 4;
        break;
    case 0x83:
        add(E);
        cycles = 4;
        break;
    case 0x85:
        add(L);
        cycles = 4;
        break;
    case 0x86:
        add(RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0x8A:
        adc(D);
        cycles = 4;
        break;
    case 0x97:
        sub(A);
        cycles = 4;
        break;
    case 0xA0:
        ana(B);
        cycles = 4;
        break;
    case 0xA1:
        ana(C);
        cycles = 4;
        break;
    case 0xA6:
        ana(RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0xa7:
        ana(A);
        cycles = 4;
        break;
    case 0xA8:
        xra(B);
        cycles = 4;
        break;
    case 0xaf:
        xra(A);
        cycles = 4;
        break;
    case 0xB0:
        ora(B);
        cycles = 4;
        break;
    case 0xB4:
        ora(H);
        cycles = 4;
        break;
    case 0xB6:
        ora(RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0xB8:
        cmp(B);
        cycles = 4;
        break;
    case 0xBC:
        cmp(H);
        cycles = 4;
        break;
    case 0xBE:
        cmp(RAM[get_word(H, L)]);
        cycles = 7;
        break;
    case 0xC0:
        rnz(cycles);
        break;
    case 0XC1:
        pop(B, C);
        cycles = 10;
        break;
    case 0XC2:
        jnz();
        cycles = 10;
        break;
    case 0XC3:
        jmp();
        cycles = 10;
        break;
    case 0xC4:
        cnz(cycles);
        break;
    case 0XC5:
        push(B, C);
        cycles = 11;
        break;
    case 0XC6:
        adi();
        pc++;
        cycles = 10;
        break;
    case 0xC8:
        rz(cycles);
        break;
    case 0XC9:
        ret();
        cycles = 10;
        break;
    case 0XCA:
        jz();
        cycles = 10;
        break;
    case 0xCC:
        cz(cycles);
        break;
    case 0XCD:
        call();
        cycles = 17;
        break;
    case 0XD1:
        pop(D, E);
        cycles = 10;
        break;
    case 0XD3:
        if (RAM[pc] == 2 || RAM[pc] == 4)
        {
            cycles = 10;
        }
        else
        {
            ports[RAM[pc]] = A;
        }
        pc++;
        cycles = 10;
        break;
    case 0xD4:
        cnc(cycles);
        break;
    case 0XD5:
        push(D, E);
        cycles = 11;
        break;
    case 0xD6:
        sui();
        cycles = 7;
        pc++;
        break;
    case 0xD0:
        rnc(cycles);
        break;
    case 0xD8:
        rc(cycles);
        break;
    case 0xDB:
        if (RAM[pc] != 3)
        {
            A = ports[RAM[pc]];
        }
        pc++;
        cycles = 10;
        break;
    case 0xDE:
        sbi();
        pc++;
        cycles = 7;
        break;
    case 0XE1:
        pop(H, L);
        cycles = 10;
        break;
    case 0xE3:
        xthl();
        cycles = 18;
        break;
    case 0XE5:
        push(H, L);
        cycles = 11;
        break;
    case 0XE6:
        ani();
        pc++;
        cycles = 7;
        break;
    case 0xE9:
        pchl();
        cycles = 5;
        break;
    case 0XEB:
        xchg();
        cycles = 4;
        break;
    case 0XF1:
        pop_psw();
        cycles = 10;
        break;
    case 0XF5:
        push_psw();
        cycles = 11;
        break;
    case 0xF6:
        ori();
        pc++;
        cycles = 7;
        break;
    case 0xFA:
        jm();
        cycles = 10;
        break;
    case 0XFB:
        interrupt_enabled = true;
        cycles = 4;
        break;
    case 0XFE:
        cpi();
        pc++;
        cycles = 7;
        break;
    default:
        debug("Unknow opcode");
        exit(1);
        break;
    }
    return cycles;
}
void CPU::cpu_run(long cycles)
{
    int i = 0;
    static int shift_amount = 0;
    static uint16_t shift_register = 0;
    while (i < cycles)
    {
        uint8_t opcode = RAM[pc];
        //printf("%d %d %X %X %X [%X] %x %x %x %x %x -> %d\n", pc, sp, get_word(B, C), get_word(D, E), get_word(H, L), opcode, CY, AC, Z, S, P, A);
        pc++;

        if (opcode == 0xd3)
        { // OUT
            if (RAM[pc] == 2)
            { // Set shift amount
                shift_amount = A;
            }
            else if (RAM[pc] == 4)
            { // Set data in shift register
                shift_register = (A << 8) | (shift_register >> 8);
            }
        }
        else if (opcode == 0xdb)
        { // IN
            if (RAM[pc] == 3)
            { // Shift and read data
                A = shift_register >> (8 - shift_amount);
            }
        }
        i += disassemble(opcode);
    }
}
void CPU::render()
{

    int i = 0x2400; // Start of Video RAM
    window->clear(sf::Color::Black);
    for (int col = 0; col < WIDTH; col++)
    {
        for (int row = HEIGHT; row > 0; row -= 8)
        {
            for (int j = 0; j < 8; j++)
            {
                int idx = (col + (row - j) * WIDTH) * 4;

                if (RAM[i] & 1 << j)
                {
                    pixels[idx] = 255;
                    pixels[idx + 1] = 255;
                    pixels[idx + 2] = 255;
                    pixels[idx + 3] = 255;
                }
                else
                {
                    pixels[idx] = 0;
                    pixels[idx + 1] = 0;
                    pixels[idx + 2] = 0;
                    pixels[idx + 3] = 0;
                }
            }

            i++;
        }
    }
    texture.update(pixels);
    sprite.setTexture(texture);
    window->draw(sprite);
    window->display();
}
void CPU::run()
{
    sf::Clock timer;
    uint32_t last_tic = timer.getElapsedTime().asMilliseconds();
    while (window->isOpen())
    {
        if ((timer.getElapsedTime().asMilliseconds() - last_tic) >= TIC)
        {
            last_tic = timer.getElapsedTime().asMilliseconds();

            cpu_run(CYCLES_PER_TIC / 2);

            if (interrupt_enabled)
            {
                generate_interrupt(0x08);
            }
            cpu_run(CYCLES_PER_TIC / 2);

            handle_input();
            render();
            if (interrupt_enabled)
            {
                generate_interrupt(0x10);
            }
        }
    }
}
