import sys, time, random, string, os, shutil

GREEN        = "\033[32m"
BRIGHT       = "\033[1;32m"
DIM          = "\033[2;32m"
WHITE_FLASH  = "\033[1;37m"
RESET        = "\033[0m"
CLEAR        = "\033[2J\033[H"
CURSOR_TOP   = "\033[H"

BLUE         = "\033[1;34m"  
BLUE_BRIGHT  = "\033[1;34m"
BLUE_DIM     = "\033[2;34m"

def term_size():
    s = shutil.get_terminal_size((80, 24))
    return s.lines, s.columns

def write(s):
    sys.stdout.write(s)
    sys.stdout.flush()

def crt_boot(banner_lines):
    rows, cols = term_size()
    print(rows,cols)

    # 1. Power-on flash
    write(CLEAR + WHITE_FLASH)
    for _ in range(rows):
        write(" " * cols + "\n")
    time.sleep(0.05)
    write(RESET + CLEAR)
    time.sleep(0.08)

    # 2. Scanline draw
    for i, line in enumerate(banner_lines):
        color = BLUE_BRIGHT if i % 2 == 0 else BLUE_DIM
        write(f"{color}{line:<{cols}}{RESET}\n")
        time.sleep(0.012)

    # 3. Phosphor bloom passes
    for shade in (BLUE_BRIGHT, BLUE, BLUE_DIM):
        write(CURSOR_TOP)
        for line in banner_lines:
            write(f"{shade}{line:<{cols}}{RESET}\n")
        time.sleep(0.05)

    # 4. Settle on final bright render
    write(CURSOR_TOP)
    for line in banner_lines:
        write(f"{BLUE_BRIGHT}{line:<{cols}}{RESET}\n")

def decrypt_reveal(text, duration=1.4):
    noise = string.ascii_letters + "!@#$%^&*01"
    steps = 50
    for step in range(steps + 1):
        frac = step / steps
        out = ""
        for i, ch in enumerate(text):
            if ch == " " or i / len(text) < frac:
                out += ch
            else:
                out += random.choice(noise)
        write(f"\r{BLUE_BRIGHT}{out}{RESET}")
        time.sleep(duration / steps)
    write("\n")

BANNER = [
    "  ███╗   ██╗ ██████╗ ███████╗██╗   ██╗███████╗███████╗  ",
    "  ████╗  ██║██╔═══██╗██╔════╝╚██╗ ██╔╝██╔════╝██╔════╝  ",
    "  ██╔██╗ ██║██║   ██║█████╗   ╚████╔╝ █████╗  ███████╗  ",
    "  ██║╚██╗██║██║   ██║██╔══╝    ╚██╔╝  ██╔══╝  ╚════██║  ",
    "  ██║ ╚████║╚██████╔╝███████╗   ██║   ███████╗███████║  ",
    "  ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝   ╚═╝   ╚══════╝╚══════╝  ",
    "",
    "  [ SECURE TERMINAL CHAT ]    v2.0",
    "",
]
BANNER2 =  [
    " ███████╗██╗██████╗ ██╗   ██╗██╗         ███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗ ",
    " ██╔════╝██║██╔══██╗╚██╗ ██╔╝██║         ██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║ ",
    " ███████╗██║██████╔╝ ╚████╔╝ ██║         ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║ ",
    " ╚════██║██║██╔══██╗  ╚██╔╝  ██║         ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║ ",
    " ███████║██║██████╔╝   ██║   ███████╗    ███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║ ",
    " ╚══════╝╚═╝╚═════╝    ╚═╝   ╚══════╝    ╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝ ",
    "",
    "  [ SECURE TERMINAL CHAT ]    v2.0",
    ""
]

BANNER3 = [
    "███╗   ███╗██╗███╗   ██╗██╗    ███████╗██╗  ██╗███████╗██╗     ██╗",     
    "████╗ ████║██║████╗  ██║██║    ██╔════╝██║  ██║██╔════╝██║     ██",    
    "██╔████╔██║██║██╔██╗ ██║██║    ███████╗███████║█████╗  ██║     ██║",     
    "██║╚██╔╝██║██║██║╚██╗██║██║    ╚════██║██╔══██║██╔══╝  ██║     ██║",  
    "██║ ╚═╝ ██║██║██║ ╚████║██║    ███████║██║  ██║███████╗███████╗███████╗",
    "╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝    ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝",
    "  [ AOS Shell ]    v1.0",
    ""                                                                
]


if __name__ == "__main__":
    write("\a")  # terminal bell = "boot beep"
    crt_boot(BANNER3)
    time.sleep(0.3)
    decrypt_reveal("  Initializing encrypted channel...")
    decrypt_reveal("  Loading identity keys...")
    time.sleep(0.2)
    write(f"\n{BLUE_BRIGHT}  > Ready.{RESET}\n\n")
