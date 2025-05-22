static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Simplified US QWERTY keyboard map (scancode -> ASCII)
const char scancode_ascii[] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b', '\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,
    '\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' '
};

// Function to wait for and read one scancode
unsigned char get_scancode() {
    while ((inb(0x64) & 0x01) == 0); // wait until key available
    return inb(0x60);
}

// Print a single char to VGA text mode at given cursor offset
void put_char(char c, int& cursor) {
    char* video = (char*)0xb8000;
    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x07;
    cursor++;
}

void print(const char* s, int& cursor) {
    for (int i = 0; s[i]; ++i)
        put_char(s[i], cursor);
}

void read_input(char* buffer, int max_len, int& cursor) {
    int i = 0;
    while (i < max_len - 1) {
        unsigned char scancode = get_scancode();

        // Skip scancode if it's a release code (>= 0x80)
        if (scancode & 0x80) continue;

        char c = scancode_ascii[scancode];
        if (c == '\n') {
            put_char('\n', cursor);
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                cursor--;
                put_char(' ', cursor);  // Erase
                cursor--;
            }
        } else if (c) {
            buffer[i++] = c;
            put_char(c, cursor);
        }
    }
    buffer[i] = '\0';
}


extern "C" void kernel_main() {
    const char* msg2 = "You have booted into cOS2, created by Diode-exe. Currently, it's just lines on a screen.";
    char* video = (char*)0xb8000;

    // Second line (row 1 starts at offset 160)
    int row_offset = 80 * 2; // 160
    for (int i = 0; msg2[i] != '\0'; ++i) {
        video[row_offset + i * 2] = msg2[i];
        video[row_offset + i * 2 + 1] = 0x07;
    }

        int cursor = 0;
    print("Enter your name: ", cursor);

    char name[128];
    read_input(name, sizeof(name), cursor);

    print("Hello, ", cursor);
    print(name, cursor);

    while (true) { /* hang */ }
}
