# Gauntlet

A self-imposed Windows lockscreen that forces you to complete a random challenge every time your PC wakes from sleep. Built in C++ using Win32 and GDI — no dependencies except a single JSON header.

No excuses. No skipping. Just do the thing.

---

## What it does

When your PC wakes from sleep, Gauntlet fires a fullscreen lockscreen and picks a random task from your personal task list. You don't get in until you complete it.

Tasks can be anything:
- **Exercise** — do 12 push-ups, hold a plank for 60 seconds
- **Coding** — answer a C++ or CS fundamentals question
- **Trivia** — science, nature, maths and logic
- **Typing** — explain a concept, write a snippet, fix a bug
- **Math** — arithmetic, binary, powers of 2

Wrong answer? Red screen, try again. Right answer? You're in.

---

## Features

- Fullscreen, always-on-top lockscreen
- Blocks Alt+F4, Alt+Tab, Win key, and Ctrl+Esc while active
- Disables Task Manager while active, re-enables on completion
- Tasks loaded from `tasks.json` — edit without recompiling
- Silent background watcher process that launches Gauntlet on wake
- Installs itself to Windows startup with a single command
- Secret kill switch for genuine emergencies

---

## Project structure

```
Gauntlet/
├── src/
│   ├── main.cpp          — entry point, keyboard hook, Task Manager disable
│   ├── lockscreen.h/.cpp — fullscreen GDI window and input handling
│   ├── taskmanager.h/.cpp — loads tasks.json and picks a random task
│   ├── watcher.cpp       — background process, listens for wake events
│   └── json.hpp          — nlohmann/json single header (see setup)
│
├── bin/
│   ├── gauntlet.exe      — the lockscreen
│   └── watcher.exe       — the background watcher
│
├── tasks.json            — your task list, edit freely
└── README.md
```

---

## Setup

1. A `tasks.example.json` is included as a starting point. Before running Gauntlet, copy and rename it:

```bat
copy tasks.example.json tasks.json
```
edit it with whatever tasks you'd like.

2. Go into `lockscreen.cpp` and edit the emergency kill switch to something you want. Default is 'skip'.
3. Build both executables into `bin/`
4. Place `tasks.json` in the same folder as the exes
5. Run once to register the watcher on startup:
   ```bat
   bin\watcher.exe --install
   ```
6. Either log out and back in, or run `watcher.exe` manually to start watching

From that point, every wake from sleep triggers Gauntlet.

---

## Building

### Prerequisites
- Windows 10/11
- MinGW (g++) or MSVC
- [`json.hpp`](https://github.com/nlohmann/json/releases/latest) — download and place in `src/`

### MinGW
```bat
g++ -o bin/gauntlet.exe src/main.cpp src/lockscreen.cpp src/taskmanager.cpp -luser32 -lgdi32 -ladvapi32 -mwindows
g++ -o bin/watcher.exe src/watcher.cpp -luser32 -ladvapi32 -mwindows
```

### MSVC (Developer Command Prompt)
```bat
cl /EHsc src/main.cpp src/lockscreen.cpp src/taskmanager.cpp /link user32.lib gdi32.lib advapi32.lib /SUBSYSTEM:WINDOWS /OUT:bin/gauntlet.exe
cl /EHsc src/watcher.cpp /link user32.lib advapi32.lib /SUBSYSTEM:WINDOWS /OUT:bin/watcher.exe
```

---

## Editing tasks

Open `tasks.json` in any text editor and tailor it to yourself. Tasks are grouped by category:

```json
{
  "tasks": {
    "typing":   [ { "prompt": "..." } ],
    "trivia":   [ { "prompt": "...", "answer": "..." } ],
    "coding":   [ { "prompt": "...", "answer": "..." } ],
    "exercise": [ { "prompt": "...", "answer": "done" } ],
    "math":     [ { "prompt": "...", "answer": "..." } ]
  }
}
```

- `"prompt"` — the text shown on screen
- `"answer"` — expected answer, lowercase. Omit for open-ended typing tasks (any response over 10 characters passes)

Save the file and changes take effect next time Gauntlet runs. No recompile needed.

If you're unsure your JSON is valid, paste it into [jsonlint.com](https://jsonlint.com).

---

## Uninstalling

To stop Gauntlet from running on startup, remove the registry entry:

```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\GauntletWatcher
```

Or open `regedit`, navigate there and delete `GauntletWatcher`.

---

## Emergency escape

Safe Mode (`Shift + Restart`) does not run startup programs, so neither the watcher nor Gauntlet will launch. Use this if something goes wrong.

There is also a secret kill switch hardcoded in `lockscreen.cpp` for genuine emergencies — set it to something memorable before you compile.

---

## Dependencies

- [nlohmann/json](https://github.com/nlohmann/json) — single header, MIT licence
- Win32 API — user32, gdi32, advapi32
- No other external dependencies

---

## Licence

Do whatever you want with it. It's a personal tool.
