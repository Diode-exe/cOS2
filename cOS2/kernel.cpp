#define VIDEO_MEMORY (char*)0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern "C" {
    #include "libc.h"
}


void scroll_if_needed(int& cursor) {
    if (cursor >= SCREEN_WIDTH * SCREEN_HEIGHT) {
        char* video = VIDEO_MEMORY;

        // Move each line up by one
        for (int row = 1; row < SCREEN_HEIGHT; row++) {
            for (int col = 0; col < SCREEN_WIDTH; col++) {
                int from = (row * SCREEN_WIDTH + col) * 2;
                int to = ((row - 1) * SCREEN_WIDTH + col) * 2;
                video[to] = video[from];
                video[to + 1] = video[from + 1];
            }
        }

        // Clear the last line
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            int index = ((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col) * 2;
            video[index] = ' ';
            video[index + 1] = 0x07;
        }

        // Move cursor to beginning of last line
        cursor -= SCREEN_WIDTH;
    }
}


bool shift_pressed = false;
bool show_cursor = true;

// Forward declarations
unsigned char get_scancode();
void put_char(char c, int& cursor);
char scancode_to_char(unsigned char scancode, bool shift);
void print(const char* s, int& cursor);
void read_input(char* buffer, int max_len, int& cursor);

// Get a key from keyboard
unsigned char get_scancode() {
    while ((inb(0x64) & 0x01) == 0);
    return inb(0x60);
}

char scancode_to_char(unsigned char scancode, bool shift) {
    const char lower[] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b', '\t',
        'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
        'a','s','d','f','g','h','j','k','l',';','\'','`', 0,
        '\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' '
    };

    const char upper[] = {
        0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b', '\t',
        'Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
        'A','S','D','F','G','H','J','K','L',':','"','~', 0,
        '|','Z','X','C','V','B','N','M','<','>','?', 0, '*', 0, ' '
    };

    if (scancode >= sizeof(lower)) return 0;
    return shift ? upper[scancode] : lower[scancode];
}

void put_char(char c, int& cursor) {
    char* video = (char*)0xb8000;
    video[cursor * 2] = c;
    video[cursor * 2 + 1] = 0x07;
    cursor++;
    scroll_if_needed(cursor);
}

void print(const char* s, int& cursor) {
    for (int i = 0; s[i]; ++i) {
        if (s[i] == '\n') {
            cursor = (cursor / 80 + 1) * 80;
            scroll_if_needed(cursor);
        } else {
            put_char(s[i], cursor);
        }
    }
}

void reboot() {
    asm volatile ("cli");  // Disable interrupts
    while ((inb(0x64) & 0x02) != 0);  // Wait for input buffer to clear
    outb(0x64, 0xFE);  // Send reset command to keyboard controller
    while (true) {}  // Halt if reboot fails
}

void clear_screen(int& cursor) {
    char* video = VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x07;
    }
    cursor = 0;
}


void read_input(char* buffer, int max_len, int& cursor) {
    int i = 0;
    while (i < max_len - 1) {
        unsigned char scancode = get_scancode();

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = true;
            continue;
        }
        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = false;
            continue;
        }

        if (scancode & 0x80) continue;

        if (scancode == 0x01) {
                reboot();
        };

        char c = scancode_to_char(scancode, shift_pressed);
        if (c == '\n') {
            put_char('\n', cursor);
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                cursor--;
                put_char(' ', cursor);
                cursor--;
            }
        } else if (c) {
            buffer[i++] = c;
            put_char(c, cursor);
        }
    }
    buffer[i] = '\0';
}

bool str_equals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }
    return *a == *b;
}

void sleep(int milliseconds) {
    for (int i = 0; i < milliseconds * 10000; i++) {
        asm volatile("nop");
    }
}

char last_char = ' ';  // Global or static variable

extern "C" void kernel_main() {
    const char* msg2 = "Welcome to cOS2. Type in your name above.";
    char* video = (char*)0xb8000;

    int row_offset = 80 * 2;
    for (int i = 0; msg2[i] != '\0'; ++i) {
        video[row_offset + i * 2] = msg2[i];
        video[row_offset + i * 2 + 1] = 0x07;
    }

    int cursor = 0;
    print("Enter your name: ", cursor);

    char name[128];
    read_input(name, sizeof(name), cursor);

    print("\n ", cursor);
    print("\n ", cursor);
    print("\n ", cursor);
    print("\nWelcome to cOS2, ", cursor);
    print(name, cursor);
    print("\n", cursor);
    print("Press Escape to reboot...", cursor);
    print("\n", cursor);

    while (true) {
        if ((inb(0x64) & 0x01) != 0) {  // Is there a key waiting?
            unsigned char scancode = inb(0x60);  // Read it

            if (scancode == 0x01) {
                reboot();
            }

            // Optional: handle key normally
            print("cOS2> ", cursor);
            char buffer[128];
            read_input(buffer, sizeof(buffer), cursor);  // Read command

            // For now, you can just echo the command back
            // print("\n", cursor);
            // print("You typed: ", cursor);
            // print(buffer, cursor);
            // print("\n", cursor);
            // char c = scancode_to_char(scancode, shift_pressed);
            // if (c) put_char(c, cursor);

            if (str_equals(buffer, "test")) {
                print("\n", cursor);
                print("You typed the test command!\n", cursor);
            }
            if (str_equals(buffer, "help")) {
                print("\n", cursor);
                print("Hello. You are currently using cOS2.\n", cursor);
                print("There is not much to be said here, as this is a small project.\n", cursor);
                print("Echo will echo commands back.\n", cursor);

            }
            if (str_equals (buffer, "echo")) {
                print("\nType a word after to echo it back to you...\n", cursor);
            }
            if (str_equals (buffer, "crash")) {
                while (true)
                {
                    print("!!! SYSTEM FAILURE !!!", cursor);
                    print("\n", cursor);
                    sleep(1000);
                    print("Error in [UNKNOWN_ERROR]", cursor);
                    print("\n", cursor);
                }
                


            }
            if (strncmp(buffer, "echo ", 5) == 0) {
                    print("\n", cursor);
                    print(buffer + 5, cursor);  // Print everything after "echo "
                    print("\n", cursor);                
                }
            if (str_equals (buffer, "clear")){
                clear_screen(cursor);
            }

}
}
}
