# Spooky Scary Security Talk 🎃

A Halloween-themed security presentation about IoT vulnerabilities.

## Structure

This presentation is split into separate files to make editing easier and avoid overwhelming LLM context:

```
securityTalk/
├── index.html          # Generated presentation (built from parts)
├── build.py            # Build script to combine everything
├── README.md           # This file
├── css/
│   └── style.css       # All custom styles
├── js/
│   └── app.js          # All custom JavaScript
└── slides/
    ├── 01-intro.html
    ├── 02-vampires.html
    ├── 03-werewolf.html
    ├── 04-ghost.html
    ├── 05-eye.html
    ├── 06-zombie.html
    ├── 07-demon.html
    ├── 08-graveyard.html
    ├── 08a-target.html
    ├── 08b-casino.html
    ├── 08c-pacemaker.html
    ├── 08d-verkada.html
    ├── 08e-lastpass.html
    ├── 08f-mgm.html
    ├── 09-witch.html
    ├── 10-exorcism.html
    ├── 11-questions.html
    └── 12-esp32.html    # Demo slide (moved to end)
```

## Editing Slides

### To edit a single slide:
1. Edit the corresponding file in `slides/` directory
2. Run the build script: `python build.py`
3. Open `index.html` in your browser

### To edit styles:
1. Edit `css/style.css`
2. Run `python build.py`

### To edit JavaScript:
1. Edit `js/app.js`
2. Run `python build.py`

## Building

To combine all files into `index.html`:

```bash
python build.py
```

This will:
- Read all slide files from `slides/` directory (in alphabetical order)
- Embed the CSS from `css/style.css`
- Embed the JavaScript from `js/app.js`
- Create a complete `index.html` file

## Viewing

Just open `index.html` in a web browser. The presentation uses Reveal.js loaded from CDN.

## Slide Order

1. Intro
2. Vampires (Smart Devices)
3. Werewolf (Default Passwords)
4. Ghost (Telnet)
5. All-Seeing Eye (Smart TVs)
6. Zombie (Update Problem)
7. Demon (Network Mapping)
8. Graveyard (Overview)
   - 8a: Target
   - 8b: Casino
   - 8c: Pacemakers
   - 8d: Verkada
   - 8e: LastPass
   - 8f: MGM/Caesars
9. Witch (Risk Scoring)
10. Exorcism (Protection)
11. Questions
12. **ESP32 Demo** ← Moved to end for live demo

## Why This Structure?

- **Small files**: Each slide is ~20-50 lines instead of one 1000+ line file
- **Easy editing**: Edit just one slide without scrolling through everything
- **LLM-friendly**: Smaller context windows for AI assistants
- **Version control**: Easier to see what changed in git diffs
- **Modular**: Reuse slides in different presentations

## Requirements

- Python 3.6+ (for build script)
- Modern web browser (for viewing)
- Internet connection (for Reveal.js CDN)

## Tips

- Run `python build.py` after ANY changes to see them in the browser
- Slides are loaded alphabetically, so the naming (01, 02, 08a, etc.) matters
- The build script is simple - feel free to customize it!