# ESP32 Port Scanner - IoT Security Presentation

Interactive Reveal.js presentation demonstrating IoT security vulnerabilities and the ESP32 Port Scanner project.

## 🌐 Live Demo

**View the presentation**: https://sam-fakhreddine.github.io/esp32-port-scanner/

## 🎨 Features

- **16:10 Aspect Ratio** - Optimized for modern displays
- **Haunted House Theme** - Dark, atmospheric design with IoT security horror stories
- **18 Slides** - Covering real-world breaches, vulnerabilities, and live demo
- **Interactive Controls** - Keyboard navigation, progress bar, slide numbers
- **Responsive Design** - Works on desktop and mobile

## 🏗️ Building Locally

The presentation is built from modular slide files:

```bash
cd presentation
python3 build.py
```

This generates `index.html` from:
- `slides/*.html` - Individual slide content
- `css/style.css` - Styling and theme
- `js/app.js` - Reveal.js configuration

## 📂 Structure

```
presentation/
├── index.html              # Built presentation (auto-generated)
├── build.py                # Build script
├── haunted-house-iot.png   # Background image
├── css/
│   └── style.css          # Theme and styling
├── js/
│   └── app.js             # Reveal.js config
└── slides/
    ├── 01-intro.html
    ├── 02-graveyard.html
    ├── 03a-target.html
    └── ...                # 18 total slides
```

## 🎯 Slide Order

1. **Introduction** - Hook and overview
2. **Graveyard** - Real-world breach overview
3. **Breach Stories** - Target, Casino, Pacemakers, Verkada, LastPass, MGM
4. **Threat Concepts** - Werewolf, Ghost, Eye, Zombie, Demon
5. **Vampires** - "Did you invite them into your home?" (pivot point)
6. **Risk Scoring** - Witch slide
7. **Protection** - Exorcism (how to secure)
8. **Live Demo** - ESP32 device (finale)
9. **Q&A** - Questions

## 🎨 Customization

Edit `css/style.css` to change:
- Colors and theme
- Font sizes
- Spacing and layout
- Background opacity

Edit `js/app.js` to change:
- Aspect ratio
- Transitions
- Controls behavior

## 📝 Adding Slides

1. Create new HTML file in `slides/` with numeric prefix (e.g., `16-newslide.html`)
2. Run `python3 build.py`
3. Slides are automatically sorted and included

## 🚀 Deployment

The presentation is automatically deployed to GitHub Pages from the `presentation/` directory.

**Live URL**: https://sam-fakhreddine.github.io/esp32-port-scanner/

## 📖 Technologies

- **Reveal.js 4.6.1** - Presentation framework
- **Custom CSS** - Haunted house theme
- **Vanilla JavaScript** - No build dependencies
- **Python** - Build script for modular slides

## ⚠️ Educational Use

This presentation is designed for security awareness and education. Use responsibly and ethically.
